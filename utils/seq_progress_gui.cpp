// Small X11 sequencer progress popup with ontarget/usercontinue controls.
// Listens to sequencerd async multicast and updates a phase progress bar.

#define Time X11Time
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#undef Time

#include <arpa/inet.h>
#include <sys/select.h>
#include <unistd.h>

#include <algorithm>
#include <chrono>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <csignal>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "common.h"
#include "config.h"
#include "logentry.h"
#include "network.h"

namespace {

constexpr int kPhaseCount = 5;
enum PhaseIndex {
  PHASE_SLEW = 0,
  PHASE_SOLVE = 1,
  PHASE_FINE = 2,
  PHASE_OFFSET = 3,
  PHASE_EXPOSE = 4
};

struct Rect {
  int x = 0;
  int y = 0;
  int w = 0;
  int h = 0;
  bool contains(int px, int py) const {
    return px >= x && px <= (x + w) && py >= y && py <= (y + h);
  }
};

struct Options {
  std::string config_path;
  std::string host = "127.0.0.1";
  int nbport = 0;
  int msgport = 0;
  std::string msggroup = "239.1.1.234";
  bool msggroup_set = false;
  std::string acam_config_path;
  std::string acam_host;
  int acam_nbport = 0;
  std::string pub_endpoint;
  std::string sub_endpoint;
  bool pub_endpoint_set = false;
  bool sub_endpoint_set = false;
  int poll_ms = 10000;
};

struct SequenceState {
  bool phase_complete[kPhaseCount] = {false, false, false, false, false};
  bool phase_active[kPhaseCount] = {false, false, false, false, false};
  bool offset_applicable = false;
  bool waiting_for_user = false;
  bool waiting_for_tcsop = false;
  bool user_wait_after_failure = false;
  bool continue_will_expose = false;
  bool continue_starts_acquisition = false;
  bool ontarget = false;
  bool guiding_on = false;
  bool guiding_failed = false;
  double exposure_progress = 0.0;
  double exposure_elapsed = 0.0;
  double exposure_total = 0.0;
  int current_phase = -1;
  bool prev_wait_tcsop = false;
  bool prev_wait_guide = false;
  std::string seqstate;
  std::string waitstate;
  std::chrono::steady_clock::time_point last_ontarget;
  int current_obsid = -1;
  std::string current_target_state;
  double offset_ra = 0.0;
  double offset_dec = 0.0;
  int nexp = 1;
  int acqmode = 1;
  int current_frame = 0;
  int max_frame_seen = 0;

  void reset() {
    for (int i = 0; i < kPhaseCount; ++i) {
      phase_complete[i] = false;
      phase_active[i] = false;
    }
    offset_applicable = false;
    waiting_for_user = false;
    waiting_for_tcsop = false;
    user_wait_after_failure = false;
    continue_will_expose = false;
    continue_starts_acquisition = false;
    ontarget = false;
    guiding_on = false;
    guiding_failed = false;
    exposure_progress = 0.0;
    exposure_elapsed = 0.0;
    exposure_total = 0.0;
    current_phase = -1;
    prev_wait_tcsop = false;
    prev_wait_guide = false;
    seqstate.clear();
    waitstate.clear();
    current_obsid = -1;
    current_target_state.clear();
    nexp = 1;
    current_frame = 0;
    max_frame_seen = 0;
  }

  void reset_progress_only() {
    for (int i = 0; i < kPhaseCount; ++i) {
      phase_complete[i] = false;
      phase_active[i] = false;
    }
    offset_applicable = false;
    waiting_for_user = false;
    waiting_for_tcsop = false;
    user_wait_after_failure = false;
    continue_will_expose = false;
    continue_starts_acquisition = false;
    ontarget = false;
    guiding_on = false;
    guiding_failed = false;
    exposure_progress = 0.0;
    exposure_elapsed = 0.0;
    exposure_total = 0.0;
    current_phase = -1;
    prev_wait_tcsop = false;
    prev_wait_guide = false;
  }
};

static bool starts_with_local(const std::string &s, const std::string &prefix) {
  return s.rfind(prefix, 0) == 0;
}

static std::string trim_copy(std::string s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));
  s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());
  return s;
}

static std::vector<std::string> split_ws(const std::string &s) {
  std::istringstream iss(s);
  std::vector<std::string> out;
  std::string tok;
  while (iss >> tok) out.push_back(tok);
  return out;
}

static bool has_token(const std::vector<std::string> &tokens, const std::string &needle) {
  return std::find(tokens.begin(), tokens.end(), needle) != tokens.end();
}

static std::string to_upper_copy(std::string s);

static std::string strip_token_edges(std::string s) {
  while (!s.empty() && !std::isalnum(static_cast<unsigned char>(s.front()))) s.erase(s.begin());
  while (!s.empty() && !std::isalnum(static_cast<unsigned char>(s.back()))) s.pop_back();
  return s;
}

static std::vector<std::string> split_state_tokens(const std::string &s) {
  std::vector<std::string> out;
  auto tokens = split_ws(to_upper_copy(s));
  out.reserve(tokens.size());
  for (auto &tok : tokens) {
    std::string cleaned = strip_token_edges(tok);
    if (!cleaned.empty()) out.push_back(cleaned);
  }
  return out;
}

static int parse_int_after_colon(const std::string &s) {
  auto pos = s.find(':');
  if (pos == std::string::npos) return 0;
  try {
    return std::stoi(s.substr(pos + 1));
  } catch (...) {
    return 0;
  }
}

static Options parse_args(int argc, char **argv) {
  Options opt;
  if (const char *env_host = std::getenv("NGPS_HOST"); env_host && *env_host) {
    opt.host = env_host;
  }
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "--config" && i + 1 < argc) {
      opt.config_path = argv[++i];
    } else if (arg == "--host" && i + 1 < argc) {
      opt.host = argv[++i];
    } else if (arg == "--nbport" && i + 1 < argc) {
      opt.nbport = std::stoi(argv[++i]);
    } else if (arg == "--msgport" && i + 1 < argc) {
      opt.msgport = std::stoi(argv[++i]);
    } else if (arg == "--group" && i + 1 < argc) {
      opt.msggroup = argv[++i];
      opt.msggroup_set = true;
    } else if (arg == "--acam-config" && i + 1 < argc) {
      opt.acam_config_path = argv[++i];
    } else if (arg == "--acam-host" && i + 1 < argc) {
      opt.acam_host = argv[++i];
    } else if (arg == "--acam-nbport" && i + 1 < argc) {
      opt.acam_nbport = std::stoi(argv[++i]);
    } else if (arg == "--pub-endpoint" && i + 1 < argc) {
      opt.pub_endpoint = argv[++i];
      opt.pub_endpoint_set = true;
    } else if (arg == "--sub-endpoint" && i + 1 < argc) {
      opt.sub_endpoint = argv[++i];
      opt.sub_endpoint_set = true;
    } else if (arg == "--poll-ms" && i + 1 < argc) {
      opt.poll_ms = std::stoi(argv[++i]);
    } else if (arg == "--help" || arg == "-h") {
      std::cout << "Usage: seq_progress_gui [--config <path>] [--host <ip>] [--nbport <port>] [--msgport <port>] [--group <mcast>]\n"
                   "                        [--acam-config <path>] [--acam-host <ip>] [--acam-nbport <port>]\n"
                   "                        [--pub-endpoint <zmq>] [--sub-endpoint <zmq>] [--poll-ms <ms>]\n";
      std::exit(0);
    }
  }
  return opt;
}

static void load_config(const std::string &path, Options &opt) {
  if (path.empty()) return;
  Config cfg(path);
  if (cfg.read_config(cfg) != 0) return;
  for (int i = 0; i < cfg.n_entries; ++i) {
    if (cfg.param[i] == "NBPORT" && opt.nbport <= 0) {
      opt.nbport = std::stoi(cfg.arg[i]);
    } else if (cfg.param[i] == "MESSAGEPORT" && opt.msgport <= 0) {
      opt.msgport = std::stoi(cfg.arg[i]);
    } else if (cfg.param[i] == "MESSAGEGROUP" && !opt.msggroup_set) {
      opt.msggroup = cfg.arg[i];
    } else if (cfg.param[i] == "PUB_ENDPOINT" && !opt.pub_endpoint_set) {
      opt.pub_endpoint = cfg.arg[i];
    } else if (cfg.param[i] == "SUB_ENDPOINT" && !opt.sub_endpoint_set) {
      opt.sub_endpoint = cfg.arg[i];
    }
  }
}

static std::string default_config_path() {
  const char *root = std::getenv("NGPS_ROOT");
  if (root && *root) {
    return std::string(root) + "/Config/sequencerd.cfg";
  }
  return "Config/sequencerd.cfg";
}

static std::string default_acam_config_path() {
  const char *root = std::getenv("NGPS_ROOT");
  if (root && *root) {
    return std::string(root) + "/Config/acamd.cfg";
  }
  return "Config/acamd.cfg";
}

static void load_acam_config(const std::string &path, Options &opt) {
  if (path.empty()) return;
  Config cfg(path);
  if (cfg.read_config(cfg) != 0) return;
  for (int i = 0; i < cfg.n_entries; ++i) {
    if (cfg.param[i] == "NBPORT" && opt.acam_nbport <= 0) {
      opt.acam_nbport = std::stoi(cfg.arg[i]);
    } else if (cfg.param[i] == "HOST" && opt.acam_host.empty()) {
      opt.acam_host = cfg.arg[i];
    }
  }
}

static std::string to_lower_copy(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
  return s;
}

static std::string to_upper_copy(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::toupper(c); });
  return s;
}

}  // namespace

class SeqProgressGui {
 public:
  SeqProgressGui(const Options &opt)
      : options_(opt),
        udp_(static_cast<uint16_t>(options_.msgport), options_.msggroup),
        cmd_iface_("sequencer", options_.host, static_cast<uint16_t>(options_.nbport)),
        acam_iface_("acam", options_.acam_host, static_cast<uint16_t>(options_.acam_nbport)) {}

  bool init() {
    display_ = XOpenDisplay(nullptr);
    if (!display_) {
      std::cerr << "ERROR opening X display\n";
      return false;
    }

    int screen = DefaultScreen(display_);
    unsigned long black = BlackPixel(display_, screen);
    unsigned long white = WhitePixel(display_, screen);

    window_ = XCreateSimpleWindow(display_, DefaultRootWindow(display_), 100, 100, kWinW, kWinH, 1, black, white);
    XStoreName(display_, window_, "NGPS Observation Sequence");
    XSelectInput(display_, window_, ExposureMask | ButtonPressMask | KeyPressMask | StructureNotifyMask);

    wm_delete_window_ = XInternAtom(display_, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display_, window_, &wm_delete_window_, 1);

    XMapWindow(display_, window_);
    gc_ = XCreateGC(display_, window_, 0, nullptr);

    load_colors();
    load_font();
    compute_layout();

    bool use_udp = options_.sub_endpoint.empty();
    std::cerr << "DEBUG: use_udp=" << use_udp << " msgport=" << options_.msgport
              << " msggroup=" << options_.msggroup << "\n";
    if (use_udp && options_.msgport > 0 && !options_.msggroup.empty() && to_upper_copy(options_.msggroup) != "NONE") {
      udp_fd_ = udp_.Listener();
      if (udp_fd_ < 0) {
        std::cerr << "ERROR starting UDP listener\n";
        return false;
      }
      std::cerr << "DEBUG: UDP listener started on port " << options_.msgport
                << " group " << options_.msggroup << " fd=" << udp_fd_ << "\n";
    } else {
      std::cerr << "DEBUG: UDP listener NOT started\n";
    }

    init_zmq();

    if (options_.nbport > 0) {
      if (cmd_iface_.open() == 0) {
        cmd_iface_.send_command("state");
        cmd_iface_.send_command("wstate");
      }
    }
    if (options_.acam_nbport > 0) {
      acam_iface_.open();
    }

    return true;
  }

  void run() {
    const int xfd = ConnectionNumber(display_);
    const int ufd = udp_fd();
    bool running = true;
    bool need_redraw = true;
    auto last_blink = std::chrono::steady_clock::now();
    bool blink_on = false;

    while (running) {
      fd_set fds;
      FD_ZERO(&fds);
      FD_SET(xfd, &fds);
      int maxfd = xfd;
      if (ufd >= 0) {
        FD_SET(ufd, &fds);
        maxfd = std::max(maxfd, ufd);
      }

      struct timeval tv;
      tv.tv_sec = 0;
      tv.tv_usec = 200000;  // 200ms

      int ret = select(maxfd + 1, &fds, nullptr, nullptr, &tv);
      if (ret > 0) {
        if (ufd >= 0 && FD_ISSET(ufd, &fds)) {
          std::string msg;
          udp_.Receive(msg);
          if (msg.find("EXPTIME:") != std::string::npos) {
            std::cerr << "DEBUG UDP received: " << msg.substr(0, 80) << "\n";
          }
          handle_message(msg);
          need_redraw = true;
        }
        if (FD_ISSET(xfd, &fds)) {
          while (XPending(display_)) {
            XEvent ev;
            XNextEvent(display_, &ev);
            if (ev.type == Expose) {
              need_redraw = true;
            } else if (ev.type == ButtonPress) {
              handle_click(ev.xbutton.x, ev.xbutton.y);
              need_redraw = true;
            } else if (ev.type == KeyPress) {
              KeySym ks = XLookupKeysym(&ev.xkey, 0);
              if (ks == XK_Escape) {
                running = false;
              }
            } else if (ev.type == ClientMessage) {
              if (static_cast<Atom>(ev.xclient.data.l[0]) == wm_delete_window_) {
                running = false;
              }
            }
          }
        }
      }

      auto now = std::chrono::steady_clock::now();
      if (process_zmq()) {
        need_redraw = true;
      }
      if (maybe_poll(now)) {
        need_redraw = true;
      }
      if (std::chrono::duration_cast<std::chrono::milliseconds>(now - last_blink).count() > 600) {
        blink_on = !blink_on;
        last_blink = now;
        need_redraw = true;
      }

      blink_on_ = blink_on;
      if (need_redraw) {
        draw();
        need_redraw = false;
      }
    }
  }

 private:
  const int kWinW = 900;
  const int kWinH = 220;

  Options options_;
  Network::UdpSocket udp_;
  Network::Interface cmd_iface_;
  Network::Interface acam_iface_;
  zmqpp::context zmq_context_;
  std::unique_ptr<Common::PubSub> zmq_sub_;
  std::unique_ptr<Common::PubSub> zmq_pub_;
  SequenceState state_;
  int udp_fd_ = -1;
  std::chrono::steady_clock::time_point last_zmq_seqstate_;
  std::chrono::steady_clock::time_point last_zmq_waitstate_;
  std::chrono::steady_clock::time_point last_zmq_any_;
  std::chrono::steady_clock::time_point last_snapshot_request_;
  std::chrono::steady_clock::time_point last_acam_poll_;
  std::chrono::steady_clock::time_point last_seq_poll_;
  std::chrono::steady_clock::time_point last_seqstate_update_;
  std::chrono::steady_clock::time_point last_waitstate_update_;

  Display *display_ = nullptr;
  Window window_ = 0;
  GC gc_ = 0;
  Atom wm_delete_window_{};
  bool blink_on_ = false;

  Rect bar_;
  Rect segments_[kPhaseCount];
  Rect ontarget_btn_;
  Rect continue_btn_;
  Rect guiding_box_;
  Rect seqstatus_box_;

  unsigned long color_bg_ = 0;
  unsigned long color_text_ = 0;
  unsigned long color_complete_ = 0;
  unsigned long color_active_ = 0;
  unsigned long color_pending_ = 0;
  unsigned long color_disabled_ = 0;
  unsigned long color_button_ = 0;
  unsigned long color_button_disabled_ = 0;
  unsigned long color_button_text_ = 0;
  unsigned long color_wait_ = 0;

  XFontStruct *font_ = nullptr;

  int udp_fd() const { return udp_fd_; }

  void init_zmq() {
    if (!options_.sub_endpoint.empty()) {
      try {
        zmq_sub_ = std::make_unique<Common::PubSub>(zmq_context_, Common::PubSub::Mode::SUB);
        zmq_sub_->connect(options_.sub_endpoint);
        zmq_sub_->subscribe("seq_seqstate");
        zmq_sub_->subscribe("seq_waitstate");
        zmq_sub_->subscribe("seq_threadstate");
        zmq_sub_->subscribe("seq_daemonstate");
        zmq_sub_->subscribe("seq_progress");
        zmq_sub_->subscribe("acamd");
      } catch (const std::exception &e) {
        std::cerr << "ERROR initializing ZMQ subscriber: " << e.what() << "\n";
      }
    }

    if (!options_.pub_endpoint.empty()) {
      try {
        zmq_pub_ = std::make_unique<Common::PubSub>(zmq_context_, Common::PubSub::Mode::PUB);
        zmq_pub_->connect_to_broker(options_.pub_endpoint, "seq_progress_gui");
      } catch (const std::exception &e) {
        std::cerr << "ERROR initializing ZMQ publisher: " << e.what() << "\n";
      }
    }

    request_snapshot();
  }

  void load_colors() {
    int screen = DefaultScreen(display_);
    Colormap cmap = DefaultColormap(display_, screen);
    color_bg_ = alloc_color(cmap, "#e6e6e6");
    color_text_ = alloc_color(cmap, "#111111");
    color_complete_ = alloc_color(cmap, "#2e7d32");
    color_active_ = alloc_color(cmap, "#f9a825");
    color_pending_ = alloc_color(cmap, "#b0b0b0");
    color_disabled_ = alloc_color(cmap, "#808080");
    color_button_ = alloc_color(cmap, "#1565c0");
    color_button_disabled_ = alloc_color(cmap, "#9e9e9e");
    color_button_text_ = alloc_color(cmap, "#ffffff");
    color_wait_ = alloc_color(cmap, "#c62828");
  }

  unsigned long alloc_color(Colormap cmap, const char *hex) {
    XColor color;
    XParseColor(display_, cmap, hex, &color);
    XAllocColor(display_, cmap, &color);
    return color.pixel;
  }

  void load_font() {
    font_ = XLoadQueryFont(display_, "9x15bold");
    if (!font_) {
      font_ = XLoadQueryFont(display_, "fixed");
    }
    if (font_) {
      XSetFont(display_, gc_, font_->fid);
    }
  }

  void compute_layout() {
    const int margin = 16;
    bar_.x = margin;
    bar_.y = 48;
    bar_.w = kWinW - 2 * margin;
    bar_.h = 26;

    int seg_w = bar_.w / kPhaseCount;
    for (int i = 0; i < kPhaseCount; ++i) {
      segments_[i] = {bar_.x + i * seg_w, bar_.y, seg_w, bar_.h};
    }
    segments_[kPhaseCount - 1].w = bar_.w - (seg_w * (kPhaseCount - 1));

    ontarget_btn_ = {margin, 150, 160, 34};
    continue_btn_ = {margin + 180, 150, 160, 34};
    guiding_box_ = {margin + 520, 150, 160, 34};
    seqstatus_box_ = {guiding_box_.x + guiding_box_.w + 12, 150, 180, 34};
  }

  void draw() {
    XSetForeground(display_, gc_, color_bg_);
    XFillRectangle(display_, window_, gc_, 0, 0, kWinW, kWinH);

    draw_title();
    draw_acqmode_indicator();
    draw_bar();
    draw_labels();
    draw_user_instruction();
    draw_buttons();
    draw_ontarget_indicator();
    draw_offset_values();
    draw_guiding_indicator();
    draw_seqstatus_indicator();

    XFlush(display_);
  }

  void draw_title() {
    XSetForeground(display_, gc_, color_text_);
    const char *title = "Observation Sequence Progress";
    XDrawString(display_, window_, gc_, 16, 24, title, std::strlen(title));
  }

  void draw_acqmode_indicator() {
    const char *label;
    switch (state_.acqmode) {
      case 2:  label = "ACQMODE: 2 SEMI-AUTO"; break;
      case 3:  label = "ACQMODE: 3 AUTO";      break;
      default: label = "ACQMODE: 1 MANUAL";    break;
    }
    int len = std::strlen(label);
    int text_width = XTextWidth(XQueryFont(display_, XGContextFromGC(gc_)), label, len);
    XSetForeground(display_, gc_, color_text_);
    XDrawString(display_, window_, gc_, kWinW - text_width - 16, 24, label, len);
  }

  void draw_bar() {
    for (int i = 0; i < kPhaseCount; ++i) {
      unsigned long fill = color_pending_;
      if (i == PHASE_OFFSET && !state_.offset_applicable) {
        fill = color_disabled_;
      } else if (state_.phase_complete[i]) {
        fill = color_complete_;
      } else if (state_.phase_active[i]) {
        fill = color_active_;
      }

      XSetForeground(display_, gc_, fill);
      XFillRectangle(display_, window_, gc_, segments_[i].x, segments_[i].y, segments_[i].w, segments_[i].h);

      if (i == PHASE_EXPOSE && state_.phase_active[i] && state_.exposure_progress > 0.0) {
        int prog_w = static_cast<int>(segments_[i].w * std::min(1.0, state_.exposure_progress));
        XSetForeground(display_, gc_, color_complete_);
        XFillRectangle(display_, window_, gc_, segments_[i].x, segments_[i].y, prog_w, segments_[i].h);
      }

      XSetForeground(display_, gc_, color_text_);
      XDrawRectangle(display_, window_, gc_, segments_[i].x, segments_[i].y, segments_[i].w, segments_[i].h);
    }
  }

  void draw_labels() {
    XSetForeground(display_, gc_, color_text_);
    const char *labels[kPhaseCount] = {"SLEW", "ASTROM SOLVE", "FINE TUNE", "OFFSET", "EXPOSURE"};
    int label_y = bar_.y + bar_.h + 16;
    for (int i = 0; i < kPhaseCount; ++i) {
      int tx = segments_[i].x + 6;
      // Add info to EXPOSURE label when active
      if (i == PHASE_EXPOSE && state_.phase_active[PHASE_EXPOSE]) {
        char exp_label[64];
        int percent = static_cast<int>(state_.exposure_progress * 100.0);
        if (state_.nexp > 1) {
          snprintf(exp_label, sizeof(exp_label), "EXPOSURE %d/%d %d%%",
                   state_.current_frame, state_.nexp, percent);
        } else {
          snprintf(exp_label, sizeof(exp_label), "EXPOSURE %d%%", percent);
        }
        static int debug_counter = 0;
        if (++debug_counter % 10 == 0) {  // Print every 10th frame to avoid spam
          std::cerr << "DEBUG drawing label: " << exp_label
                    << " (exposure_progress=" << state_.exposure_progress << ")\n";
        }
        XDrawString(display_, window_, gc_, tx, label_y, exp_label, std::strlen(exp_label));
      } else {
        XDrawString(display_, window_, gc_, tx, label_y, labels[i], std::strlen(labels[i]));
      }
    }
  }

  void draw_status() {
    std::string status = "IDLE";
    if (state_.waiting_for_tcsop) {
      status = "WAITING FOR TCS OPERATOR (ONTARGET)";
    } else if (state_.waiting_for_user) {
      status = "WAITING FOR USER ACTION";
    } else if (state_.phase_active[PHASE_SLEW]) {
      status = "SLEWING";
    } else if (state_.phase_active[PHASE_SOLVE]) {
      status = "ASTROM SOLVE";
    } else if (state_.phase_active[PHASE_FINE]) {
      status = "FINE TUNE";
    } else if (state_.phase_active[PHASE_OFFSET]) {
      status = "APPLYING OFFSET";
    } else if (state_.phase_active[PHASE_EXPOSE]) {
      std::ostringstream oss;
      // Show frame count if NEXP > 1
      if (state_.nexp > 1) {
        oss << "EXPOSURE " << state_.current_frame << " / " << state_.nexp;
        if (state_.exposure_total > 0.0) {
          oss.setf(std::ios::fixed);
          oss.precision(1);
          oss << " (" << state_.exposure_elapsed << " / " << state_.exposure_total << " s)";
        }
      } else {
        oss << "EXPOSURE";
        if (state_.exposure_total > 0.0) {
          oss.setf(std::ios::fixed);
          oss.precision(1);
          oss << " " << state_.exposure_elapsed << " / " << state_.exposure_total << " s";
        }
        if (state_.exposure_progress > 0.0) {
          oss << " " << static_cast<int>(state_.exposure_progress * 100.0) << "%";
        }
      }
      status = oss.str();
    }

    unsigned long color = (state_.waiting_for_tcsop || state_.waiting_for_user) && blink_on_ ? color_wait_ : color_text_;
    XSetForeground(display_, gc_, color);
    XDrawString(display_, window_, gc_, 16, 116, status.c_str(), static_cast<int>(status.size()));
  }

  void draw_button(const Rect &r, const char *label, bool enabled) {
    unsigned long fill = enabled ? color_button_ : color_button_disabled_;
    XSetForeground(display_, gc_, fill);
    XFillRectangle(display_, window_, gc_, r.x, r.y, r.w, r.h);
    XSetForeground(display_, gc_, color_text_);
    XDrawRectangle(display_, window_, gc_, r.x, r.y, r.w, r.h);
    XSetForeground(display_, gc_, color_button_text_);
    int tx = r.x + 10;
    int ty = r.y + 22;
    XDrawString(display_, window_, gc_, tx, ty, label, std::strlen(label));
  }

  void draw_user_instruction() {
    if (!state_.waiting_for_user || !state_.continue_will_expose) return;
    if (!blink_on_) return;  // blink the instruction for visibility

    bool has_offset = (state_.offset_ra != 0.0 || state_.offset_dec != 0.0);
    char instruction[256];
    if (has_offset) {
      snprintf(instruction, sizeof(instruction),
               ">>> Click OFFSET & EXPOSE to apply offset (RA=%.2f\" DEC=%.2f\") then expose <<<",
               state_.offset_ra, state_.offset_dec);
    } else {
      snprintf(instruction, sizeof(instruction),
               ">>> Click EXPOSE to begin exposure (no target offset) <<<");
    }

    XSetForeground(display_, gc_, color_wait_);  // red bold
    XDrawString(display_, window_, gc_, 16, 118, instruction, std::strlen(instruction));
  }

  void draw_buttons() {
    draw_button(ontarget_btn_, "ONTARGET", state_.waiting_for_tcsop);
    const char *continue_label = "EXPOSE";
    if (state_.waiting_for_user) {
      if (state_.continue_starts_acquisition) {
        continue_label = "START ACQUISITION";
      } else {
        bool has_offset = (state_.offset_ra != 0.0 || state_.offset_dec != 0.0);
        continue_label = has_offset ? "OFFSET & EXPOSE" : "EXPOSE";
      }
    }
    draw_button(continue_btn_, continue_label, state_.waiting_for_user);
  }

  void draw_ontarget_indicator() {
    const char *label = state_.ontarget ? "ONTARGET: YES" : "ONTARGET: NO";
    unsigned long color = state_.ontarget ? color_complete_ : color_pending_;
    XSetForeground(display_, gc_, color);
    XFillArc(display_, window_, gc_, 370, 150, 18, 18, 0, 360 * 64);
    XSetForeground(display_, gc_, color_text_);
    XDrawString(display_, window_, gc_, 394, 164, label, std::strlen(label));
  }

  void draw_offset_values() {
    // Display offset values above the guiding indicator box
    char label[64];
    snprintf(label, sizeof(label), "Offset: RA=%.2f\" DEC=%.2f\"",
             state_.offset_ra, state_.offset_dec);
    XSetForeground(display_, gc_, color_text_);
    XDrawString(display_, window_, gc_, guiding_box_.x, guiding_box_.y - 8, label, std::strlen(label));
  }

  void draw_guiding_indicator() {
    const char *label = state_.guiding_on ? "GUIDING ON" : "GUIDING OFF";
    unsigned long fill = state_.guiding_on ? color_complete_ : color_wait_;
    XSetForeground(display_, gc_, fill);
    XFillRectangle(display_, window_, gc_, guiding_box_.x, guiding_box_.y, guiding_box_.w, guiding_box_.h);
    XSetForeground(display_, gc_, color_text_);
    XDrawRectangle(display_, window_, gc_, guiding_box_.x, guiding_box_.y, guiding_box_.w, guiding_box_.h);
    XSetForeground(display_, gc_, color_button_text_);
    int tx = guiding_box_.x + 10;
    int ty = guiding_box_.y + 22;
    XDrawString(display_, window_, gc_, tx, ty, label, std::strlen(label));
  }

  void draw_seqstatus_indicator() {
    std::string label;
    unsigned long fill = color_pending_;
    auto seq_tokens = split_state_tokens(state_.seqstate);
    const bool has_ready = has_token(seq_tokens, "READY");
    const bool has_notready = has_token(seq_tokens, "NOTREADY") || (has_token(seq_tokens, "NOT") && has_ready);
    const bool has_running = has_token(seq_tokens, "RUNNING");
    const bool has_paused = has_token(seq_tokens, "PAUSED");
    const bool has_starting = has_token(seq_tokens, "STARTING");
    const bool has_stopping = has_token(seq_tokens, "STOPPING");
    const bool have_zmq = (zmq_sub_ != nullptr);
    const int stale_ms = have_zmq ? 5000 : (options_.poll_ms > 0 ? options_.poll_ms * 2 : 5000);
    const bool seq_stale = is_stale(last_seqstate_update_, stale_ms);
    const bool wait_stale = is_stale(last_waitstate_update_, stale_ms);

    if (seq_stale && wait_stale) {
      label = "STALE";
      fill = color_disabled_;
    } else if (state_.waiting_for_tcsop) {
      label = "WAITING: TCSOP";
      fill = color_wait_;
    } else if (state_.waiting_for_user) {
      label = "WAITING: USER";
      fill = color_wait_;
    } else if (!state_.waitstate.empty()) {
      label = "WAITING";
      fill = color_wait_;
    } else if (has_stopping) {
      label = "STOPPING";
      fill = color_active_;
    } else if (has_starting) {
      label = "STARTING";
      fill = color_active_;
    } else if (has_paused) {
      label = "PAUSED";
      fill = color_wait_;
    } else if (has_running) {
      label = "RUNNING";
      fill = color_active_;
    } else if (has_notready) {
      label = "NOTREADY";
      fill = color_disabled_;
    } else if (has_ready) {
      label = "READY";
      fill = color_complete_;
    } else {
      label = "UNKNOWN";
      fill = color_pending_;
    }

    std::string text = "SEQ " + label;
    XSetForeground(display_, gc_, fill);
    XFillRectangle(display_, window_, gc_, seqstatus_box_.x, seqstatus_box_.y, seqstatus_box_.w, seqstatus_box_.h);
    XSetForeground(display_, gc_, color_text_);
    XDrawRectangle(display_, window_, gc_, seqstatus_box_.x, seqstatus_box_.y, seqstatus_box_.w, seqstatus_box_.h);
    XSetForeground(display_, gc_, color_button_text_);
    int tx = seqstatus_box_.x + 10;
    int ty = seqstatus_box_.y + 22;
    XDrawString(display_, window_, gc_, tx, ty, text.c_str(), static_cast<int>(text.size()));
  }

  void handle_click(int x, int y) {
    if (ontarget_btn_.contains(x, y) && state_.waiting_for_tcsop) {
      send_command("ontarget");
    }
    if (continue_btn_.contains(x, y) && state_.waiting_for_user) {
      send_command("usercontinue");
    }
  }

  void send_command(const std::string &cmd) {
    if (!cmd_iface_.isopen()) {
      cmd_iface_.open();
    }
    if (cmd_iface_.send_command(cmd) != 0) {
      cmd_iface_.reconnect();
      cmd_iface_.send_command(cmd);
    }
  }

  bool process_zmq() {
    if (!zmq_sub_) return false;
    if (!zmq_sub_->has_message()) return false;
    auto [topic, payload] = zmq_sub_->receive();
    handle_zmq_message(topic, payload);
    return true;
  }

  void handle_zmq_message(const std::string &topic, const std::string &payload) {
    try {
      nlohmann::json jmessage = nlohmann::json::parse(payload);
      auto now = std::chrono::steady_clock::now();
      last_zmq_any_ = now;
      if (topic == "seq_seqstate" && jmessage.contains("seqstate")) {
        handle_message("SEQSTATE: " + jmessage["seqstate"].get<std::string>());
        last_zmq_seqstate_ = now;
      } else if (topic == "seq_waitstate") {
        static const char *kWaitTokens[] = {"ACAM",    "CALIB",  "CAMERA", "FLEXURE", "FOCUS",
                                            "POWER",   "SLICECAM", "SLIT",   "TCS",     "ACQUIRE",
                                            "GUIDE",   "EXPOSE", "READOUT", "TCSOP",  "USER"};
        std::string waitstate;
        for (const char *token : kWaitTokens) {
          if (jmessage.contains(token) && jmessage[token].is_boolean() && jmessage[token].get<bool>()) {
            if (!waitstate.empty()) waitstate += " ";
            waitstate += token;
          }
        }
        handle_waitstate(waitstate);
        last_zmq_waitstate_ = now;
      } else if (topic == "seq_progress") {
        if (jmessage.contains("obsid") && jmessage["obsid"].is_number_integer()) {
          int obsid = jmessage["obsid"].get<int>();
          if (obsid > 0 && state_.current_obsid != obsid) {
            state_.reset_progress_only();
          }
          state_.current_obsid = obsid;
        }
        if (jmessage.contains("target_state") && jmessage["target_state"].is_string()) {
          std::string tstate = to_upper_copy(jmessage["target_state"].get<std::string>());
          if (!tstate.empty() && tstate != state_.current_target_state) {
            if (tstate == "ACTIVE" || tstate == "COMPLETE" || tstate == "PENDING") {
              state_.reset_progress_only();
            }
          }
          state_.current_target_state = tstate;
        }
        if (jmessage.contains("event") && jmessage["event"].is_string()) {
          std::string event = to_lower_copy(jmessage["event"].get<std::string>());
          if (event == "target_start" || event == "target_complete" || event == "wait_tcsop") {
            state_.reset_progress_only();
          }
        }
        if (jmessage.contains("ontarget") && jmessage["ontarget"].is_boolean()) {
          state_.ontarget = jmessage["ontarget"].get<bool>();
        }
        if (jmessage.contains("fine_tune_active") && jmessage["fine_tune_active"].is_boolean()) {
          bool active = jmessage["fine_tune_active"].get<bool>();
          if (active) {
            set_phase(PHASE_FINE);
          } else if (state_.phase_active[PHASE_FINE]) {
            state_.phase_complete[PHASE_FINE] = true;
            clear_phase_active(PHASE_FINE);
          }
        }
        bool offset_active = false;
        if (jmessage.contains("offset_active") && jmessage["offset_active"].is_boolean()) {
          offset_active = jmessage["offset_active"].get<bool>();
        }
        if (jmessage.contains("offset_settle") && jmessage["offset_settle"].is_boolean()) {
          offset_active = offset_active || jmessage["offset_settle"].get<bool>();
        }
        if (offset_active) {
          state_.offset_applicable = true;
          set_phase(PHASE_OFFSET);
        } else if (state_.phase_active[PHASE_OFFSET]) {
          state_.phase_complete[PHASE_OFFSET] = true;
          clear_phase_active(PHASE_OFFSET);
        }
        if (jmessage.contains("offset_ra") && jmessage["offset_ra"].is_number()) {
          state_.offset_ra = jmessage["offset_ra"].get<double>();
        }
        if (jmessage.contains("offset_dec") && jmessage["offset_dec"].is_number()) {
          state_.offset_dec = jmessage["offset_dec"].get<double>();
        }
        if (jmessage.contains("acqmode") && jmessage["acqmode"].is_number()) {
          state_.acqmode = jmessage["acqmode"].get<int>();
        }
        if (jmessage.contains("nexp") && jmessage["nexp"].is_number()) {
          int new_nexp = jmessage["nexp"].get<int>();
          if (new_nexp != state_.nexp) {
            state_.nexp = new_nexp;
            // Reset frame tracking when nexp changes
            state_.current_frame = 0;
            state_.max_frame_seen = 0;
          }
        }
        if (jmessage.contains("current_frame") && jmessage["current_frame"].is_number()) {
          int frame = jmessage["current_frame"].get<int>();
          if (frame > state_.max_frame_seen) {
            state_.max_frame_seen = frame;
            state_.current_frame = frame;
            // Keep EXPOSE phase active if more frames expected
            if (state_.current_frame < state_.nexp) {
              set_phase(PHASE_EXPOSE);
            } else if (state_.current_frame >= state_.nexp) {
              // All frames complete
              state_.phase_complete[PHASE_EXPOSE] = true;
              clear_phase_active(PHASE_EXPOSE);
            }
          }
        }
        if (jmessage.contains("exptime_percent") && jmessage["exptime_percent"].is_number()) {
          int percent = jmessage["exptime_percent"].get<int>();
          double new_progress = std::min(1.0, percent / 100.0);
          // Smooth the percentage (exponential moving average)
          if (state_.exposure_progress > 0.0) {
            state_.exposure_progress = 0.7 * state_.exposure_progress + 0.3 * new_progress;
          } else {
            state_.exposure_progress = new_progress;
          }
          set_phase(PHASE_EXPOSE);
        }
      } else if (topic == "acamd") {
        if (jmessage.contains("ACAM_GUIDING") && jmessage["ACAM_GUIDING"].is_boolean()) {
          state_.guiding_on = jmessage["ACAM_GUIDING"].get<bool>();
        } else if (jmessage.contains("ACAM_ACQUIRE_MODE") && jmessage["ACAM_ACQUIRE_MODE"].is_string()) {
          std::string mode = to_lower_copy(jmessage["ACAM_ACQUIRE_MODE"].get<std::string>());
          state_.guiding_on = (mode.find("guiding") != std::string::npos);
        }
      }
    } catch (const std::exception &) {
      // ignore malformed telemetry
    }
  }

  void request_snapshot() {
    if (!zmq_pub_) return;
    nlohmann::json jmessage;
    jmessage["sequencerd"] = true;
    jmessage["acamd"] = true;
    zmq_pub_->publish(jmessage, "_snapshot");
    last_snapshot_request_ = std::chrono::steady_clock::now();
  }

  bool is_stale(const std::chrono::steady_clock::time_point &tp, int stale_ms) const {
    if (tp.time_since_epoch().count() == 0) return true;
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now - tp).count() > stale_ms;
  }

  bool should_poll_acam() const {
    if (state_.seqstate.find("RUNNING") != std::string::npos) return true;
    if (state_.waitstate.find("ACQUIRE") != std::string::npos) return true;
    if (state_.waitstate.find("GUIDE") != std::string::npos) return true;
    if (state_.waitstate.find("EXPOSE") != std::string::npos) return true;
    if (state_.waitstate.find("READOUT") != std::string::npos) return true;
    return false;
  }

  bool maybe_poll(const std::chrono::steady_clock::time_point &now) {
    bool updated = false;
    const int stale_ms = 5000;
    const bool have_zmq = (zmq_sub_ != nullptr);
    const bool stale_seq = have_zmq &&
                           (is_stale(last_zmq_seqstate_, stale_ms) ||
                            is_stale(last_zmq_waitstate_, stale_ms));
    const bool zmq_quiet = have_zmq && is_stale(last_zmq_any_, options_.poll_ms > 0 ? options_.poll_ms * 3 : stale_ms * 3);
    const bool allow_tcp_poll = !have_zmq || (zmq_quiet && stale_seq);

    if (options_.poll_ms > 0) {
      if (have_zmq && zmq_pub_) {
        if (stale_seq &&
            std::chrono::duration_cast<std::chrono::milliseconds>(now - last_snapshot_request_).count() >= stale_ms) {
          request_snapshot();
          updated = true;
        }
      }
      if (allow_tcp_poll) {
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - last_seq_poll_).count() >= options_.poll_ms) {
          poll_sequencer();
          last_seq_poll_ = now;
          updated = true;
        }
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - last_acam_poll_).count() >= options_.poll_ms) {
          if (should_poll_acam()) {
            poll_acam();
            updated = true;
          }
          last_acam_poll_ = now;
        }
      }
    } else if (have_zmq && zmq_pub_) {
      if (std::chrono::duration_cast<std::chrono::milliseconds>(now - last_snapshot_request_).count() >= stale_ms) {
        request_snapshot();
        updated = true;
      }
    }

    return updated;
  }

  void poll_status() {
    poll_sequencer();
    poll_acam();
  }

  void poll_sequencer() {
    if (options_.nbport <= 0) return;
    if (!cmd_iface_.isopen()) {
      if (cmd_iface_.open() != 0) return;
    }

    std::string reply;
    if (cmd_iface_.send_command("state", reply, 200) == 0 && !reply.empty()) {
      handle_message("SEQSTATE: " + reply);
    } else if (!cmd_iface_.isopen()) {
      cmd_iface_.reconnect();
      return;
    }

    if (cmd_iface_.send_command("wstate") != 0 && !cmd_iface_.isopen()) {
      cmd_iface_.reconnect();
      return;
    }

    // Poll acqmode — the no-arg response includes "current mode = N"
    reply.clear();
    if (cmd_iface_.send_command("acqmode", reply, 200) == 0 && !reply.empty()) {
      auto pos = reply.find("current mode = ");
      if (pos != std::string::npos) {
        try { state_.acqmode = std::stoi(reply.substr(pos + 15)); } catch (...) {}
      }
    }
  }

  void poll_acam() {
    if (options_.acam_nbport <= 0) return;
    if (!acam_iface_.isopen()) {
      if (acam_iface_.open() != 0) return;
    }

    std::string reply;
    if (acam_iface_.send_command("acquire", reply, 200) == 0 && !reply.empty()) {
      std::string lower = to_lower_copy(reply);
      if (lower.find("guiding") != std::string::npos) {
        state_.guiding_on = true;
        state_.guiding_failed = false;
      } else if (lower.find("stopped") != std::string::npos || lower.find("acquir") != std::string::npos) {
        state_.guiding_on = false;
      }
    } else if (!acam_iface_.isopen()) {
      acam_iface_.reconnect();
      return;
    }
  }

  void set_phase(int idx) {
    if (idx < 0 || idx >= kPhaseCount) return;
    if (state_.current_phase != idx) {
      if (state_.current_phase >= 0 && state_.current_phase < idx) {
        state_.phase_complete[state_.current_phase] = true;
      }
      for (int i = 0; i < kPhaseCount; ++i) state_.phase_active[i] = false;
      state_.current_phase = idx;
    }
    state_.phase_active[idx] = true;
  }

  void clear_phase_active(int idx) {
    if (idx < 0 || idx >= kPhaseCount) return;
    state_.phase_active[idx] = false;
    if (state_.current_phase == idx) {
      state_.current_phase = -1;
    }
  }

  void handle_waitstate(const std::string &waitstate) {
    state_.waitstate = waitstate;
    last_waitstate_update_ = std::chrono::steady_clock::now();
    auto tokens = split_ws(waitstate);
    bool has_tcsop = has_token(tokens, "TCSOP");
    bool has_user = has_token(tokens, "USER");
    bool has_acquire = has_token(tokens, "ACQUIRE");
    bool has_guide = has_token(tokens, "GUIDE");
    bool has_expose = has_token(tokens, "EXPOSE");
    bool has_readout = has_token(tokens, "READOUT");

    state_.waiting_for_tcsop = has_tcsop;
    state_.waiting_for_user = has_user;

    if (!has_tcsop && (has_acquire || has_guide || has_expose || has_readout || has_user)) {
      state_.ontarget = true;
    }

    if (has_tcsop) {
      set_phase(PHASE_SLEW);
      state_.ontarget = false;
    } else if (state_.prev_wait_tcsop && !has_tcsop) {
      state_.phase_complete[PHASE_SLEW] = true;
      clear_phase_active(PHASE_SLEW);
    }

    if (has_acquire || has_guide) {
      set_phase(PHASE_SOLVE);
    }
    if (has_expose) {
      set_phase(PHASE_EXPOSE);
    }
    if (has_readout) {
      state_.phase_complete[PHASE_EXPOSE] = true;
      clear_phase_active(PHASE_EXPOSE);
    }

    state_.prev_wait_tcsop = has_tcsop;
    state_.prev_wait_guide = has_guide;
  }

  void handle_message(const std::string &raw) {
    std::string msg = trim_copy(raw);
    if (starts_with_local(msg, "SEQSTATE:")) {
      std::string prev_state = state_.seqstate;
      state_.seqstate = trim_copy(msg.substr(9));
      last_seqstate_update_ = std::chrono::steady_clock::now();
      auto is_ready_only = [](const std::string &s) {
        auto tokens = split_state_tokens(s);
        const bool has_ready = has_token(tokens, "READY");
        const bool has_notready = has_token(tokens, "NOTREADY") || (has_token(tokens, "NOT") && has_ready);
        if (!has_ready || has_notready) return false;
        if (has_token(tokens, "RUNNING")) return false;
        if (has_token(tokens, "STARTING")) return false;
        if (has_token(tokens, "STOPPING")) return false;
        if (has_token(tokens, "PAUSED")) return false;
        return true;
      };
      if (is_ready_only(state_.seqstate) && !is_ready_only(prev_state)) {
        std::string keep_state = state_.seqstate;
        state_.reset();
        state_.seqstate = keep_state;
      }
    } else if (starts_with_local(msg, "WAITSTATE:")) {
      handle_waitstate(trim_copy(msg.substr(10)));
    } else if (starts_with_local(msg, "EXPTIME:")) {
      // Parse EXPTIME:remaining total percent
      auto parts = split_ws(msg.substr(8)); // Skip "EXPTIME:"
      std::cerr << "DEBUG EXPTIME parsing, parts.size=" << parts.size() << "\n";
      if (parts.size() >= 3) {
        try {
          int remaining_ms = std::stoi(parts[0]);
          int total_ms = std::stoi(parts[1]);
          int percent = std::stoi(parts[2]);
          std::cerr << "DEBUG EXPTIME parsed: remaining=" << remaining_ms
                    << " total=" << total_ms << " percent=" << percent << "\n";
          if (total_ms > 0) {
            int elapsed_ms = total_ms - remaining_ms;
            state_.exposure_elapsed = elapsed_ms / 1000.0;
            state_.exposure_total = total_ms / 1000.0;
            // Smooth the percentage (exponential moving average to handle multiple cameras)
            double new_progress = std::min(1.0, percent / 100.0);
            if (state_.exposure_progress > 0.0) {
              state_.exposure_progress = 0.7 * state_.exposure_progress + 0.3 * new_progress;
            } else {
              state_.exposure_progress = new_progress;
            }
            std::cerr << "DEBUG exposure_progress now: " << state_.exposure_progress
                      << " (from percent=" << percent << ")\n";
            set_phase(PHASE_EXPOSE);
          }
        } catch (const std::exception &e) {
          std::cerr << "DEBUG EXPTIME parse exception: " << e.what() << "\n";
        }
      }
    } else if (starts_with_local(msg, "ELAPSEDTIME")) {
      auto parts = split_ws(msg);
      if (parts.size() >= 2) {
        int elapsed_ms = parse_int_after_colon(parts[0]);
        int total_ms = parse_int_after_colon(parts[1]);
        if (total_ms > 0) {
          state_.exposure_elapsed = elapsed_ms / 1000.0;
          state_.exposure_total = total_ms / 1000.0;
          state_.exposure_progress = std::min(1.0, state_.exposure_elapsed / state_.exposure_total);
          set_phase(PHASE_EXPOSE);
        }
      }
    }

    if (msg.find("NOTICE: waiting for TCS operator") != std::string::npos) {
      state_.waiting_for_tcsop = true;
      set_phase(PHASE_SLEW);
    }
    if (starts_with_local(msg, "TARGETSTATE:")) {
      std::string upper = to_upper_copy(msg);
      int obsid = -1;
      auto pos = upper.find("OBSID:");
      if (pos != std::string::npos) {
        try {
          obsid = std::stoi(upper.substr(pos + 6));
        } catch (...) {
        }
      }
      if (obsid > 0 && state_.current_obsid != obsid) {
        state_.reset_progress_only();
        state_.current_obsid = obsid;
      }
      auto state_pos = upper.find("TARGETSTATE:");
      if (state_pos != std::string::npos) {
        std::string rest = upper.substr(state_pos + 12);
        rest = trim_copy(rest);
        auto space = rest.find(' ');
        std::string tstate = (space == std::string::npos) ? rest : rest.substr(0, space);
        if (!tstate.empty() && tstate != state_.current_target_state) {
          if (tstate == "ACTIVE" || tstate == "COMPLETE" || tstate == "PENDING") {
            state_.reset_progress_only();
          }
          state_.current_target_state = tstate;
        }
      }
    }
    if (msg.find("NOTICE: received ontarget") != std::string::npos) {
      state_.ontarget = true;
      state_.phase_complete[PHASE_SLEW] = true;
      clear_phase_active(PHASE_SLEW);
      state_.waiting_for_tcsop = false;
      state_.last_ontarget = std::chrono::steady_clock::now();
    }
    if (msg.find("NOTICE: waiting for USER") != std::string::npos) {
      state_.waiting_for_user = true;
      // Determine what "continue" will do based on the wait message
      state_.continue_starts_acquisition = (msg.find("start acquisition") != std::string::npos);
      state_.continue_will_expose = !state_.continue_starts_acquisition;
      // Detect if this is a failure-based user wait
      if (msg.find("guiding failed") != std::string::npos ||
          msg.find("fine tune failed") != std::string::npos) {
        state_.user_wait_after_failure = true;
      }
    }
    if (msg.find("NOTICE: received continue") != std::string::npos) {
      state_.waiting_for_user = false;
      state_.user_wait_after_failure = false;
      state_.continue_will_expose = false;
      state_.continue_starts_acquisition = false;
    }
    if (msg.find("NOTICE: waiting for ACAM guiding") != std::string::npos) {
      state_.guiding_on = false;
      state_.guiding_failed = false;
      set_phase(PHASE_SOLVE);
    }
    if (msg.find("failed to reach guiding state") != std::string::npos ||
        msg.find("guiding failed") != std::string::npos) {
      state_.guiding_failed = true;
      state_.guiding_on = false;
    }
    if (msg.find("NOTICE: running fine tune command") != std::string::npos) {
      state_.guiding_on = true;
      state_.guiding_failed = false;
      set_phase(PHASE_FINE);
    }
    if (msg.find("NOTICE: fine tune complete") != std::string::npos) {
      state_.guiding_on = true;
      state_.guiding_failed = false;
      state_.phase_complete[PHASE_FINE] = true;
      clear_phase_active(PHASE_FINE);
    }
    if (msg.find("NOTICE: applying target offset") != std::string::npos) {
      state_.offset_applicable = true;
      set_phase(PHASE_OFFSET);
    }
    if (msg.find("NOTICE: waiting for offset settle") != std::string::npos) {
      set_phase(PHASE_OFFSET);
    }
    if (msg.find("NOTICE: running fine tune command") != std::string::npos &&
        state_.phase_complete[PHASE_SOLVE]) {
      clear_phase_active(PHASE_SOLVE);
    }
  }
};

int main(int argc, char **argv) {
  std::signal(SIGPIPE, SIG_IGN);
  Options opt = parse_args(argc, argv);
  if (opt.config_path.empty()) {
    opt.config_path = default_config_path();
  }
  if (opt.acam_config_path.empty()) {
    opt.acam_config_path = default_acam_config_path();
  }
  if (opt.acam_host.empty()) {
    opt.acam_host = opt.host;
  }

  load_config(opt.config_path, opt);
  load_acam_config(opt.acam_config_path, opt);

  if (opt.nbport <= 0) {
    std::cerr << "ERROR: NBPORT not set (check sequencerd.cfg)\n";
    return 1;
  }
  if (opt.msgport <= 0 && opt.sub_endpoint.empty()) {
    std::cerr << "WARNING: MESSAGEPORT not set and SUB_ENDPOINT empty; only polling will be available\n";
  }

  SeqProgressGui gui(opt);
  if (!gui.init()) {
    return 1;
  }
  gui.run();
  return 0;
}
