// ngps_acq.c
// NGPS acquisition / fine-acquire helper for the slice-viewing camera.
//
// Goals:
//  - robust source detection + centroiding under poor seeing
//  - robust statistics before issuing any TCS offset
//  - reject duplicate frames and frames taken during/just after telescope motion
//  - optional "manual" frame acquisition via:  scam framegrab one <path>
//
// Build (typical on NGPS machines):
//   gcc -O3 -Wall -Wextra -std=c11 -o ngps_acq ngps_acq.c -lcfitsio -lwcs -lm
//
// Example (stream mode, file updated by camera):
//   ./ngps_acq --input /tmp/slicecam.fits --goal-x 512 --goal-y 512 --loop 1 \
//              --bg-x1 50 --bg-x2 950 --bg-y1 30 --bg-y2 980 --debug 1
//
// Example (manual framegrab mode, synchronous):
//   ./ngps_acq --frame-mode framegrab --framegrab-out /tmp/ngps_acq.fits \
//              --goal-x 512 --goal-y 512 --loop 1
//
// Notes:
//  - TCS command assumed:  tcs native pt <dra> <ddec>
//  - We set native units (dra/ddec arcsec) once per run by default.
//  - Commanded offsets are computed as (star - goal) (arcsec), i.e. the move
//    that should place the star on the goal pixel. If your TCS sign convention
//    is opposite, set --tcs-sign -1.

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <math.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include <sys/stat.h>
#include <limits.h>
#include <stdarg.h>

#include "fitsio.h"
#include "ngps_acq_embed.h"

#ifdef __has_include
#  if __has_include(<wcslib/wcs.h>)
#    include <wcslib/wcs.h>
#    include <wcslib/wcshdr.h>
#  else
#    include <wcs.h>
#    include <wcshdr.h>
#  endif
#else
#  include <wcslib/wcs.h>
#  include <wcslib/wcshdr.h>
#endif

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

// ROI mask bits
#define ROI_X1_SET 0x01
#define ROI_X2_SET 0x02
#define ROI_Y1_SET 0x04
#define ROI_Y2_SET 0x08

typedef enum {
  FRAME_STREAM = 0,
  FRAME_FRAMEGRAB = 1
} FrameMode;

typedef struct {
  char   input[PATH_MAX];        // stream file path

  FrameMode frame_mode;
  char   framegrab_out[PATH_MAX];
  int    framegrab_use_tmp;      // if 1: write to framegrab_out then atomically rename

  double goal_x;
  double goal_y;
  int    pixel_origin;           // 0 or 1

  // Candidate search constraints
  double max_dist_pix;           // circular cut around goal (pix)
  double snr_thresh;             // detection threshold (sigma)
  int    min_adjacent;           // raw-pixel neighbors above raw threshold

  // Filtering for detection
  double filt_sigma_pix;         // Gaussian sigma for smoothing (pix)

  // Centroiding
  int    centroid_halfwin;       // window half-width (pix)
  double centroid_sigma_pix;     // Gaussian window sigma (pix)
  int    centroid_maxiter;
  double centroid_eps_pix;

  // FITS selection
  int    extnum;                 // 0=primary, 1=first extension, ...
  char   extname[32];            // preferred EXTNAME, empty disables

  // Background statistics ROI (inclusive; user origin)
  int    bg_roi_mask;
  long   bg_x1, bg_x2, bg_y1, bg_y2;

  // Candidate search ROI (inclusive; user origin)
  int    search_roi_mask;
  long   search_x1, search_x2, search_y1, search_y2;

  // Closed-loop / guiding wrapper
  int    loop;
  double cadence_sec;            // minimum seconds between accepted samples (stream mode)
  int    max_samples;            // per-move gather (accepted samples)
  int    min_samples;            // minimum before testing scatter
  double prec_arcsec;            // required robust scatter (MAD->sigma) per axis
  double goal_arcsec;            // convergence threshold on robust offset magnitude
  int    max_cycles;             // number of move cycles
  double gain;                   // gain applied to commanded move (0..1 recommended)
  int    adaptive;               // if 1: adapt exposure + loop thresholds from measured source counts
  double adaptive_faint;         // start faint adaptation at/under this metric
  double adaptive_faint_goal;    // faint-mode target metric
  double adaptive_bright;        // start bright adaptation at/over this metric
  double adaptive_bright_goal;   // bright-mode target metric

  // Frame quality & safety
  int    reject_identical;       // reject identical image signatures
  int    reject_after_move;      // reject N new frames after any TCS move
  double settle_sec;             // optional sleep after move (additional to rejecting frames)
  double max_move_arcsec;        // safety cap; do not issue moves larger than this
  int    continue_on_fail;       // if 0: exit on failure to build stats; if 1: keep trying

  // Offset conventions
  int    dra_use_cosdec;         // 1: dra = dRA*cos(dec)
  int    tcs_sign;               // multiply commanded offsets by +/-1

  // TCS options
  int    tcs_set_units;          // if 1: run "tcs native dra 'arcsec'" and "... ddec 'arcsec'" once

  // SCAM daemon option (guiding-friendly moves)
  int    use_putonslit;          // if 1: call putonslit <slitra> <slitdec> <crossra> <crossdec>
  int    wait_guiding;           // if 1: wait for ACAM guiding state after putonslit
  double guiding_poll_sec;       // polling interval for "acam acquire"
  double guiding_timeout_sec;    // timeout for waiting on guiding (0 = no timeout)

  // Debug
  int    debug;
  char   debug_out[PATH_MAX];

  // General
  int    dry_run;
  int    verbose;
} AcqParams;

typedef struct {
  int    found;
  // peak in pixel coords (user origin)
  double peak_x, peak_y;
  // centroid (user origin)
  double cx, cy;
  // quality
  double peak_val;
  double peak_snr_raw;
  double snr_ap;
  double src_top10_mean;         // mean of top 10% positive residual pixels in centroid window
  double bkg;
  double sigma;
  // debug ROI bounds (0-based inclusive)
  long   rx1, rx2, ry1, ry2;     // stats ROI
  long   sx1, sx2, sy1, sy2;     // search ROI
} Detection;

typedef struct {
  int    ok;
  Detection det;

  // Pixel offsets star - goal (pix)
  double dx_pix;
  double dy_pix;

  // WCS
  int    wcs_ok;
  double ra_goal_deg, dec_goal_deg;
  double ra_star_deg, dec_star_deg;

  // Commanded offsets (arcsec) for tcs native pt <dra> <ddec>
  double dra_cmd_arcsec;
  double ddec_cmd_arcsec;
  double r_cmd_arcsec;
} FrameResult;

typedef struct {
  time_t mtime;
  off_t  size;
  uint64_t sig;        // image signature (subsampled)
  int have_sig;
  struct timespec t_accept; // time we accepted this frame
} FrameState;

typedef enum {
  ADAPT_MODE_BASELINE = 0,
  ADAPT_MODE_FAINT = 1,
  ADAPT_MODE_BRIGHT = 2
} AdaptiveMode;

typedef struct {
  AdaptiveMode mode;
  double cadence_sec;
  int reject_after_move;
  double prec_arcsec;
  double goal_arcsec;
  int apply_camera;
  double exptime_sec;
  int avgframes;
} AdaptiveCycleConfig;

typedef struct {
  AdaptiveMode mode;
  int have_metric;
  double metric_ewma;
  int have_last_camera;
  double last_exptime_sec;
  int last_avgframes;
} AdaptiveRuntime;

static volatile sig_atomic_t g_stop = 0;
static ngps_acq_hooks_t g_hooks;
static int g_hooks_initialized = 0;
static void on_sigint(int sig) { (void)sig; g_stop = 1; }

static int stop_requested(void) {
  if (g_stop) return 1;
  if (g_hooks_initialized && g_hooks.is_stop_requested) {
    return g_hooks.is_stop_requested(g_hooks.user) ? 1 : 0;
  }
  return 0;
}

static void acq_vlogf(const char* fmt, va_list ap) {
  va_list copy;
  va_copy(copy, ap);
  vfprintf(stderr, fmt, copy);
  va_end(copy);

  if (g_hooks_initialized && g_hooks.log_message) {
    char buffer[2048];
    va_copy(copy, ap);
    vsnprintf(buffer, sizeof(buffer), fmt, copy);
    va_end(copy);
    g_hooks.log_message(g_hooks.user, buffer);
  }
}

static void acq_logf(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  acq_vlogf(fmt, ap);
  va_end(ap);
}

static void die(const char* msg) {
  acq_logf("FATAL: %s\n", msg);
  exit(4);
}

static void sleep_seconds(double sec) {
  if (sec <= 0) return;
  struct timespec ts;
  ts.tv_sec  = (time_t)floor(sec);
  ts.tv_nsec = (long)((sec - (double)ts.tv_sec) * 1e9);
  while (nanosleep(&ts, &ts) == -1 && errno == EINTR) {}
}

static double now_monotonic_sec(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (double)ts.tv_sec + 1e-9*(double)ts.tv_nsec;
}

static int cmp_float(const void* a, const void* b) {
  float fa = *(const float*)a;
  float fb = *(const float*)b;
  return (fa < fb) ? -1 : (fa > fb) ? 1 : 0;
}

static int cmp_double(const void* a, const void* b) {
  double da = *(const double*)a;
  double db = *(const double*)b;
  return (da < db) ? -1 : (da > db) ? 1 : 0;
}

static double wrap_dra_deg(double dra) {
  while (dra > 180.0) dra -= 360.0;
  while (dra < -180.0) dra += 360.0;
  return dra;
}

// Convert a user-specified ROI into clamped 0-based inclusive bounds.
// If mask is 0, returns full-frame bounds.
static void compute_roi_0based(long nx, long ny, int pixel_origin,
                               int mask, long ux1_in, long ux2_in, long uy1_in, long uy2_in,
                               long* x1, long* x2, long* y1, long* y2)
{
  long ux1 = (pixel_origin == 0) ? 0  : 1;
  long ux2 = (pixel_origin == 0) ? (nx - 1) : nx;
  long uy1 = (pixel_origin == 0) ? 0  : 1;
  long uy2 = (pixel_origin == 0) ? (ny - 1) : ny;

  if (mask & ROI_X1_SET) ux1 = ux1_in;
  if (mask & ROI_X2_SET) ux2 = ux2_in;
  if (mask & ROI_Y1_SET) uy1 = uy1_in;
  if (mask & ROI_Y2_SET) uy2 = uy2_in;

  long ax1 = ux1, ax2 = ux2, ay1 = uy1, ay2 = uy2;
  if (pixel_origin == 1) { ax1--; ax2--; ay1--; ay2--; }

  if (ax2 < ax1) { long t=ax1; ax1=ax2; ax2=t; }
  if (ay2 < ay1) { long t=ay1; ay1=ay2; ay2=t; }

  if (ax1 < 0) ax1 = 0;
  if (ay1 < 0) ay1 = 0;
  if (ax2 > nx-1) ax2 = nx-1;
  if (ay2 > ny-1) ay2 = ny-1;

  if (ax2 < ax1) { ax1=0; ax2=nx-1; }
  if (ay2 < ay1) { ay1=0; ay2=ny-1; }

  *x1=ax1; *x2=ax2; *y1=ay1; *y2=ay2;
}

// Subsample pixels in ROI into a float array.
static float* roi_subsample(const float* img, long nx, long ny,
                            long x1, long x2, long y1, long y2,
                            long target, long* n_out)
{
  if (x1 < 0) x1 = 0;
  if (y1 < 0) y1 = 0;
  if (x2 > nx-1) x2 = nx-1;
  if (y2 > ny-1) y2 = ny-1;

  long wx = x2 - x1 + 1;
  long wy = y2 - y1 + 1;
  if (wx <= 0 || wy <= 0) { *n_out = 0; return NULL; }

  long Nroi = wx * wy;
  long stride = (Nroi > target) ? (Nroi / target) : 1;
  if (stride < 1) stride = 1;

  long ns = (Nroi + stride - 1) / stride;
  float* sample = (float*)malloc((size_t)ns * sizeof(float));
  if (!sample) die("malloc sample failed");

  long k = 0;
  long idx = 0;
  for (long y = y1; y <= y2; y++) {
    long row0 = y * nx;
    for (long x = x1; x <= x2; x++, idx++) {
      if ((idx % stride) == 0) sample[k++] = img[row0 + x];
    }
  }

  *n_out = k;
  return sample;
}

// SExtractor-like background + sigma estimation within ROI:
//  - initial median + MAD
//  - iterative sigma-clipping around median
//  - background via mode = 2.5*median - 1.5*mean (unless skewed, then median)
static void bg_sigma_sextractor_like(const float* img, long nx, long ny,
                                     long x1, long x2, long y1, long y2,
                                     double* bkg_out, double* sigma_out)
{
  *bkg_out = 0.0;
  *sigma_out = 1.0;

  long ns = 0;
  float* sample = roi_subsample(img, nx, ny, x1, x2, y1, y2, 200000, &ns);
  if (!sample || ns < 64) {
    if (sample) free(sample);
    return;
  }

  qsort(sample, (size_t)ns, sizeof(float), cmp_float);
  double median = (ns % 2) ? sample[ns/2] : 0.5*(sample[ns/2 - 1] + sample[ns/2]);

  // MAD
  float* dev = (float*)malloc((size_t)ns * sizeof(float));
  if (!dev) die("malloc dev failed");
  for (long i = 0; i < ns; i++) dev[i] = (float)fabs((double)sample[i] - median);
  qsort(dev, (size_t)ns, sizeof(float), cmp_float);
  double mad = (ns % 2) ? dev[ns/2] : 0.5*(dev[ns/2 - 1] + dev[ns/2]);
  free(dev);

  double sigma = 1.4826 * mad;
  if (!isfinite(sigma) || sigma <= 0) sigma = 1.0;

  // Iterative clip
  const double clip = 3.0;
  double mean = median;
  double sigma_prev = sigma;
  for (int it = 0; it < 8; it++) {
    double lo = median - clip * sigma;
    double hi = median + clip * sigma;

    double sum = 0.0, sum2 = 0.0;
    long n = 0;
    for (long i = 0; i < ns; i++) {
      double v = sample[i];
      if (v < lo || v > hi) continue;
      sum += v;
      sum2 += v*v;
      n++;
    }
    if (n < 32) break;

    mean = sum / (double)n;
    double var = (sum2 / (double)n) - mean*mean;
    if (var < 0) var = 0;
    sigma = sqrt(var);
    if (!isfinite(sigma) || sigma <= 0) sigma = sigma_prev;

    double rel = fabs(sigma - sigma_prev) / (sigma_prev > 0 ? sigma_prev : 1.0);
    sigma_prev = sigma;
    if (rel < 0.01) break;
  }

  double mode = 2.5*median - 1.5*mean;
  double bkg = mode;
  if (sigma > 0 && (mean - median)/sigma > 0.3) bkg = median;
  if (!isfinite(bkg)) bkg = median;
  if (!isfinite(sigma) || sigma <= 0) sigma = 1.0;

  *bkg_out = bkg;
  *sigma_out = sigma;

  free(sample);
}

static double median_of_doubles(double* a, int n)
{
  if (n <= 0) return 0.0;
  qsort(a, (size_t)n, sizeof(double), cmp_double);
  return (n % 2) ? a[n/2] : 0.5*(a[n/2 - 1] + a[n/2]);
}

static double mad_sigma_of_doubles(const double* a_in, int n, double med)
{
  if (n <= 1) return 0.0;
  double* d = (double*)malloc((size_t)n * sizeof(double));
  if (!d) die("malloc mad failed");
  for (int i = 0; i < n; i++) d[i] = fabs(a_in[i] - med);
  qsort(d, (size_t)n, sizeof(double), cmp_double);
  double mad = (n % 2) ? d[n/2] : 0.5*(d[n/2 - 1] + d[n/2]);
  free(d);
  return 1.4826 * mad;
}

// Gaussian kernel (1D) normalized to sum=1.
static double* make_gaussian_kernel(double sigma, int* radius_out)
{
  if (sigma <= 0.2) sigma = 0.2;
  int r = (int)ceil(3.0*sigma);
  if (r < 1) r = 1;
  int len = 2*r + 1;
  double* k = (double*)malloc((size_t)len * sizeof(double));
  if (!k) die("malloc kernel failed");

  double sum = 0.0;
  for (int i = -r; i <= r; i++) {
    double x = (double)i / sigma;
    double v = exp(-0.5 * x * x);
    k[i+r] = v;
    sum += v;
  }
  if (sum <= 0) sum = 1.0;
  for (int i = 0; i < len; i++) k[i] /= sum;

  *radius_out = r;
  return k;
}

static double kernel_sum_sq(const double* k, int radius)
{
  int len = 2*radius + 1;
  double s2 = 0.0;
  for (int i = 0; i < len; i++) s2 += k[i]*k[i];
  return s2;
}

// Separable convolution on a patch (w*h). Border handling: clamp.
static void convolve_separable(const float* in, float* tmp, float* out,
                               int w, int h, const double* k, int r)
{
  // horizontal
  for (int y = 0; y < h; y++) {
    const float* row = in + y*w;
    float* trow = tmp + y*w;
    for (int x = 0; x < w; x++) {
      double acc = 0.0;
      for (int dx = -r; dx <= r; dx++) {
        int xx = x + dx;
        if (xx < 0) xx = 0;
        if (xx >= w) xx = w-1;
        acc += (double)row[xx] * k[dx+r];
      }
      trow[x] = (float)acc;
    }
  }

  // vertical
  for (int y = 0; y < h; y++) {
    float* orow = out + y*w;
    for (int x = 0; x < w; x++) {
      double acc = 0.0;
      for (int dy = -r; dy <= r; dy++) {
        int yy = y + dy;
        if (yy < 0) yy = 0;
        if (yy >= h) yy = h-1;
        acc += (double)tmp[yy*w + x] * k[dy+r];
      }
      orow[x] = (float)acc;
    }
  }
}

// Simple debug drawing (PPM)
static void set_px(uint8_t* rgb, int w, int h, int x, int y, uint8_t r, uint8_t g, uint8_t b)
{
  if (x < 0 || y < 0 || x >= w || y >= h) return;
  size_t idx = (size_t)(y*w + x) * 3;
  rgb[idx+0] = r;
  rgb[idx+1] = g;
  rgb[idx+2] = b;
}

static void draw_plus(uint8_t* rgb, int w, int h, int x, int y, int rad, uint8_t r, uint8_t g, uint8_t b)
{
  for (int dx = -rad; dx <= rad; dx++) set_px(rgb, w, h, x+dx, y, r,g,b);
  for (int dy = -rad; dy <= rad; dy++) set_px(rgb, w, h, x, y+dy, r,g,b);
}

static void draw_x(uint8_t* rgb, int w, int h, int x, int y, int rad, uint8_t r, uint8_t g, uint8_t b)
{
  for (int d = -rad; d <= rad; d++) {
    set_px(rgb, w, h, x+d, y+d, r,g,b);
    set_px(rgb, w, h, x+d, y-d, r,g,b);
  }
}

static void draw_circle(uint8_t* rgb, int w, int h, int xc, int yc, int rad, uint8_t r, uint8_t g, uint8_t b)
{
  int x = rad;
  int y = 0;
  int err = 0;
  while (x >= y) {
    set_px(rgb,w,h, xc + x, yc + y, r,g,b);
    set_px(rgb,w,h, xc + y, yc + x, r,g,b);
    set_px(rgb,w,h, xc - y, yc + x, r,g,b);
    set_px(rgb,w,h, xc - x, yc + y, r,g,b);
    set_px(rgb,w,h, xc - x, yc - y, r,g,b);
    set_px(rgb,w,h, xc - y, yc - x, r,g,b);
    set_px(rgb,w,h, xc + y, yc - x, r,g,b);
    set_px(rgb,w,h, xc + x, yc - y, r,g,b);
    y++;
    if (err <= 0) {
      err += 2*y + 1;
    } else {
      x--;
      err += 2*(y - x) + 1;
    }
  }
}

static void draw_line(uint8_t* rgb, int w, int h, int x0, int y0, int x1, int y1, uint8_t r, uint8_t g, uint8_t b)
{
  int dx = abs(x1 - x0), sx = (x0 < x1) ? 1 : -1;
  int dy = -abs(y1 - y0), sy = (y0 < y1) ? 1 : -1;
  int err = dx + dy;
  while (1) {
    set_px(rgb,w,h,x0,y0,r,g,b);
    if (x0 == x1 && y0 == y1) break;
    int e2 = 2*err;
    if (e2 >= dy) { err += dy; x0 += sx; }
    if (e2 <= dx) { err += dx; y0 += sy; }
  }
}

static void draw_arrow(uint8_t* rgb, int w, int h, int x0, int y0, int x1, int y1, uint8_t r, uint8_t g, uint8_t b)
{
  draw_line(rgb,w,h,x0,y0,x1,y1,r,g,b);
  double ang = atan2((double)(y1 - y0), (double)(x1 - x0));
  double a1 = ang + M_PI/8.0;
  double a2 = ang - M_PI/8.0;
  int L = 10;
  int hx1 = x1 - (int)lround(L * cos(a1));
  int hy1 = y1 - (int)lround(L * sin(a1));
  int hx2 = x1 - (int)lround(L * cos(a2));
  int hy2 = y1 - (int)lround(L * sin(a2));
  draw_line(rgb,w,h,x1,y1,hx1,hy1,r,g,b);
  draw_line(rgb,w,h,x1,y1,hx2,hy2,r,g,b);
}

static int write_debug_ppm(const char* outpath,
                           const float* img, long nx,
                           long rx1, long rx2, long ry1, long ry2,
                           double bkg, double sigma, double snr_thresh,
                           const Detection* det,
                           const AcqParams* p)
{
  int w = (int)(rx2 - rx1 + 1);
  int h = (int)(ry2 - ry1 + 1);
  if (w <= 0 || h <= 0) return 1;

  uint8_t* rgb = (uint8_t*)malloc((size_t)w * (size_t)h * 3);
  if (!rgb) return 2;

  double vmin = bkg - 2.0*sigma;
  double vmax = bkg + 6.0*sigma;
  double inv = (vmax > vmin) ? (1.0/(vmax - vmin)) : 1.0;

  for (int yy = 0; yy < h; yy++) {
    long y = ry1 + yy;
    for (int xx = 0; xx < w; xx++) {
      long x = rx1 + xx;
      double v = img[y*nx + x];
      double t = (v - vmin) * inv;
      if (t < 0) t = 0;
      if (t > 1) t = 1;
      uint8_t g = (uint8_t)lround(255.0 * t);
      size_t idx = (size_t)(yy*w + xx) * 3;
      rgb[idx+0] = g;
      rgb[idx+1] = g;
      rgb[idx+2] = g;

      double snr = (sigma > 0) ? ((v - bkg) / sigma) : 0;
      if (snr >= snr_thresh) {
        // color above-threshold pixels (cyan-ish)
        rgb[idx+0] = (uint8_t)((int)rgb[idx+0] / 2);
        rgb[idx+1] = 255;
        rgb[idx+2] = 255;
      }
    }
  }

  if (det && det->found) {
    int px0 = (p->pixel_origin == 0) ? (int)lround(det->peak_x) : (int)lround(det->peak_x - 1.0);
    int py0 = (p->pixel_origin == 0) ? (int)lround(det->peak_y) : (int)lround(det->peak_y - 1.0);
    int cx0 = (p->pixel_origin == 0) ? (int)lround(det->cx)     : (int)lround(det->cx - 1.0);
    int cy0 = (p->pixel_origin == 0) ? (int)lround(det->cy)     : (int)lround(det->cy - 1.0);

    int gx0 = (p->pixel_origin == 0) ? (int)lround(p->goal_x)   : (int)lround(p->goal_x - 1.0);
    int gy0 = (p->pixel_origin == 0) ? (int)lround(p->goal_y)   : (int)lround(p->goal_y - 1.0);

    // shift into ROI image coordinates
    int px = px0 - (int)rx1;
    int py = py0 - (int)ry1;
    int cx = cx0 - (int)rx1;
    int cy = cy0 - (int)ry1;
    int gx = gx0 - (int)rx1;
    int gy = gy0 - (int)ry1;

    draw_circle(rgb, w, h, px, py, 10, 255, 50, 50);
    draw_plus(rgb,   w, h, cx, cy, 6,  50, 255, 50);
    draw_x(rgb,      w, h, gx, gy, 8,  255, 255, 0);
    draw_arrow(rgb,  w, h, cx, cy, gx, gy, 255, 200, 0);
  }

  FILE* f = fopen(outpath, "wb");
  if (!f) { free(rgb); return 3; }
  fprintf(f, "P6\n%d %d\n255\n", w, h);
  fwrite(rgb, 1, (size_t)w*(size_t)h*3, f);
  fclose(f);
  free(rgb);
  return 0;
}

// --- FITS I/O helpers ---
static int move_to_image_hdu_by_extname(fitsfile* fptr, const char* want_extname, int* out_hdu_index, int* status)
{
  if (!want_extname || want_extname[0] == '\0') return 1;

  int nhdus = 0;
  if (fits_get_num_hdus(fptr, &nhdus, status)) return 2;

  for (int hdu = 1; hdu <= nhdus; hdu++) {
    int hdutype = 0;
    if (fits_movabs_hdu(fptr, hdu, &hdutype, status)) return 3;
    if (hdutype != IMAGE_HDU) continue;

    char extname[FLEN_VALUE] = {0};
    int keystat = 0;
    if (fits_read_key(fptr, TSTRING, "EXTNAME", extname, NULL, &keystat)) {
      extname[0] = '\0';
    }

    if (extname[0] && strcasecmp(extname, want_extname) == 0) {
      if (out_hdu_index) *out_hdu_index = hdu;
      return 0;
    }
  }

  return 4;
}

static int read_fits_image_and_header(const char* path, const AcqParams* p,
                                      float** img_out, long* nx_out, long* ny_out,
                                      char** header_out, int* nkeys_out,
                                      int* used_hdu_out, char* used_extname_out, size_t used_extname_sz,
                                      double* exptime_sec_out)
{
  fitsfile* fptr = NULL;
  int status = 0;

  if (fits_open_file(&fptr, path, READONLY, &status)) return status;

  int used_hdu = 1;
  char used_extname[FLEN_VALUE] = {0};

  // Prefer EXTNAME
  if (p->extname[0]) {
    int found_hdu = 0;
    int rc = move_to_image_hdu_by_extname(fptr, p->extname, &found_hdu, &status);
    if (rc == 0) {
      used_hdu = found_hdu;
    } else {
      if (p->verbose) acq_logf( "WARNING: EXTNAME='%s' not found; falling back to extnum=%d\n", p->extname, p->extnum);
      status = 0;
      int hdutype = 0;
      if (fits_movabs_hdu(fptr, p->extnum + 1, &hdutype, &status)) {
        fits_close_file(fptr, &status);
        return status;
      }
      used_hdu = p->extnum + 1;
    }
  } else {
    int hdutype = 0;
    if (fits_movabs_hdu(fptr, p->extnum + 1, &hdutype, &status)) {
      fits_close_file(fptr, &status);
      return status;
    }
    used_hdu = p->extnum + 1;
  }

  // record EXTNAME
  {
    int keystat = 0;
    if (fits_read_key(fptr, TSTRING, "EXTNAME", used_extname, NULL, &keystat)) {
      used_extname[0] = '\0';
    }
  }

  // record EXPTIME (fallback to 1s if unavailable)
  double exptime_sec = 1.0;
  {
    int keystat = 0;
    double exptmp = 0.0;
    if (!fits_read_key(fptr, TDOUBLE, "EXPTIME", &exptmp, NULL, &keystat) &&
        isfinite(exptmp) && exptmp > 0.0) {
      exptime_sec = exptmp;
    }
  }

  int bitpix = 0, naxis = 0;
  long naxes[3] = {0,0,0};
  if (fits_get_img_param(fptr, 3, &bitpix, &naxis, naxes, &status)) {
    fits_close_file(fptr, &status);
    return status;
  }
  if (naxis < 2) {
    fits_close_file(fptr, &status);
    return BAD_NAXIS;
  }

  long nx = naxes[0];
  long ny = naxes[1];

  float* img = (float*)malloc((size_t)nx * (size_t)ny * sizeof(float));
  if (!img) {
    fits_close_file(fptr, &status);
    return MEMORY_ALLOCATION;
  }

  long fpixel[3] = {1,1,1};
  long nelem = nx * ny;
  int anynul = 0;
  if (fits_read_pix(fptr, TFLOAT, fpixel, nelem, NULL, img, &anynul, &status)) {
    free(img);
    fits_close_file(fptr, &status);
    return status;
  }

  char* header = NULL;
  int nkeys = 0;
  int hstatus = 0;
  if (fits_hdr2str(fptr, 1, NULL, 0, &header, &nkeys, &hstatus)) {
    header = NULL;
    nkeys = 0;
  }

  fits_close_file(fptr, &status);

  *img_out = img;
  *nx_out  = nx;
  *ny_out  = ny;
  *header_out = header;
  *nkeys_out  = nkeys;
  if (used_hdu_out) *used_hdu_out = used_hdu;
  if (used_extname_out && used_extname_sz > 0) {
    snprintf(used_extname_out, used_extname_sz, "%s", used_extname);
  }
  if (exptime_sec_out) *exptime_sec_out = exptime_sec;

  return 0;
}

// --- WCS helpers ---
static int init_wcs_from_header(const char* header, int nkeys, struct wcsprm** wcs_out, int* nwcs_out)
{
  if (!header || nkeys <= 0) return 1;

  int relax = WCSHDR_all;
  int ctrl  = 2;
  int nrej = 0, nwcs = 0;
  struct wcsprm* wcs = NULL;

  int stat = wcspih((char*)header, nkeys, relax, ctrl, &nrej, &nwcs, &wcs);
  if (stat || nwcs < 1 || !wcs) return 2;

  if (wcsset(&wcs[0])) {
    wcsvfree(&nwcs, &wcs);
    return 3;
  }

  *wcs_out = wcs;
  *nwcs_out = nwcs;
  return 0;
}

// Pixel -> (RA,Dec) degrees using WCSLIB. Inputs are FITS 1-based pixels.
static int pix2world_wcs(const struct wcsprm* wcs0, double pix_x, double pix_y,
                         double* ra_deg, double* dec_deg)
{
  if (!wcs0) return 1;

  double pixcrd[2] = {pix_x, pix_y};
  double imgcrd[2] = {0,0};
  double phi=0, theta=0;
  double world[2] = {0,0};
  int stat[2] = {0,0};

  int rc = wcsp2s((struct wcsprm*)wcs0, 1, 2, pixcrd, imgcrd, &phi, &theta, world, stat);
  if (rc) return 2;

  *ra_deg  = world[0];
  *dec_deg = world[1];
  return 0;
}

// --- Frame signature (reject identical) ---
static uint64_t fnv1a64_init(void) { return 1469598103934665603ULL; }
static uint64_t fnv1a64_step(uint64_t h, uint64_t x) {
  h ^= x;
  return h * 1099511628211ULL;
}

static uint64_t image_signature_subsample(const float* img, long nx, long ny,
                                          long x1, long x2, long y1, long y2)
{
  if (!img || nx <= 0 || ny <= 0) return 0;
  if (x1 < 0) x1 = 0;
  if (y1 < 0) y1 = 0;
  if (x2 > nx-1) x2 = nx-1;
  if (y2 > ny-1) y2 = ny-1;
  if (x2 < x1 || y2 < y1) return 0;

  long wx = x2 - x1 + 1;
  long wy = y2 - y1 + 1;
  long N = wx * wy;

  // Aim for ~50k samples max.
  long target = 50000;
  long stride = (N > target) ? (N / target) : 1;
  if (stride < 1) stride = 1;

  uint64_t h = fnv1a64_init();
  long idx = 0;
  for (long y = y1; y <= y2; y++) {
    long row0 = y * nx;
    for (long x = x1; x <= x2; x++, idx++) {
      if ((idx % stride) != 0) continue;
      // quantize float to int32 with mild scaling to be stable
      float v = img[row0 + x];
      int32_t q = (int32_t)lrintf(v * 8.0f);
      h = fnv1a64_step(h, (uint64_t)(uint32_t)q);
    }
  }
  // Mix in ROI bounds to reduce accidental collisions
  h = fnv1a64_step(h, (uint64_t)(uint32_t)x1);
  h = fnv1a64_step(h, (uint64_t)(uint32_t)y1);
  h = fnv1a64_step(h, (uint64_t)(uint32_t)x2);
  h = fnv1a64_step(h, (uint64_t)(uint32_t)y2);
  return h;
}

// --- TCS helpers ---
static int tcs_set_native_units(int dry_run, int verbose)
{
  if (g_hooks_initialized && g_hooks.tcs_set_native_units) {
    return g_hooks.tcs_set_native_units(g_hooks.user, dry_run, verbose);
  }

  const char* cmd1 = "tcs native dra 'arcsec'";
  const char* cmd2 = "tcs native ddec 'arcsec'";
  if (verbose || dry_run) {
    acq_logf( "TCS CMD: %s\n", cmd1);
    acq_logf( "TCS CMD: %s\n", cmd2);
  }
  if (dry_run) return 0;
  int rc1 = system(cmd1);
  int rc2 = system(cmd2);
  if (rc1 != 0 || rc2 != 0) {
    acq_logf( "WARNING: TCS unit command failed rc1=%d rc2=%d\n", rc1, rc2);
    return 1;
  }
  return 0;
}

static int tcs_move_arcsec(double dra_arcsec, double ddec_arcsec, int dry_run, int verbose)
{
  if (g_hooks_initialized && g_hooks.tcs_move_arcsec) {
    return g_hooks.tcs_move_arcsec(g_hooks.user, dra_arcsec, ddec_arcsec, dry_run, verbose);
  }

  char cmd[512];
  snprintf(cmd, sizeof(cmd), "tcs native pt %.3f %.3f", dra_arcsec, ddec_arcsec);
  if (verbose || dry_run) acq_logf( "TCS CMD: %s\n", cmd);
  if (dry_run) return 0;

  int rc = system(cmd);
  if (rc != 0) {
    acq_logf( "WARNING: TCS command returned %d\n", rc);
    return 1;
  }
  return 0;
}

// --- SCAM daemon helper (guiding-friendly move) ---
static double wrap_ra_deg(double ra)
{
  // keep in [0,360)
  while (ra < 0.0)   ra += 360.0;
  while (ra >= 360.0) ra -= 360.0;
  return ra;
}

// Convert desired PT offsets (arcsec) into a synthetic "crosshair" RA/Dec given a "slit" RA/Dec.
// This preserves robust median offset logic while still invoking putonslit (which computes PT internally).
static void slit_cross_from_offsets(const AcqParams* p,
                                    double slit_ra_deg, double slit_dec_deg,
                                    double dra_arcsec, double ddec_arcsec,
                                    double* cross_ra_deg, double* cross_dec_deg)
{
  double cosdec = cos(slit_dec_deg * M_PI / 180.0);
  double ddec_deg = ddec_arcsec / 3600.0;

  double dra_deg = dra_arcsec / 3600.0;
  if (p->dra_use_cosdec) {
    // dra_arcsec was computed as dRA*cos(dec)*3600, so invert the cos(dec) here.
    double denom = (fabs(cosdec) > 1e-12) ? cosdec : (cosdec >= 0 ? 1e-12 : -1e-12);
    dra_deg /= denom;
  }

  *cross_dec_deg = slit_dec_deg + ddec_deg;
  *cross_ra_deg  = wrap_ra_deg(slit_ra_deg + dra_deg);
}

// Hard-coded interface: call via the SCAM daemon.
// Intentionally NOT user-adjustable.
static int scam_putonslit_deg(double slit_ra_deg, double slit_dec_deg,
                              double cross_ra_deg, double cross_dec_deg,
                              int dry_run, int verbose)
{
  if (g_hooks_initialized && g_hooks.scam_putonslit_deg) {
    return g_hooks.scam_putonslit_deg(g_hooks.user,
                                      slit_ra_deg, slit_dec_deg,
                                      cross_ra_deg, cross_dec_deg,
                                      dry_run, verbose);
  }

  char cmd[1024];
  // putonslit <slitra> <slitdec> <crossra> <crossdec>  (all decimal degrees)
  snprintf(cmd, sizeof(cmd), "scam putonslit %.10f %.10f %.10f %.10f",
           slit_ra_deg, slit_dec_deg, cross_ra_deg, cross_dec_deg);

  if (verbose || dry_run) acq_logf( "SCAM CMD: %s\n", cmd);
  if (dry_run) return 0;

  int rc = system(cmd);
  if (rc != 0) {
    acq_logf( "WARNING: putonslit command returned %d\n", rc);
    return 1;
  }
  return 0;
}

// --- ACAM guiding state helpers ---
static int acam_query_state(char* state, size_t state_sz, int verbose)
{
  if (g_hooks_initialized && g_hooks.acam_query_state) {
    return g_hooks.acam_query_state(g_hooks.user, state, state_sz, verbose);
  }

  if (!state || state_sz == 0) return 2;
  state[0] = '\0';

  FILE* fp = popen("acam acquire", "r");
  if (!fp) {
    if (verbose) acq_logf( "WARNING: failed to run 'acam acquire': %s\n", strerror(errno));
    return 1;
  }

  char buf[256];
  int found = 0;
  while (fgets(buf, sizeof(buf), fp)) {
    if (strcasestr(buf, "guiding")) {
      snprintf(state, state_sz, "guiding");
      found = 1;
    } else if (strcasestr(buf, "acquiring")) {
      snprintf(state, state_sz, "acquiring");
      found = 1;
    }
  }

  int rc = pclose(fp);
  if (rc != 0 && verbose) {
    acq_logf( "WARNING: 'acam acquire' returned %d\n", rc);
  }

  return found ? 0 : 2;
}

static int wait_for_guiding(const AcqParams* p)
{
  if (!p->wait_guiding || p->dry_run) return 0;

  const double poll_sec = (p->guiding_poll_sec > 0) ? p->guiding_poll_sec : 1.0;
  const double timeout_sec = p->guiding_timeout_sec;

  double t0 = now_monotonic_sec();
  char last_state[32] = {0};
  int warned = 0;

  for (;;) {
    if (stop_requested()) return 2;

    char state[32] = {0};
    int rc = acam_query_state(state, sizeof(state), p->verbose);
    if (rc == 0) {
      if (strcmp(state, last_state) != 0) {
        if (p->verbose) acq_logf( "ACAM state: %s\n", state);
        snprintf(last_state, sizeof(last_state), "%s", state);
      }
      if (!strcmp(state, "guiding")) return 0;
    } else {
      if (!warned) {
        acq_logf( "WARNING: could not determine ACAM state; will keep polling.\n");
        warned = 1;
      }
    }

    if (timeout_sec > 0) {
      double dt = now_monotonic_sec() - t0;
      if (dt >= timeout_sec) {
        acq_logf( "WARNING: timed out waiting for guiding (%.1fs).\n", timeout_sec);
        return 1;
      }
    }

    sleep_seconds(poll_sec);
  }
}


// --- Camera command helpers ---
static int scam_framegrab_one(const char* outpath, int verbose)
{
  if (g_hooks_initialized && g_hooks.scam_framegrab_one) {
    return g_hooks.scam_framegrab_one(g_hooks.user, outpath, verbose);
  }

  char cmd[PATH_MAX + 128];
  // We assume scam writes the file atomically enough; if not, stream stability check handles it.
  snprintf(cmd, sizeof(cmd), "scam framegrab one %s", outpath);
  if (verbose) acq_logf( "CAM CMD: %s\n", cmd);
  int rc = system(cmd);
  if (rc != 0) {
    acq_logf( "WARNING: framegrab command failed (rc=%d)\n", rc);
    return 1;
  }
  return 0;
}

static int scam_set_exptime(double exptime_sec, int dry_run, int verbose)
{
  if (g_hooks_initialized && g_hooks.scam_set_exptime) {
    return g_hooks.scam_set_exptime(g_hooks.user, exptime_sec, dry_run, verbose);
  }

  if (!isfinite(exptime_sec) || exptime_sec <= 0) return 1;
  char cmd[256];
  snprintf(cmd, sizeof(cmd), "scam exptime %.3f", exptime_sec);
  if (verbose || dry_run) acq_logf( "SCAM CMD: %s\n", cmd);
  if (dry_run) return 0;
  int rc = system(cmd);
  if (rc != 0) {
    acq_logf( "WARNING: scam exptime command failed (rc=%d)\n", rc);
    return 1;
  }
  return 0;
}

static int scam_set_avgframes(int avgframes, int dry_run, int verbose)
{
  if (g_hooks_initialized && g_hooks.scam_set_avgframes) {
    return g_hooks.scam_set_avgframes(g_hooks.user, avgframes, dry_run, verbose);
  }

  if (avgframes < 1) avgframes = 1;
  char cmd[256];
  snprintf(cmd, sizeof(cmd), "scam avgframes %d", avgframes);
  if (verbose || dry_run) acq_logf( "SCAM CMD: %s\n", cmd);
  if (dry_run) return 0;
  int rc = system(cmd);
  if (rc != 0) {
    acq_logf( "WARNING: scam avgframes command failed (rc=%d)\n", rc);
    return 1;
  }
  return 0;
}

static const char* adaptive_mode_name(AdaptiveMode m)
{
  if (m == ADAPT_MODE_FAINT) return "faint";
  if (m == ADAPT_MODE_BRIGHT) return "bright";
  return "baseline";
}

static AdaptiveMode adaptive_next_mode(AdaptiveMode current, double metric, const AcqParams* p)
{
  if (!isfinite(metric) || metric <= 0) return current;

  // Hysteresis prevents flapping when source counts jitter near thresholds.
  const double faint_enter = p->adaptive_faint;
  const double faint_exit  = p->adaptive_faint * 1.15;
  const double bright_enter = p->adaptive_bright;
  const double bright_exit  = p->adaptive_bright * 0.85;

  if (current == ADAPT_MODE_FAINT) {
    if (metric >= bright_enter) return ADAPT_MODE_BRIGHT;
    return (metric >= faint_exit) ? ADAPT_MODE_BASELINE : ADAPT_MODE_FAINT;
  }
  if (current == ADAPT_MODE_BRIGHT) {
    if (metric <= faint_enter) return ADAPT_MODE_FAINT;
    return (metric <= bright_exit) ? ADAPT_MODE_BASELINE : ADAPT_MODE_BRIGHT;
  }

  if (metric <= faint_enter) return ADAPT_MODE_FAINT;
  if (metric >= bright_enter) return ADAPT_MODE_BRIGHT;
  return ADAPT_MODE_BASELINE;
}

static int adaptive_bright_avg_target(double metric, double goal_metric)
{
  double ratio = (goal_metric > 0) ? (metric / goal_metric) : 1.0;
  if (!isfinite(ratio)) ratio = 1.0;
  if (ratio > 8.0) return 5;
  if (ratio > 4.0) return 4;
  if (ratio > 2.0) return 3;
  if (ratio > 1.2) return 2;
  return 1;
}

static void adaptive_build_cycle_config(const AcqParams* p, const AdaptiveRuntime* rt,
                                        AdaptiveMode mode, double metric, AdaptiveCycleConfig* cfg)
{
  memset(cfg, 0, sizeof(*cfg));
  cfg->mode = mode;
  cfg->cadence_sec = p->cadence_sec;
  cfg->reject_after_move = p->reject_after_move;
  cfg->prec_arcsec = p->prec_arcsec;
  cfg->goal_arcsec = p->goal_arcsec;

  double cur_exp = (rt && rt->have_last_camera) ? rt->last_exptime_sec : 1.0;
  int cur_avg = (rt && rt->have_last_camera) ? rt->last_avgframes : 1;
  if (!isfinite(cur_exp) || cur_exp <= 0) cur_exp = 1.0;
  if (cur_exp < 0.1) cur_exp = 0.1;
  if (cur_exp > 15.0) cur_exp = 15.0;
  if (cur_avg < 1) cur_avg = 1;
  if (cur_avg > 5) cur_avg = 5;
  cfg->exptime_sec = cur_exp;
  cfg->avgframes = cur_avg;

  if (mode == ADAPT_MODE_BASELINE) return;

  double goal_metric = (mode == ADAPT_MODE_FAINT) ? p->adaptive_faint_goal : p->adaptive_bright_goal;
  if (!isfinite(goal_metric) || goal_metric <= 0) {
    goal_metric = (mode == ADAPT_MODE_FAINT) ? p->adaptive_faint : p->adaptive_bright;
  }

  double factor = 1.0;
  if (isfinite(metric) && metric > 0 && isfinite(goal_metric) && goal_metric > 0) {
    double lo = goal_metric * 0.85;
    double hi = goal_metric * 1.15;
    if (metric < lo || metric > hi) {
      factor = sqrt(goal_metric / metric);
      if (!isfinite(factor) || factor <= 0) factor = 1.0;
      if (factor < 0.5) factor = 0.5;
      if (factor > 2.0) factor = 2.0;
    }
  }

  double target_exp = cur_exp * factor;
  if (target_exp < 0.1) target_exp = 0.1;
  if (target_exp > 15.0) target_exp = 15.0;

  int target_avg = cur_avg;
  if (mode == ADAPT_MODE_BRIGHT) {
    int desired = adaptive_bright_avg_target(metric, goal_metric);
    if (target_avg < desired) target_avg++;
    else if (target_avg > desired) target_avg--;
  } else if (mode == ADAPT_MODE_FAINT) {
    if (target_avg > 1) target_avg--;
    else if (target_avg < 1) target_avg++;
    cfg->reject_after_move = (p->reject_after_move > 1) ? 1 : p->reject_after_move;
    cfg->prec_arcsec = fmax(p->prec_arcsec, 0.20);
    cfg->goal_arcsec = fmax(p->goal_arcsec, 0.30);
  }

  cfg->exptime_sec = target_exp;
  cfg->avgframes = target_avg;
  cfg->apply_camera = (fabs(target_exp - cur_exp) >= 0.02 || target_avg != cur_avg);

  double min_cadence = cfg->exptime_sec * (double)cfg->avgframes + 0.20;
  cfg->cadence_sec = fmax(cfg->cadence_sec, min_cadence);
}

static int adaptive_apply_camera(const AcqParams* p, AdaptiveRuntime* rt, const AdaptiveCycleConfig* cfg)
{
  if (!p->adaptive) return 0;
  if (!cfg->apply_camera) return 0;

  int rc = 0;
  if (scam_set_exptime(cfg->exptime_sec, p->dry_run, p->verbose)) rc = 1;
  if (scam_set_avgframes(cfg->avgframes, p->dry_run, p->verbose)) rc = 1;
  rt->have_last_camera = 1;
  rt->last_exptime_sec = cfg->exptime_sec;
  rt->last_avgframes = cfg->avgframes;
  return rc;
}

static void adaptive_update_metric_and_mode(const AcqParams* p, AdaptiveRuntime* rt, double cycle_metric)
{
  if (!p->adaptive) return;
  if (!isfinite(cycle_metric) || cycle_metric <= 0) return;

  if (!rt->have_metric) {
    rt->metric_ewma = cycle_metric;
    rt->have_metric = 1;
  } else {
    const double alpha = 0.35;
    double lprev = log(fmax(rt->metric_ewma, 1e-6));
    double lnow  = log(fmax(cycle_metric, 1e-6));
    rt->metric_ewma = exp((1.0 - alpha) * lprev + alpha * lnow);
  }

  rt->mode = adaptive_next_mode(rt->mode, rt->metric_ewma, p);
}

static void adaptive_finish_cycle(const AcqParams* p, AdaptiveRuntime* rt,
                                  const double* metric_samp, int n)
{
  if (!p->adaptive || n <= 0) return;

  double mtmp[n];
  for (int i = 0; i < n; i++) mtmp[i] = metric_samp[i];
  double cycle_metric = median_of_doubles(mtmp, n);

  AdaptiveMode prev = rt->mode;
  adaptive_update_metric_and_mode(p, rt, cycle_metric);

  if (p->verbose) {
    if (prev != rt->mode) {
      acq_logf( "Adaptive transition: %s -> %s  cycle_metric=%.1f ewma=%.1f\n",
              adaptive_mode_name(prev), adaptive_mode_name(rt->mode),
              cycle_metric, rt->metric_ewma);
    } else {
      acq_logf( "Adaptive hold: mode=%s cycle_metric=%.1f ewma=%.1f\n",
              adaptive_mode_name(rt->mode), cycle_metric, rt->metric_ewma);
    }
  }
}

static double source_top_fraction_mean(const float* img, long nx, long ny,
                                       const Detection* d, const AcqParams* p,
                                       double frac)
{
  if (!img || !d || !p || nx <= 0 || ny <= 0) return 0.0;
  if (!d->found) return 0.0;
  if (!isfinite(frac) || frac <= 0 || frac > 1) frac = 0.10;

  double cx0 = (p->pixel_origin == 0) ? d->cx : (d->cx - 1.0);
  double cy0 = (p->pixel_origin == 0) ? d->cy : (d->cy - 1.0);

  long xlo = (long)floor(cx0) - p->centroid_halfwin;
  long xhi = (long)floor(cx0) + p->centroid_halfwin;
  long ylo = (long)floor(cy0) - p->centroid_halfwin;
  long yhi = (long)floor(cy0) + p->centroid_halfwin;

  if (xlo < d->sx1) xlo = d->sx1;
  if (xhi > d->sx2) xhi = d->sx2;
  if (ylo < d->sy1) ylo = d->sy1;
  if (yhi > d->sy2) yhi = d->sy2;

  if (xhi < xlo || yhi < ylo) return 0.0;

  long maxn = (xhi - xlo + 1) * (yhi - ylo + 1);
  if (maxn <= 0) return 0.0;

  float* vals = (float*)malloc((size_t)maxn * sizeof(float));
  if (!vals) return 0.0;

  long n = 0;
  for (long y = ylo; y <= yhi; y++) {
    long row0 = y * nx;
    for (long x = xlo; x <= xhi; x++) {
      double v = (double)img[row0 + x] - d->bkg;
      if (v > 0) vals[n++] = (float)v;
    }
  }

  if (n <= 0) {
    free(vals);
    return 0.0;
  }

  qsort(vals, (size_t)n, sizeof(float), cmp_float);

  long k = (long)ceil(frac * (double)n);
  if (k < 1) k = 1;
  if (k > n) k = n;

  long start = n - k;
  double sum = 0.0;
  for (long i = start; i < n; i++) sum += (double)vals[i];

  free(vals);
  return sum / (double)k;
}

// --- Detection + centroiding ---
static Detection detect_star_near_goal(const float* img, long nx, long ny, const AcqParams* p)
{
  Detection d;
  memset(&d, 0, sizeof(d));

  // Stats ROI
  compute_roi_0based(nx, ny, p->pixel_origin,
                     p->bg_roi_mask, p->bg_x1, p->bg_x2, p->bg_y1, p->bg_y2,
                     &d.rx1, &d.rx2, &d.ry1, &d.ry2);

  // Search ROI (defaults to stats ROI)
  if (p->search_roi_mask == 0) {
    d.sx1 = d.rx1; d.sx2 = d.rx2; d.sy1 = d.ry1; d.sy2 = d.ry2;
  } else {
    compute_roi_0based(nx, ny, p->pixel_origin,
                       p->search_roi_mask, p->search_x1, p->search_x2, p->search_y1, p->search_y2,
                       &d.sx1, &d.sx2, &d.sy1, &d.sy2);
  }

  // Background and sigma from stats ROI
  bg_sigma_sextractor_like(img, nx, ny, d.rx1, d.rx2, d.ry1, d.ry2, &d.bkg, &d.sigma);
  if (!isfinite(d.sigma) || d.sigma <= 0) {
    d.found = 0;
    return d;
  }

  // Goal (0-based)
  double goal_x0 = (p->pixel_origin == 0) ? p->goal_x : (p->goal_x - 1.0);
  double goal_y0 = (p->pixel_origin == 0) ? p->goal_y : (p->goal_y - 1.0);

  // Build detection patch (search ROI) and background-subtract
  int w = (int)(d.sx2 - d.sx1 + 1);
  int h = (int)(d.sy2 - d.sy1 + 1);
  if (w <= 3 || h <= 3) {
    d.found = 0;
    return d;
  }

  float* patch = (float*)malloc((size_t)w*(size_t)h*sizeof(float));
  float* tmp   = (float*)malloc((size_t)w*(size_t)h*sizeof(float));
  float* filt  = (float*)malloc((size_t)w*(size_t)h*sizeof(float));
  if (!patch || !tmp || !filt) die("malloc patch/tmp/filt failed");

  for (int yy = 0; yy < h; yy++) {
    long y = d.sy1 + yy;
    long row0 = y * nx;
    float* prow = patch + (size_t)yy*(size_t)w;
    for (int xx = 0; xx < w; xx++) {
      long x = d.sx1 + xx;
      double v = (double)img[row0 + x] - d.bkg;
      // keep negatives; filter uses them too
      prow[xx] = (float)v;
    }
  }

  // Filter for detection
  int kr = 0;
  double* k = make_gaussian_kernel(p->filt_sigma_pix, &kr);
  convolve_separable(patch, tmp, filt, w, h, k, kr);
  double sumsq1d = kernel_sum_sq(k, kr);
  free(k);

  // Detection threshold in filtered image
  // For separable 2D kernel, sigma_filt = sigma_raw * sqrt(sum(K^2)) = sigma_raw * sumsq1d
  double sigma_filt = d.sigma * sumsq1d;
  if (!isfinite(sigma_filt) || sigma_filt <= 0) sigma_filt = d.sigma;
  double thr_filt = p->snr_thresh * sigma_filt;

  // Raw threshold for adjacency check
  double thr_raw = p->snr_thresh * d.sigma;

  // Search best local maximum
  double best_val = -1e300;
  int best_x = -1, best_y = -1;
  double best_snr_raw = 0;

  for (int yy = 1; yy < h-1; yy++) {
    for (int xx = 1; xx < w-1; xx++) {
      float v = filt[(size_t)yy*(size_t)w + (size_t)xx];
      if ((double)v < thr_filt) continue;

      // local max in filtered
      if (v < filt[(size_t)yy*(size_t)w + (size_t)(xx-1)]) continue;
      if (v < filt[(size_t)yy*(size_t)w + (size_t)(xx+1)]) continue;
      if (v < filt[(size_t)(yy-1)*(size_t)w + (size_t)xx]) continue;
      if (v < filt[(size_t)(yy+1)*(size_t)w + (size_t)xx]) continue;

      long x0 = d.sx1 + xx;
      long y0 = d.sy1 + yy;

      // within max distance from goal
      double dxg = (double)x0 - goal_x0;
      double dyg = (double)y0 - goal_y0;
      if (hypot(dxg, dyg) > p->max_dist_pix) continue;

      // adjacency check in raw residual image (patch)
      int nadj = 0;
      for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
          if (dx == 0 && dy == 0) continue;
          float rv = patch[(size_t)(yy+dy)*(size_t)w + (size_t)(xx+dx)];
          if ((double)rv > thr_raw) nadj++;
        }
      }
      if (nadj < p->min_adjacent) continue;

      // best peak by raw peak value at that location (not filtered)
      float rawv = patch[(size_t)yy*(size_t)w + (size_t)xx];
      double snr_here = (d.sigma > 0) ? ((double)rawv / d.sigma) : 0;
      if ((double)rawv > best_val) {
        best_val = (double)rawv;
        best_x = (int)x0;
        best_y = (int)y0;
        best_snr_raw = snr_here;
      }
    }
  }

  if (best_x < 0) {
    free(patch); free(tmp); free(filt);
    d.found = 0;
    return d;
  }

  // Iterative Gaussian-windowed centroid on raw residuals (img - bkg)
  double cx = (double)best_x;
  double cy = (double)best_y;

  int hw = p->centroid_halfwin;
  double s2 = p->centroid_sigma_pix * p->centroid_sigma_pix;
  if (s2 <= 0.1) s2 = 0.1;

  double sumI = 0, sumX = 0, sumY = 0;
  for (int it = 0; it < p->centroid_maxiter; it++) {
    long xlo = (long)floor(cx) - hw;
    long xhi = (long)floor(cx) + hw;
    long ylo = (long)floor(cy) - hw;
    long yhi = (long)floor(cy) + hw;

    // clamp to search ROI
    if (xlo < d.sx1) xlo = d.sx1;
    if (xhi > d.sx2) xhi = d.sx2;
    if (ylo < d.sy1) ylo = d.sy1;
    if (yhi > d.sy2) yhi = d.sy2;

    sumI = sumX = sumY = 0.0;
    for (long y = ylo; y <= yhi; y++) {
      long row0 = y * nx;
      for (long x = xlo; x <= xhi; x++) {
        double I = (double)img[row0 + x] - d.bkg;
        if (I <= 0) continue;
        double dx = (double)x - cx;
        double dy = (double)y - cy;
        double wgt = exp(-0.5*(dx*dx + dy*dy)/s2);
        double ww = wgt * I;
        sumI += ww;
        sumX += ww * (double)x;
        sumY += ww * (double)y;
      }
    }

    if (sumI <= 0) break;

    double nxp = sumX / sumI;
    double nyp = sumY / sumI;

    double shift = hypot(nxp - cx, nyp - cy);
    cx = nxp;
    cy = nyp;

    if (shift < p->centroid_eps_pix) break;
  }

  if (sumI <= 0 || !isfinite(cx) || !isfinite(cy)) {
    free(patch); free(tmp); free(filt);
    d.found = 0;
    return d;
  }

  // aperture-like SNR within centroid window
  long xlo = (long)floor(cx) - hw;
  long xhi = (long)floor(cx) + hw;
  long ylo = (long)floor(cy) - hw;
  long yhi = (long)floor(cy) + hw;
  if (xlo < d.sx1) xlo = d.sx1;
  if (xhi > d.sx2) xhi = d.sx2;
  if (ylo < d.sy1) ylo = d.sy1;
  if (yhi > d.sy2) yhi = d.sy2;

  double sig_sum = 0.0;
  long npix = 0;
  for (long y = ylo; y <= yhi; y++) {
    long row0 = y * nx;
    for (long x = xlo; x <= xhi; x++) {
      double I = (double)img[row0 + x] - d.bkg;
      if (I <= 0) continue;
      sig_sum += I;
      npix++;
    }
  }
  double noise = d.sigma * sqrt((double)(npix > 1 ? npix : 1));
  double snr_ap = (noise > 0) ? (sig_sum / noise) : 0.0;

  d.found = 1;
  d.peak_val = best_val;
  d.peak_snr_raw = best_snr_raw;
  d.snr_ap = snr_ap;

  d.peak_x = (p->pixel_origin == 0) ? (double)best_x : (double)best_x + 1.0;
  d.peak_y = (p->pixel_origin == 0) ? (double)best_y : (double)best_y + 1.0;
  d.cx     = (p->pixel_origin == 0) ? cx          : cx + 1.0;
  d.cy     = (p->pixel_origin == 0) ? cy          : cy + 1.0;
  d.src_top10_mean = source_top_fraction_mean(img, nx, ny, &d, p, 0.10);

  free(patch); free(tmp); free(filt);
  return d;
}

// Compute a full FrameResult from an already-loaded FITS image and header.
static FrameResult solve_frame(const float* img, long nx, long ny, const char* header, int nkeys, const AcqParams* p)
{
  FrameResult r;
  memset(&r, 0, sizeof(r));

  r.det = detect_star_near_goal(img, nx, ny, p);
  if (!r.det.found) {
    r.ok = 0;
    return r;
  }

  r.dx_pix = r.det.cx - p->goal_x;
  r.dy_pix = r.det.cy - p->goal_y;

  // WCS
  struct wcsprm* wcs = NULL;
  int nwcs = 0;
  int wcs_stat = init_wcs_from_header(header, nkeys, &wcs, &nwcs);
  if (wcs_stat != 0) {
    r.wcs_ok = 0;
    r.ok = 0;
    return r;
  }

  // Convert pixels to FITS 1-based for wcsp2s
  double goal_x1 = (p->pixel_origin == 0) ? (p->goal_x + 1.0) : p->goal_x;
  double goal_y1 = (p->pixel_origin == 0) ? (p->goal_y + 1.0) : p->goal_y;
  double star_x1 = (p->pixel_origin == 0) ? (r.det.cx + 1.0)   : r.det.cx;
  double star_y1 = (p->pixel_origin == 0) ? (r.det.cy + 1.0)   : r.det.cy;

  if (pix2world_wcs(&wcs[0], goal_x1, goal_y1, &r.ra_goal_deg, &r.dec_goal_deg) ||
      pix2world_wcs(&wcs[0], star_x1, star_y1, &r.ra_star_deg, &r.dec_star_deg)) {
    r.wcs_ok = 0;
    r.ok = 0;
    wcsvfree(&nwcs, &wcs);
    return r;
  }

  wcsvfree(&nwcs, &wcs);

  r.wcs_ok = 1;

  // Commanded offsets: (star - goal)
  double dra_deg  = wrap_dra_deg(r.ra_star_deg - r.ra_goal_deg);
  double ddec_deg = (r.dec_star_deg - r.dec_goal_deg);

  double cosdec = cos(r.dec_goal_deg * M_PI / 180.0);
  double dra_arcsec = dra_deg * 3600.0;
  if (p->dra_use_cosdec) dra_arcsec *= cosdec;
  double ddec_arcsec = ddec_deg * 3600.0;

  dra_arcsec  *= (double)p->tcs_sign;
  ddec_arcsec *= (double)p->tcs_sign;

  r.dra_cmd_arcsec = dra_arcsec;
  r.ddec_cmd_arcsec = ddec_arcsec;
  r.r_cmd_arcsec = hypot(dra_arcsec, ddec_arcsec);

  // Final accept criteria for "good sample"
  //  - windowed SNR must be >= threshold (it is typically more stable than raw peak SNR)
  if (r.det.snr_ap < p->snr_thresh) {
    r.ok = 0;
    return r;
  }

  r.ok = 1;
  return r;
}

// --- Frame acquisition / gating ---
static int stat_file(const char* path, struct stat* st)
{
  if (stat(path, st) != 0) return 1;
  if (st->st_size <= 0) return 2;
  return 0;
}

static int wait_for_stream_update(const char* path, FrameState* fs, double settle_check_sec, double cadence_sec, int verbose)
{
  // Wait until file mtime/size changes from last accepted, then is stable across settle_check_sec.
  // Also enforce a minimum time between accepted frames (cadence_sec).
  struct stat st1, st2;
  for (;;) {
    if (stop_requested()) return 2;

    if (stat_file(path, &st1) != 0) {
      sleep_seconds(0.1);
      continue;
    }

    // Newness check
    if (fs->mtime == st1.st_mtime && fs->size == st1.st_size) {
      sleep_seconds(0.1);
      continue;
    }

    // Stability check (not writing)
    sleep_seconds(settle_check_sec);
    if (stat_file(path, &st2) != 0) {
      sleep_seconds(0.1);
      continue;
    }
    if (st2.st_mtime != st1.st_mtime || st2.st_size != st1.st_size) {
      // still changing
      sleep_seconds(0.05);
      continue;
    }

    // Cadence check
    if (cadence_sec > 0) {
      double tnow = now_monotonic_sec();
      double tlast = (double)fs->t_accept.tv_sec + 1e-9*(double)fs->t_accept.tv_nsec;
      if (tlast > 0 && (tnow - tlast) < cadence_sec) {
        sleep_seconds(0.05);
        continue;
      }
    }

    // accept this as a candidate to read
    if (verbose) {
      acq_logf( "Frame candidate: mtime=%ld size=%ld\n", (long)st2.st_mtime, (long)st2.st_size);
    }
    fs->mtime = st2.st_mtime;
    fs->size = st2.st_size;
    clock_gettime(CLOCK_MONOTONIC, &fs->t_accept);
    return 0;
  }
}

// Read next frame into memory (img/header) according to frame_mode.
// Returns 0 on success.
static int acquire_next_frame(const AcqParams* p, FrameState* fs,
                              double cadence_sec,
                              float** img_out, long* nx_out, long* ny_out,
                              char** header_out, int* nkeys_out,
                              double* exptime_sec_out)
{
  const char* path = NULL;

  if (p->frame_mode == FRAME_FRAMEGRAB) {
    // Acquire synchronously via scam
    if (scam_framegrab_one(p->framegrab_out, p->verbose)) return 2;
    // Give filesystem a tiny moment; and then read when stable
    // (framegrab should generally be complete when system() returns, but be safe)
    (void)wait_for_stream_update(p->framegrab_out, fs, 0.05, 0.0, 0);
    path = p->framegrab_out;
  } else {
    // Stream mode: wait until file updates
    if (wait_for_stream_update(p->input, fs, 0.05, cadence_sec, 0)) return 3;
    path = p->input;
  }

  int used_hdu = 0;
  char used_extname[64] = {0};
  double exptime_sec = 1.0;
  int st = read_fits_image_and_header(path, p, img_out, nx_out, ny_out, header_out, nkeys_out,
                                      &used_hdu, used_extname, sizeof(used_extname),
                                      &exptime_sec);
  if (st) {
    acq_logf( "ERROR: CFITSIO read failed for %s (status=%d)\n", path, st);
    return 4;
  }
  if (exptime_sec_out) *exptime_sec_out = exptime_sec;
  return 0;
}

// --- CLI ---
static void usage(const char* argv0)
{
  acq_logf(
    "Usage: %s --input PATH --goal-x X --goal-y Y [options]\n"
    "\n"
    "Core options:\n"
    "  --input PATH                 Streamed FITS file path (default /tmp/slicecam.fits)\n"
    "  --goal-x X --goal-y Y        Goal pixel coordinates\n"
    "  --pixel-origin 0|1           Coordinate origin for goal/ROI (default 0)\n"
    "\n"
    "Frame acquisition:\n"
    "  --frame-mode stream|framegrab    (default stream)\n"
    "  --framegrab-out PATH             Output FITS path for framegrab (default /tmp/ngps_acq.fits)\n"
    "\n"
    "Detection / centroiding:\n"
    "  --snr S                     SNR threshold (sigma) (default 8)\n"
    "  --max-dist PIX               Search radius around goal (default 200)\n"
    "  --min-adj N                  Min adjacent raw pixels above threshold (default 4)\n"
    "  --filt-sigma PIX             Detection filter sigma (default 1.2)\n"
    "  --centroid-hw N              Centroid half-window (default 6)\n"
    "  --centroid-sigma PIX         Centroid window sigma (default 2.0)\n"
    "\n"
    "FITS selection:\n"
    "  --extname NAME               Prefer this EXTNAME (default L). Use 'none' to disable.\n"
    "  --extnum N                   Fallback HDU index (0=primary,1=first ext...) (default 1)\n"
    "\n"
    "ROIs (inclusive bounds; same origin as goal):\n"
    "  --bg-x1 N --bg-x2 N --bg-y1 N --bg-y2 N        Background/stats ROI\n"
    "  --search-x1 N --search-x2 N --search-y1 N --search-y2 N  Search ROI (defaults to bg ROI)\n"
    "\n"
    "Closed-loop acquisition:\n"
    "  --loop 0|1                   Enable closed-loop mode (default 0)\n"
    "  --cadence-sec S              Minimum seconds between accepted frames (stream) (default 0.0)\n"
    "  --max-samples N              Samples to collect per move (default 10)\n"
    "  --min-samples N              Minimum before evaluating precision (default 3)\n"
    "  --prec-arcsec A              Required robust scatter per axis (default 0.1)\n"
    "  --goal-arcsec A              Converge threshold on |offset| (default 0.1)\n"
    "  --max-cycles N               Move cycles (default 20)\n"
    "  --gain G                     Gain applied to move (default 0.7)\n"
    "  --adaptive 0|1               Adaptive extremes-only mode (default 0)\n"
    "  --adaptive-faint X           Start faint adaptation when metric <= X (default 500)\n"
    "  --adaptive-faint-goal X      Faint-mode target metric (default 500)\n"
    "  --adaptive-bright Y          Start bright adaptation when metric >= Y (default 10000)\n"
    "  --adaptive-bright-goal Y     Bright-mode target metric (default 10000)\n"
    "                               Metric is top-10%% mean source counts (background-subtracted).\n"
    "                               In-range [faint,bright] keeps baseline behavior.\n"
    "\n"
    "Robustness / safety:\n"
    "  --reject-identical 0|1       Reject identical frames (default 1)\n"
    "  --reject-after-move N        Reject N new frames after move (default 2)\n"
    "  --settle-sec S               Sleep after move (default 0.0)\n"
    "  --max-move-arcsec A          Do not issue moves larger than this (default 10)\n"
    "  --continue-on-fail 0|1       If 0: exit on failure; if 1: keep trying (default 0)\n"
    "\n"
    "TCS conventions:\n"
    "  --tcs-set-units 0|1          Set native dra/ddec units to arcsec once (default 1)\n"
    "  --use-putonslit 0|1          Use scam daemon putonslit for moves (default 0)\n"
    "  --dra-use-cosdec 0|1         dra = dRA*cos(dec) (default 1)\n"
    "  --tcs-sign +1|-1             Multiply commanded offsets by sign (default +1)\n"
    "\n"
    "Guiding wait (after putonslit):\n"
    "  --wait-guiding 0|1           Wait for ACAM guiding state after putonslit (default 1)\n"
    "  --guiding-poll-sec S         Poll period for 'acam acquire' (default 1.0)\n"
    "  --guiding-timeout-sec S      Timeout waiting for guiding; 0=wait forever (default 120)\n"
    "\n"
    "Debug:\n"
    "  --debug 0|1                  Write overlay PPM (default 0)\n"
    "  --debug-out PATH             PPM path (default ./ngps_acq_debug.ppm)\n"
    "\n"
    "General:\n"
    "  --dry-run 0|1                Do not call TCS (default 0)\n"
    "  --verbose 0|1                Verbose logs (default 1)\n",
    argv0
  );
}

static void set_defaults(AcqParams* p)
{
  memset(p, 0, sizeof(*p));
  snprintf(p->input, sizeof(p->input), "/tmp/slicecam.fits");
  p->frame_mode = FRAME_STREAM;
  snprintf(p->framegrab_out, sizeof(p->framegrab_out), "/tmp/ngps_acq.fits");
  p->framegrab_use_tmp = 0;

  p->goal_x = 0;
  p->goal_y = 0;
  p->pixel_origin = 0;

  p->max_dist_pix = 200.0;
  p->snr_thresh = 8.0;
  p->min_adjacent = 4;

  p->filt_sigma_pix = 1.2;

  p->centroid_halfwin = 6;
  p->centroid_sigma_pix = 2.0;
  p->centroid_maxiter = 12;
  p->centroid_eps_pix = 0.01;

  p->extnum = 1;
  snprintf(p->extname, sizeof(p->extname), "L");

  p->bg_roi_mask = 0;
  p->search_roi_mask = 0;

  p->loop = 0;
  p->cadence_sec = 0.0;
  p->max_samples = 10;
  p->min_samples = 3;
  p->prec_arcsec = 0.1;
  p->goal_arcsec = 0.1;
  p->max_cycles = 20;
  p->gain = 0.7;
  p->adaptive = 0;
  p->adaptive_faint = 500.0;
  p->adaptive_faint_goal = 500.0;
  p->adaptive_bright = 10000.0;
  p->adaptive_bright_goal = 10000.0;

  p->reject_identical = 1;
  p->reject_after_move = 2;
  p->settle_sec = 0.0;
  p->max_move_arcsec = 10.0;
  p->continue_on_fail = 0;

  p->dra_use_cosdec = 1;
  p->tcs_sign = +1;

  p->tcs_set_units = 1;

  p->use_putonslit = 0;
  p->wait_guiding = 1;
  p->guiding_poll_sec = 1.0;
  p->guiding_timeout_sec = 120.0;

  p->debug = 0;
  snprintf(p->debug_out, sizeof(p->debug_out), "./ngps_acq_debug.ppm");

  p->dry_run = 0;
  p->verbose = 1;
}

static int parse_args(int argc, char** argv, AcqParams* p)
{
  for (int i = 1; i < argc; i++) {
    const char* a = argv[i];
    if (!strcmp(a, "--input") && i+1 < argc) {
      snprintf(p->input, sizeof(p->input), "%s", argv[++i]);
    } else if (!strcmp(a, "--goal-x") && i+1 < argc) {
      p->goal_x = atof(argv[++i]);
    } else if (!strcmp(a, "--goal-y") && i+1 < argc) {
      p->goal_y = atof(argv[++i]);
    } else if (!strcmp(a, "--pixel-origin") && i+1 < argc) {
      p->pixel_origin = atoi(argv[++i]);

    } else if (!strcmp(a, "--frame-mode") && i+1 < argc) {
      const char* m = argv[++i];
      if (!strcasecmp(m, "stream")) p->frame_mode = FRAME_STREAM;
      else if (!strcasecmp(m, "framegrab")) p->frame_mode = FRAME_FRAMEGRAB;
      else { acq_logf( "Invalid --frame-mode: %s\n", m); return -1; }
    } else if (!strcmp(a, "--framegrab-out") && i+1 < argc) {
      snprintf(p->framegrab_out, sizeof(p->framegrab_out), "%s", argv[++i]);

    } else if (!strcmp(a, "--max-dist") && i+1 < argc) {
      p->max_dist_pix = atof(argv[++i]);
    } else if (!strcmp(a, "--snr") && i+1 < argc) {
      p->snr_thresh = atof(argv[++i]);
    } else if (!strcmp(a, "--min-adj") && i+1 < argc) {
      p->min_adjacent = atoi(argv[++i]);
    } else if (!strcmp(a, "--filt-sigma") && i+1 < argc) {
      p->filt_sigma_pix = atof(argv[++i]);
    } else if (!strcmp(a, "--centroid-hw") && i+1 < argc) {
      p->centroid_halfwin = atoi(argv[++i]);
    } else if (!strcmp(a, "--centroid-sigma") && i+1 < argc) {
      p->centroid_sigma_pix = atof(argv[++i]);

    } else if (!strcmp(a, "--extnum") && i+1 < argc) {
      p->extnum = atoi(argv[++i]);
    } else if (!strcmp(a, "--extname") && i+1 < argc) {
      snprintf(p->extname, sizeof(p->extname), "%s", argv[++i]);
      if (!strcasecmp(p->extname, "none")) p->extname[0] = '\0';

    } else if (!strcmp(a, "--bg-x1") && i+1 < argc) {
      p->bg_x1 = atol(argv[++i]); p->bg_roi_mask |= ROI_X1_SET;
    } else if (!strcmp(a, "--bg-x2") && i+1 < argc) {
      p->bg_x2 = atol(argv[++i]); p->bg_roi_mask |= ROI_X2_SET;
    } else if (!strcmp(a, "--bg-y1") && i+1 < argc) {
      p->bg_y1 = atol(argv[++i]); p->bg_roi_mask |= ROI_Y1_SET;
    } else if (!strcmp(a, "--bg-y2") && i+1 < argc) {
      p->bg_y2 = atol(argv[++i]); p->bg_roi_mask |= ROI_Y2_SET;

    } else if (!strcmp(a, "--search-x1") && i+1 < argc) {
      p->search_x1 = atol(argv[++i]); p->search_roi_mask |= ROI_X1_SET;
    } else if (!strcmp(a, "--search-x2") && i+1 < argc) {
      p->search_x2 = atol(argv[++i]); p->search_roi_mask |= ROI_X2_SET;
    } else if (!strcmp(a, "--search-y1") && i+1 < argc) {
      p->search_y1 = atol(argv[++i]); p->search_roi_mask |= ROI_Y1_SET;
    } else if (!strcmp(a, "--search-y2") && i+1 < argc) {
      p->search_y2 = atol(argv[++i]); p->search_roi_mask |= ROI_Y2_SET;

    } else if (!strcmp(a, "--loop") && i+1 < argc) {
      p->loop = atoi(argv[++i]);
    } else if (!strcmp(a, "--cadence-sec") && i+1 < argc) {
      p->cadence_sec = atof(argv[++i]);
    } else if (!strcmp(a, "--max-samples") && i+1 < argc) {
      p->max_samples = atoi(argv[++i]);
    } else if (!strcmp(a, "--min-samples") && i+1 < argc) {
      p->min_samples = atoi(argv[++i]);
    } else if (!strcmp(a, "--prec-arcsec") && i+1 < argc) {
      p->prec_arcsec = atof(argv[++i]);
    } else if (!strcmp(a, "--goal-arcsec") && i+1 < argc) {
      p->goal_arcsec = atof(argv[++i]);
    } else if (!strcmp(a, "--max-cycles") && i+1 < argc) {
      p->max_cycles = atoi(argv[++i]);
    } else if (!strcmp(a, "--gain") && i+1 < argc) {
      p->gain = atof(argv[++i]);
    } else if (!strcmp(a, "--adaptive") && i+1 < argc) {
      p->adaptive = atoi(argv[++i]);
    } else if (!strcmp(a, "--adaptive-faint") && i+1 < argc) {
      p->adaptive_faint = atof(argv[++i]);
    } else if (!strcmp(a, "--adaptive-faint-goal") && i+1 < argc) {
      p->adaptive_faint_goal = atof(argv[++i]);
    } else if (!strcmp(a, "--adaptive-bright") && i+1 < argc) {
      p->adaptive_bright = atof(argv[++i]);
    } else if (!strcmp(a, "--adaptive-bright-goal") && i+1 < argc) {
      p->adaptive_bright_goal = atof(argv[++i]);

    } else if (!strcmp(a, "--reject-identical") && i+1 < argc) {
      p->reject_identical = atoi(argv[++i]);
    } else if (!strcmp(a, "--reject-after-move") && i+1 < argc) {
      p->reject_after_move = atoi(argv[++i]);
    } else if (!strcmp(a, "--settle-sec") && i+1 < argc) {
      p->settle_sec = atof(argv[++i]);
    } else if (!strcmp(a, "--max-move-arcsec") && i+1 < argc) {
      p->max_move_arcsec = atof(argv[++i]);
    } else if (!strcmp(a, "--continue-on-fail") && i+1 < argc) {
      p->continue_on_fail = atoi(argv[++i]);

    } else if (!strcmp(a, "--dra-use-cosdec") && i+1 < argc) {
      p->dra_use_cosdec = atoi(argv[++i]);
    } else if (!strcmp(a, "--tcs-sign") && i+1 < argc) {
      p->tcs_sign = atoi(argv[++i]);
      if (!(p->tcs_sign == 1 || p->tcs_sign == -1)) { acq_logf( "--tcs-sign must be +1 or -1\n"); return -1; }
    } else if (!strcmp(a, "--tcs-set-units") && i+1 < argc) {
      p->tcs_set_units = atoi(argv[++i]);

    } else if (!strcmp(a, "--use-putonslit") && i+1 < argc) {
      p->use_putonslit = atoi(argv[++i]);

    } else if (!strcmp(a, "--wait-guiding") && i+1 < argc) {
      p->wait_guiding = atoi(argv[++i]);
    } else if (!strcmp(a, "--guiding-poll-sec") && i+1 < argc) {
      p->guiding_poll_sec = atof(argv[++i]);
    } else if (!strcmp(a, "--guiding-timeout-sec") && i+1 < argc) {
      p->guiding_timeout_sec = atof(argv[++i]);

    } else if (!strcmp(a, "--debug") && i+1 < argc) {
      p->debug = atoi(argv[++i]);
    } else if (!strcmp(a, "--debug-out") && i+1 < argc) {
      snprintf(p->debug_out, sizeof(p->debug_out), "%s", argv[++i]);

    } else if (!strcmp(a, "--dry-run") && i+1 < argc) {
      p->dry_run = atoi(argv[++i]);
    } else if (!strcmp(a, "--verbose") && i+1 < argc) {
      p->verbose = atoi(argv[++i]);

    } else if (!strcmp(a, "--help") || !strcmp(a, "-h")) {
      return 0;
    } else {
      acq_logf( "Unknown/invalid arg: %s\n", a);
      return -1;
    }
  }

  if (!(p->pixel_origin == 0 || p->pixel_origin == 1)) {
    acq_logf( "--pixel-origin must be 0 or 1\n");
    return -1;
  }
  if (!isfinite(p->goal_x) || !isfinite(p->goal_y)) {
    acq_logf( "You must provide --goal-x and --goal-y\n");
    return -1;
  }
  if (p->max_samples < 1) p->max_samples = 1;
  if (p->min_samples < 1) p->min_samples = 1;
  if (p->min_samples > p->max_samples) p->min_samples = p->max_samples;
  if (p->gain < 0) p->gain = 0;
  if (p->gain > 1.5) p->gain = 1.5;
  if (!(p->adaptive == 0 || p->adaptive == 1)) {
    acq_logf( "--adaptive must be 0 or 1\n");
    return -1;
  }
  if (!isfinite(p->adaptive_faint) || p->adaptive_faint <= 0) {
    acq_logf( "--adaptive-faint must be > 0\n");
    return -1;
  }
  if (!isfinite(p->adaptive_bright) || p->adaptive_bright <= p->adaptive_faint) {
    acq_logf( "--adaptive-bright must be > --adaptive-faint\n");
    return -1;
  }
  if (!isfinite(p->adaptive_faint_goal) || p->adaptive_faint_goal <= 0) {
    acq_logf( "--adaptive-faint-goal must be > 0\n");
    return -1;
  }
  if (!isfinite(p->adaptive_bright_goal) || p->adaptive_bright_goal <= 0) {
    acq_logf( "--adaptive-bright-goal must be > 0\n");
    return -1;
  }
  if (p->max_move_arcsec <= 0) p->max_move_arcsec = 10.0;
  if (p->reject_after_move < 0) p->reject_after_move = 0;
  if (p->cadence_sec < 0) p->cadence_sec = 0;
  if (p->filt_sigma_pix <= 0) p->filt_sigma_pix = 1.2;
  if (p->centroid_sigma_pix <= 0) p->centroid_sigma_pix = 2.0;
  if (p->centroid_halfwin < 2) p->centroid_halfwin = 2;
  if (p->guiding_poll_sec <= 0) p->guiding_poll_sec = 1.0;
  if (p->guiding_timeout_sec < 0) p->guiding_timeout_sec = 0;

  if (p->use_putonslit) {
    // putonslit computes PT internally; no need to set native dra/ddec units.
    p->tcs_set_units = 0;
  }

  return 1;
}

// --- One-shot processing ---
static int process_once(const AcqParams* p, FrameState* fs)
{
  float* img = NULL;
  long nx=0, ny=0;
  char* header = NULL;
  int nkeys = 0;
  double exptime_sec = 1.0;

  int rc = acquire_next_frame(p, fs, p->cadence_sec, &img, &nx, &ny, &header, &nkeys, &exptime_sec);
  if (rc) {
    if (img) free(img);
    if (header) free(header);
    return 4;
  }

  (void)exptime_sec;

  // signature for rejecting identical frames
  if (p->reject_identical) {
    long sx1,sx2,sy1,sy2;
    compute_roi_0based(nx, ny, p->pixel_origin,
                       p->bg_roi_mask, p->bg_x1, p->bg_x2, p->bg_y1, p->bg_y2,
                       &sx1,&sx2,&sy1,&sy2);
    uint64_t sig = image_signature_subsample(img, nx, ny, sx1,sx2,sy1,sy2);
    if (fs->have_sig && sig == fs->sig) {
      if (p->verbose) acq_logf( "Duplicate frame signature (reject).\n");
      free(img);
      if (header) free(header);
      return 2;
    }
    fs->sig = sig;
    fs->have_sig = 1;
  }

  FrameResult fr = solve_frame(img, nx, ny, header, nkeys, p);

  if (p->debug) {
    (void)write_debug_ppm(p->debug_out, img, nx,
                          fr.det.rx1, fr.det.rx2, fr.det.ry1, fr.det.ry2,
                          fr.det.bkg, fr.det.sigma, p->snr_thresh,
                          &fr.det, p);
  }

  free(img);
  if (header) free(header);

  if (!fr.ok) {
    if (p->verbose) acq_logf( "No valid solution (star/WCS/quality).\n");
    return 2;
  }

  if (p->verbose) {
    acq_logf( "Centroid=(%.3f,%.3f) dx=%.3f dy=%.3f pix  SNR_ap=%.2f\n",
            fr.det.cx, fr.det.cy, fr.dx_pix, fr.dy_pix, fr.det.snr_ap);
    if (p->adaptive) {
      acq_logf( "Source metric (top10%% mean count): %.1f\n", fr.det.src_top10_mean);
    }
    acq_logf( "Command offsets (arcsec): dra=%.3f ddec=%.3f  r=%.3f\n",
            fr.dra_cmd_arcsec, fr.ddec_cmd_arcsec, fr.r_cmd_arcsec);
  }

  // Machine-readable line
  printf("NGPS_ACQ_RESULT ok=1 cx=%.6f cy=%.6f dra_arcsec=%.6f ddec_arcsec=%.6f r_arcsec=%.6f snr_ap=%.3f\n",
         fr.det.cx, fr.det.cy, fr.dra_cmd_arcsec, fr.ddec_cmd_arcsec, fr.r_cmd_arcsec, fr.det.snr_ap);

  // Safety: do not move without stats in loop mode; in one-shot we do a single move only if --loop=0
  if (!p->loop) {
    if (fr.r_cmd_arcsec > p->max_move_arcsec) {
      acq_logf( "REFUSE MOVE: |offset|=%.3f"" exceeds --max-move-arcsec=%.3f\n", fr.r_cmd_arcsec, p->max_move_arcsec);
      return 3;
    }

    double dra = p->gain * fr.dra_cmd_arcsec;
    double ddec = p->gain * fr.ddec_cmd_arcsec;

    if (p->use_putonslit) {
      if (!fr.wcs_ok) {
        acq_logf( "REFUSE MOVE: WCS not available for putonslit\n");
        return 3;
      }
      double cross_ra=0.0, cross_dec=0.0;
      slit_cross_from_offsets(p, fr.ra_goal_deg, fr.dec_goal_deg, dra, ddec, &cross_ra, &cross_dec);
      (void)scam_putonslit_deg(fr.ra_goal_deg, fr.dec_goal_deg, cross_ra, cross_dec, p->dry_run, p->verbose);
      (void)wait_for_guiding(p);
    } else {
      (void)tcs_move_arcsec(dra, ddec, p->dry_run, p->verbose);
    }
  }

  return 0;
}

// --- Closed-loop acquisition ---
static int run_loop(const AcqParams* p)
{
  FrameState fs;
  memset(&fs, 0, sizeof(fs));
  fs.t_accept.tv_sec = 0;
  fs.t_accept.tv_nsec = 0;

  if (!p->use_putonslit && p->tcs_set_units) (void)tcs_set_native_units(p->dry_run, p->verbose);

  int skip_after_move = 0;
  AdaptiveRuntime adaptive_rt;
  memset(&adaptive_rt, 0, sizeof(adaptive_rt));
  adaptive_rt.mode = ADAPT_MODE_BASELINE;
  double initial_metric_exptime_sec = 1.0;
  int have_initial_metric_exptime = 0;
  int initial_metric_scaled_once = 0;

  for (int cycle = 1; cycle <= p->max_cycles && !stop_requested(); cycle++) {
    if (p->verbose) acq_logf( "\n=== Cycle %d/%d ===\n", cycle, p->max_cycles);

    AdaptiveCycleConfig cycle_cfg;
    adaptive_build_cycle_config(p, &adaptive_rt, ADAPT_MODE_BASELINE, 0.0, &cycle_cfg);
    if (p->adaptive) {
      double metric_cfg = adaptive_rt.have_metric ? adaptive_rt.metric_ewma : 0.0;
      if ( !initial_metric_scaled_once &&
           adaptive_rt.have_metric &&
           adaptive_rt.mode != ADAPT_MODE_BASELINE &&
           have_initial_metric_exptime &&
           initial_metric_exptime_sec > 0.0 ) {
        metric_cfg /= initial_metric_exptime_sec;
        initial_metric_scaled_once = 1;
        if ( p->verbose ) {
          acq_logf( "Adaptive one-shot metric scaling: metric/EXPTIME using initial EXPTIME=%.3fs\n",
                    initial_metric_exptime_sec );
        }
      }
      adaptive_build_cycle_config(p, &adaptive_rt, adaptive_rt.mode, metric_cfg, &cycle_cfg);
      (void)adaptive_apply_camera(p, &adaptive_rt, &cycle_cfg);
      if (p->verbose) {
        if (adaptive_rt.have_metric) {
          acq_logf(
                  "Adaptive mode=%s ewma=%.1f exp=%.2fs avg=%d cadence=%.2fs reject_after_move=%d prec=%.3f\" goal=%.3f\"\n",
                  adaptive_mode_name(adaptive_rt.mode), adaptive_rt.metric_ewma,
                  cycle_cfg.exptime_sec, cycle_cfg.avgframes,
                  cycle_cfg.cadence_sec, cycle_cfg.reject_after_move,
                  cycle_cfg.prec_arcsec, cycle_cfg.goal_arcsec);
        } else {
          acq_logf(
                  "Adaptive mode=%s exp=%.2fs avg=%d cadence=%.2fs reject_after_move=%d prec=%.3f\" goal=%.3f\"\n",
                  adaptive_mode_name(adaptive_rt.mode), cycle_cfg.exptime_sec, cycle_cfg.avgframes,
                  cycle_cfg.cadence_sec, cycle_cfg.reject_after_move,
                  cycle_cfg.prec_arcsec, cycle_cfg.goal_arcsec);
        }
      }
    }

    double runtime_cadence_sec = cycle_cfg.cadence_sec;
    int runtime_reject_after_move = cycle_cfg.reject_after_move;
    double runtime_prec_arcsec = cycle_cfg.prec_arcsec;
    double runtime_goal_arcsec = cycle_cfg.goal_arcsec;

    double dra_samp[p->max_samples];
    double ddec_samp[p->max_samples];
    double metric_samp[p->max_samples];
    int n = 0;

    double slit_ra_deg = 0.0, slit_dec_deg = 0.0;
    int have_slit = 0;

    int attempts = 0;
    int max_attempts = p->max_samples * 10;
    int invalid_solution_total = 0;
    int faint_rescue_steps = 0;

    while (n < p->max_samples && attempts < max_attempts && !stop_requested()) {
      attempts++;

      float* img = NULL;
      long nx=0, ny=0;
      char* header = NULL;
      int nkeys = 0;
      double exptime_sec = 1.0;

      int rc = acquire_next_frame(p, &fs, runtime_cadence_sec, &img, &nx, &ny, &header, &nkeys, &exptime_sec);
      if (rc) {
        if (img) free(img);
        if (header) free(header);
        acq_logf( "WARNING: failed to acquire frame (rc=%d)\n", rc);
        sleep_seconds(0.1);
        continue;
      }

      // signature-based duplicate reject (compute on stats ROI)
      if (p->reject_identical) {
        long sx1,sx2,sy1,sy2;
        compute_roi_0based(nx, ny, p->pixel_origin,
                           p->bg_roi_mask, p->bg_x1, p->bg_x2, p->bg_y1, p->bg_y2,
                           &sx1,&sx2,&sy1,&sy2);
        uint64_t sig = image_signature_subsample(img, nx, ny, sx1,sx2,sy1,sy2);
        if (fs.have_sig && sig == fs.sig) {
          if (p->verbose) acq_logf( "Reject: identical frame signature\n");
          free(img);
          if (header) free(header);
          continue;
        }
        fs.sig = sig;
        fs.have_sig = 1;
      }

      if (skip_after_move > 0) {
        skip_after_move--;
        if (p->verbose) acq_logf( "Reject: post-move frame (%d remaining)\n", skip_after_move);
        free(img);
        if (header) free(header);
        continue;
      }

      FrameResult fr = solve_frame(img, nx, ny, header, nkeys, p);

      if (p->debug) {
        (void)write_debug_ppm(p->debug_out, img, nx,
                              fr.det.rx1, fr.det.rx2, fr.det.ry1, fr.det.ry2,
                              fr.det.bkg, fr.det.sigma, p->snr_thresh,
                              &fr.det, p);
      }

      free(img);
      if (header) free(header);

      if (!fr.ok) {
        invalid_solution_total++;
        if (p->adaptive && invalid_solution_total >= (6 * (faint_rescue_steps + 1))) {
          double cur_exp = adaptive_rt.have_last_camera ? adaptive_rt.last_exptime_sec : exptime_sec;
          int cur_avg = adaptive_rt.have_last_camera ? adaptive_rt.last_avgframes : 1;
          if (!isfinite(cur_exp) || cur_exp <= 0.0) cur_exp = 1.0;
          if (cur_avg < 1) cur_avg = 1;
          if (cur_avg > 5) cur_avg = 5;

          static const double rescue_exptime_ladder[] = { 0.1, 0.5, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 10.0, 15.0 };
          double target_exp = cur_exp;
          for (size_t i = 0; i < sizeof(rescue_exptime_ladder)/sizeof(rescue_exptime_ladder[0]); i++) {
            if (rescue_exptime_ladder[i] > cur_exp + 1e-6) {
              target_exp = rescue_exptime_ladder[i];
              break;
            }
          }

          if (target_exp > cur_exp + 1e-6) {
            if (p->verbose) {
              acq_logf( "Adaptive faint rescue: %d invalid frames; exptime %.3fs -> %.3fs\n",
                        invalid_solution_total, cur_exp, target_exp);
            }
            (void)scam_set_exptime(target_exp, p->dry_run, p->verbose);
            adaptive_rt.have_last_camera = 1;
            adaptive_rt.last_exptime_sec = target_exp;
            adaptive_rt.last_avgframes = cur_avg;
            runtime_cadence_sec = fmax(runtime_cadence_sec, target_exp * (double)cur_avg + 0.20);
          }
          else {
            if (p->verbose) {
              acq_logf( "Adaptive faint rescue: %d invalid frames; exptime cannot increase further (cur=%.3fs)\n",
                        invalid_solution_total, cur_exp);
            }
          }

          faint_rescue_steps++;
        }
        if (p->verbose) acq_logf( "Reject: no valid solution (SNR/WCS/star).\n");
        continue;
      }

      if (fr.r_cmd_arcsec > p->max_move_arcsec) {
        acq_logf( "Reject: |offset|=%.3f"" exceeds --max-move-arcsec=%.3f\n", fr.r_cmd_arcsec, p->max_move_arcsec);
        continue;
      }

      if ( !have_initial_metric_exptime && isfinite(exptime_sec) && exptime_sec > 0.0 ) {
        initial_metric_exptime_sec = exptime_sec;
        have_initial_metric_exptime = 1;
      }

      metric_samp[n] = fr.det.src_top10_mean;
      dra_samp[n]  = fr.dra_cmd_arcsec;
      ddec_samp[n] = fr.ddec_cmd_arcsec;
      slit_ra_deg  = fr.ra_goal_deg;
      slit_dec_deg = fr.dec_goal_deg;
      have_slit = 1;
      n++;

      // Compute robust stats on the fly
      double dra_tmp[p->max_samples];
      double ddec_tmp[p->max_samples];
      for (int i = 0; i < n; i++) { dra_tmp[i]=dra_samp[i]; ddec_tmp[i]=ddec_samp[i]; }
      double med_dra = median_of_doubles(dra_tmp, n);
      double med_ddec = median_of_doubles(ddec_tmp, n);
      double sig_dra = mad_sigma_of_doubles(dra_samp, n, med_dra);
      double sig_ddec = mad_sigma_of_doubles(ddec_samp, n, med_ddec);
      double rmed = hypot(med_dra, med_ddec);

      if (p->verbose) {
        if (p->adaptive) {
          acq_logf( "Sample %d/%d: dra=%.3f"" ddec=%.3f"" |med|=%.3f""  scatter(MAD)=(%.3f,%.3f)"" metric=%.1f mode=%s\n",
                  n, p->max_samples, dra_samp[n-1], ddec_samp[n-1], rmed, sig_dra, sig_ddec,
                  fr.det.src_top10_mean, adaptive_mode_name(adaptive_rt.mode));
        } else {
          acq_logf( "Sample %d/%d: dra=%.3f"" ddec=%.3f"" |med|=%.3f""  scatter(MAD)=(%.3f,%.3f)""\n",
                  n, p->max_samples, dra_samp[n-1], ddec_samp[n-1], rmed, sig_dra, sig_ddec);
        }
      }

      // If already within goal threshold, finish (no move)
      if (n >= p->min_samples && rmed <= runtime_goal_arcsec) {
        if (p->verbose) acq_logf( "Converged: |median offset|=%.3f"" <= %.3f""\n", rmed, runtime_goal_arcsec);
        return 0;
      }

      // If centroiding precision is good enough, we can move now
      if (n >= p->min_samples && sig_dra <= runtime_prec_arcsec && sig_ddec <= runtime_prec_arcsec) {
        // Issue robust move
        double cmd_dra = p->gain * med_dra;
        double cmd_ddec = p->gain * med_ddec;

        if (p->verbose) {
          acq_logf( "MOVE (robust median): dra=%.3f"" ddec=%.3f""  (gain=%.3f)\n", cmd_dra, cmd_ddec, p->gain);
        }

        if (p->use_putonslit) {
          if (!have_slit) {
            acq_logf( "REFUSE MOVE: missing slit RA/Dec for putonslit\n");
          } else {
            double cross_ra=0.0, cross_dec=0.0;
            slit_cross_from_offsets(p, slit_ra_deg, slit_dec_deg, cmd_dra, cmd_ddec, &cross_ra, &cross_dec);
            (void)scam_putonslit_deg(slit_ra_deg, slit_dec_deg, cross_ra, cross_dec, p->dry_run, p->verbose);
            (void)wait_for_guiding(p);
          }
        } else {
          (void)tcs_move_arcsec(cmd_dra, cmd_ddec, p->dry_run, p->verbose);
        }

        // After move, reject a few new frames to avoid trailed exposures.
        adaptive_finish_cycle(p, &adaptive_rt, metric_samp, n);
        if (p->settle_sec > 0) sleep_seconds(p->settle_sec);
        skip_after_move = runtime_reject_after_move;

        // proceed to next cycle
        goto next_cycle;
      }
    }

    // If we get here, we did not reach required precision.
    if (n >= p->min_samples) {
      double dra_tmp[p->max_samples];
      double ddec_tmp[p->max_samples];
      for (int i = 0; i < n; i++) { dra_tmp[i]=dra_samp[i]; ddec_tmp[i]=ddec_samp[i]; }
      double med_dra = median_of_doubles(dra_tmp, n);
      double med_ddec = median_of_doubles(ddec_tmp, n);
      double sig_dra = mad_sigma_of_doubles(dra_samp, n, med_dra);
      double sig_ddec = mad_sigma_of_doubles(ddec_samp, n, med_ddec);
      double rmed = hypot(med_dra, med_ddec);

      acq_logf( "FAIL: insufficient precision to move safely after %d samples (attempts=%d).\n", n, attempts);
      acq_logf( "      median(dra,ddec)=(%.3f,%.3f)""  scatter(MAD)=(%.3f,%.3f)""  |med|=%.3f""\n",
              med_dra, med_ddec, sig_dra, sig_ddec, rmed);
    } else {
      acq_logf( "FAIL: too few valid samples (n=%d) after attempts=%d.\n", n, attempts);
    }

    adaptive_finish_cycle(p, &adaptive_rt, metric_samp, n);

    if (!p->continue_on_fail) return 2;

next_cycle:
    ;
  }

  if (stop_requested()) {
    acq_logf( "Stopped by user request.\n");
    return 1;
  }

  acq_logf( "FAILED: reached max cycles (%d) without convergence.\n", p->max_cycles);
  return 1;
}

#ifdef __cplusplus
extern "C" {
#endif

void ngps_acq_set_hooks(const ngps_acq_hooks_t* hooks) {
  if (!hooks) {
    memset(&g_hooks, 0, sizeof(g_hooks));
    g_hooks_initialized = 0;
    return;
  }

  g_hooks = *hooks;
  g_hooks_initialized = 1;
}

void ngps_acq_clear_hooks(void) {
  memset(&g_hooks, 0, sizeof(g_hooks));
  g_hooks_initialized = 0;
}

void ngps_acq_request_stop(int stop_requested_flag) {
  g_stop = stop_requested_flag ? 1 : 0;
}

static int ngps_acq_run_internal(int argc, char** argv, int install_signal_handler)
{
  g_stop = 0;
  if (install_signal_handler) signal(SIGINT, on_sigint);

  AcqParams p;
  set_defaults(&p);

  int pr = parse_args(argc, argv, &p);
  if (pr <= 0) { usage(argv[0]); return (pr == 0) ? 0 : 4; }

  if (p.verbose) {
    acq_logf(
      "NGPS ACQ start:\n"
      "  mode=%s input=%s framegrab_out=%s\n"
      "  goal=(%.3f,%.3f) origin=%d max_dist=%.1f snr=%.1f filt_sigma=%.2f\n"
      "  centroid_hw=%d centroid_sigma=%.2f\n"
      "  loop=%d cadence=%.2fs max_samples=%d min_samples=%d prec=%.3f\" goal=%.3f\" gain=%.2f\n"
      "  reject_identical=%d reject_after_move=%d settle=%.2fs max_move=%.2f\"\n"
      "  use_putonslit=%d wait_guiding=%d guiding_poll=%.2fs guiding_timeout=%.1fs\n"
      "  tcs_set_units=%d dra_use_cosdec=%d tcs_sign=%d dry_run=%d\n",
      (p.frame_mode == FRAME_FRAMEGRAB) ? "framegrab" : "stream",
      p.input, p.framegrab_out,
      p.goal_x, p.goal_y, p.pixel_origin, p.max_dist_pix, p.snr_thresh, p.filt_sigma_pix,
      p.centroid_halfwin, p.centroid_sigma_pix,
      p.loop, p.cadence_sec, p.max_samples, p.min_samples, p.prec_arcsec, p.goal_arcsec, p.gain,
      p.reject_identical, p.reject_after_move, p.settle_sec, p.max_move_arcsec,
      p.use_putonslit, p.wait_guiding, p.guiding_poll_sec, p.guiding_timeout_sec,
      p.tcs_set_units, p.dra_use_cosdec, p.tcs_sign, p.dry_run);
    if (p.adaptive) {
      acq_logf( "  adaptive=1 faint-start=%.1f faint-goal=%.1f bright-start=%.1f bright-goal=%.1f\n",
              p.adaptive_faint, p.adaptive_faint_goal, p.adaptive_bright, p.adaptive_bright_goal);
    }
  }

  if (p.loop) {
    return run_loop(&p);
  }

  // One-shot: acquire one frame and (optionally) move once.
  if (p.tcs_set_units) (void)tcs_set_native_units(p.dry_run, p.verbose);
  FrameState fs;
  memset(&fs, 0, sizeof(fs));
  fs.t_accept.tv_sec = 0;
  fs.t_accept.tv_nsec = 0;
  return process_once(&p, &fs);
}

int ngps_acq_run_from_argv(int argc, char** argv) {
  return ngps_acq_run_internal(argc, argv, 0);
}

#ifdef __cplusplus
}
#endif

#ifndef NGPS_ACQ_EMBEDDED
int main(int argc, char** argv)
{
  return ngps_acq_run_internal(argc, argv, 1);
}
#endif
