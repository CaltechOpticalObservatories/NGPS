#include <QtWidgets>
#include <QCoreApplication>
#include <QRegularExpression>
#include <QStandardItemModel>
#include <QDebug>
#include <QLocale>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPointer>
#include <QUuid>
#include <mysqlx/xdevapi.h>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <functional>
#include <memory>
#include <sstream>
#include <limits>

namespace {
const char kSettingsOrg[] = "NGPS";
const char kSettingsApp[] = "ngps_db_gui";

const double kDefaultWrangeHalfWidthNm = 15.0;  // +/- 150 Å
const char kDefaultExptime[] = "SET 5";
const char kDefaultSlitwidth[] = "SET 1";
const char kDefaultSlitangle[] = "PA";
const char kDefaultMagsystem[] = "AB";
const char kDefaultMagfilter[] = "match";
const char kDefaultChannel[] = "R";
const char kDefaultPointmode[] = "SLIT";
const char kDefaultCcdmode[] = "default";
const char kDefaultNotBefore[] = "1999-12-31T12:34:56";
const double kDefaultMagnitude = 19.0;
const double kDefaultAirmassMax = 4.0;
const int kDefaultBin = 1;
const double kDefaultOtmSlitwidth = 1.0;
const char kDefaultTargetState[] = "pending";
const double kGroupCoordTolArcsec = 1.0;
const double kOffsetZeroTolArcsec = 0.1;
}

struct NormalizationResult {
  QStringList changedColumns;
  QString message;
};

struct TimelineTarget {
  QString obsId;
  QString name;
  QDateTime startUtc;
  QDateTime endUtc;
  QVector<QPair<QDateTime, QDateTime>> segments;
  QDateTime slewGoUtc;
  QVector<double> airmass;
  int obsOrder = 0;
  QString flag;
  int severity = 0; // 0=none,1=warn,2=error
  bool observed = false;
  double waitSec = 0.0;
};

struct TimelineData {
  QVector<QDateTime> timesUtc;
  QDateTime twilightEvening16;
  QDateTime twilightEvening12;
  QDateTime twilightMorning12;
  QDateTime twilightMorning16;
  QVector<TimelineTarget> targets;
  double airmassLimit = 0.0;
  double delaySlewSec = 0.0;
  QVector<QPair<QDateTime, QDateTime>> idleIntervals;
};

static QString explainOtmFlagToken(const QString &token) {
  const QString t = token.trimmed().toUpper();
  if (t.isEmpty()) return QString();
  if (t == "DAY-0") {
    return "DAY-0: Start time was before twilight; scheduler waited until night.";
  }
  if (t == "DAY-1") {
    return "DAY-1: Daylight reached during/after exposure; remaining targets skipped.";
  }
  if (t == "DAY-0-1") {
    return "DAY-0-1: Daylight reached before this target; target and remaining skipped.";
  }
  if (t == "SKY") {
    return "SKY: Sky background model unavailable/invalid; used fallback sky magnitude.";
  }
  if (t == "EXPT") {
    return "EXPT: Exposure time exceeded warning threshold.";
  }
  QRegularExpression dofRe("^(AIR|ALT|HA)-([01])$");
  const QRegularExpressionMatch m = dofRe.match(t);
  if (m.hasMatch()) {
    const QString dof = m.captured(1);
    const QString phase = m.captured(2);
    const QString phaseText = (phase == "0") ? "before exposure" : "after exposure";
    if (dof == "AIR") {
      return QString("AIR-%1: Airmass limit violated %2.").arg(phase, phaseText);
    }
    if (dof == "ALT") {
      return QString("ALT-%1: Altitude limit violated %2.").arg(phase, phaseText);
    }
    if (dof == "HA") {
      return QString("HA-%1: Hour angle limit violated %2.").arg(phase, phaseText);
    }
  }
  return QString("Unknown flag: %1").arg(token);
}

static QString explainOtmFlags(const QString &flagText) {
  QStringList tokens = flagText.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
  if (tokens.isEmpty()) return QString();
  QStringList lines;
  for (const QString &token : tokens) {
    const QString explanation = explainOtmFlagToken(token);
    if (!explanation.isEmpty()) lines << "- " + explanation;
  }
  if (lines.isEmpty()) return QString();
  return lines.join('\n');
}

static QString formatNumber(double value, int precision = 8) {
  return QLocale::c().toString(value, 'g', precision);
}

static QString variantToString(const QVariant &value) {
  if (!value.isValid() || value.isNull()) return QString();
  const int type = value.userType();
  if (type == QMetaType::Double || type == QMetaType::Float) {
    return formatNumber(value.toDouble());
  }
  if (type == QMetaType::Int || type == QMetaType::LongLong) {
    return QString::number(value.toLongLong());
  }
  if (type == QMetaType::UInt || type == QMetaType::ULongLong) {
    return QString::number(value.toULongLong());
  }
  return value.toString().trimmed();
}

static QString trimOrEmpty(const QString &text) {
  return text.trimmed();
}

static QStringList splitTokens(const QString &text) {
  return text.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
}

static QString findKeyCaseInsensitive(const QVariantMap &values, const QString &key) {
  if (values.contains(key)) return key;
  for (auto it = values.begin(); it != values.end(); ++it) {
    if (it.key().compare(key, Qt::CaseInsensitive) == 0) {
      return it.key();
    }
  }
  return key;
}

static QString valueForKeyCaseInsensitive(const QVariantMap &values, const QString &key) {
  const QString actual = findKeyCaseInsensitive(values, key);
  return values.value(actual).toString();
}

static QString valueToStringCaseInsensitive(const QVariantMap &values, const QString &key) {
  const QString actual = findKeyCaseInsensitive(values, key);
  return variantToString(values.value(actual));
}

static bool containsKeyCaseInsensitive(const QVariantMap &values, const QString &key) {
  if (values.contains(key)) return true;
  for (auto it = values.begin(); it != values.end(); ++it) {
    if (it.key().compare(key, Qt::CaseInsensitive) == 0) {
      return true;
    }
  }
  return false;
}

static bool setContainsCaseInsensitive(const QSet<QString> &set, const QString &key) {
  if (set.contains(key)) return true;
  for (const QString &entry : set) {
    if (entry.compare(key, Qt::CaseInsensitive) == 0) return true;
  }
  return false;
}

static void setRemoveCaseInsensitive(QSet<QString> &set, const QString &key) {
  if (set.contains(key)) {
    set.remove(key);
    return;
  }
  QString removeKey;
  for (const QString &entry : set) {
    if (entry.compare(key, Qt::CaseInsensitive) == 0) {
      removeKey = entry;
      break;
    }
  }
  if (!removeKey.isEmpty()) set.remove(removeKey);
}

static bool parseDouble(const QString &text, double *value) {
  bool ok = false;
  const double v = QLocale::c().toDouble(text.trimmed(), &ok);
  if (ok && value) *value = v;
  return ok;
}

static bool parseInt(const QString &text, int *value) {
  bool ok = false;
  const int v = text.trimmed().toInt(&ok);
  if (ok && value) *value = v;
  return ok;
}

static bool parseSexagesimalAngle(const QString &text, bool isRa, double *degOut) {
  const QString trimmed = text.trimmed();
  if (!trimmed.contains(':')) return false;
  const QStringList parts = trimmed.split(':');
  if (parts.size() < 2) return false;
  bool ok0 = false;
  const QString first = parts.at(0).trimmed();
  const bool neg = (!isRa && first.startsWith('-'));
  double a = first.toDouble(&ok0);
  if (!ok0) return false;
  a = std::abs(a);
  bool ok1 = false;
  double b = parts.at(1).trimmed().toDouble(&ok1);
  if (!ok1) return false;
  b = std::abs(b);
  double c = 0.0;
  if (parts.size() > 2) {
    bool ok2 = false;
    c = parts.at(2).trimmed().toDouble(&ok2);
    if (!ok2) c = 0.0;
    c = std::abs(c);
  }
  double value = a + b / 60.0 + c / 3600.0;
  if (isRa) {
    if (degOut) *degOut = value * 15.0;
  } else {
    if (degOut) *degOut = neg ? -value : value;
  }
  return true;
}

static bool parseAngleDegrees(const QString &text, bool isRa, double *degOut) {
  if (parseSexagesimalAngle(text, isRa, degOut)) return true;
  double value = 0.0;
  if (!parseDouble(text, &value)) return false;
  if (isRa) {
    if (std::abs(value) <= 24.0) {
      value *= 15.0;
    }
  }
  if (degOut) *degOut = value;
  return true;
}

static double offsetArcsecFromValues(const QVariantMap &values, const QStringList &keys, bool *found) {
  for (const QString &key : keys) {
    const QString actual = findKeyCaseInsensitive(values, key);
    if (!values.contains(actual)) continue;
    const QString text = values.value(actual).toString().trimmed();
    if (text.isEmpty()) continue;
    double val = 0.0;
    if (parseDouble(text, &val)) {
      if (found) *found = true;
      return val;
    }
  }
  if (found) *found = false;
  return 0.0;
}

static bool computeScienceCoordDegreesProjected(const QVariantMap &values,
                                                double *raDegOut,
                                                double *decDegOut);

static bool computeScienceCoordKey(const QVariantMap &values, QString *keyOut) {
  double raDeg = 0.0;
  double decDeg = 0.0;
  if (!computeScienceCoordDegreesProjected(values, &raDeg, &decDeg)) return false;

  const double tolDeg = kGroupCoordTolArcsec / 3600.0;
  if (tolDeg > 0.0) {
    raDeg = std::round(raDeg / tolDeg) * tolDeg;
    decDeg = std::round(decDeg / tolDeg) * tolDeg;
  }

  while (raDeg < 0.0) raDeg += 360.0;
  while (raDeg >= 360.0) raDeg -= 360.0;

  if (keyOut) {
    const QString key = QString("%1:%2")
                            .arg(QString::number(raDeg, 'f', 6))
                            .arg(QString::number(decDeg, 'f', 6));
    *keyOut = key;
  }
  return true;
}

static bool computeScienceCoordDegrees(const QVariantMap &values, double *raDegOut, double *decDegOut) {
  const QString raText = valueToStringCaseInsensitive(values, "RA");
  const QString decText = valueToStringCaseInsensitive(values, "DECL");
  if (raText.isEmpty() || decText.isEmpty()) return false;
  double raDeg = 0.0;
  double decDeg = 0.0;
  if (!parseAngleDegrees(raText, true, &raDeg)) return false;
  if (!parseAngleDegrees(decText, false, &decDeg)) return false;

  bool hasRa = false;
  bool hasDec = false;
  const double offsetRa = offsetArcsecFromValues(values, {"OFFSET_RA", "DRA"}, &hasRa);
  const double offsetDec = offsetArcsecFromValues(values, {"OFFSET_DEC", "DDEC"}, &hasDec);
  raDeg += offsetRa / 3600.0;
  decDeg += offsetDec / 3600.0;

  while (raDeg < 0.0) raDeg += 360.0;
  while (raDeg >= 360.0) raDeg -= 360.0;

  if (raDegOut) *raDegOut = raDeg;
  if (decDegOut) *decDegOut = decDeg;
  return true;
}

// Gnomonic (tangent plane) projection using offsets in arcsec along RA/Dec axes.
static bool computeScienceCoordDegreesProjected(const QVariantMap &values,
                                                double *raDegOut,
                                                double *decDegOut) {
  const QString raText = valueToStringCaseInsensitive(values, "RA");
  const QString decText = valueToStringCaseInsensitive(values, "DECL");
  if (raText.isEmpty() || decText.isEmpty()) return false;
  double raDeg = 0.0;
  double decDeg = 0.0;
  if (!parseAngleDegrees(raText, true, &raDeg)) return false;
  if (!parseAngleDegrees(decText, false, &decDeg)) return false;

  bool hasRa = false;
  bool hasDec = false;
  const double offsetRa = offsetArcsecFromValues(values, {"OFFSET_RA", "DRA"}, &hasRa);
  const double offsetDec = offsetArcsecFromValues(values, {"OFFSET_DEC", "DDEC"}, &hasDec);

  const double deg2rad = 3.14159265358979323846 / 180.0;
  const double ra0 = raDeg * deg2rad;
  const double dec0 = decDeg * deg2rad;
  const double xi = (offsetRa / 3600.0) * deg2rad;
  const double eta = (offsetDec / 3600.0) * deg2rad;

  const double sinDec0 = std::sin(dec0);
  const double cosDec0 = std::cos(dec0);
  const double denom = cosDec0 - eta * sinDec0;
  const double ra1 = ra0 + std::atan2(xi, denom);
  const double dec1 = std::atan2(sinDec0 + eta * cosDec0, std::sqrt(denom * denom + xi * xi));

  double raDegOutLocal = ra1 / deg2rad;
  double decDegOutLocal = dec1 / deg2rad;
  while (raDegOutLocal < 0.0) raDegOutLocal += 360.0;
  while (raDegOutLocal >= 360.0) raDegOutLocal -= 360.0;

  if (raDegOut) *raDegOut = raDegOutLocal;
  if (decDegOut) *decDegOut = decDegOutLocal;
  return true;
}

static double angularSeparationArcsec(double raDeg1, double decDeg1,
                                      double raDeg2, double decDeg2) {
  const double deg2rad = 3.14159265358979323846 / 180.0;
  const double ra1 = raDeg1 * deg2rad;
  const double dec1 = decDeg1 * deg2rad;
  const double ra2 = raDeg2 * deg2rad;
  const double dec2 = decDeg2 * deg2rad;
  const double cosd = std::sin(dec1) * std::sin(dec2) +
                      std::cos(dec1) * std::cos(dec2) * std::cos(ra1 - ra2);
  const double clamped = std::max(-1.0, std::min(1.0, cosd));
  const double sepRad = std::acos(clamped);
  return sepRad * (180.0 / 3.14159265358979323846) * 3600.0;
}

static bool channelRangeFor(const QString &channel, double *minNm, double *maxNm) {
  const QString ch = channel.trimmed().toUpper();
  if (ch == "U") { if (minNm) *minNm = 310.0; if (maxNm) *maxNm = 436.0; return true; }
  if (ch == "G") { if (minNm) *minNm = 417.0; if (maxNm) *maxNm = 590.0; return true; }
  if (ch == "R") { if (minNm) *minNm = 561.0; if (maxNm) *maxNm = 794.0; return true; }
  if (ch == "I") { if (minNm) *minNm = 756.0; if (maxNm) *maxNm = 1040.0; return true; }
  return false;
}

static QPair<double, double> defaultWrangeForChannel(const QString &channel) {
  double minNm = 0.0;
  double maxNm = 0.0;
  if (!channelRangeFor(channel, &minNm, &maxNm)) {
    channelRangeFor(kDefaultChannel, &minNm, &maxNm);
  }
  const double center = 0.5 * (minNm + maxNm);
  return {center - kDefaultWrangeHalfWidthNm, center + kDefaultWrangeHalfWidthNm};
}

static QString normalizeChannelValue(const QString &text) {
  const QString ch = text.trimmed().toUpper();
  if (ch == "U" || ch == "G" || ch == "R" || ch == "I") return ch;
  return kDefaultChannel;
}

static QString normalizeMagsystemValue(const QString &text) {
  const QString val = text.trimmed().toUpper();
  if (val == "AB" || val == "VEGA") return val;
  return kDefaultMagsystem;
}

static QString normalizeMagfilterValue(const QString &text) {
  const QString val = text.trimmed();
  if (val.isEmpty()) return kDefaultMagfilter;
  const QString upper = val.toUpper();
  if (upper == "G") return kDefaultMagfilter;
  if (upper == "MATCH") return kDefaultMagfilter;
  if (upper == "USER") return "user";
  if (upper == "U" || upper == "B" || upper == "V" || upper == "R" ||
      upper == "I" || upper == "J" || upper == "K") {
    return upper;
  }
  return kDefaultMagfilter;
}

static QString normalizePointmodeValue(const QString &text) {
  const QString val = text.trimmed().toUpper();
  if (val == "SLIT" || val == "ACAM") return val;
  return kDefaultPointmode;
}

static QString normalizeCcdmodeValue(const QString &text) {
  const QString val = text.trimmed();
  if (val.compare("default", Qt::CaseInsensitive) == 0) return kDefaultCcdmode;
  return kDefaultCcdmode;
}

static QString normalizeSrcmodelValue(const QString &text) {
  QString val = text.trimmed();
  if (val.isEmpty()) return "-model constant";
  if (val.startsWith("-", Qt::CaseInsensitive)) return val;
  if (val.startsWith("model", Qt::CaseInsensitive)) return "-" + val;
  return QString("-model %1").arg(val);
}

static QString normalizeSlitangleValue(const QString &text) {
  const QString val = text.trimmed();
  if (val.isEmpty()) return kDefaultSlitangle;
  if (val.compare("PA", Qt::CaseInsensitive) == 0) return "PA";
  double num = 0.0;
  if (parseDouble(val, &num)) return formatNumber(num);
  return kDefaultSlitangle;
}

static QString normalizeExptimeValue(const QString &text, bool isCalib) {
  Q_UNUSED(isCalib);
  QString val = text.trimmed();
  if (val.isEmpty()) return kDefaultExptime;
  QStringList parts = splitTokens(val);
  if (parts.size() == 1) {
    double num = 0.0;
    if (parseDouble(parts[0], &num) && num > 0) {
      return QString("SET %1").arg(formatNumber(num));
    }
    return kDefaultExptime;
  }
  QString key = parts[0].toUpper();
  if (key == "EXPTIME") key = "SET";
  if (key == "SET" || key == "SNR") {
    double num = 0.0;
    if (parts.size() >= 2 && parseDouble(parts[1], &num) && num > 0) {
      return QString("%1 %2").arg(key, formatNumber(num));
    }
  }
  return kDefaultExptime;
}

static QString normalizeSlitwidthValue(const QString &text, bool isCalib) {
  Q_UNUSED(isCalib);
  QString val = text.trimmed();
  if (val.isEmpty()) return kDefaultSlitwidth;
  QStringList parts = splitTokens(val);
  if (parts.size() == 1) {
    const QString key = parts[0].toUpper();
    if (key == "AUTO") return "AUTO";
    double num = 0.0;
    if (parseDouble(parts[0], &num) && num > 0) {
      return QString("SET %1").arg(formatNumber(num));
    }
    return kDefaultSlitwidth;
  }
  QString key = parts[0].toUpper();
  if (key == "AUTO") return "AUTO";
  if (key == "SET" || key == "SNR" || key == "RES" || key == "LOSS") {
    double num = 0.0;
    if (parts.size() >= 2 && parseDouble(parts[1], &num) && num > 0) {
      return QString("%1 %2").arg(key, formatNumber(num));
    }
  }
  return kDefaultSlitwidth;
}

static bool extractSetNumeric(const QString &text, double *valueOut) {
  QStringList parts = splitTokens(text);
  if (parts.size() >= 2 && parts[0].compare("SET", Qt::CaseInsensitive) == 0) {
    double num = 0.0;
    if (parseDouble(parts[1], &num) && num > 0) {
      if (valueOut) *valueOut = num;
      return true;
    }
  }
  return false;
}

static QStringList parseCsvLine(const QString &line) {
  QStringList fields;
  QString field;
  bool inQuotes = false;
  for (int i = 0; i < line.size(); ++i) {
    const QChar ch = line.at(i);
    if (ch == '"') {
      if (inQuotes && i + 1 < line.size() && line.at(i + 1) == '"') {
        field += '"';
        ++i;
      } else {
        inQuotes = !inQuotes;
      }
    } else if (ch == ',' && !inQuotes) {
      fields << field.trimmed();
      field.clear();
    } else {
      field += ch;
    }
  }
  fields << field.trimmed();
  return fields;
}

static QString csvEscape(const QString &value) {
  if (value.contains(',') || value.contains('"') || value.contains('\n') || value.contains('\r')) {
    QString escaped = value;
    escaped.replace('"', "\"\"");
    return QString("\"%1\"").arg(escaped);
  }
  return value;
}

static QString normalizeOtmTimestamp(const QString &value) {
  QString out = value.trimmed();
  if (out.isEmpty()) return out;
  if (out.compare("None", Qt::CaseInsensitive) == 0) return QString();
  out.replace('T', ' ');
  return out;
}

static QDateTime parseUtcIso(const QString &text) {
  QString trimmed = text.trimmed();
  if (trimmed.isEmpty()) return QDateTime();
  QDateTime dt = QDateTime::fromString(trimmed, Qt::ISODateWithMs);
  if (!dt.isValid()) {
    dt = QDateTime::fromString(trimmed, Qt::ISODate);
  }
  if (!dt.isValid()) return QDateTime();
  dt.setTimeSpec(Qt::UTC);
  return dt;
}

static int otmFlagSeverity(const QString &flag) {
  const QString trimmed = flag.trimmed();
  if (trimmed.isEmpty()) return 0;
  const QString upper = trimmed.toUpper();
  if (upper.contains("DAY-0-1") || upper.contains("DAY-1")) return 2;
  QRegularExpression errRe("\\b[A-Z]+1\\b");
  if (errRe.match(upper).hasMatch()) return 2;
  return 1;
}

static QString mergeFlagText(const QString &existing, const QString &incoming) {
  QStringList ordered;
  QSet<QString> seen;
  auto addTokens = [&](const QString &text) {
    const QStringList tokens = text.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    for (const QString &token : tokens) {
      const QString upper = token.trimmed();
      if (upper.isEmpty()) continue;
      if (seen.contains(upper)) continue;
      seen.insert(upper);
      ordered << upper;
    }
  };
  addTokens(existing);
  addTokens(incoming);
  return ordered.join(' ');
}

static bool loadTimelineJson(const QString &path, TimelineData *data, QString *error) {
  if (!data) return false;
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    if (error) *error = "Unable to read timeline data.";
    return false;
  }
  const QByteArray raw = file.readAll();
  QJsonParseError parseError;
  QJsonDocument doc = QJsonDocument::fromJson(raw, &parseError);
  if (doc.isNull()) {
    if (error) *error = parseError.errorString();
    return false;
  }
  const QJsonObject root = doc.object();
  TimelineData tmp;
  const QJsonArray times = root.value("times_utc").toArray();
  tmp.timesUtc.reserve(times.size());
  for (const QJsonValue &val : times) {
    QDateTime dt = parseUtcIso(val.toString());
    if (dt.isValid()) tmp.timesUtc.append(dt);
  }

  const QJsonObject twilight = root.value("twilight").toObject();
  tmp.twilightEvening16 = parseUtcIso(twilight.value("evening_16").toString());
  tmp.twilightEvening12 = parseUtcIso(twilight.value("evening_12").toString());
  tmp.twilightMorning12 = parseUtcIso(twilight.value("morning_12").toString());
  tmp.twilightMorning16 = parseUtcIso(twilight.value("morning_16").toString());
  tmp.delaySlewSec = root.value("delay_slew_sec").toDouble(0.0);

  const QJsonArray idle = root.value("idle_intervals").toArray();
  tmp.idleIntervals.reserve(idle.size());
  for (const QJsonValue &val : idle) {
    const QJsonObject obj = val.toObject();
    const QDateTime s = parseUtcIso(obj.value("start").toString());
    const QDateTime e = parseUtcIso(obj.value("end").toString());
    if (s.isValid() && e.isValid() && e > s) {
      tmp.idleIntervals.append({s, e});
    }
  }

  const QJsonArray targets = root.value("targets").toArray();
  tmp.targets.reserve(targets.size());
  QHash<QString, int> targetIndexByKey;
  for (const QJsonValue &val : targets) {
    const QJsonObject obj = val.toObject();
    TimelineTarget target;
    target.obsId = obj.value("obs_id").toString();
    target.name = obj.value("name").toString();
    target.startUtc = parseUtcIso(obj.value("start").toString());
    target.endUtc = parseUtcIso(obj.value("end").toString());
    target.slewGoUtc = parseUtcIso(obj.value("slew_go").toString());
    target.flag = obj.value("flag").toString();
    target.observed = obj.value("observed").toBool();
    if (!target.observed) {
      target.observed = target.startUtc.isValid() && target.endUtc.isValid();
    }
    target.waitSec = obj.value("wait_sec").toDouble(0.0);
    target.severity = otmFlagSeverity(target.flag);
    const QJsonArray airmass = obj.value("airmass").toArray();
    target.airmass.reserve(airmass.size());
    for (const QJsonValue &av : airmass) {
      target.airmass.append(av.toDouble(std::numeric_limits<double>::quiet_NaN()));
    }
    const QString key = !target.obsId.isEmpty() ? target.obsId : target.name;
    if (key.isEmpty()) continue;
    if (target.startUtc.isValid() && target.endUtc.isValid()) {
      target.segments.append({target.startUtc, target.endUtc});
    }
    if (!targetIndexByKey.contains(key)) {
      targetIndexByKey.insert(key, tmp.targets.size());
      tmp.targets.append(target);
      continue;
    }
    TimelineTarget &existing = tmp.targets[targetIndexByKey.value(key)];
    if (!target.obsId.isEmpty()) existing.obsId = target.obsId;
    if (existing.name.isEmpty()) existing.name = target.name;
    if (existing.airmass.isEmpty() && !target.airmass.isEmpty()) {
      existing.airmass = target.airmass;
    }
    if (target.startUtc.isValid() && target.endUtc.isValid()) {
      existing.segments.append({target.startUtc, target.endUtc});
      if (!existing.startUtc.isValid() || target.startUtc < existing.startUtc) {
        existing.startUtc = target.startUtc;
      }
      if (!existing.endUtc.isValid() || target.endUtc > existing.endUtc) {
        existing.endUtc = target.endUtc;
      }
    }
    if (target.slewGoUtc.isValid() && !existing.slewGoUtc.isValid()) {
      existing.slewGoUtc = target.slewGoUtc;
    }
    existing.waitSec = std::max(existing.waitSec, target.waitSec);
    existing.observed = existing.observed || target.observed;
    existing.severity = std::max(existing.severity, target.severity);
    existing.flag = mergeFlagText(existing.flag, target.flag);
  }

  *data = tmp;
  return true;
}

static void setNormalizedValue(QVariantMap &values,
                               QSet<QString> &nullColumns,
                               const QString &column,
                               const QString &newValue,
                               QStringList *changedColumns) {
  const QString actualKey = findKeyCaseInsensitive(values, column);
  const QString current = values.value(actualKey).toString();
  if (current != newValue) {
    if (changedColumns) changedColumns->append(column);
  }
  values.insert(actualKey, newValue);
  setRemoveCaseInsensitive(nullColumns, column);
}

static NormalizationResult normalizeTargetRow(QVariantMap &values, QSet<QString> &nullColumns) {
  NormalizationResult result;

  const QString name = values.value("NAME").toString().trimmed();
  const bool isCalib = name.toUpper().startsWith("CAL_");

  if (containsKeyCaseInsensitive(values, "CHANNEL")) {
    setNormalizedValue(values, nullColumns, "CHANNEL",
                       normalizeChannelValue(valueForKeyCaseInsensitive(values, "CHANNEL")),
                       &result.changedColumns);
  }

  if (containsKeyCaseInsensitive(values, "SLITANGLE")) {
    setNormalizedValue(values, nullColumns, "SLITANGLE",
                       normalizeSlitangleValue(valueForKeyCaseInsensitive(values, "SLITANGLE")),
                       &result.changedColumns);
  }

  if (containsKeyCaseInsensitive(values, "EXPTIME")) {
    const QString normalized = normalizeExptimeValue(valueForKeyCaseInsensitive(values, "EXPTIME"), isCalib);
    setNormalizedValue(values, nullColumns, "EXPTIME", normalized, &result.changedColumns);
  }

  if (containsKeyCaseInsensitive(values, "SLITWIDTH")) {
    const QString normalized = normalizeSlitwidthValue(valueForKeyCaseInsensitive(values, "SLITWIDTH"), isCalib);
    setNormalizedValue(values, nullColumns, "SLITWIDTH", normalized, &result.changedColumns);
  }

  if (containsKeyCaseInsensitive(values, "BINSPECT")) {
    int val = 0;
    if (!parseInt(valueForKeyCaseInsensitive(values, "BINSPECT"), &val) || val <= 0) {
      setNormalizedValue(values, nullColumns, "BINSPECT", QString::number(kDefaultBin),
                         &result.changedColumns);
    }
  }

  if (containsKeyCaseInsensitive(values, "NEXP")) {
    int val = 0;
    if (!parseInt(valueForKeyCaseInsensitive(values, "NEXP"), &val) || val <= 0) {
      setNormalizedValue(values, nullColumns, "NEXP", "1", &result.changedColumns);
    }
  }

  if (containsKeyCaseInsensitive(values, "BINSPAT")) {
    int val = 0;
    if (!parseInt(valueForKeyCaseInsensitive(values, "BINSPAT"), &val) || val <= 0) {
      setNormalizedValue(values, nullColumns, "BINSPAT", QString::number(kDefaultBin),
                         &result.changedColumns);
    }
  }

  if (containsKeyCaseInsensitive(values, "AIRMASS_MAX")) {
    double val = 0.0;
    if (!parseDouble(valueForKeyCaseInsensitive(values, "AIRMASS_MAX"), &val) || val < 1.0) {
      setNormalizedValue(values, nullColumns, "AIRMASS_MAX", formatNumber(kDefaultAirmassMax),
                         &result.changedColumns);
    }
  }

  if (containsKeyCaseInsensitive(values, "POINTMODE")) {
    setNormalizedValue(values, nullColumns, "POINTMODE",
                       normalizePointmodeValue(valueForKeyCaseInsensitive(values, "POINTMODE")),
                       &result.changedColumns);
  }

  if (containsKeyCaseInsensitive(values, "CCDMODE")) {
    setNormalizedValue(values, nullColumns, "CCDMODE",
                       normalizeCcdmodeValue(valueForKeyCaseInsensitive(values, "CCDMODE")),
                       &result.changedColumns);
  }

  if (containsKeyCaseInsensitive(values, "MAGNITUDE")) {
    double val = 0.0;
    if (!parseDouble(valueForKeyCaseInsensitive(values, "MAGNITUDE"), &val)) {
      setNormalizedValue(values, nullColumns, "MAGNITUDE", formatNumber(kDefaultMagnitude),
                         &result.changedColumns);
    }
  }

  if (containsKeyCaseInsensitive(values, "MAGSYSTEM")) {
    setNormalizedValue(values, nullColumns, "MAGSYSTEM",
                       normalizeMagsystemValue(valueForKeyCaseInsensitive(values, "MAGSYSTEM")),
                       &result.changedColumns);
  }

  if (containsKeyCaseInsensitive(values, "MAGFILTER")) {
    setNormalizedValue(values, nullColumns, "MAGFILTER",
                       normalizeMagfilterValue(valueForKeyCaseInsensitive(values, "MAGFILTER")),
                       &result.changedColumns);
  }

  if (containsKeyCaseInsensitive(values, "NOTBEFORE")) {
    const QString val = valueForKeyCaseInsensitive(values, "NOTBEFORE").trimmed();
    if (val.isEmpty()) {
      setNormalizedValue(values, nullColumns, "NOTBEFORE", kDefaultNotBefore, &result.changedColumns);
    }
  }

  const bool hasWrangeLow = containsKeyCaseInsensitive(values, "WRANGE_LOW");
  const bool hasWrangeHigh = containsKeyCaseInsensitive(values, "WRANGE_HIGH");
  if (hasWrangeLow || hasWrangeHigh) {
    const QString channel = valueForKeyCaseInsensitive(values, "CHANNEL");
    double low = 0.0;
    double high = 0.0;
    bool lowOk = hasWrangeLow && parseDouble(valueForKeyCaseInsensitive(values, "WRANGE_LOW"), &low);
    bool highOk = hasWrangeHigh && parseDouble(valueForKeyCaseInsensitive(values, "WRANGE_HIGH"), &high);

    if (!lowOk || !highOk || high <= low) {
      const auto def = defaultWrangeForChannel(channel);
      low = def.first;
      high = def.second;
    }

    if (hasWrangeLow) {
      setNormalizedValue(values, nullColumns, "WRANGE_LOW", formatNumber(low),
                         &result.changedColumns);
    }
    if (hasWrangeHigh) {
      setNormalizedValue(values, nullColumns, "WRANGE_HIGH", formatNumber(high),
                         &result.changedColumns);
    }
  }

  if (values.contains("EXPTIME") && isCalib) {
    const QString exptime = valueForKeyCaseInsensitive(values, "EXPTIME");
    if (!exptime.toUpper().startsWith("SET")) {
      setNormalizedValue(values, nullColumns, "EXPTIME", kDefaultExptime, &result.changedColumns);
    }
  }
  if (containsKeyCaseInsensitive(values, "SLITWIDTH") && isCalib) {
    const QString slitwidth = valueForKeyCaseInsensitive(values, "SLITWIDTH");
    if (!slitwidth.toUpper().startsWith("SET")) {
      setNormalizedValue(values, nullColumns, "SLITWIDTH", kDefaultSlitwidth, &result.changedColumns);
    }
  }

  if (containsKeyCaseInsensitive(values, "OTMslitwidth")) {
    const QString slitwidth = valueForKeyCaseInsensitive(values, "SLITWIDTH");
    double numeric = 0.0;
    bool haveSet = extractSetNumeric(slitwidth, &numeric);
    QString current = valueForKeyCaseInsensitive(values, "OTMslitwidth").trimmed();
    if (current.isEmpty()) {
      const double toUse = haveSet ? numeric : kDefaultOtmSlitwidth;
      setNormalizedValue(values, nullColumns, "OTMslitwidth", formatNumber(toUse),
                         &result.changedColumns);
    }
  }

  if (containsKeyCaseInsensitive(values, "OTMexpt")) {
    const QString exptime = valueForKeyCaseInsensitive(values, "EXPTIME");
    double numeric = 0.0;
    if (extractSetNumeric(exptime, &numeric)) {
      setNormalizedValue(values, nullColumns, "OTMexpt", formatNumber(numeric),
                         &result.changedColumns);
    }
  }

  return result;
}

struct ColumnMeta {
  QString name;
  QString type;
  QString key;
  QVariant defaultValue;
  QString extra;
  bool nullable = true;

  bool isPrimaryKey() const { return key.contains("PRI", Qt::CaseInsensitive); }
  bool isAutoIncrement() const { return extra.contains("auto_increment", Qt::CaseInsensitive); }
  bool hasDefault() const { return !defaultValue.isNull(); }
  bool isDateTime() const {
    const QString t = type.toLower();
    return t.contains("timestamp") || t.contains("datetime");
  }
};

struct DbConfig {
  QString host;
  int port = 33060;
  QString user;
  QString pass;
  QString schema;
  QString tableTargets;
  QString tableSets;

  bool isComplete() const {
    return !host.isEmpty() && !user.isEmpty() && !schema.isEmpty() &&
           !tableTargets.isEmpty() && !tableSets.isEmpty();
  }
};

static QString stripInlineComment(const QString &line) {
  int idx = line.indexOf('#');
  if (idx >= 0) {
    return line.left(idx).trimmed();
  }
  return line.trimmed();
}

static DbConfig loadConfigFile(const QString &path) {
  DbConfig cfg;
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return cfg;
  }
  QTextStream in(&file);
  while (!in.atEnd()) {
    QString line = in.readLine().trimmed();
    if (line.isEmpty() || line.startsWith('#')) {
      continue;
    }
    line = stripInlineComment(line);
    int eq = line.indexOf('=');
    if (eq <= 0) {
      continue;
    }
    const QString key = line.left(eq).trimmed();
    const QString value = line.mid(eq + 1).trimmed();
    if (key == "DB_HOST") cfg.host = value;
    else if (key == "DB_PORT") cfg.port = value.toInt();
    else if (key == "DB_USER") cfg.user = value;
    else if (key == "DB_PASS") cfg.pass = value;
    else if (key == "DB_SCHEMA") cfg.schema = value;
    else if (key == "DB_ACTIVE") cfg.tableTargets = value.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts).value(0);
    else if (key == "DB_SETS") cfg.tableSets = value.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts).value(0);
  }
  return cfg;
}

static QString detectDefaultConfigPath() {
  if (qEnvironmentVariableIsSet("NGPS_CONFIG")) {
    return qEnvironmentVariable("NGPS_CONFIG");
  }
  if (qEnvironmentVariableIsSet("NGPS_ROOT")) {
    const QString root = qEnvironmentVariable("NGPS_ROOT");
    const QString candidate = QDir(root).filePath("Config/sequencerd.cfg");
    if (QFile::exists(candidate)) return candidate;
  }
  const QString appDir = QCoreApplication::applicationDirPath();
  QDir dir(appDir);
  for (int i = 0; i < 6; ++i) {
    const QString candidate = dir.filePath("Config/sequencerd.cfg");
    if (QFile::exists(candidate)) return candidate;
    if (!dir.cdUp()) break;
  }
  const QString cwd = QDir::currentPath();
  const QString direct = QDir(cwd).filePath("Config/sequencerd.cfg");
  if (QFile::exists(direct)) return direct;
  const QString upOne = QDir(cwd).filePath("../Config/sequencerd.cfg");
  if (QFile::exists(upOne)) return upOne;
  return QString();
}

static QString inferNgpsRootFromConfig(const QString &configPath) {
  QFileInfo info(configPath);
  if (!info.exists()) return QString();
  QDir dir = info.dir();
  if (dir.dirName().toLower() == "config") {
    dir.cdUp();
    return dir.absolutePath();
  }
  return info.absoluteDir().absolutePath();
}

static QVariant mysqlValueToVariant(const mysqlx::Value &value) {
  using mysqlx::Value;
  switch (value.getType()) {
    case Value::VNULL:
      return QVariant();
    case Value::INT64:
      return QVariant::fromValue<qlonglong>(value.get<int64_t>());
    case Value::UINT64:
      return QVariant::fromValue<qulonglong>(value.get<uint64_t>());
    case Value::FLOAT:
      return QVariant::fromValue<double>(value.get<float>());
    case Value::DOUBLE:
      return QVariant::fromValue<double>(value.get<double>());
    case Value::BOOL:
      return QVariant::fromValue<bool>(value.get<bool>());
    case Value::STRING:
      return QString::fromStdString(value.get<std::string>());
    case Value::RAW: {
      mysqlx::bytes raw = value.get<mysqlx::bytes>();
      QByteArray bytes(reinterpret_cast<const char *>(raw.first),
                       static_cast<int>(raw.second));
      bool printable = true;
      for (unsigned char ch : bytes) {
        if (ch == 0) { printable = false; break; }
        if (!std::isprint(ch) && !std::isspace(ch)) { printable = false; break; }
      }
      if (printable) {
        return QString::fromUtf8(bytes);
      }
      return bytes;
    }
    case Value::ARRAY: {
      std::ostringstream stream;
      stream << value;
      return QString::fromStdString(stream.str());
    }
    case Value::DOCUMENT:
      return QString::fromStdString(value.get<std::string>());
  }
  return QVariant();
}

static QString displayForVariant(const QVariant &value, bool isNull) {
  if (isNull) {
    return "NULL";
  }
  if (value.userType() == QMetaType::QByteArray) {
    return value.toByteArray().toHex(' ').toUpper();
  }
  return value.toString();
}

static bool isLongMessage(const QString &text) {
  const int maxChars = 600;
  const int maxLines = 20;
  return text.size() > maxChars || text.count('\n') > maxLines;
}

static void showMessageDialog(QWidget *parent,
                              QMessageBox::Icon icon,
                              const QString &title,
                              const QString &text) {
  if (!isLongMessage(text)) {
    QMessageBox box(parent);
    box.setIcon(icon);
    box.setWindowTitle(title);
    box.setText(text);
    box.setStandardButtons(QMessageBox::Ok);
    box.exec();
    return;
  }

  QDialog dialog(parent);
  dialog.setWindowTitle(title);
  dialog.setModal(true);

  QVBoxLayout *layout = new QVBoxLayout(&dialog);
  QHBoxLayout *header = new QHBoxLayout();

  QLabel *iconLabel = new QLabel(&dialog);
  QIcon iconObj = QApplication::style()->standardIcon(
      icon == QMessageBox::Warning  ? QStyle::SP_MessageBoxWarning :
      icon == QMessageBox::Critical ? QStyle::SP_MessageBoxCritical :
                                      QStyle::SP_MessageBoxInformation);
  iconLabel->setPixmap(iconObj.pixmap(32, 32));
  header->addWidget(iconLabel);

  QLabel *titleLabel = new QLabel("Message is long. See details below.", &dialog);
  titleLabel->setWordWrap(true);
  header->addWidget(titleLabel, 1);
  layout->addLayout(header);

  QPlainTextEdit *details = new QPlainTextEdit(text, &dialog);
  details->setReadOnly(true);
  details->setLineWrapMode(QPlainTextEdit::NoWrap);
  layout->addWidget(details, 1);

  QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok, &dialog);
  QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
  layout->addWidget(buttons);

  QScreen *screen = parent ? parent->screen() : QGuiApplication::primaryScreen();
  QRect avail = screen ? screen->availableGeometry() : QRect(0, 0, 1200, 800);
  dialog.resize(int(avail.width() * 0.8), int(avail.height() * 0.8));
  dialog.exec();
}

static void showWarning(QWidget *parent, const QString &title, const QString &text) {
  showMessageDialog(parent, QMessageBox::Warning, title, text);
}

static void showInfo(QWidget *parent, const QString &title, const QString &text) {
  showMessageDialog(parent, QMessageBox::Information, title, text);
}

static void showError(QWidget *parent, const QString &title, const QString &text) {
  showMessageDialog(parent, QMessageBox::Critical, title, text);
}

class ReorderTableView : public QTableView {
  Q_OBJECT
public:
  explicit ReorderTableView(QWidget *parent = nullptr) : QTableView(parent) {}
  void setAllowDeleteShortcut(bool enabled) { allowDeleteShortcut_ = enabled; }
  void setIconHitTest(const std::function<bool(const QModelIndex &, const QPoint &)> &fn) {
    iconHitTest_ = fn;
  }

signals:
  void dragSwapRequested(int sourceRow, int targetRow);
  void cellClicked(const QModelIndex &index, const QPoint &pos);
  void deleteRequested();

protected:
  void paintEvent(QPaintEvent *event) override {
    QTableView::paintEvent(event);
    if (!dragging_ || dragTargetRow_ < 0 || !model()) return;
    const QModelIndex idx = model()->index(dragTargetRow_, 0);
    if (!idx.isValid()) return;
    QRect rowRect = visualRect(idx);
    if (!rowRect.isValid()) return;
    QPainter painter(viewport());
    QColor lineColor(90, 160, 255);
    QPen pen(lineColor, 2);
    painter.setPen(pen);
    const int y = rowRect.bottom();
    painter.drawLine(0, y, viewport()->width(), y);
  }

  void mousePressEvent(QMouseEvent *event) override {
    if (event->button() == Qt::LeftButton) {
      pressPos_ = event->pos();
      pressRow_ = indexAt(pressPos_).row();
      dragging_ = false;
      dragTargetRow_ = -1;
      const QModelIndex index = indexAt(event->pos());
      if (index.isValid() && iconHitTest_ && iconHitTest_(index, event->pos())) {
        suppressReleaseClick_ = true;
        suppressDoubleClick_ = true;
        if (selectionModel()) {
          selectionModel()->setCurrentIndex(
              index, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
        }
        emit cellClicked(index, event->pos());
        event->accept();
        return;
      }
    }
    QTableView::mousePressEvent(event);
  }

  void mouseMoveEvent(QMouseEvent *event) override {
    if (!(event->buttons() & Qt::LeftButton) || pressRow_ < 0) {
      QTableView::mouseMoveEvent(event);
      return;
    }
    if (!dragging_) {
      if ((event->pos() - pressPos_).manhattanLength() < QApplication::startDragDistance()) {
        QTableView::mouseMoveEvent(event);
        return;
      }
      dragging_ = true;
      setCursor(Qt::ClosedHandCursor);
    }
    int targetRow = indexAt(event->pos()).row();
    if (targetRow < 0 && model()) {
      if (event->pos().y() < 0) {
        targetRow = 0;
      } else if (event->pos().y() > viewport()->height()) {
        targetRow = model()->rowCount() - 1;
      }
    }
    if (targetRow != dragTargetRow_) {
      dragTargetRow_ = targetRow;
      viewport()->update();
    }
    QTableView::mouseMoveEvent(event);
  }

  void mouseReleaseEvent(QMouseEvent *event) override {
    if (dragging_ && event->button() == Qt::LeftButton) {
      int targetRow = indexAt(event->pos()).row();
      if (targetRow < 0 && model()) {
        targetRow = model()->rowCount() - 1;
      }
      if (pressRow_ >= 0 && targetRow >= 0 && pressRow_ != targetRow) {
        emit dragSwapRequested(pressRow_, targetRow);
      }
    }
    dragging_ = false;
    pressRow_ = -1;
    dragTargetRow_ = -1;
    viewport()->update();
    unsetCursor();
    QTableView::mouseReleaseEvent(event);
    if (event->button() == Qt::LeftButton) {
      if (suppressReleaseClick_) {
        suppressReleaseClick_ = false;
        return;
      }
      const QModelIndex index = indexAt(event->pos());
      if (index.isValid()) {
        emit cellClicked(index, event->pos());
      }
    }
  }

  void mouseDoubleClickEvent(QMouseEvent *event) override {
    if (suppressDoubleClick_) {
      suppressDoubleClick_ = false;
      event->accept();
      return;
    }
    const QModelIndex index = indexAt(event->pos());
    if (event->button() == Qt::LeftButton && index.isValid() && iconHitTest_) {
      if (iconHitTest_(index, event->pos())) {
        event->accept();
        return;
      }
    }
    QTableView::mouseDoubleClickEvent(event);
  }

  void keyPressEvent(QKeyEvent *event) override {
    if (allowDeleteShortcut_ &&
        (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace)) {
      if (state() != QAbstractItemView::EditingState) {
        emit deleteRequested();
        event->accept();
        return;
      }
    }
    QTableView::keyPressEvent(event);
  }

private:
  QPoint pressPos_;
  int pressRow_ = -1;
  bool dragging_ = false;
  int dragTargetRow_ = -1;
  bool allowDeleteShortcut_ = false;
  std::function<bool(const QModelIndex &, const QPoint &)> iconHitTest_;
  bool suppressReleaseClick_ = false;
  bool suppressDoubleClick_ = false;
};

class DbClient {
public:
  bool connect(const DbConfig &cfg, QString *error) {
    try {
      session_ = std::make_unique<mysqlx::Session>(
          mysqlx::SessionOption::HOST, cfg.host.toStdString(),
          mysqlx::SessionOption::PORT, cfg.port,
          mysqlx::SessionOption::USER, cfg.user.toStdString(),
          mysqlx::SessionOption::PWD, cfg.pass.toStdString(),
          mysqlx::SessionOption::DB, cfg.schema.toStdString());
      logLine(QString("CONNECTED %1@%2:%3/%4")
                  .arg(cfg.user)
                  .arg(cfg.host)
                  .arg(cfg.port)
                  .arg(cfg.schema));
      schemaName_ = cfg.schema;
      connected_ = true;
      return true;
    } catch (const mysqlx::Error &e) {
      if (error) *error = QString::fromStdString(e.what());
    } catch (...) {
      if (error) *error = "Unknown database error";
    }
    connected_ = false;
    return false;
  }

  void close() {
    if (session_) {
      try {
        session_->close();
        logLine("DISCONNECTED");
      } catch (...) {
      }
    }
    session_.reset();
    connected_ = false;
  }

  bool isOpen() const { return connected_ && session_ != nullptr; }

  bool loadColumns(const QString &tableName, QList<ColumnMeta> &columns, QString *error) {
    columns.clear();
    if (!isOpen()) {
      if (error) *error = "Not connected";
      return false;
    }
    try {
      const std::string sql = "SHOW COLUMNS FROM `" + tableName.toStdString() + "`";
      logSql(QString::fromStdString(sql), {});
      mysqlx::SqlResult result = session_->sql(sql).execute();
      for (mysqlx::Row row : result) {
        if (row.colCount() < 6) continue;
        ColumnMeta meta;
        meta.name = QString::fromStdString(row[0].get<std::string>());
        meta.type = QString::fromStdString(row[1].get<std::string>());
        meta.nullable = QString::fromStdString(row[2].get<std::string>()).toUpper() == "YES";
        meta.key = QString::fromStdString(row[3].get<std::string>());
        if (row[4].getType() == mysqlx::Value::VNULL) {
          meta.defaultValue = QVariant();
        } else {
          meta.defaultValue = mysqlValueToVariant(row[4]);
        }
        meta.extra = QString::fromStdString(row[5].get<std::string>());
        columns.append(meta);
      }
      return !columns.isEmpty();
    } catch (const mysqlx::Error &e) {
      if (error) *error = QString::fromStdString(e.what());
    } catch (...) {
      if (error) *error = "Failed to read columns";
    }
    return false;
  }

  bool fetchRows(const QString &tableName,
                 const QList<ColumnMeta> &columns,
                 const QString &fixedFilterColumn,
                 const QString &fixedFilterValue,
                 const QString &searchColumn,
                 const QString &searchValue,
                 const QString &orderByColumn,
                 QVector<QVector<QVariant>> &rows,
                 QString *error) {
    rows.clear();
    if (!isOpen()) {
      if (error) *error = "Not connected";
      return false;
    }
    try {
      QStringList selectCols;
      selectCols.reserve(columns.size());
      for (const ColumnMeta &meta : columns) {
        if (meta.isDateTime()) {
          selectCols << QString("CAST(`%1` AS CHAR) AS `%1`").arg(meta.name);
        } else {
          selectCols << QString("`%1`").arg(meta.name);
        }
      }
      QString sql = QString("SELECT %1 FROM `%2`").arg(selectCols.join(", "), tableName);
      QStringList conditions;
      QList<QString> binds;
      if (!fixedFilterColumn.isEmpty() && !fixedFilterValue.isEmpty()) {
        conditions << QString("`%1` = ?").arg(fixedFilterColumn);
        binds << fixedFilterValue;
      }
      if (!searchColumn.isEmpty() && !searchValue.isEmpty()) {
        conditions << QString("LOWER(`%1`) LIKE ?").arg(searchColumn);
        binds << QString("%%%1%%").arg(searchValue.toLower());
      }
      if (!conditions.isEmpty()) {
        sql += " WHERE " + conditions.join(" AND ");
      }
      if (!orderByColumn.isEmpty()) {
        sql += QString(" ORDER BY `%1`").arg(orderByColumn);
      }
      mysqlx::SqlStatement stmt = session_->sql(sql.toStdString());
      logSql(sql, binds);
      for (const QString &bind : binds) {
        stmt.bind(bind.toStdString());
      }
      mysqlx::SqlResult result = stmt.execute();
      for (mysqlx::Row row : result) {
        QVector<QVariant> rowValues;
        rowValues.reserve(static_cast<int>(row.colCount()));
        for (mysqlx::col_count_t i = 0; i < row.colCount(); ++i) {
          rowValues.append(mysqlValueToVariant(row[i]));
        }
        rows.append(rowValues);
      }
      return true;
    } catch (const mysqlx::Error &e) {
      if (error) *error = QString::fromStdString(e.what());
    } catch (...) {
      if (error) *error = "Failed to read rows";
    }
    return false;
  }

  bool insertRecord(const QString &tableName,
                    const QList<ColumnMeta> &columns,
                    const QVariantMap &values,
                    const QSet<QString> &nullColumns,
                    QString *error) {
    if (!isOpen()) {
      if (error) *error = "Not connected";
      return false;
    }

    QStringList cols;
    QStringList vals;
    QList<QVariant> binds;

    for (const ColumnMeta &meta : columns) {
      QVariant raw;
      bool hasValue = values.contains(meta.name);
      if (!hasValue) {
        const QString actualKey = findKeyCaseInsensitive(values, meta.name);
        if (values.contains(actualKey)) {
          raw = values.value(actualKey);
          hasValue = true;
        }
      } else {
        raw = values.value(meta.name);
      }
      const QString text = raw.toString();
      const bool hasText = !text.isEmpty();

      if (meta.isAutoIncrement() && !hasText && !setContainsCaseInsensitive(nullColumns, meta.name)) {
        continue;
      }

      if (setContainsCaseInsensitive(nullColumns, meta.name)) {
        cols << QString("`%1`").arg(meta.name);
        vals << "NULL";
        continue;
      }

      if (!hasText) {
        if (meta.hasDefault()) {
          continue;
        }
        if (meta.isDateTime()) {
          cols << QString("`%1`").arg(meta.name);
          vals << "?";
          binds << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");
          continue;
        }
        if (meta.nullable) {
          cols << QString("`%1`").arg(meta.name);
          vals << "NULL";
          continue;
        }
        if (error) *error = QString("Missing required value for %1").arg(meta.name);
        return false;
      }

      cols << QString("`%1`").arg(meta.name);
      vals << "?";
      binds << text;
    }

    if (cols.isEmpty()) {
      if (error) *error = "No values to insert";
      return false;
    }

    const QString sql = QString("INSERT INTO `%1` (%2) VALUES (%3)")
                            .arg(tableName)
                            .arg(cols.join(", "))
                            .arg(vals.join(", "));
    try {
      logLine("START TRANSACTION");
      logSql(sql, toStringList(binds));
      session_->startTransaction();
      mysqlx::SqlStatement stmt = session_->sql(sql.toStdString());
      for (const QVariant &val : binds) {
        stmt.bind(val.toString().toStdString());
      }
      stmt.execute();
      session_->commit();
      logLine("COMMIT");
      return true;
    } catch (const mysqlx::Error &e) {
      try { session_->rollback(); logLine("ROLLBACK"); } catch (...) {}
      if (error) *error = QString::fromStdString(e.what());
    } catch (...) {
      try { session_->rollback(); logLine("ROLLBACK"); } catch (...) {}
      if (error) *error = "Insert failed";
    }
    return false;
  }

  bool updateRecord(const QString &tableName,
                    const QList<ColumnMeta> &columns,
                    const QVariantMap &values,
                    const QSet<QString> &nullColumns,
                    const QVariantMap &keyValues,
                    QString *error) {
    if (!isOpen()) {
      if (error) *error = "Not connected";
      return false;
    }

    QStringList sets;
    QList<QVariant> binds;

    for (const ColumnMeta &meta : columns) {
      QVariant raw;
      bool hasValue = values.contains(meta.name);
      if (!hasValue) {
        const QString actualKey = findKeyCaseInsensitive(values, meta.name);
        if (values.contains(actualKey)) {
          raw = values.value(actualKey);
          hasValue = true;
        }
      } else {
        raw = values.value(meta.name);
      }
      const QString text = raw.toString();
      const bool hasText = !text.isEmpty();

      if (setContainsCaseInsensitive(nullColumns, meta.name)) {
        sets << QString("`%1`=NULL").arg(meta.name);
        continue;
      }

      if (!hasText) {
        if (meta.nullable) {
          sets << QString("`%1`=?").arg(meta.name);
          binds << QString();
          continue;
        }
        if (error) *error = QString("Missing required value for %1").arg(meta.name);
        return false;
      }

      sets << QString("`%1`=?").arg(meta.name);
      binds << text;
    }

    if (sets.isEmpty()) {
      if (error) *error = "No values to update";
      return false;
    }

    QStringList where;
    for (auto it = keyValues.begin(); it != keyValues.end(); ++it) {
      where << QString("`%1`=?").arg(it.key());
      binds << it.value();
    }

    if (where.isEmpty()) {
      if (error) *error = "Missing primary key values";
      return false;
    }

    const QString sql = QString("UPDATE `%1` SET %2 WHERE %3")
                            .arg(tableName)
                            .arg(sets.join(", "))
                            .arg(where.join(" AND "));
    try {
      logLine("START TRANSACTION");
      logSql(sql, toStringList(binds));
      session_->startTransaction();
      mysqlx::SqlStatement stmt = session_->sql(sql.toStdString());
      for (const QVariant &val : binds) {
        stmt.bind(val.toString().toStdString());
      }
      stmt.execute();
      session_->commit();
      logLine("COMMIT");
      return true;
    } catch (const mysqlx::Error &e) {
      try { session_->rollback(); logLine("ROLLBACK"); } catch (...) {}
      if (error) *error = QString::fromStdString(e.what());
    } catch (...) {
      try { session_->rollback(); logLine("ROLLBACK"); } catch (...) {}
      if (error) *error = "Update failed";
    }
    return false;
  }

  bool updateColumnByKeyBatch(const QString &tableName,
                              const QString &columnName,
                              const QList<QVariantMap> &keyValuesList,
                              const QList<QVariant> &values,
                              QString *error) {
    if (!isOpen()) {
      if (error) *error = "Not connected";
      return false;
    }
    if (keyValuesList.size() != values.size()) {
      if (error) *error = "Batch size mismatch";
      return false;
    }
    try {
      logLine("START TRANSACTION");
      session_->startTransaction();
      for (int i = 0; i < keyValuesList.size(); ++i) {
        const QVariantMap &keyValues = keyValuesList.at(i);
        if (keyValues.isEmpty()) {
          session_->rollback();
          if (error) *error = "Missing primary key values";
          return false;
        }
        QStringList where;
        QList<QString> binds;
        binds << values.at(i).toString();
        for (auto it = keyValues.begin(); it != keyValues.end(); ++it) {
          where << QString("`%1`=?").arg(it.key());
          binds << it.value().toString();
        }
        const QString sql = QString("UPDATE `%1` SET `%2`=? WHERE %3")
                                .arg(tableName, columnName, where.join(" AND "));
        mysqlx::SqlStatement stmt = session_->sql(sql.toStdString());
        logSql(sql, binds);
        for (const QString &bind : binds) {
          stmt.bind(bind.toStdString());
        }
        stmt.execute();
      }
      session_->commit();
      logLine("COMMIT");
      return true;
    } catch (const mysqlx::Error &e) {
      try { session_->rollback(); logLine("ROLLBACK"); } catch (...) {}
      if (error) *error = QString::fromStdString(e.what());
    } catch (...) {
      try { session_->rollback(); logLine("ROLLBACK"); } catch (...) {}
      if (error) *error = "Batch update failed";
    }
    return false;
  }

  bool updateObsOrderByObservationId(const QString &tableName,
                                     const QList<QVariant> &obsIds,
                                     const QList<QVariant> &orderValues,
                                     QString *error) {
    if (!isOpen()) {
      if (error) *error = "Not connected";
      return false;
    }
    if (obsIds.size() != orderValues.size() || obsIds.isEmpty()) {
      if (error) *error = "Invalid OBS_ORDER update data";
      return false;
    }
    try {
      logLine("START TRANSACTION");
      session_->startTransaction();

      long long maxOrder = 0;
      try {
        const QString maxSql = QString("SELECT MAX(`OBS_ORDER`) FROM `%1`").arg(tableName);
        logSql(maxSql, {});
        mysqlx::SqlResult maxRes = session_->sql(maxSql.toStdString()).execute();
        mysqlx::Row maxRow = maxRes.fetchOne();
        if (maxRow && maxRow.colCount() > 0 && maxRow[0].getType() != mysqlx::Value::VNULL) {
          maxOrder = maxRow[0].get<int64_t>();
        }
      } catch (...) {
        maxOrder = 0;
      }
      const long long offset = maxOrder + 1000;

      QStringList inParts;
      inParts.reserve(obsIds.size());
      for (int i = 0; i < obsIds.size(); ++i) {
        inParts << "?";
      }

      const QString bumpSql = QString("UPDATE `%1` SET `OBS_ORDER` = `OBS_ORDER` + ? WHERE `OBSERVATION_ID` IN (%2)")
                                  .arg(tableName, inParts.join(", "));
      mysqlx::SqlStatement bumpStmt = session_->sql(bumpSql.toStdString());
      QList<QString> bumpBinds;
      bumpBinds << QString::number(offset);
      for (const QVariant &obsId : obsIds) bumpBinds << obsId.toString();
      logSql(bumpSql, bumpBinds);
      bumpStmt.bind(QString::number(offset).toStdString());
      for (const QVariant &obsId : obsIds) {
        bumpStmt.bind(obsId.toString().toStdString());
      }
      bumpStmt.execute();

      QStringList caseParts;
      caseParts.reserve(obsIds.size());
      for (int i = 0; i < obsIds.size(); ++i) {
        caseParts << "WHEN ? THEN ?";
      }
      const QString finalSql = QString("UPDATE `%1` SET `OBS_ORDER` = CASE `OBSERVATION_ID` %2 END WHERE `OBSERVATION_ID` IN (%3)")
                                   .arg(tableName, caseParts.join(" "), inParts.join(", "));
      mysqlx::SqlStatement finalStmt = session_->sql(finalSql.toStdString());
      QList<QString> finalBinds;
      for (int i = 0; i < obsIds.size(); ++i) {
        finalBinds << obsIds.at(i).toString() << orderValues.at(i).toString();
      }
      for (const QVariant &obsId : obsIds) {
        finalBinds << obsId.toString();
      }
      logSql(finalSql, finalBinds);
      for (int i = 0; i < obsIds.size(); ++i) {
        finalStmt.bind(obsIds.at(i).toString().toStdString());
        finalStmt.bind(orderValues.at(i).toString().toStdString());
      }
      for (const QVariant &obsId : obsIds) {
        finalStmt.bind(obsId.toString().toStdString());
      }
      finalStmt.execute();
      session_->commit();
      logLine("COMMIT");
      return true;
    } catch (const mysqlx::Error &e) {
      try { session_->rollback(); logLine("ROLLBACK"); } catch (...) {}
      if (error) *error = QString::fromStdString(e.what());
    } catch (...) {
      try { session_->rollback(); logLine("ROLLBACK"); } catch (...) {}
      if (error) *error = "OBS_ORDER update failed";
    }
    return false;
  }

  bool updateObsOrderByCompositeKey(const QString &tableName,
                                    const QList<QVariant> &obsIds,
                                    const QList<QVariant> &slitWidths,
                                    const QList<QVariant> &orderValues,
                                    QString *error) {
    if (!isOpen()) {
      if (error) *error = "Not connected";
      return false;
    }
    if (obsIds.size() != orderValues.size() || obsIds.size() != slitWidths.size() || obsIds.isEmpty()) {
      if (error) *error = "Invalid OBS_ORDER update data";
      return false;
    }
    try {
      logLine("START TRANSACTION");
      session_->startTransaction();

      long long maxOrder = 0;
      try {
        const QString maxSql = QString("SELECT MAX(`OBS_ORDER`) FROM `%1`").arg(tableName);
        logSql(maxSql, {});
        mysqlx::SqlResult maxRes = session_->sql(maxSql.toStdString()).execute();
        mysqlx::Row maxRow = maxRes.fetchOne();
        if (maxRow && maxRow.colCount() > 0 && maxRow[0].getType() != mysqlx::Value::VNULL) {
          maxOrder = maxRow[0].get<int64_t>();
        }
      } catch (...) {
        maxOrder = 0;
      }
      const long long offset = maxOrder + 1000;

      QStringList tupleParts;
      tupleParts.reserve(obsIds.size());
      for (int i = 0; i < obsIds.size(); ++i) {
        tupleParts << "(?, ?)";
      }

      const QString bumpSql = QString("UPDATE `%1` SET `OBS_ORDER` = `OBS_ORDER` + ? "
                                      "WHERE (`OBSERVATION_ID`, `OTMslitwidth`) IN (%2)")
                                  .arg(tableName, tupleParts.join(", "));
      mysqlx::SqlStatement bumpStmt = session_->sql(bumpSql.toStdString());
      QList<QString> bumpBinds;
      bumpBinds << QString::number(offset);
      for (int i = 0; i < obsIds.size(); ++i) {
        bumpBinds << obsIds.at(i).toString() << slitWidths.at(i).toString();
      }
      logSql(bumpSql, bumpBinds);
      bumpStmt.bind(QString::number(offset).toStdString());
      for (int i = 0; i < obsIds.size(); ++i) {
        bumpStmt.bind(obsIds.at(i).toString().toStdString());
        bumpStmt.bind(slitWidths.at(i).toString().toStdString());
      }
      bumpStmt.execute();

      QStringList caseParts;
      caseParts.reserve(obsIds.size());
      for (int i = 0; i < obsIds.size(); ++i) {
        caseParts << "WHEN `OBSERVATION_ID`=? AND `OTMslitwidth`=? THEN ?";
      }
      const QString finalSql = QString("UPDATE `%1` SET `OBS_ORDER` = CASE %2 END "
                                       "WHERE (`OBSERVATION_ID`, `OTMslitwidth`) IN (%3)")
                                   .arg(tableName, caseParts.join(" "), tupleParts.join(", "));
      mysqlx::SqlStatement finalStmt = session_->sql(finalSql.toStdString());
      QList<QString> finalBinds;
      for (int i = 0; i < obsIds.size(); ++i) {
        finalBinds << obsIds.at(i).toString() << slitWidths.at(i).toString()
                   << orderValues.at(i).toString();
      }
      for (int i = 0; i < obsIds.size(); ++i) {
        finalBinds << obsIds.at(i).toString() << slitWidths.at(i).toString();
      }
      logSql(finalSql, finalBinds);
      // CASE bindings
      for (int i = 0; i < obsIds.size(); ++i) {
        finalStmt.bind(obsIds.at(i).toString().toStdString());
        finalStmt.bind(slitWidths.at(i).toString().toStdString());
        finalStmt.bind(orderValues.at(i).toString().toStdString());
      }
      // WHERE tuple bindings
      for (int i = 0; i < obsIds.size(); ++i) {
        finalStmt.bind(obsIds.at(i).toString().toStdString());
        finalStmt.bind(slitWidths.at(i).toString().toStdString());
      }
      finalStmt.execute();

      session_->commit();
      logLine("COMMIT");
      return true;
    } catch (const mysqlx::Error &e) {
      try { session_->rollback(); logLine("ROLLBACK"); } catch (...) {}
      if (error) *error = QString::fromStdString(e.what());
    } catch (...) {
      try { session_->rollback(); logLine("ROLLBACK"); } catch (...) {}
      if (error) *error = "OBS_ORDER update failed";
    }
    return false;
  }

  bool moveObsOrder(const QString &tableName,
                    int setId,
                    const QVariant &obsId,
                    const QVariant &slitWidth,
                    int oldPos,
                    int newPos,
                    QString *error) {
    if (!isOpen()) {
      if (error) *error = "Not connected";
      return false;
    }
    if (oldPos == newPos) return true;
    try {
      logLine("START TRANSACTION");
      session_->startTransaction();
      if (newPos < oldPos) {
        const QString shiftSql = QString("UPDATE `%1` SET `OBS_ORDER` = `OBS_ORDER` + 1 "
                                         "WHERE `SET_ID` = ? AND `OBS_ORDER` >= ? AND `OBS_ORDER` < ?")
                                     .arg(tableName);
        mysqlx::SqlStatement shiftStmt = session_->sql(shiftSql.toStdString());
        logSql(shiftSql, {QString::number(setId), QString::number(newPos), QString::number(oldPos)});
        shiftStmt.bind(QString::number(setId).toStdString());
        shiftStmt.bind(QString::number(newPos).toStdString());
        shiftStmt.bind(QString::number(oldPos).toStdString());
        shiftStmt.execute();
      } else {
        const QString shiftSql = QString("UPDATE `%1` SET `OBS_ORDER` = `OBS_ORDER` - 1 "
                                         "WHERE `SET_ID` = ? AND `OBS_ORDER` <= ? AND `OBS_ORDER` > ?")
                                     .arg(tableName);
        mysqlx::SqlStatement shiftStmt = session_->sql(shiftSql.toStdString());
        logSql(shiftSql, {QString::number(setId), QString::number(newPos), QString::number(oldPos)});
        shiftStmt.bind(QString::number(setId).toStdString());
        shiftStmt.bind(QString::number(newPos).toStdString());
        shiftStmt.bind(QString::number(oldPos).toStdString());
        shiftStmt.execute();
      }

      const QString updateSql = QString("UPDATE `%1` SET `OBS_ORDER` = ? "
                                        "WHERE `OBSERVATION_ID` = ? AND `OTMslitwidth` = ?")
                                    .arg(tableName);
      mysqlx::SqlStatement updateStmt = session_->sql(updateSql.toStdString());
      logSql(updateSql, {QString::number(newPos), obsId.toString(), slitWidth.toString()});
      updateStmt.bind(QString::number(newPos).toStdString());
      updateStmt.bind(obsId.toString().toStdString());
      updateStmt.bind(slitWidth.toString().toStdString());
      updateStmt.execute();

      session_->commit();
      logLine("COMMIT");
      return true;
    } catch (const mysqlx::Error &e) {
      try { session_->rollback(); logLine("ROLLBACK"); } catch (...) {}
      if (error) *error = QString::fromStdString(e.what());
    } catch (...) {
      try { session_->rollback(); logLine("ROLLBACK"); } catch (...) {}
      if (error) *error = "OBS_ORDER move failed";
    }
    return false;
  }

  bool swapTargets(const QString &tableName,
                   const QVariant &obsIdX,
                   const QVariant &orderX,
                   const QVariant &obsIdY,
                   const QVariant &orderY,
                   QString *error) {
    if (!isOpen()) {
      if (error) *error = "Not connected";
      return false;
    }
    if (obsIdX == obsIdY) return true;
    try {
      logLine("START TRANSACTION");
      session_->startTransaction();

      long long maxObsId = 0;
      long long maxOrder = 0;
      try {
        const QString maxObsSql = QString("SELECT MAX(`OBSERVATION_ID`) FROM `%1`").arg(tableName);
        logSql(maxObsSql, {});
        mysqlx::SqlResult maxObsRes = session_->sql(maxObsSql.toStdString()).execute();
        mysqlx::Row maxObsRow = maxObsRes.fetchOne();
        if (maxObsRow && maxObsRow.colCount() > 0 && maxObsRow[0].getType() != mysqlx::Value::VNULL) {
          maxObsId = maxObsRow[0].get<int64_t>();
        }
      } catch (...) {
        maxObsId = 0;
      }
      try {
        const QString maxOrderSql = QString("SELECT MAX(`OBS_ORDER`) FROM `%1`").arg(tableName);
        logSql(maxOrderSql, {});
        mysqlx::SqlResult maxOrderRes = session_->sql(maxOrderSql.toStdString()).execute();
        mysqlx::Row maxOrderRow = maxOrderRes.fetchOne();
        if (maxOrderRow && maxOrderRow.colCount() > 0 && maxOrderRow[0].getType() != mysqlx::Value::VNULL) {
          maxOrder = maxOrderRow[0].get<int64_t>();
        }
      } catch (...) {
        maxOrder = 0;
      }
      const long long tempObsId = maxObsId + 100000;
      const long long tempOrder = maxOrder + 100000;

      const QString bumpSql = QString("UPDATE `%1` SET `OBSERVATION_ID`=?, `OBS_ORDER`=? "
                                      "WHERE `OBSERVATION_ID`=?")
                                  .arg(tableName);
      logSql(bumpSql, {QString::number(tempObsId), QString::number(tempOrder),
                       obsIdY.toString()});
      mysqlx::SqlStatement bumpStmt = session_->sql(bumpSql.toStdString());
      bumpStmt.bind(QString::number(tempObsId).toStdString());
      bumpStmt.bind(QString::number(tempOrder).toStdString());
      bumpStmt.bind(obsIdY.toString().toStdString());
      bumpStmt.execute();

      const QString updateXSql = QString("UPDATE `%1` SET `OBSERVATION_ID`=?, `OBS_ORDER`=? "
                                         "WHERE `OBSERVATION_ID`=?")
                                     .arg(tableName);
      logSql(updateXSql, {obsIdY.toString(), orderY.toString(),
                          obsIdX.toString()});
      mysqlx::SqlStatement updateX = session_->sql(updateXSql.toStdString());
      updateX.bind(obsIdY.toString().toStdString());
      updateX.bind(orderY.toString().toStdString());
      updateX.bind(obsIdX.toString().toStdString());
      updateX.execute();

      const QString updateYSql = QString("UPDATE `%1` SET `OBSERVATION_ID`=?, `OBS_ORDER`=? "
                                         "WHERE `OBSERVATION_ID`=?")
                                     .arg(tableName);
      logSql(updateYSql, {obsIdX.toString(), orderX.toString(),
                          QString::number(tempObsId)});
      mysqlx::SqlStatement updateY = session_->sql(updateYSql.toStdString());
      updateY.bind(obsIdX.toString().toStdString());
      updateY.bind(orderX.toString().toStdString());
      updateY.bind(QString::number(tempObsId).toStdString());
      updateY.execute();

      session_->commit();
      logLine("COMMIT");
      return true;
    } catch (const mysqlx::Error &e) {
      try { session_->rollback(); logLine("ROLLBACK"); } catch (...) {}
      if (error) *error = QString::fromStdString(e.what());
    } catch (...) {
      try { session_->rollback(); logLine("ROLLBACK"); } catch (...) {}
      if (error) *error = "Swap failed";
    }
    return false;
  }

  bool shiftObsOrderAfterDelete(const QString &tableName,
                                int setId,
                                int deletedPos,
                                QString *error) {
    if (!isOpen()) {
      if (error) *error = "Not connected";
      return false;
    }
    try {
      const QString sql = QString("UPDATE `%1` SET `OBS_ORDER` = `OBS_ORDER` - 1 "
                                  "WHERE `SET_ID` = ? AND `OBS_ORDER` > ?")
                              .arg(tableName);
      mysqlx::SqlStatement stmt = session_->sql(sql.toStdString());
      logSql(sql, {QString::number(setId), QString::number(deletedPos)});
      stmt.bind(QString::number(setId).toStdString());
      stmt.bind(QString::number(deletedPos).toStdString());
      stmt.execute();
      return true;
    } catch (const mysqlx::Error &e) {
      if (error) *error = QString::fromStdString(e.what());
    } catch (...) {
      if (error) *error = "OBS_ORDER shift failed";
    }
    return false;
  }

  bool shiftObsOrderForInsert(const QString &tableName,
                              int setId,
                              int startPos,
                              int count,
                              QString *error) {
    if (!isOpen()) {
      if (error) *error = "Not connected";
      return false;
    }
    if (count <= 0) return true;
    try {
      const QString sql = QString("UPDATE `%1` SET `OBS_ORDER` = `OBS_ORDER` + ? "
                                  "WHERE `SET_ID` = ? AND `OBS_ORDER` >= ?")
                              .arg(tableName);
      mysqlx::SqlStatement stmt = session_->sql(sql.toStdString());
      logSql(sql, {QString::number(count), QString::number(setId), QString::number(startPos)});
      stmt.bind(QString::number(count).toStdString());
      stmt.bind(QString::number(setId).toStdString());
      stmt.bind(QString::number(startPos).toStdString());
      stmt.execute();
      return true;
    } catch (const mysqlx::Error &e) {
      if (error) *error = QString::fromStdString(e.what());
    } catch (...) {
      if (error) *error = "OBS_ORDER shift failed";
    }
    return false;
  }

  bool nextObsOrderForSet(const QString &tableName,
                          int setId,
                          int *nextOrder,
                          QString *error) {
    if (!isOpen()) {
      if (error) *error = "Not connected";
      return false;
    }
    if (!nextOrder) {
      if (error) *error = "Missing output parameter";
      return false;
    }
    try {
      const QString sql = QString("SELECT MAX(`OBS_ORDER`) FROM `%1` WHERE `SET_ID` = ?")
                              .arg(tableName);
      logSql(sql, {QString::number(setId)});
      mysqlx::SqlStatement stmt = session_->sql(sql.toStdString());
      stmt.bind(QString::number(setId).toStdString());
      mysqlx::SqlResult res = stmt.execute();
      mysqlx::Row row = res.fetchOne();
      long long maxOrder = 0;
      if (row && row.colCount() > 0 && row[0].getType() != mysqlx::Value::VNULL) {
        maxOrder = row[0].get<int64_t>();
      }
      *nextOrder = static_cast<int>(maxOrder + 1);
      return true;
    } catch (const mysqlx::Error &e) {
      if (error) *error = QString::fromStdString(e.what());
    } catch (...) {
      if (error) *error = "Failed to read OBS_ORDER";
    }
    return false;
  }

  bool deleteRecordByKey(const QString &tableName,
                         const QVariantMap &keyValues,
                         QString *error) {
    if (!isOpen()) {
      if (error) *error = "Not connected";
      return false;
    }
    if (keyValues.isEmpty()) {
      if (error) *error = "Missing primary key values";
      return false;
    }
    try {
      QStringList where;
      QList<QString> binds;
      for (auto it = keyValues.begin(); it != keyValues.end(); ++it) {
        where << QString("`%1`=?").arg(it.key());
        binds << it.value().toString();
      }
      const QString sql = QString("DELETE FROM `%1` WHERE %2")
                              .arg(tableName, where.join(" AND "));
      mysqlx::SqlStatement stmt = session_->sql(sql.toStdString());
      logSql(sql, binds);
      for (const QString &bind : binds) {
        stmt.bind(bind.toStdString());
      }
      stmt.execute();
      return true;
    } catch (const mysqlx::Error &e) {
      if (error) *error = QString::fromStdString(e.what());
    } catch (...) {
      if (error) *error = "Delete failed";
    }
    return false;
  }

  bool deleteRecordsByColumn(const QString &tableName,
                             const QString &columnName,
                             const QVariant &value,
                             QString *error) {
    if (!isOpen()) {
      if (error) *error = "Not connected";
      return false;
    }
    try {
      const QString sql = QString("DELETE FROM `%1` WHERE `%2`=?")
                              .arg(tableName, columnName);
      mysqlx::SqlStatement stmt = session_->sql(sql.toStdString());
      logSql(sql, {value.toString()});
      stmt.bind(value.toString().toStdString());
      stmt.execute();
      return true;
    } catch (const mysqlx::Error &e) {
      if (error) *error = QString::fromStdString(e.what());
    } catch (...) {
      if (error) *error = "Delete failed";
    }
    return false;
  }

  bool fetchSingleValue(const QString &sql,
                        const QList<QVariant> &binds,
                        QVariant *valueOut,
                        QString *error) {
    if (!isOpen()) {
      if (error) *error = "Not connected";
      return false;
    }
    try {
      mysqlx::SqlStatement stmt = session_->sql(sql.toStdString());
      logSql(sql, toStringList(binds));
      for (const QVariant &val : binds) {
        stmt.bind(val.toString().toStdString());
      }
      mysqlx::SqlResult result = stmt.execute();
      for (mysqlx::Row row : result) {
        if (row.colCount() > 0) {
          if (valueOut) *valueOut = mysqlValueToVariant(row[0]);
          return true;
        }
        break;
      }
      if (error) *error = "No results";
    } catch (const mysqlx::Error &e) {
      if (error) *error = QString::fromStdString(e.what());
    } catch (...) {
      if (error) *error = "Query failed";
    }
    return false;
  }

  bool updateColumnsByKey(const QString &tableName,
                          const QVariantMap &updates,
                          const QVariantMap &keyValues,
                          QString *error) {
    if (!isOpen()) {
      if (error) *error = "Not connected";
      return false;
    }
    if (updates.isEmpty()) {
      if (error) *error = "No columns to update";
      return false;
    }
    if (keyValues.isEmpty()) {
      if (error) *error = "Missing primary key values";
      return false;
    }
    try {
      QStringList sets;
      QList<QVariant> binds;
      for (auto it = updates.begin(); it != updates.end(); ++it) {
        const QVariant val = it.value();
        if (!val.isValid() || val.isNull()) {
          sets << QString("`%1`=NULL").arg(it.key());
        } else {
          sets << QString("`%1`=?").arg(it.key());
          binds << val;
        }
      }
      QStringList where;
      for (auto it = keyValues.begin(); it != keyValues.end(); ++it) {
        where << QString("`%1`=?").arg(it.key());
        binds << it.value();
      }
      const QString sql = QString("UPDATE `%1` SET %2 WHERE %3")
                              .arg(tableName)
                              .arg(sets.join(", "))
                              .arg(where.join(" AND "));
      logLine("START TRANSACTION");
      logSql(sql, toStringList(binds));
      session_->startTransaction();
      mysqlx::SqlStatement stmt = session_->sql(sql.toStdString());
      for (const QVariant &val : binds) {
        stmt.bind(val.toString().toStdString());
      }
      stmt.execute();
      session_->commit();
      logLine("COMMIT");
      return true;
    } catch (const mysqlx::Error &e) {
      try { session_->rollback(); logLine("ROLLBACK"); } catch (...) {}
      if (error) *error = QString::fromStdString(e.what());
    } catch (...) {
      try { session_->rollback(); logLine("ROLLBACK"); } catch (...) {}
      if (error) *error = "Update failed";
    }
    return false;
  }

private:
  std::unique_ptr<mysqlx::Session> session_;
  QString schemaName_;
  bool connected_ = false;
  bool logSql_ = true;

  void logLine(const QString &line) const {
    if (!logSql_) return;
    qInfo().noquote() << line;
  }

  static QString quoteBind(const QString &value) {
    QString v = value;
    v.replace('\'', "''");
    return "'" + v + "'";
  }

  static QString expandSql(const QString &sql, const QList<QString> &binds) {
    QString out;
    out.reserve(sql.size() + binds.size() * 4);
    int bindIndex = 0;
    for (QChar ch : sql) {
      if (ch == '?' && bindIndex < binds.size()) {
        out += quoteBind(binds.at(bindIndex++));
      } else {
        out += ch;
      }
    }
    if (bindIndex < binds.size()) {
      QStringList extra;
      for (int i = bindIndex; i < binds.size(); ++i) {
        extra << quoteBind(binds.at(i));
      }
      out += QString(" /* extra binds: %1 */").arg(extra.join(", "));
    }
    return out;
  }

  static QList<QString> toStringList(const QList<QVariant> &vars) {
    QList<QString> out;
    out.reserve(vars.size());
    for (const QVariant &v : vars) {
      out << v.toString();
    }
    return out;
  }

  void logSql(const QString &sql, const QList<QString> &binds) const {
    if (!logSql_) return;
    qInfo().noquote() << "SQL:" << expandSql(sql, binds);
  }
};

class RecordEditorDialog : public QDialog {
  Q_OBJECT
public:
  RecordEditorDialog(const QString &tableName,
                     const QList<ColumnMeta> &columns,
                     const QVariantMap &initialValues,
                     bool isInsert,
                     QWidget *parent = nullptr)
      : QDialog(parent), columns_(columns) {
    setWindowTitle(isInsert ? QString("Add %1").arg(tableName)
                            : QString("Edit %1").arg(tableName));
    setModal(true);

    QVBoxLayout *layout = new QVBoxLayout(this);
    QFormLayout *form = new QFormLayout();

    for (const ColumnMeta &meta : columns_) {
      QWidget *fieldWidget = new QWidget(this);
      QHBoxLayout *fieldLayout = new QHBoxLayout(fieldWidget);
      fieldLayout->setContentsMargins(0, 0, 0, 0);

      QLineEdit *edit = new QLineEdit(fieldWidget);
      QCheckBox *nullCheck = nullptr;

      const QVariant val = initialValues.value(meta.name);
      if (val.isValid() && !val.isNull()) {
        edit->setText(val.toString());
      } else if (!val.isValid() || val.isNull()) {
        edit->setText(QString());
      }

      if (meta.nullable) {
        nullCheck = new QCheckBox("NULL", fieldWidget);
        if (!val.isValid() || val.isNull()) {
          nullCheck->setChecked(true);
          edit->setEnabled(false);
        }
        connect(nullCheck, &QCheckBox::toggled, edit, &QWidget::setDisabled);
      }

      if (isInsert && meta.isAutoIncrement() && edit->text().isEmpty()) {
        edit->setPlaceholderText("AUTO");
      }

      fieldLayout->addWidget(edit, 1);
      if (nullCheck) fieldLayout->addWidget(nullCheck);

      QString label = meta.name + " (" + meta.type + ")";
      if (!meta.nullable) label += " *";
      form->addRow(label, fieldWidget);

      edits_.insert(meta.name, edit);
      if (nullCheck) nullChecks_.insert(meta.name, nullCheck);
    }

    layout->addLayout(form);

    QHBoxLayout *buttons = new QHBoxLayout();
    buttons->addStretch();
    QPushButton *cancel = new QPushButton("Cancel", this);
    QPushButton *ok = new QPushButton("Save", this);
    connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
    connect(ok, &QPushButton::clicked, this, &QDialog::accept);
    buttons->addWidget(cancel);
    buttons->addWidget(ok);
    layout->addLayout(buttons);
  }

  QVariantMap values() const {
    QVariantMap map;
    for (const ColumnMeta &meta : columns_) {
      QLineEdit *edit = edits_.value(meta.name);
      if (!edit) continue;
      map.insert(meta.name, edit->text());
    }
    return map;
  }

  QSet<QString> nullColumns() const {
    QSet<QString> cols;
    for (auto it = nullChecks_.begin(); it != nullChecks_.end(); ++it) {
      if (it.value()->isChecked()) {
        cols.insert(it.key());
      }
    }
    return cols;
  }

private:
  QList<ColumnMeta> columns_;
  QHash<QString, QLineEdit *> edits_;
  QHash<QString, QCheckBox *> nullChecks_;
};

struct OtmSettings {
  double seeingFwhm = 1.1;
  double seeingPivot = 500.0;
  double airmassMax = 4.0;
  bool useSkySim = true;
  QString pythonCmd;
};

class OtmSettingsDialog : public QDialog {
  Q_OBJECT
public:
  explicit OtmSettingsDialog(const OtmSettings &initial, QWidget *parent = nullptr)
      : QDialog(parent) {
    setWindowTitle("OTM Settings");
    setModal(true);

    QVBoxLayout *layout = new QVBoxLayout(this);
    QFormLayout *form = new QFormLayout();

    pythonEdit_ = new QLineEdit(initial.pythonCmd, this);
    pythonEdit_->setPlaceholderText("auto-detect (python3)");
    pythonEdit_->setToolTip("Optional. Leave blank to auto-detect.");
    form->addRow("Python (OTM)", pythonEdit_);

    seeingFwhm_ = new QDoubleSpinBox(this);
    seeingFwhm_->setRange(0.1, 10.0);
    seeingFwhm_->setDecimals(3);
    seeingFwhm_->setValue(initial.seeingFwhm);
    form->addRow("Seeing FWHM (arcsec)", seeingFwhm_);

    seeingPivot_ = new QDoubleSpinBox(this);
    seeingPivot_->setRange(100.0, 2000.0);
    seeingPivot_->setDecimals(1);
    seeingPivot_->setValue(initial.seeingPivot);
    form->addRow("Seeing Pivot (nm)", seeingPivot_);

    airmassMax_ = new QDoubleSpinBox(this);
    airmassMax_->setRange(1.0, 10.0);
    airmassMax_->setDecimals(2);
    airmassMax_->setValue(initial.airmassMax);
    form->addRow("Airmass Max", airmassMax_);

    useSkySim_ = new QCheckBox("Use sky simulation", this);
    useSkySim_->setChecked(initial.useSkySim);
    form->addRow(useSkySim_);

    layout->addLayout(form);

    QHBoxLayout *buttons = new QHBoxLayout();
    buttons->addStretch();
    QPushButton *cancel = new QPushButton("Cancel", this);
    QPushButton *ok = new QPushButton("Run", this);
    ok->setDefault(true);
    ok->setAutoDefault(true);
    cancel->setAutoDefault(false);
    connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
    connect(ok, &QPushButton::clicked, this, &QDialog::accept);
    buttons->addWidget(cancel);
    buttons->addWidget(ok);
    layout->addLayout(buttons);
  }

  OtmSettings settings() const {
    OtmSettings s;
    s.seeingFwhm = seeingFwhm_->value();
    s.seeingPivot = seeingPivot_->value();
    s.airmassMax = airmassMax_->value();
    s.useSkySim = useSkySim_->isChecked();
    s.pythonCmd = pythonEdit_->text().trimmed();
    return s;
  }

private:
  QLineEdit *pythonEdit_ = nullptr;
  QDoubleSpinBox *seeingFwhm_ = nullptr;
  QDoubleSpinBox *seeingPivot_ = nullptr;
  QDoubleSpinBox *airmassMax_ = nullptr;
  QCheckBox *useSkySim_ = nullptr;
};

class TablePanel : public QWidget {
  Q_OBJECT
public:
  TablePanel(const QString &title, QWidget *parent = nullptr)
      : QWidget(parent) {
    QVBoxLayout *layout = new QVBoxLayout(this);

    QHBoxLayout *topBar = new QHBoxLayout();
    QLabel *titleLabel = new QLabel("<b>" + title + "</b>", this);
    topBar->addWidget(titleLabel);
    topBar->addStretch();

    refreshButton_ = new QPushButton("Refresh", this);
    addButton_ = new QPushButton("Add", this);
    topBar->addWidget(refreshButton_);
    topBar->addWidget(addButton_);

    layout->addLayout(topBar);

    QHBoxLayout *filterBar = new QHBoxLayout();
    searchLabel_ = new QLabel("Search:", this);
    searchEdit_ = new QLineEdit(this);
    searchApply_ = new QPushButton("Search", this);
    searchClear_ = new QPushButton("Clear", this);
    searchLabel_->setVisible(false);
    searchEdit_->setVisible(false);
    searchApply_->setVisible(false);
    searchClear_->setVisible(false);

    filterBar->addWidget(searchLabel_);
    filterBar->addWidget(searchEdit_);
    filterBar->addWidget(searchApply_);
    filterBar->addWidget(searchClear_);

    filterBar->addStretch();

    layout->addLayout(filterBar);

    model_ = new QStandardItemModel(this);
    view_ = new ReorderTableView(this);
    view_->setModel(model_);
    view_->setSelectionBehavior(QAbstractItemView::SelectRows);
    view_->setSelectionMode(QAbstractItemView::SingleSelection);
    view_->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
    view_->horizontalHeader()->setStretchLastSection(false);
    view_->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    view_->setSortingEnabled(sortingEnabled_);
    view_->setContextMenuPolicy(Qt::CustomContextMenu);
    view_->setAllowDeleteShortcut(allowDelete_);
    layout->addWidget(view_, 1);

    statusLabel_ = new QLabel("Not connected", this);
    layout->addWidget(statusLabel_);

    connect(refreshButton_, &QPushButton::clicked, this, &TablePanel::refresh);
    connect(addButton_, &QPushButton::clicked, this, &TablePanel::addRecord);
    connect(searchApply_, &QPushButton::clicked, this, &TablePanel::refresh);
    connect(searchClear_, &QPushButton::clicked, this, &TablePanel::clearSearch);
    connect(view_, &QWidget::customContextMenuRequested, this, &TablePanel::showContextMenu);
    connect(view_, &ReorderTableView::dragSwapRequested, this, &TablePanel::handleDragSwap);
    connect(view_, &ReorderTableView::cellClicked, this, &TablePanel::handleCellClick);
    connect(view_, &ReorderTableView::deleteRequested, this, &TablePanel::handleDeleteShortcut);
    connect(model_, &QStandardItemModel::itemChanged, this, &TablePanel::handleItemChanged);
    view_->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(view_->horizontalHeader(), &QHeaderView::customContextMenuRequested,
            this, &TablePanel::showColumnHeaderContextMenu);
    view_->horizontalHeader()->installEventFilter(this);
    if (view_->horizontalHeader()->viewport()) {
      view_->horizontalHeader()->viewport()->installEventFilter(this);
    }
    connect(view_->selectionModel(), &QItemSelectionModel::currentRowChanged, this,
            [this](const QModelIndex &, const QModelIndex &) { emit selectionChanged(); });
    view_->setIconHitTest([this](const QModelIndex &index, const QPoint &pos) {
      if (!groupingEnabled_) return false;
      if (!index.isValid()) return false;
      const int nameCol = columnIndex("NAME");
      if (nameCol < 0 || index.column() != nameCol) return false;
      const int row = index.row();
      if (!isGroupHeaderRow(row)) return false;
      QRect cellRect = view_->visualRect(index);
      const int iconSize = view_->style()->pixelMetric(QStyle::PM_SmallIconSize);
      QRect iconRect(cellRect.left() + 4,
                     cellRect.center().y() - iconSize / 2,
                     iconSize, iconSize);
      return iconRect.contains(pos);
    });

    headerSaveTimer_ = new QTimer(this);
    headerSaveTimer_->setSingleShot(true);
    connect(headerSaveTimer_, &QTimer::timeout, this, &TablePanel::saveHeaderState);
    QHeaderView *header = view_->horizontalHeader();
    connect(header, &QHeaderView::sectionResized, this,
            [this](int, int, int) { scheduleHeaderStateSave(); });
    connect(header, &QHeaderView::sectionMoved, this,
            [this](int, int, int) { scheduleHeaderStateSave(); });
  }

  ~TablePanel() override {
    if (headerSaveTimer_ && headerSaveTimer_->isActive()) {
      headerSaveTimer_->stop();
    }
    saveHeaderState();
  }

  void setDatabase(DbClient *db, const QString &tableName) {
    db_ = db;
    tableName_ = tableName;
    columns_.clear();
    headerStateLoaded_ = false;
    groupingStateLoaded_ = false;
    manualUngroupObsIds_.clear();
    manualGroupKeyByObsId_.clear();
    refresh();
  }

  void setSearchColumn(const QString &columnName) {
    searchColumn_ = columnName;
    const bool enabled = !searchColumn_.isEmpty();
    searchLabel_->setVisible(enabled);
    searchEdit_->setVisible(enabled);
    searchApply_->setVisible(enabled);
    searchClear_->setVisible(enabled);
    if (enabled) {
      searchLabel_->setText(QString("Search %1:").arg(searchColumn_));
    }
  }

  void setFixedFilter(const QString &columnName, const QString &value) {
    fixedFilterColumn_ = columnName;
    fixedFilterValue_ = value;
  }

  void clearFixedFilter() {
    fixedFilterColumn_.clear();
    fixedFilterValue_.clear();
  }

  QString fixedFilterColumn() const { return fixedFilterColumn_; }
  QString fixedFilterValue() const { return fixedFilterValue_; }

  void setOrderByColumn(const QString &columnName) {
    orderByColumn_ = columnName;
  }

  void setSortingEnabled(bool enabled) {
    sortingEnabled_ = enabled;
    if (view_) view_->setSortingEnabled(sortingEnabled_);
  }

  void setAllowReorder(bool enabled) {
    allowReorder_ = enabled;
    if (!view_) return;
    if (allowReorder_) {
      view_->setDragDropMode(QAbstractItemView::NoDragDrop);
      view_->setDragEnabled(false);
      view_->setAcceptDrops(false);
      view_->setDropIndicatorShown(false);
    } else {
      view_->setDragDropMode(QAbstractItemView::NoDragDrop);
      view_->setDragEnabled(false);
      view_->setAcceptDrops(false);
    }
  }

  void setAllowDelete(bool enabled) {
    allowDelete_ = enabled;
    if (view_) view_->setAllowDeleteShortcut(enabled);
  }

  void setAllowColumnHeaderBulkEdit(bool enabled) { allowColumnHeaderBulkEdit_ = enabled; }

  void setRowNormalizer(const std::function<NormalizationResult(QVariantMap &, QSet<QString> &)> &normalizer) {
    normalizer_ = normalizer;
  }

  void setQuickAddEnabled(bool enabled) { quickAddEnabled_ = enabled; }

  void setQuickAddBuilder(const std::function<bool(QVariantMap &, QSet<QString> &, QString *)> &builder) {
    quickAddBuilder_ = builder;
  }

  void setQuickAddInsertAtTop(bool enabled) { quickAddInsertAtTop_ = enabled; }

  void setGroupingEnabled(bool enabled) {
    groupingEnabled_ = enabled;
    applyGrouping();
  }

  void setHiddenColumns(const QStringList &columns) {
    hiddenColumns_.clear();
    for (const QString &name : columns) {
      hiddenColumns_ << name.toUpper();
    }
    applyHiddenColumns();
  }

  void showContextMenuForObsId(const QString &obsId, const QPoint &globalPos) {
    if (obsId.isEmpty()) return;
    const int row = findRowByColumnValue("OBSERVATION_ID", obsId);
    if (row < 0) return;
    showContextMenuAtRow(row, globalPos);
  }

  void setColumnAfterRules(const QVector<QPair<QString, QString>> &rules) {
    columnAfterRules_.clear();
    for (const auto &rule : rules) {
      columnAfterRules_.append({rule.first.toUpper(), rule.second.toUpper()});
    }
    headerRulesPending_ = true;
    applyColumnOrderRules();
  }

  QVariantMap currentRowValues() const {
    QVariantMap map;
    const QModelIndex current = view_->currentIndex();
    if (!current.isValid()) return map;
    const int row = current.row();
    for (int col = 0; col < columns_.size(); ++col) {
      QStandardItem *item = model_->item(row, col);
      if (!item) continue;
      map.insert(columns_[col].name, item->data(Qt::EditRole));
    }
    return map;
  }

  bool selectRowByColumnValue(const QString &columnName, const QVariant &value) {
    const int col = columnIndex(columnName);
    if (col < 0) return false;
    for (int row = 0; row < model_->rowCount(); ++row) {
      QStandardItem *item = model_->item(row, col);
      if (!item) continue;
      const QVariant cellValue = item->data(Qt::UserRole + 1);
      if (cellValue.toString() == value.toString()) {
        const QModelIndex idx = model_->index(row, 0);
        view_->selectionModel()->setCurrentIndex(
            idx, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
        return true;
      }
    }
    return false;
  }

  QSet<QString> ungroupedObsIds() const { return manualUngroupObsIds_; }

  bool updateColumnForObsIds(const QStringList &obsIds, const QString &column, const QString &value,
                             QStringList *errors = nullptr) {
    if (!db_ || !db_->isOpen()) {
      if (errors) errors->append("Not connected");
      return false;
    }
    if (obsIds.isEmpty()) return false;
    QVariantMap updates;
    updates.insert(column, value);
    bool ok = true;
    for (const QString &obsId : obsIds) {
      QVariantMap keyValues;
      keyValues.insert("OBSERVATION_ID", obsId);
      QString error;
      if (!db_->updateColumnsByKey(tableName_, updates, keyValues, &error)) {
        ok = false;
        if (errors) {
          errors->append(QString("%1: %2").arg(obsId, error.isEmpty() ? "Update failed" : error));
        }
      }
    }
    if (ok) {
      refreshWithState(captureViewState());
      emit dataMutated();
    }
    return ok;
  }

  QHash<QString, QStringList> groupMembersByHeaderObsId() const {
    QHash<QString, QStringList> result;
    if (!groupingEnabled_) return result;
    const int obsIdCol = columnIndex("OBSERVATION_ID");
    if (obsIdCol < 0) return result;
    for (auto it = groupRowsByKey_.begin(); it != groupRowsByKey_.end(); ++it) {
      const QString key = it.key();
      const int headerRow = groupHeaderRowByKey_.value(key, -1);
      if (headerRow < 0) continue;
      QStandardItem *headerItem = model_->item(headerRow, obsIdCol);
      if (!headerItem) continue;
      const QString headerObsId = headerItem->data(Qt::UserRole + 1).toString();
      if (headerObsId.isEmpty()) continue;
      QStringList members;
      for (int row : it.value()) {
        QStandardItem *obsItem = model_->item(row, obsIdCol);
        if (!obsItem) continue;
        const QString obsId = obsItem->data(Qt::UserRole + 1).toString();
        if (!obsId.isEmpty()) members.append(obsId);
      }
      if (!members.isEmpty()) {
        result.insert(headerObsId, members);
      }
    }
    return result;
  }

  QVariant valueForColumnInRow(const QString &matchColumn,
                               const QVariant &matchValue,
                               const QString &columnName) const {
    const int matchCol = columnIndex(matchColumn);
    const int valueCol = columnIndex(columnName);
    if (matchCol < 0 || valueCol < 0) return QVariant();
    for (int row = 0; row < model_->rowCount(); ++row) {
      QStandardItem *matchItem = model_->item(row, matchCol);
      if (!matchItem) continue;
      const QVariant cellValue = matchItem->data(Qt::UserRole + 1);
      if (cellValue.toString() == matchValue.toString()) {
        QStandardItem *valueItem = model_->item(row, valueCol);
        if (!valueItem) return QVariant();
        return valueItem->data(Qt::UserRole + 1);
      }
    }
    return QVariant();
  }

  bool hasColumn(const QString &name) const {
    return columnIndex(name) >= 0;
  }

  QVariantMap currentKeyValues() const {
    QVariantMap map;
    const QModelIndex current = view_->currentIndex();
    if (!current.isValid()) return map;
    const int row = current.row();
    for (int col = 0; col < columns_.size(); ++col) {
      if (!columns_[col].isPrimaryKey()) continue;
      QStandardItem *item = model_->item(row, col);
      if (!item) continue;
      map.insert(columns_[col].name, item->data(Qt::UserRole + 1));
    }
    return map;
  }

  bool moveObsAfter(const QString &fromObsId, const QString &toObsId, QString *error = nullptr) {
    const int fromRow = findRowByColumnValue("OBSERVATION_ID", fromObsId);
    const int toRow = findRowByColumnValue("OBSERVATION_ID", toObsId);
    if (fromRow < 0 || toRow < 0) {
      if (error) *error = "Target not found in view.";
      return false;
    }
    return moveRowAfter(fromRow, toRow, error);
  }

  bool moveGroupAfterObsId(const QString &fromObsId, const QString &toObsId, QString *error = nullptr) {
    const int fromRow = findRowByColumnValue("OBSERVATION_ID", fromObsId);
    const int toRow = findRowByColumnValue("OBSERVATION_ID", toObsId);
    if (fromRow < 0 || toRow < 0) {
      if (error) *error = "Target not found in view.";
      return false;
    }
    if (!db_ || !db_->isOpen()) {
      if (error) *error = "Not connected";
      return false;
    }
    ViewState state = captureViewState();
    QString err;
    bool ok = false;
    if (groupingEnabled_) {
      ok = moveGroupAfterRow(fromRow, toRow, &err);
    } else {
      const int setIdCol = columnIndex("SET_ID");
      int setId = -1;
      if (setIdCol >= 0) {
        QStandardItem *setItem = model_->item(fromRow, setIdCol);
        if (setItem) setId = setItem->data(Qt::UserRole + 1).toInt();
      }
      ok = moveSingleAfterRow(fromRow, toRow, setId, &err);
    }
    if (!ok) {
      if (error) *error = err.isEmpty() ? "Move failed." : err;
      return false;
    }
    refreshWithState(state);
    emit dataMutated();
    return true;
  }

  bool moveGroupToTopObsId(const QString &fromObsId, QString *error = nullptr) {
    const int fromRow = findRowByColumnValue("OBSERVATION_ID", fromObsId);
    if (fromRow < 0) {
      if (error) *error = "Target not found in view.";
      return false;
    }
    if (!db_ || !db_->isOpen()) {
      if (error) *error = "Not connected";
      return false;
    }
    ViewState state = captureViewState();
    QString err;
    bool ok = false;
    if (groupingEnabled_) {
      ok = moveGroupToTopRow(fromRow, &err);
    } else {
      const int setIdCol = columnIndex("SET_ID");
      int setId = -1;
      if (setIdCol >= 0) {
        QStandardItem *setItem = model_->item(fromRow, setIdCol);
        if (setItem) setId = setItem->data(Qt::UserRole + 1).toInt();
      }
      ok = moveSingleToTopRow(fromRow, setId, &err);
    }
    if (!ok) {
      if (error) *error = err.isEmpty() ? "Move failed." : err;
      return false;
    }
    refreshWithState(state);
    emit dataMutated();
    return true;
  }

  void persistHeaderState() { saveHeaderState(); }

  QList<ColumnMeta> columns() const { return columns_; }

public slots:
  void refresh() {
    refreshWithState(captureViewState());
  }

  void addRecord() {
    if (!db_ || !db_->isOpen()) return;
    if (quickAddEnabled_ && quickAddBuilder_) {
      QVariantMap values;
      QSet<QString> nullColumns;
      QString buildError;
      if (!quickAddBuilder_(values, nullColumns, &buildError)) {
        showWarning(this, "Add failed", buildError.isEmpty() ? "Unable to add target." : buildError);
        return;
      }
      if (normalizer_) {
        normalizer_(values, nullColumns);
      }
      QString error;
      ViewState state = captureViewState();
      if (quickAddInsertAtTop_) {
        bool okSet = false;
        bool okOrder = false;
        const int setId = values.value("SET_ID").toInt(&okSet);
        const int obsOrder = values.value("OBS_ORDER").toInt(&okOrder);
        if (okSet && okOrder && setId > 0 && obsOrder > 0) {
          QString shiftErr;
          if (!db_->shiftObsOrderForInsert(tableName_, setId, obsOrder, 1, &shiftErr)) {
            showWarning(this, "Add failed",
                        shiftErr.isEmpty() ? "Failed to insert at top." : shiftErr);
            return;
          }
        }
      }
      if (!insertRecord(values, nullColumns, &error)) {
        showWarning(this, "Add failed", error);
        return;
      }
      refreshWithState(state);
      if (groupingEnabled_) {
        const int setId = values.value("SET_ID").toInt();
        const int obsOrder = values.value("OBS_ORDER").toInt();
        if (setId > 0 && obsOrder > 0) {
          const QStringList newObsIds = obsIdsForObsOrderRange(setId, obsOrder, 1);
          if (!newObsIds.isEmpty()) {
            for (const QString &newObsId : newObsIds) {
              manualUngroupObsIds_.insert(newObsId);
              manualGroupKeyByObsId_.remove(newObsId);
            }
            saveGroupingState();
            applyGrouping();
          }
        }
      }
      emit dataMutated();
      return;
    }
    RecordEditorDialog dialog(tableName_, columns_, QVariantMap(), true, this);
    bool inserted = false;
    if (dialog.exec() == QDialog::Accepted) {
      QVariantMap values = dialog.values();
      QSet<QString> nullColumns = dialog.nullColumns();
      if (normalizer_) {
        normalizer_(values, nullColumns);
      }
      QString error;
      if (!insertRecord(values, nullColumns, &error)) {
        showWarning(this, "Insert failed", error);
      } else {
        inserted = true;
      }
    }
    refresh();
    if (inserted) emit dataMutated();
  }

  void clearSearch() {
    searchEdit_->clear();
    refresh();
  }

signals:
  void selectionChanged();
  void dataMutated();

private slots:
  void handleItemChanged(QStandardItem *item) {
    if (suppressItemChange_) return;
    if (!db_ || !db_->isOpen()) return;
    if (!item) return;

    const int row = item->row();
    const int col = item->column();
    if (row < 0 || col < 0 || col >= columns_.size()) return;

    ColumnMeta meta = columns_.at(col);
    const QVariant oldValue = item->data(Qt::UserRole + 1);
    const bool oldIsNull = item->data(Qt::UserRole + 2).toBool();

    QString text = item->text().trimmed();
    bool newIsNull = false;
    QVariant newValue;
    if (text.compare("NULL", Qt::CaseInsensitive) == 0 ||
        (text.isEmpty() && meta.nullable)) {
      newIsNull = true;
      newValue = QVariant();
    } else {
      newIsNull = false;
      newValue = text;
    }

    if (newIsNull && !meta.nullable) {
      showWarning(this, "Update failed",
                           QString("%1 cannot be NULL").arg(meta.name));
      revertItem(item, oldValue, oldIsNull);
      return;
    }
    if (!newIsNull && text.isEmpty() && !meta.nullable) {
      showWarning(this, "Update failed",
                           QString("%1 is required").arg(meta.name));
      revertItem(item, oldValue, oldIsNull);
      return;
    }

    QVariantMap values;
    QSet<QString> nullColumns;
    for (int c = 0; c < columns_.size(); ++c) {
      ColumnMeta m = columns_.at(c);
      QStandardItem *rowItem = model_->item(row, c);
      QVariant value = rowItem ? rowItem->data(Qt::EditRole) : QVariant();
      bool isNull = rowItem ? rowItem->data(Qt::UserRole + 2).toBool() : true;

      if (c == col) {
        isNull = newIsNull;
        value = newValue;
      }

      values.insert(m.name, value);
      if (isNull) nullColumns.insert(m.name);
    }

    NormalizationResult norm;
    if (normalizer_) {
      norm = normalizer_(values, nullColumns);
    }

    QVariantMap keyValues;
    for (int c = 0; c < columns_.size(); ++c) {
      if (!columns_[c].isPrimaryKey()) continue;
      QStandardItem *rowItem = model_->item(row, c);
      if (!rowItem) continue;
      const QVariant keyValue = rowItem->data(Qt::UserRole + 1);
      const bool keyIsNull = rowItem->data(Qt::UserRole + 2).toBool();
      if (!keyValue.isValid() || keyIsNull) {
        showWarning(this, "Update failed",
                             QString("Primary key %1 is NULL").arg(columns_[c].name));
        revertItem(item, oldValue, oldIsNull);
        return;
      }
      keyValues.insert(columns_[c].name, keyValue);
    }

    QString error;
    if (!db_->updateRecord(tableName_, columns_, values, nullColumns, keyValues, &error)) {
      showWarning(this, "Update failed", error);
      revertItem(item, oldValue, oldIsNull);
      return;
    }

    const QVariant normalizedValue = values.value(meta.name);
    const bool normalizedIsNull = nullColumns.contains(meta.name);

    suppressItemChange_ = true;
    item->setData(normalizedIsNull ? QVariant() : normalizedValue, Qt::EditRole);
    item->setData(normalizedIsNull ? QVariant() : normalizedValue, Qt::UserRole + 1);
    item->setData(normalizedIsNull, Qt::UserRole + 2);
    item->setText(displayForVariant(normalizedValue, normalizedIsNull));
    item->setForeground(QBrush(normalizedIsNull ? view_->palette().color(QPalette::Disabled, QPalette::Text)
                                                : view_->palette().color(QPalette::Text)));

    for (const QString &colName : norm.changedColumns) {
      const int colIndex = columnIndex(colName);
      if (colIndex < 0 || colIndex >= columns_.size()) continue;
      if (colIndex == col) continue;
      QStandardItem *targetItem = model_->item(row, colIndex);
      if (!targetItem) continue;
      const QVariant cellValue = values.value(colName);
      const bool cellIsNull = nullColumns.contains(colName);
      targetItem->setData(cellIsNull ? QVariant() : cellValue, Qt::EditRole);
      targetItem->setData(cellIsNull ? QVariant() : cellValue, Qt::UserRole + 1);
      targetItem->setData(cellIsNull, Qt::UserRole + 2);
      targetItem->setText(displayForVariant(cellValue, cellIsNull));
      targetItem->setForeground(QBrush(cellIsNull ? view_->palette().color(QPalette::Disabled, QPalette::Text)
                                                  : view_->palette().color(QPalette::Text)));
    }
    suppressItemChange_ = false;
    applyGrouping();
    emit dataMutated();
  }

  void handleDragSwap(int sourceRow, int targetRow) {
    moveRowAfter(sourceRow, targetRow, nullptr);
  }

  void handleDeleteShortcut() {
    if (!allowDelete_) return;
    const int row = view_->currentIndex().row();
    if (row < 0) return;
    deleteRow(row);
  }

  bool eventFilter(QObject *obj, QEvent *event) override {
    if ((obj == view_->horizontalHeader() || obj == view_->horizontalHeader()->viewport()) && event) {
      if (event->type() == QEvent::ContextMenu) {
        auto *ctx = static_cast<QContextMenuEvent *>(event);
        showColumnHeaderContextMenu(ctx->pos());
        return true;
      }
    }
    return QWidget::eventFilter(obj, event);
  }

  void showColumnHeaderContextMenu(const QPoint &pos) {
    if (!allowColumnHeaderBulkEdit_) return;
    const int col = view_->horizontalHeader()->logicalIndexAt(pos);
    if (col < 0 || col >= columns_.size()) return;
    if (!isColumnBulkEditable(col)) {
      showInfo(this, "Bulk update", "This column cannot be edited.");
      return;
    }

    QMenu menu(this);
    const QString colName = columns_.at(col).name;
    QAction *applyAll = menu.addAction(QString("Set %1 For All Targets...").arg(colName));
    QAction *chosen = menu.exec(view_->horizontalHeader()->mapToGlobal(pos));
    if (chosen != applyAll) return;
    showColumnHeaderBulkEditDialog(col);
  }

  void showColumnHeaderBulkEditDialog(int col) {
    if (!allowColumnHeaderBulkEdit_) return;
    if (!db_ || !db_->isOpen()) {
      showWarning(this, "Bulk update", "Not connected");
      return;
    }
    if (col < 0 || col >= columns_.size()) return;
    if (!isColumnBulkEditable(col)) {
      showInfo(this, "Bulk update", "This column cannot be edited.");
      return;
    }

    QString defaultValue;
    const QModelIndex current = view_->currentIndex();
    if (current.isValid()) {
      QStandardItem *item = model_->item(current.row(), col);
      if (item) {
        const bool isNull = item->data(Qt::UserRole + 2).toBool();
        const QVariant val = item->data(Qt::UserRole + 1);
        defaultValue = displayForVariant(val, isNull);
      }
    }

    bool ok = false;
    const QString column = columns_.at(col).name;
    const QString value = QInputDialog::getText(
        this, "Bulk update",
        QString("Set %1 for all targets:").arg(column),
        QLineEdit::Normal, defaultValue, &ok);
    if (!ok) return;

    const QStringList obsIds = obsIdsInView();
    if (obsIds.isEmpty()) {
      showInfo(this, "Bulk update", "No targets found.");
      return;
    }

    QStringList errors;
    if (!updateColumnForObsIds(obsIds, column, value, &errors)) {
      if (!errors.isEmpty()) {
        showWarning(this, "Bulk update", errors.join("\n"));
      } else {
        showWarning(this, "Bulk update", "Update failed.");
      }
    }
  }

  void handleCellClick(const QModelIndex &index, const QPoint &pos) {
    if (!groupingEnabled_) return;
    if (!index.isValid()) return;
    const int nameCol = columnIndex("NAME");
    if (nameCol < 0 || index.column() != nameCol) return;
    const int row = index.row();
    if (!isGroupHeaderRow(row)) return;
    QRect cellRect = view_->visualRect(index);
    const int iconSize = view_->style()->pixelMetric(QStyle::PM_SmallIconSize);
    QRect iconRect(cellRect.left() + 4,
                   cellRect.center().y() - iconSize / 2,
                   iconSize, iconSize);
    if (!iconRect.contains(pos)) return;
    const QString key = groupKeyForRow(row);
    if (key.isEmpty()) return;
    toggleGroup(key);
  }

  void moveRowToPositionDialog(int row) {
    if (!allowReorder_) return;
    if (!searchEdit_->text().trimmed().isEmpty()) {
      showInfo(this, "Reorder disabled", "Clear the search filter before reordering.");
      return;
    }
    if (row < 0 || row >= model_->rowCount()) return;
    if (!db_ || !db_->isOpen()) {
      showWarning(this, "Reorder failed", "Not connected");
      return;
    }

    const int obsIdCol = columnIndex("OBSERVATION_ID");
    if (obsIdCol < 0) return;
    const QString fromObsId = model_->item(row, obsIdCol)->data(Qt::UserRole + 1).toString();
    if (fromObsId.isEmpty()) return;

    const bool groupMove = groupingEnabled_ && isGroupHeaderRow(row) &&
                           !expandedGroups_.contains(groupKeyForRow(row));

    int maxPos = 0;
    int currentPos = 1;
    if (groupMove) {
      QVector<QPair<QString, int>> order;
      order.reserve(groupRowsByKey_.size());
      const int obsOrderCol = columnIndex("OBS_ORDER");
      if (obsIdCol < 0 || obsOrderCol < 0) return;
      for (auto it = groupRowsByKey_.begin(); it != groupRowsByKey_.end(); ++it) {
        int minOrder = std::numeric_limits<int>::max();
        const int headerRow = groupHeaderRowByKey_.value(it.key(), -1);
        for (int memberRow : it.value()) {
          QStandardItem *orderItem = model_->item(memberRow, obsOrderCol);
          if (!orderItem) continue;
          const int orderVal = orderItem->data(Qt::UserRole + 1).toInt();
          if (memberRow == headerRow) {
            minOrder = orderVal;
          } else if (minOrder == std::numeric_limits<int>::max()) {
            minOrder = orderVal;
          } else if (headerRow < 0) {
            minOrder = std::min(minOrder, orderVal);
          }
        }
        if (minOrder == std::numeric_limits<int>::max()) minOrder = 0;
        order.append({it.key(), minOrder});
      }
      std::sort(order.begin(), order.end(),
                [](const auto &a, const auto &b) { return a.second < b.second; });
      maxPos = order.size();
      const QString fromKey = groupKeyForRow(row);
      for (int i = 0; i < order.size(); ++i) {
        if (order[i].first == fromKey) {
          currentPos = i + 1;
          break;
        }
      }
    } else {
      const int obsIdCol = columnIndex("OBSERVATION_ID");
      const int obsOrderCol = columnIndex("OBS_ORDER");
      const int setIdCol = columnIndex("SET_ID");
      if (obsIdCol < 0 || obsOrderCol < 0) return;
      QList<RowInfo> infos;
      infos.reserve(model_->rowCount());
      int setId = -1;
      if (setIdCol >= 0) {
        QStandardItem *setItem = model_->item(row, setIdCol);
        if (setItem) setId = setItem->data(Qt::UserRole + 1).toInt();
      }
      for (int r = 0; r < model_->rowCount(); ++r) {
        QStandardItem *obsItem = model_->item(r, obsIdCol);
        QStandardItem *orderItem = model_->item(r, obsOrderCol);
        if (!obsItem || !orderItem) continue;
        if (setIdCol >= 0 && setId >= 0) {
          QStandardItem *setItem = model_->item(r, setIdCol);
          if (!setItem || setItem->data(Qt::UserRole + 1).toInt() != setId) continue;
        }
        RowInfo info;
        info.row = r;
        info.obsId = obsItem->data(Qt::UserRole + 1).toString();
        info.obsOrder = orderItem->data(Qt::UserRole + 1).toInt();
        infos.append(info);
      }
      std::sort(infos.begin(), infos.end(),
                [](const RowInfo &a, const RowInfo &b) { return a.obsOrder < b.obsOrder; });
      maxPos = infos.size();
      const QString fromObsId = model_->item(row, obsIdCol)->data(Qt::UserRole + 1).toString();
      for (int i = 0; i < infos.size(); ++i) {
        if (infos[i].obsId == fromObsId) {
          currentPos = i + 1;
          break;
        }
      }
    }

    if (maxPos <= 0) return;
    if (moveToDialog_) {
      moveToDialog_->close();
      moveToDialog_.clear();
    }

    QDialog *dialog = new QDialog(this);
    moveToDialog_ = dialog;
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setWindowModality(Qt::NonModal);
    dialog->setModal(false);
    dialog->setWindowTitle("Move to Position");
    QVBoxLayout *layout = new QVBoxLayout(dialog);
    QFormLayout *form = new QFormLayout();
    QSpinBox *posSpin = new QSpinBox(dialog);
    posSpin->setRange(1, maxPos);
    posSpin->setValue(currentPos);
    form->addRow(QString("Position (1-%1):").arg(maxPos), posSpin);
    layout->addLayout(form);
    QDialogButtonBox *buttons =
        new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, dialog);
    connect(buttons, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, dialog, &QDialog::reject);
    layout->addWidget(buttons);

    connect(dialog, &QDialog::accepted, this, [this, fromObsId, groupMove, maxPos, currentPos]() {
      if (!moveToDialog_) return;
      const QList<QSpinBox *> spins = moveToDialog_->findChildren<QSpinBox *>();
      if (spins.isEmpty()) return;
      int newPos = spins.first()->value();
      if (newPos < 1) newPos = 1;
      if (newPos > maxPos) newPos = maxPos;
      if (newPos == currentPos) return;

      const int fromRow = findRowByColumnValue("OBSERVATION_ID", fromObsId);
      if (fromRow < 0) return;

      QString err;
      ViewState state = captureViewState();
      bool moved = false;
      if (groupMove) {
        moved = moveGroupToPositionRow(fromRow, newPos, &err);
      } else {
        const int setIdCol = columnIndex("SET_ID");
        int setId = -1;
        if (setIdCol >= 0) {
          QStandardItem *setItem = model_->item(fromRow, setIdCol);
          if (setItem) setId = setItem->data(Qt::UserRole + 1).toInt();
        }
        moved = moveSingleToPositionRow(fromRow, newPos, setId, &err);
      }
      if (!moved) {
        if (!err.isEmpty()) showWarning(this, "Reorder failed", err);
        return;
      }
      refreshWithState(state);
      emit dataMutated();
    });
    connect(dialog, &QDialog::finished, this, [this](int) { moveToDialog_.clear(); });
    dialog->show();
  }

  void showContextMenu(const QPoint &pos) {
    const QModelIndex index = view_->indexAt(pos);
    if (!index.isValid()) return;
    showContextMenuAtRow(index.row(), view_->viewport()->mapToGlobal(pos));
  }

private:
  void showContextMenuAtRow(int row, const QPoint &globalPos) {
    if (row < 0 || row >= model_->rowCount()) return;
    if (!allowDelete_ && !allowReorder_) return;
    const bool searchActive = !searchEdit_->text().trimmed().isEmpty();

    QMenu menu(this);
    QMenu *seqMenu = nullptr;
    QAction *deleteAction = nullptr;
    QAction *duplicateAction = nullptr;
    QAction *moveUp = nullptr;
    QAction *moveDown = nullptr;
    QAction *moveTop = nullptr;
    QAction *moveBottom = nullptr;
    QAction *moveTo = nullptr;
    QList<QAction *> seqActions;
    QAction *ungroupAction = nullptr;
    QAction *regroupAction = nullptr;
    const int obsIdCol = columnIndex("OBSERVATION_ID");
    const int nameCol = columnIndex("NAME");
    const int obsOrderCol = columnIndex("OBS_ORDER");
    QString obsId;
    if (obsIdCol >= 0) {
      QStandardItem *obsItem = model_->item(row, obsIdCol);
      if (obsItem) obsId = obsItem->data(Qt::UserRole + 1).toString();
    }
    const bool canUngroup = groupingEnabled_ && !obsId.isEmpty();
    const bool isUngrouped = canUngroup && manualUngroupObsIds_.contains(obsId);
    if (allowReorder_ && !searchActive) {
      moveUp = menu.addAction("Move Up");
      moveDown = menu.addAction("Move Down");
      moveTop = menu.addAction("Move to Top");
      moveBottom = menu.addAction("Move to Bottom");
      moveTo = menu.addAction("Move to Position...");
      menu.addSeparator();
    }
    if (allowReorder_) {
      duplicateAction = menu.addAction("Duplicate");
    }
    if (groupingEnabled_ && !obsId.isEmpty()) {
      const QString groupKey = groupKeyForRow(row);
      const QList<int> members = groupRowsByKey_.value(groupKey);
      if (!members.isEmpty()) {
        seqMenu = menu.addMenu("Use For Sequencer");
        QList<int> orderedMembers = members;
        if (obsOrderCol >= 0) {
          std::sort(orderedMembers.begin(), orderedMembers.end(), [&](int a, int b) {
            QStandardItem *orderA = model_->item(a, obsOrderCol);
            QStandardItem *orderB = model_->item(b, obsOrderCol);
            const int oa = orderA ? orderA->data(Qt::UserRole + 1).toInt() : 0;
            const int ob = orderB ? orderB->data(Qt::UserRole + 1).toInt() : 0;
            return oa < ob;
          });
        }
        const QString headerObsId = headerObsIdForGroupKey(groupKey);
        const QString selectedObsId = selectedObsIdByHeader_.value(headerObsId);
        for (int memberRow : orderedMembers) {
          const QString memberObsId = obsIdForRow(memberRow);
          if (memberObsId.isEmpty()) continue;
          QString label = memberObsId;
          if (nameCol >= 0) {
            QStandardItem *nameItem = model_->item(memberRow, nameCol);
            const QString rawName = nameItem ? nameItem->data(Qt::UserRole + 1).toString() : QString();
            if (!rawName.isEmpty()) label = rawName;
          }
          QAction *act = seqMenu->addAction(label);
          act->setData(memberObsId);
          act->setCheckable(true);
          if (!selectedObsId.isEmpty() && memberObsId == selectedObsId) {
            act->setChecked(true);
          }
          seqActions.append(act);
        }
        menu.addSeparator();
      }
    }
    if (canUngroup) {
      if (isUngrouped) {
        regroupAction = menu.addAction("Restore Grouping");
      } else {
        ungroupAction = menu.addAction("Remove From Group");
      }
      menu.addSeparator();
    }
    if (allowDelete_) {
      deleteAction = menu.addAction("Delete");
    }
    QAction *chosen = menu.exec(globalPos);
    if (!chosen) return;

    if (seqMenu && seqActions.contains(chosen)) {
      const QString selectedObsId = chosen->data().toString();
      const QString key = groupKeyForRow(row);
      setGroupSequencerSelection(key, selectedObsId);
      return;
    }

    if (chosen == ungroupAction) {
      if (!obsId.isEmpty()) {
        const QString key = groupKeyForRow(row);
        QList<int> members = groupRowsByKey_.value(key);
        const int obsOrderCol = columnIndex("OBS_ORDER");
        int lastRow = row;
        int bestOrder = std::numeric_limits<int>::min();
        if (members.size() > 1 && obsOrderCol >= 0) {
          for (int memberRow : members) {
            if (memberRow == row) continue;
            QStandardItem *orderItem = model_->item(memberRow, obsOrderCol);
            if (!orderItem) continue;
            const int orderVal = orderItem->data(Qt::UserRole + 1).toInt();
            if (orderVal >= bestOrder) {
              bestOrder = orderVal;
              lastRow = memberRow;
            }
          }
        }
        manualUngroupObsIds_.insert(obsId);
        manualGroupKeyByObsId_.remove(obsId);
        saveGroupingState();
        if (members.size() > 1 && lastRow != row) {
          QString err;
          if (!moveSingleAfterRowWithRefresh(row, lastRow, &err)) {
            if (!err.isEmpty()) showWarning(this, "Reorder failed", err);
            applyGrouping();
          }
        } else {
          applyGrouping();
        }
      }
      return;
    }
    if (chosen == regroupAction) {
      if (!obsId.isEmpty()) {
        manualUngroupObsIds_.remove(obsId);
        manualGroupKeyByObsId_.remove(obsId);
        saveGroupingState();
        applyGrouping();
      }
      return;
    }
    if (chosen == deleteAction) {
      deleteRow(row);
    } else if (chosen == duplicateAction) {
      duplicateRow(row);
    } else if (chosen == moveUp) {
      const int target = previousVisibleRow(row);
      if (target >= 0) moveRowAfter(row, target, nullptr);
    } else if (chosen == moveDown) {
      const int target = nextVisibleRow(row);
      if (target >= 0) moveRowAfter(row, target, nullptr);
    } else if (chosen == moveTop) {
      if (!obsId.isEmpty()) {
        QString err;
        if (!moveGroupToTopObsId(obsId, &err)) {
          showWarning(this, "Reorder failed", err.isEmpty() ? "Move to top failed." : err);
        }
      } else {
        const int target = firstVisibleRow();
        if (target >= 0) moveRowAfter(row, target, nullptr);
      }
    } else if (chosen == moveBottom) {
      const int target = lastVisibleRow();
      if (target >= 0) moveRowAfter(row, target, nullptr);
    } else if (chosen == moveTo) {
      moveRowToPositionDialog(row);
    }
  }
  struct SwapInfo {
    bool valid = false;
    QVariant obsId;
    QVariant obsOrder;
  };

  struct RowInfo {
    QString obsId;
    int obsOrder = 0;
    int row = -1;
    int setId = -1;
    QString groupKey;
  };

  struct ViewState {
    int vScroll = 0;
    int hScroll = 0;
    QVariantMap keyValues;
    int sortColumn = -1;
    Qt::SortOrder sortOrder = Qt::AscendingOrder;
  };

  ViewState captureViewState() const {
    ViewState state;
    state.vScroll = view_->verticalScrollBar()->value();
    state.hScroll = view_->horizontalScrollBar()->value();
    state.keyValues = currentKeyValues();
    if (sortingEnabled_) {
      state.sortColumn = view_->horizontalHeader()->sortIndicatorSection();
      state.sortOrder = view_->horizontalHeader()->sortIndicatorOrder();
    } else {
      state.sortColumn = -1;
    }
    return state;
  }

  void restoreViewState(const ViewState &state) {
    view_->setUpdatesEnabled(false);
    if (sortingEnabled_ && state.sortColumn >= 0) {
      model_->sort(state.sortColumn, state.sortOrder);
    }
    if (!state.keyValues.isEmpty()) {
      const int row = findRowByKey(state.keyValues);
      if (row >= 0) {
        const QModelIndex idx = model_->index(row, 0);
        view_->selectionModel()->setCurrentIndex(
            idx, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
      }
    }
    view_->horizontalScrollBar()->setValue(state.hScroll);
    view_->verticalScrollBar()->setValue(state.vScroll);
    view_->setUpdatesEnabled(true);

    QTimer::singleShot(0, this, [this, state]() {
      if (!view_) return;
      view_->horizontalScrollBar()->setValue(state.hScroll);
      view_->verticalScrollBar()->setValue(state.vScroll);
    });
  }

  void refreshWithState(const ViewState &state) {
    if (!db_ || !db_->isOpen() || tableName_.isEmpty()) {
      statusLabel_->setText("Not connected");
      return;
    }
    QString error;
    if (!db_->loadColumns(tableName_, columns_, &error)) {
      statusLabel_->setText(error.isEmpty() ? "Failed to read columns" : error);
      return;
    }
    QVector<QVector<QVariant>> rows;
    const QString searchValue = searchEdit_->text().trimmed();
    if (!db_->fetchRows(tableName_, columns_,
                        fixedFilterColumn_, fixedFilterValue_,
                        searchColumn_, searchValue,
                        orderByColumn_,
                        rows, &error)) {
      statusLabel_->setText(error.isEmpty() ? "Failed to read rows" : error);
      return;
    }

    suppressItemChange_ = true;
    model_->clear();
    model_->setColumnCount(columns_.size());
    QStringList headers;
    for (const ColumnMeta &meta : columns_) {
      headers << meta.name;
    }
    model_->setHorizontalHeaderLabels(headers);
    if (headerSaveTimer_ && headerSaveTimer_->isActive()) {
      headerSaveTimer_->stop();
      saveHeaderState();
    }
    const bool restoredHeader = restoreHeaderState();
    if (!restoredHeader) {
      applyColumnOrderRules();
    } else if (headerRulesPending_) {
      applyColumnOrderRules();
      headerRulesPending_ = false;
      saveHeaderState();
    }
    applyHiddenColumns();

    const QColor textColor = view_->palette().color(QPalette::Text);
    const QColor nullColor = view_->palette().color(QPalette::Disabled, QPalette::Text);

    for (const QVector<QVariant> &rowValues : rows) {
      QList<QStandardItem *> items;
      items.reserve(columns_.size());
      for (int col = 0; col < columns_.size(); ++col) {
        QVariant value;
        if (col < rowValues.size()) {
          value = rowValues.at(col);
        }
        const bool isNull = !value.isValid() || value.isNull();
        QStandardItem *item = new QStandardItem(displayForVariant(value, isNull));
        const QString colName = columns_.at(col).name.toUpper();
        if (colName == "EXPTIME") {
          item->setToolTip("Format: SET <sec> or SNR <value>. Example: SET 600");
        } else if (colName == "SLITWIDTH") {
          item->setToolTip("Format: SET <arcsec>, SNR <percent>, LOSS <percent>, RES <R>, AUTO");
        } else if (colName == "SLITANGLE") {
          item->setToolTip("Format: numeric degrees or PA");
        } else if (colName == "MAGFILTER") {
          item->setToolTip("U,B,V,R,I,J,K, or match. G is mapped to match.");
        } else if (colName == "CHANNEL") {
          item->setToolTip("U, G, R, or I");
        } else if (colName == "WRANGE_LOW" || colName == "WRANGE_HIGH") {
          item->setToolTip("Wavelength range in nm; defaults around channel center");
        }
        item->setEditable(true);
        if (!isNull) {
          item->setData(value, Qt::EditRole);
        } else {
          item->setData(QVariant(), Qt::EditRole);
        }
        item->setData(isNull ? QVariant() : value, Qt::UserRole + 1);
        item->setData(isNull, Qt::UserRole + 2);
        item->setForeground(QBrush(isNull ? nullColor : textColor));
        items.push_back(item);
      }
      model_->appendRow(items);
    }

    suppressItemChange_ = false;
    restoreViewState(state);
    if (!restoredHeader) {
      saveHeaderState();
    }
    const QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    statusLabel_->setText(QString("Last refresh: %1").arg(timestamp));

    if (!groupingStateLoaded_) {
      loadGroupingState();
      groupingStateLoaded_ = true;
    }
    applyGrouping();
  }

  int findRowByKey(const QVariantMap &keyValues) const {
    if (keyValues.isEmpty()) return -1;
    for (int row = 0; row < model_->rowCount(); ++row) {
      bool match = true;
      for (int col = 0; col < columns_.size(); ++col) {
        if (!columns_[col].isPrimaryKey()) continue;
        QStandardItem *item = model_->item(row, col);
        if (!item) { match = false; break; }
        QVariant val = item->data(Qt::UserRole + 1);
        if (val.toString() != keyValues.value(columns_[col].name).toString()) {
          match = false;
          break;
        }
      }
      if (match) return row;
    }
    return -1;
  }

  int findRowByColumnValue(const QString &columnName, const QVariant &value) const {
    const int col = columnIndex(columnName);
    if (col < 0) return -1;
    for (int row = 0; row < model_->rowCount(); ++row) {
      QStandardItem *item = model_->item(row, col);
      if (!item) continue;
      const QVariant cellValue = item->data(Qt::UserRole + 1);
      if (cellValue.toString() == value.toString()) {
        return row;
      }
    }
    return -1;
  }

  SwapInfo swapInfoForRow(int row) const {
    SwapInfo info;
    if (row < 0 || row >= model_->rowCount()) return info;
    const int obsIdCol = columnIndex("OBSERVATION_ID");
    const int obsOrderCol = columnIndex("OBS_ORDER");
    if (obsIdCol < 0 || obsOrderCol < 0) return info;
    QStandardItem *obsItem = model_->item(row, obsIdCol);
    QStandardItem *orderItem = model_->item(row, obsOrderCol);
    if (!obsItem || !orderItem) return info;
    QVariant obsId = obsItem->data(Qt::UserRole + 1);
    QVariant obsOrder = orderItem->data(Qt::UserRole + 1);
    if (!obsId.isValid() || !obsOrder.isValid()) return info;
    info.valid = true;
    info.obsId = obsId;
    info.obsOrder = obsOrder;
    return info;
  }

  bool isGroupHeaderRow(int row) const {
    if (!groupingEnabled_) return false;
    if (!groupKeyByRow_.contains(row)) return false;
    const QString key = groupKeyByRow_.value(row);
    const int headerRow = groupHeaderRowByKey_.value(key, -1);
    return headerRow == row;
  }

  QString groupKeyForRow(int row) const {
    return groupKeyByRow_.value(row);
  }

  QString obsIdForRow(int row) const {
    const int obsIdCol = columnIndex("OBSERVATION_ID");
    if (obsIdCol < 0 || row < 0 || row >= model_->rowCount()) return QString();
    QStandardItem *item = model_->item(row, obsIdCol);
    if (!item) return QString();
    return item->data(Qt::UserRole + 1).toString();
  }

  QString headerObsIdForGroupKey(const QString &groupKey) const {
    if (groupKey.isEmpty()) return QString();
    const int headerRow = groupHeaderRowByKey_.value(groupKey, -1);
    QString headerObsId = obsIdForRow(headerRow);
    if (!headerObsId.isEmpty()) return headerObsId;
    const QList<int> members = groupRowsByKey_.value(groupKey);
    if (!members.isEmpty()) {
      headerObsId = obsIdForRow(members.first());
    }
    return headerObsId;
  }

  void setGroupSequencerSelection(const QString &groupKey, const QString &selectedObsId) {
    if (groupKey.isEmpty() || selectedObsId.isEmpty()) return;
    const QList<int> members = groupRowsByKey_.value(groupKey);
    bool inGroup = false;
    for (int row : members) {
      if (obsIdForRow(row) == selectedObsId) {
        inGroup = true;
        break;
      }
    }
    if (!inGroup) return;
    const QString headerObsId = headerObsIdForGroupKey(groupKey);
    if (headerObsId.isEmpty()) return;
    selectedObsIdByHeader_[headerObsId] = selectedObsId;
    saveGroupingState();
    applyGrouping();
  }

  void toggleGroup(const QString &key) {
    if (key.isEmpty()) return;
    if (expandedGroups_.contains(key)) {
      expandedGroups_.remove(key);
    } else {
      expandedGroups_.insert(key);
    }
    applyGrouping();
  }

  void applyGrouping() {
    if (!groupingEnabled_) {
      for (int row = 0; row < model_->rowCount(); ++row) {
        view_->setRowHidden(row, false);
      }
      return;
    }

    const bool prevSuppress = suppressItemChange_;
    suppressItemChange_ = true;

    const int iconSize = view_->style()->pixelMetric(QStyle::PM_SmallIconSize);
    const QColor iconColor(90, 160, 255);
    auto makeArrowIcon = [&](Qt::ArrowType arrow) {
      QPixmap pix(iconSize, iconSize);
      pix.fill(Qt::transparent);
      QPainter p(&pix);
      p.setRenderHint(QPainter::Antialiasing, true);
      p.setPen(Qt::NoPen);
      p.setBrush(iconColor);
      QPolygon poly;
      if (arrow == Qt::DownArrow) {
        poly << QPoint(iconSize / 2, iconSize - 2)
             << QPoint(2, 2)
             << QPoint(iconSize - 2, 2);
      } else {
        poly << QPoint(iconSize - 2, iconSize / 2)
             << QPoint(2, 2)
             << QPoint(2, iconSize - 2);
      }
      p.drawPolygon(poly);
      return QIcon(pix);
    };
    QIcon collapsedIcon = makeArrowIcon(Qt::RightArrow);
    QIcon expandedIcon = makeArrowIcon(Qt::DownArrow);

    const int obsIdCol = columnIndex("OBSERVATION_ID");
    const int nameCol = columnIndex("NAME");
    if (obsIdCol < 0 || nameCol < 0) return;

    const int raCol = columnIndex("RA");
    const int decCol = columnIndex("DECL");
    const int offsetRaCol = columnIndex("OFFSET_RA");
    const int offsetDecCol = columnIndex("OFFSET_DEC");
    const int draCol = columnIndex("DRA");
    const int ddecCol = columnIndex("DDEC");

    struct RowInfo {
      int row = -1;
      QString obsId;
      QString name;
      QString groupKey;
      bool isScience = false;
      int obsOrder = 0;
      bool coordOk = false;
      double raDeg = 0.0;
      double decDeg = 0.0;
    };

    auto assignGroupKeys = [&](QVector<RowInfo> &rows) {
      struct GroupCenter {
        QString key;
        double ra = 0.0;
        double dec = 0.0;
      };
      QVector<GroupCenter> centers;
      auto makeKey = [](double raDeg, double decDeg) {
        return QString("%1:%2")
            .arg(QString::number(raDeg, 'f', 6))
            .arg(QString::number(decDeg, 'f', 6));
      };
      for (const RowInfo &info : rows) {
        if (manualUngroupObsIds_.contains(info.obsId)) {
          continue;
        }
        if (info.coordOk && info.isScience) {
          centers.push_back({makeKey(info.raDeg, info.decDeg), info.raDeg, info.decDeg});
        }
      }
      const bool hasScienceCenters = !centers.isEmpty();
      for (RowInfo &info : rows) {
        if (manualUngroupObsIds_.contains(info.obsId)) {
          info.groupKey = QString("UNGROUP:%1").arg(info.obsId);
          continue;
        }
        const QString manualKey = manualGroupKeyByObsId_.value(info.obsId);
        if (!manualKey.isEmpty()) {
          info.groupKey = manualKey;
          continue;
        }
        if (!info.coordOk) {
          info.groupKey = QString("OBS:%1").arg(info.obsId);
          continue;
        }
        if (!hasScienceCenters) {
          info.groupKey = QString("OBS:%1").arg(info.obsId);
          continue;
        }
        double bestSep = 1e12;
        int bestIdx = -1;
        for (int i = 0; i < centers.size(); ++i) {
          const double sep = angularSeparationArcsec(info.raDeg, info.decDeg,
                                                     centers[i].ra, centers[i].dec);
          if (sep < bestSep) {
            bestSep = sep;
            bestIdx = i;
          }
        }
        if (bestIdx >= 0 && bestSep <= kGroupCoordTolArcsec) {
          info.groupKey = centers[bestIdx].key;
        } else {
          info.groupKey = QString("OBS:%1").arg(info.obsId);
        }
      }
    };

    auto collectRows = [&]() {
      QVector<RowInfo> rows;
      rows.reserve(model_->rowCount());
      for (int row = 0; row < model_->rowCount(); ++row) {
        QStandardItem *obsItem = model_->item(row, obsIdCol);
        QStandardItem *nameItem = model_->item(row, nameCol);
        if (!obsItem || !nameItem) continue;
        const QString obsId = obsItem->data(Qt::UserRole + 1).toString();
        const QString name = nameItem->data(Qt::UserRole + 1).toString();
        QVariantMap values;
        if (raCol >= 0) values.insert("RA", model_->item(row, raCol)->data(Qt::UserRole + 1));
        if (decCol >= 0) values.insert("DECL", model_->item(row, decCol)->data(Qt::UserRole + 1));
        if (offsetRaCol >= 0) values.insert("OFFSET_RA", model_->item(row, offsetRaCol)->data(Qt::UserRole + 1));
        if (offsetDecCol >= 0) values.insert("OFFSET_DEC", model_->item(row, offsetDecCol)->data(Qt::UserRole + 1));
        if (draCol >= 0) values.insert("DRA", model_->item(row, draCol)->data(Qt::UserRole + 1));
        if (ddecCol >= 0) values.insert("DDEC", model_->item(row, ddecCol)->data(Qt::UserRole + 1));

        bool hasRa = false;
        bool hasDec = false;
        const double offsetRa = offsetArcsecFromValues(values, {"OFFSET_RA", "DRA"}, &hasRa);
        const double offsetDec = offsetArcsecFromValues(values, {"OFFSET_DEC", "DDEC"}, &hasDec);
        const bool isScience = (!hasRa && !hasDec) ||
                               (std::abs(offsetRa) <= kOffsetZeroTolArcsec &&
                                std::abs(offsetDec) <= kOffsetZeroTolArcsec);

        int obsOrder = 0;
        const int obsOrderCol = columnIndex("OBS_ORDER");
        if (obsOrderCol >= 0) {
          QStandardItem *orderItem = model_->item(row, obsOrderCol);
          if (orderItem) obsOrder = orderItem->data(Qt::UserRole + 1).toInt();
        }

        double raDeg = 0.0;
        double decDeg = 0.0;
        const bool coordOk = computeScienceCoordDegreesProjected(values, &raDeg, &decDeg);
        RowInfo info;
        info.row = row;
        info.obsId = obsId;
        info.name = name;
        info.groupKey = QString();
        info.isScience = isScience;
        info.obsOrder = obsOrder;
        info.coordOk = coordOk;
        info.raDeg = raDeg;
        info.decDeg = decDeg;
        rows.push_back(info);
      }
      assignGroupKeys(rows);
      return rows;
    };

    auto buildMaps = [&](const QVector<RowInfo> &rows,
                         QHash<QString, QList<int>> &rowsByKey,
                         QHash<QString, int> &headerByKey,
                         QHash<int, QString> &keyByRow) {
      rowsByKey.clear();
      headerByKey.clear();
      keyByRow.clear();
      for (const RowInfo &info : rows) {
        rowsByKey[info.groupKey].append(info.row);
        if (info.isScience && !headerByKey.contains(info.groupKey)) {
          headerByKey[info.groupKey] = info.row;
        }
        keyByRow.insert(info.row, info.groupKey);
      }
      for (auto it = rowsByKey.begin(); it != rowsByKey.end(); ++it) {
        if (!headerByKey.contains(it.key()) && !it.value().isEmpty()) {
          headerByKey[it.key()] = it.value().first();
        }
      }
    };

    QVector<RowInfo> rows = collectRows();
    QHash<QString, QList<int>> rowsByKey;
    QHash<QString, int> headerByKey;
    QHash<int, QString> keyByRow;
    buildMaps(rows, rowsByKey, headerByKey, keyByRow);

    QVector<QString> currentOrder;
    currentOrder.reserve(model_->rowCount());
    for (int row = 0; row < model_->rowCount(); ++row) {
      QStandardItem *obsItem = model_->item(row, obsIdCol);
      if (!obsItem) continue;
      currentOrder << obsItem->data(Qt::UserRole + 1).toString();
    }

    struct GroupOrder {
      QString key;
      int headerOrder = 0;
      QVector<RowInfo> members;
      QString headerObsId;
    };

    QHash<QString, QVector<RowInfo>> rowsByGroup;
    for (const RowInfo &info : rows) {
      rowsByGroup[info.groupKey].append(info);
    }

    QVector<GroupOrder> groupOrder;
    groupOrder.reserve(rowsByGroup.size());
    for (auto it = rowsByGroup.begin(); it != rowsByGroup.end(); ++it) {
      QVector<RowInfo> members = it.value();
      std::sort(members.begin(), members.end(),
                [](const RowInfo &a, const RowInfo &b) { return a.obsOrder < b.obsOrder; });
      QString headerObsId;
      int headerOrder = members.isEmpty() ? 0 : members.first().obsOrder;
      for (const RowInfo &info : members) {
        if (info.isScience) {
          headerObsId = info.obsId;
          headerOrder = info.obsOrder;
          break;
        }
      }
      if (headerObsId.isEmpty() && !members.isEmpty()) {
        headerObsId = members.first().obsId;
      }
      groupOrder.push_back({it.key(), headerOrder, members, headerObsId});
    }

    std::sort(groupOrder.begin(), groupOrder.end(),
              [](const GroupOrder &a, const GroupOrder &b) { return a.headerOrder < b.headerOrder; });

    QVector<QString> desiredOrder;
    for (const GroupOrder &group : groupOrder) {
      if (!group.headerObsId.isEmpty()) {
        desiredOrder.append(group.headerObsId);
      }
      for (const RowInfo &member : group.members) {
        if (member.obsId == group.headerObsId) continue;
        desiredOrder.append(member.obsId);
      }
    }

    if (desiredOrder.size() == currentOrder.size() && desiredOrder != currentOrder) {
      QVariantMap selectedKeys = currentKeyValues();
      const int vScroll = view_->verticalScrollBar()->value();
      const int hScroll = view_->horizontalScrollBar()->value();

      QHash<int, QString> obsIdByRow;
      obsIdByRow.reserve(model_->rowCount());
      for (int row = 0; row < model_->rowCount(); ++row) {
        obsIdByRow.insert(row, currentOrder.value(row));
      }
      QHash<QString, QList<QStandardItem *>> itemsByObsId;
      for (int row = model_->rowCount() - 1; row >= 0; --row) {
        const QString obsId = obsIdByRow.value(row);
        itemsByObsId.insert(obsId, model_->takeRow(row));
      }
      for (const QString &obsId : desiredOrder) {
        if (!itemsByObsId.contains(obsId)) continue;
        model_->insertRow(model_->rowCount(), itemsByObsId.take(obsId));
      }
      for (auto it = itemsByObsId.begin(); it != itemsByObsId.end(); ++it) {
        model_->insertRow(model_->rowCount(), it.value());
      }

      if (!selectedKeys.isEmpty()) {
        const int selRow = findRowByKey(selectedKeys);
        if (selRow >= 0) {
          const QModelIndex idx = model_->index(selRow, 0);
          view_->selectionModel()->setCurrentIndex(
              idx, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
        }
      }
      view_->verticalScrollBar()->setValue(vScroll);
      view_->horizontalScrollBar()->setValue(hScroll);
    }

    rows = collectRows();
    buildMaps(rows, groupRowsByKey_, groupHeaderRowByKey_, groupKeyByRow_);

    rowsByGroup.clear();
    rowsByGroup.reserve(groupRowsByKey_.size());
    for (const RowInfo &info : rows) {
      rowsByGroup[info.groupKey].append(info);
    }

    QHash<QString, QString> selectedByKey;
    QSet<QString> validHeaderObsIds;
    bool selectionChanged = false;
    for (auto it = rowsByGroup.begin(); it != rowsByGroup.end(); ++it) {
      QVector<RowInfo> members = it.value();
      if (members.isEmpty()) continue;
      std::sort(members.begin(), members.end(),
                [](const RowInfo &a, const RowInfo &b) { return a.obsOrder < b.obsOrder; });

      QString headerObsId;
      const int headerRow = groupHeaderRowByKey_.value(it.key(), -1);
      if (headerRow >= 0) {
        QStandardItem *headerItem = model_->item(headerRow, obsIdCol);
        if (headerItem) headerObsId = headerItem->data(Qt::UserRole + 1).toString();
      }
      if (headerObsId.isEmpty()) {
        headerObsId = members.first().obsId;
      }
      if (headerObsId.isEmpty()) continue;
      validHeaderObsIds.insert(headerObsId);

      QString selectedObsId = selectedObsIdByHeader_.value(headerObsId);
      bool selectedValid = false;
      for (const RowInfo &member : members) {
        if (member.obsId == selectedObsId) {
          selectedValid = true;
          break;
        }
      }
      if (!selectedValid) {
        QString defaultObsId;
        for (const RowInfo &member : members) {
          if (!member.isScience) {
            defaultObsId = member.obsId;
            break;
          }
        }
        if (defaultObsId.isEmpty()) defaultObsId = headerObsId;
        selectedObsId = defaultObsId;
        selectedObsIdByHeader_[headerObsId] = selectedObsId;
        selectionChanged = true;
      }
      selectedByKey.insert(it.key(), selectedObsId);
    }

    for (auto it = selectedObsIdByHeader_.begin(); it != selectedObsIdByHeader_.end(); ) {
      if (!validHeaderObsIds.contains(it.key())) {
        it = selectedObsIdByHeader_.erase(it);
        selectionChanged = true;
      } else {
        ++it;
      }
    }
    if (selectionChanged) {
      saveGroupingState();
    }

    const int stateCol = columnIndex("STATE");
    if (stateCol >= 0 && db_ && db_->isOpen() && !rowsByGroup.isEmpty()) {
      QList<QVariantMap> keyValuesList;
      QList<QVariant> stateValues;
      struct StateUpdate {
        int row = -1;
        QString value;
      };
      QVector<StateUpdate> stateUpdates;

      auto shouldOverrideState = [](const QString &state) {
        const QString s = state.trimmed().toLower();
        return s.isEmpty() || s == "pending" || s == "unassigned";
      };

      for (auto it = rowsByGroup.begin(); it != rowsByGroup.end(); ++it) {
        const QString selectedObsId = selectedByKey.value(it.key());
        if (selectedObsId.isEmpty()) continue;
        for (const RowInfo &member : it.value()) {
          QStandardItem *stateItem = model_->item(member.row, stateCol);
          if (!stateItem) continue;
          const QString currentState = stateItem->data(Qt::UserRole + 1).toString();
          if (!shouldOverrideState(currentState)) continue;
          const bool isSelected = (member.obsId == selectedObsId);
          const QString desiredState = isSelected ? kDefaultTargetState : QString("unassigned");
          if (currentState.compare(desiredState, Qt::CaseInsensitive) == 0) continue;

          QVariantMap keyValues = keyValuesForRow(member.row);
          if (keyValues.isEmpty()) continue;
          keyValuesList.append(keyValues);
          stateValues.append(desiredState);
          stateUpdates.push_back({member.row, desiredState});
        }
      }

      if (!keyValuesList.isEmpty()) {
        QString err;
        if (db_->updateColumnByKeyBatch(tableName_, "STATE", keyValuesList, stateValues, &err)) {
          const bool prev = suppressItemChange_;
          suppressItemChange_ = true;
          for (const StateUpdate &upd : stateUpdates) {
            if (upd.row < 0) continue;
            QStandardItem *item = model_->item(upd.row, stateCol);
            if (!item) continue;
            item->setData(upd.value, Qt::EditRole);
            item->setData(upd.value, Qt::UserRole + 1);
            item->setData(false, Qt::UserRole + 2);
            item->setText(displayForVariant(upd.value, false));
            item->setForeground(QBrush(view_->palette().color(QPalette::Text)));
          }
          suppressItemChange_ = prev;
        } else {
          qWarning().noquote() << QString("WARN: failed to update STATE selection: %1").arg(err);
        }
      }
    }

    QHash<QString, bool> groupAnyPending;
    if (stateCol >= 0) {
      groupAnyPending.reserve(groupRowsByKey_.size());
      for (auto it = groupRowsByKey_.begin(); it != groupRowsByKey_.end(); ++it) {
        bool anyPending = false;
        for (int memberRow : it.value()) {
          QStandardItem *stateItem = model_->item(memberRow, stateCol);
          if (!stateItem) continue;
          const QString state = stateItem->data(Qt::UserRole + 1).toString().trimmed().toLower();
          if (state == "pending") {
            anyPending = true;
            break;
          }
        }
        groupAnyPending.insert(it.key(), anyPending);
      }
    }

    for (const RowInfo &info : rows) {
      const QString key = info.groupKey;
      const bool isHeader = (groupHeaderRowByKey_.value(key, -1) == info.row);
      const bool expanded = expandedGroups_.contains(key);
      const int count = groupRowsByKey_.value(key).size();
      view_->setRowHidden(info.row, !(isHeader || expanded));

      QStandardItem *nameItem = model_->item(info.row, nameCol);
      if (!nameItem) continue;
      const QString rawName = nameItem->data(Qt::UserRole + 1).toString();
      QString baseName = rawName;
      QRegularExpression countSuffixRe("\\s*\\((\\d+)\\)\\s*$");
      QRegularExpressionMatch countMatch = countSuffixRe.match(baseName);
      if (countMatch.hasMatch()) {
        bool okCount = false;
        const int suffixCount = countMatch.captured(1).toInt(&okCount);
        if (okCount && (suffixCount == count || suffixCount == 1)) {
          baseName = baseName.left(countMatch.capturedStart()).trimmed();
        }
      }
      QString displayName = baseName;
      if (isHeader) {
        if (count > 1) {
          nameItem->setData(expanded ? expandedIcon : collapsedIcon, Qt::DecorationRole);
        } else {
          nameItem->setData(QVariant(), Qt::DecorationRole);
        }
      } else {
        nameItem->setData(QVariant(), Qt::DecorationRole);
        if (expanded) {
          displayName = QString("  %1").arg(baseName);
        }
      }
      const QString selectedObsId = selectedByKey.value(key);
      const bool isSelected = (!selectedObsId.isEmpty() && info.obsId == selectedObsId);
      QFont nameFont = nameItem->font();
      nameFont.setBold(isSelected);
      nameItem->setFont(nameFont);
      nameItem->setData(displayName, Qt::DisplayRole);

      if (stateCol >= 0) {
        QStandardItem *stateItem = model_->item(info.row, stateCol);
        if (stateItem) {
          const bool collapsed = isHeader && !expanded;
          if (collapsed && groupAnyPending.value(key, false)) {
            stateItem->setData("pending", Qt::DisplayRole);
          } else {
            const QVariant val = stateItem->data(Qt::UserRole + 1);
            const bool isNull = stateItem->data(Qt::UserRole + 2).toBool();
            stateItem->setData(displayForVariant(val, isNull), Qt::DisplayRole);
          }
        }
      }
    }

    suppressItemChange_ = prevSuppress;
    updateGroupSequenceNumbers();
  }

  bool moveSingleAfterRow(int fromRow, int toRow, int setId, QString *error) {
    const int obsIdCol = columnIndex("OBSERVATION_ID");
    const int obsOrderCol = columnIndex("OBS_ORDER");
    const int setIdCol = columnIndex("SET_ID");
    if (obsIdCol < 0 || obsOrderCol < 0) {
      if (error) *error = "OBSERVATION_ID/OBS_ORDER columns missing.";
      return false;
    }

    QList<RowInfo> infos;
    infos.reserve(model_->rowCount());
    for (int row = 0; row < model_->rowCount(); ++row) {
      QStandardItem *obsItem = model_->item(row, obsIdCol);
      QStandardItem *orderItem = model_->item(row, obsOrderCol);
      if (!obsItem || !orderItem) continue;
      if (setIdCol >= 0 && setId >= 0) {
        QStandardItem *setItem = model_->item(row, setIdCol);
        if (!setItem || setItem->data(Qt::UserRole + 1).toInt() != setId) continue;
      }
      RowInfo info;
      info.row = row;
      info.obsId = obsItem->data(Qt::UserRole + 1).toString();
      info.obsOrder = orderItem->data(Qt::UserRole + 1).toInt();
      infos.append(info);
    }
    std::sort(infos.begin(), infos.end(),
              [](const RowInfo &a, const RowInfo &b) { return a.obsOrder < b.obsOrder; });

    const QString fromObsId = model_->item(fromRow, obsIdCol)->data(Qt::UserRole + 1).toString();
    const QString toObsId = model_->item(toRow, obsIdCol)->data(Qt::UserRole + 1).toString();
    int fromIdx = -1;
    int toIdx = -1;
    for (int i = 0; i < infos.size(); ++i) {
      if (infos[i].obsId == fromObsId) fromIdx = i;
      if (infos[i].obsId == toObsId) toIdx = i;
    }
    if (fromIdx < 0 || toIdx < 0) {
      if (error) *error = "Target not found for reorder.";
      return false;
    }
    if (fromIdx == toIdx) return true;

    RowInfo moving = infos.takeAt(fromIdx);
    if (fromIdx < toIdx) toIdx--;
    const int insertIdx = std::min(toIdx + 1, static_cast<int>(infos.size()));
    infos.insert(insertIdx, moving);

    QVector<QVariant> obsIds;
    QVector<QVariant> orderValues;
    obsIds.reserve(infos.size());
    orderValues.reserve(infos.size());
    for (int i = 0; i < infos.size(); ++i) {
      obsIds << infos[i].obsId;
      orderValues << (i + 1);
    }

    QString err;
    if (!db_->updateObsOrderByObservationId(tableName_, obsIds, orderValues, &err)) {
      if (error) *error = err;
      return false;
    }
    return true;
  }

  bool moveSingleToPositionRow(int fromRow, int position, int setId, QString *error) {
    const int obsIdCol = columnIndex("OBSERVATION_ID");
    const int obsOrderCol = columnIndex("OBS_ORDER");
    const int setIdCol = columnIndex("SET_ID");
    if (obsIdCol < 0 || obsOrderCol < 0) {
      if (error) *error = "OBSERVATION_ID/OBS_ORDER columns missing.";
      return false;
    }

    QList<RowInfo> infos;
    infos.reserve(model_->rowCount());
    for (int row = 0; row < model_->rowCount(); ++row) {
      QStandardItem *obsItem = model_->item(row, obsIdCol);
      QStandardItem *orderItem = model_->item(row, obsOrderCol);
      if (!obsItem || !orderItem) continue;
      if (setIdCol >= 0 && setId >= 0) {
        QStandardItem *setItem = model_->item(row, setIdCol);
        if (!setItem || setItem->data(Qt::UserRole + 1).toInt() != setId) continue;
      }
      RowInfo info;
      info.row = row;
      info.obsId = obsItem->data(Qt::UserRole + 1).toString();
      info.obsOrder = orderItem->data(Qt::UserRole + 1).toInt();
      infos.append(info);
    }
    std::sort(infos.begin(), infos.end(),
              [](const RowInfo &a, const RowInfo &b) { return a.obsOrder < b.obsOrder; });

    const QString fromObsId = model_->item(fromRow, obsIdCol)->data(Qt::UserRole + 1).toString();
    int fromIdx = -1;
    for (int i = 0; i < infos.size(); ++i) {
      if (infos[i].obsId == fromObsId) {
        fromIdx = i;
        break;
      }
    }
    if (fromIdx < 0) {
      if (error) *error = "Target not found for reorder.";
      return false;
    }
    if (infos.isEmpty()) return true;
    const int targetIdx = std::clamp(position - 1, 0, static_cast<int>(infos.size() - 1));
    if (fromIdx == targetIdx) return true;

    RowInfo moving = infos.takeAt(fromIdx);
    int insertIdx = targetIdx;
    if (fromIdx < targetIdx) insertIdx--;
    if (insertIdx < 0) insertIdx = 0;
    if (insertIdx > infos.size()) insertIdx = infos.size();
    infos.insert(insertIdx, moving);

    QVector<QVariant> obsIds;
    QVector<QVariant> orderValues;
    obsIds.reserve(infos.size());
    orderValues.reserve(infos.size());
    for (int i = 0; i < infos.size(); ++i) {
      obsIds << infos[i].obsId;
      orderValues << (i + 1);
    }

    QString err;
    if (!db_->updateObsOrderByObservationId(tableName_, obsIds, orderValues, &err)) {
      if (error) *error = err;
      return false;
    }
    return true;
  }

  bool moveSingleToTopRow(int fromRow, int setId, QString *error) {
    const int obsIdCol = columnIndex("OBSERVATION_ID");
    const int obsOrderCol = columnIndex("OBS_ORDER");
    const int setIdCol = columnIndex("SET_ID");
    if (obsIdCol < 0 || obsOrderCol < 0) {
      if (error) *error = "OBSERVATION_ID/OBS_ORDER columns missing.";
      return false;
    }

    QList<RowInfo> infos;
    infos.reserve(model_->rowCount());
    for (int row = 0; row < model_->rowCount(); ++row) {
      QStandardItem *obsItem = model_->item(row, obsIdCol);
      QStandardItem *orderItem = model_->item(row, obsOrderCol);
      if (!obsItem || !orderItem) continue;
      if (setIdCol >= 0 && setId >= 0) {
        QStandardItem *setItem = model_->item(row, setIdCol);
        if (!setItem || setItem->data(Qt::UserRole + 1).toInt() != setId) continue;
      }
      RowInfo info;
      info.row = row;
      info.obsId = obsItem->data(Qt::UserRole + 1).toString();
      info.obsOrder = orderItem->data(Qt::UserRole + 1).toInt();
      infos.append(info);
    }
    std::sort(infos.begin(), infos.end(),
              [](const RowInfo &a, const RowInfo &b) { return a.obsOrder < b.obsOrder; });

    const QString fromObsId = model_->item(fromRow, obsIdCol)->data(Qt::UserRole + 1).toString();
    int fromIdx = -1;
    for (int i = 0; i < infos.size(); ++i) {
      if (infos[i].obsId == fromObsId) {
        fromIdx = i;
        break;
      }
    }
    if (fromIdx < 0) {
      if (error) *error = "Target not found for reorder.";
      return false;
    }
    if (fromIdx == 0) return true;

    RowInfo moving = infos.takeAt(fromIdx);
    infos.insert(0, moving);

    QVector<QVariant> obsIds;
    QVector<QVariant> orderValues;
    obsIds.reserve(infos.size());
    orderValues.reserve(infos.size());
    for (int i = 0; i < infos.size(); ++i) {
      obsIds << infos[i].obsId;
      orderValues << (i + 1);
    }

    QString err;
    if (!db_->updateObsOrderByObservationId(tableName_, obsIds, orderValues, &err)) {
      if (error) *error = err;
      return false;
    }
    return true;
  }

  bool moveGroupAfterRow(int fromRow, int toRow, QString *error) {
    const QString fromKey = groupKeyForRow(fromRow);
    const QString toKey = groupKeyForRow(toRow);
    if (fromKey.isEmpty() || toKey.isEmpty()) {
      if (error) *error = "Group not found.";
      return false;
    }
    if (fromKey == toKey) return true;

    const int obsIdCol = columnIndex("OBSERVATION_ID");
    const int obsOrderCol = columnIndex("OBS_ORDER");
    if (obsIdCol < 0 || obsOrderCol < 0) {
      if (error) *error = "OBSERVATION_ID/OBS_ORDER columns missing.";
      return false;
    }

    struct GroupBlock {
      QString key;
      int minOrder = 0;
      QVector<RowInfo> members;
    };

    QHash<QString, GroupBlock> blocks;
    for (auto it = groupRowsByKey_.begin(); it != groupRowsByKey_.end(); ++it) {
      GroupBlock block;
      block.key = it.key();
      block.minOrder = std::numeric_limits<int>::max();
      const int headerRow = groupHeaderRowByKey_.value(block.key, -1);
      for (int row : it.value()) {
        QStandardItem *obsItem = model_->item(row, obsIdCol);
        QStandardItem *orderItem = model_->item(row, obsOrderCol);
        if (!obsItem || !orderItem) continue;
        RowInfo info;
        info.row = row;
        info.obsId = obsItem->data(Qt::UserRole + 1).toString();
        info.obsOrder = orderItem->data(Qt::UserRole + 1).toInt();
        block.members.append(info);
        if (row == headerRow) {
          block.minOrder = info.obsOrder;
        } else if (block.minOrder == std::numeric_limits<int>::max()) {
          block.minOrder = info.obsOrder;
        } else if (headerRow < 0) {
          block.minOrder = std::min(block.minOrder, info.obsOrder);
        }
      }
      std::sort(block.members.begin(), block.members.end(),
                [](const RowInfo &a, const RowInfo &b) { return a.obsOrder < b.obsOrder; });
      blocks.insert(block.key, block);
    }

    QVector<GroupBlock> order;
    order.reserve(blocks.size());
    for (const GroupBlock &block : blocks) {
      order.push_back(block);
    }
    std::sort(order.begin(), order.end(),
              [](const GroupBlock &a, const GroupBlock &b) { return a.minOrder < b.minOrder; });

    int fromIdx = -1;
    int toIdx = -1;
    for (int i = 0; i < order.size(); ++i) {
      if (order[i].key == fromKey) fromIdx = i;
      if (order[i].key == toKey) toIdx = i;
    }
    if (fromIdx < 0 || toIdx < 0) {
      if (error) *error = "Group not found for reorder.";
      return false;
    }
    if (fromIdx == toIdx) return true;

    GroupBlock moving = order.takeAt(fromIdx);
    if (fromIdx < toIdx) toIdx--;
    const int insertIdx = std::min(toIdx + 1, static_cast<int>(order.size()));
    order.insert(insertIdx, moving);

    QVector<QVariant> obsIds;
    QVector<QVariant> orderValues;
    int counter = 1;
    for (const GroupBlock &block : order) {
      for (const RowInfo &member : block.members) {
        obsIds << member.obsId;
        orderValues << counter++;
      }
    }

    QString err;
    if (!db_->updateObsOrderByObservationId(tableName_, obsIds, orderValues, &err)) {
      if (error) *error = err;
      return false;
    }
    return true;
  }

  bool moveGroupToPositionRow(int fromRow, int position, QString *error) {
    const QString fromKey = groupKeyForRow(fromRow);
    if (fromKey.isEmpty()) {
      if (error) *error = "Group not found.";
      return false;
    }

    const int obsIdCol = columnIndex("OBSERVATION_ID");
    const int obsOrderCol = columnIndex("OBS_ORDER");
    if (obsIdCol < 0 || obsOrderCol < 0) {
      if (error) *error = "OBSERVATION_ID/OBS_ORDER columns missing.";
      return false;
    }

    struct GroupBlock {
      QString key;
      int minOrder = 0;
      QVector<RowInfo> members;
    };

    QHash<QString, GroupBlock> blocks;
    for (auto it = groupRowsByKey_.begin(); it != groupRowsByKey_.end(); ++it) {
      GroupBlock block;
      block.key = it.key();
      block.minOrder = std::numeric_limits<int>::max();
      const int headerRow = groupHeaderRowByKey_.value(block.key, -1);
      for (int row : it.value()) {
        QStandardItem *obsItem = model_->item(row, obsIdCol);
        QStandardItem *orderItem = model_->item(row, obsOrderCol);
        if (!obsItem || !orderItem) continue;
        RowInfo info;
        info.row = row;
        info.obsId = obsItem->data(Qt::UserRole + 1).toString();
        info.obsOrder = orderItem->data(Qt::UserRole + 1).toInt();
        block.members.append(info);
        if (row == headerRow) {
          block.minOrder = info.obsOrder;
        } else if (block.minOrder == std::numeric_limits<int>::max()) {
          block.minOrder = info.obsOrder;
        } else if (headerRow < 0) {
          block.minOrder = std::min(block.minOrder, info.obsOrder);
        }
      }
      std::sort(block.members.begin(), block.members.end(),
                [](const RowInfo &a, const RowInfo &b) { return a.obsOrder < b.obsOrder; });
      blocks.insert(block.key, block);
    }

    QVector<GroupBlock> order;
    order.reserve(blocks.size());
    for (const GroupBlock &block : blocks) {
      order.push_back(block);
    }
    std::sort(order.begin(), order.end(),
              [](const GroupBlock &a, const GroupBlock &b) { return a.minOrder < b.minOrder; });

    int fromIdx = -1;
    for (int i = 0; i < order.size(); ++i) {
      if (order[i].key == fromKey) {
        fromIdx = i;
        break;
      }
    }
    if (fromIdx < 0) {
      if (error) *error = "Group not found for reorder.";
      return false;
    }
    if (order.isEmpty()) return true;
    const int targetIdx = std::clamp(position - 1, 0, static_cast<int>(order.size() - 1));
    if (fromIdx == targetIdx) return true;

    GroupBlock moving = order.takeAt(fromIdx);
    int insertIdx = targetIdx;
    if (fromIdx < targetIdx) insertIdx--;
    if (insertIdx < 0) insertIdx = 0;
    if (insertIdx > order.size()) insertIdx = order.size();
    order.insert(insertIdx, moving);

    QVector<QVariant> obsIds;
    QVector<QVariant> orderValues;
    int counter = 1;
    for (const GroupBlock &block : order) {
      for (const RowInfo &member : block.members) {
        obsIds << member.obsId;
        orderValues << counter++;
      }
    }

    QString err;
    if (!db_->updateObsOrderByObservationId(tableName_, obsIds, orderValues, &err)) {
      if (error) *error = err;
      return false;
    }
    return true;
  }

  bool moveGroupToTopRow(int fromRow, QString *error) {
    const QString fromKey = groupKeyForRow(fromRow);
    if (fromKey.isEmpty()) {
      if (error) *error = "Group not found.";
      return false;
    }

    const int obsIdCol = columnIndex("OBSERVATION_ID");
    const int obsOrderCol = columnIndex("OBS_ORDER");
    if (obsIdCol < 0 || obsOrderCol < 0) {
      if (error) *error = "OBSERVATION_ID/OBS_ORDER columns missing.";
      return false;
    }

    struct GroupBlock {
      QString key;
      int minOrder = 0;
      QVector<RowInfo> members;
    };

    QHash<QString, GroupBlock> blocks;
    for (auto it = groupRowsByKey_.begin(); it != groupRowsByKey_.end(); ++it) {
      GroupBlock block;
      block.key = it.key();
      block.minOrder = std::numeric_limits<int>::max();
      const int headerRow = groupHeaderRowByKey_.value(block.key, -1);
      for (int row : it.value()) {
        QStandardItem *obsItem = model_->item(row, obsIdCol);
        QStandardItem *orderItem = model_->item(row, obsOrderCol);
        if (!obsItem || !orderItem) continue;
        RowInfo info;
        info.row = row;
        info.obsId = obsItem->data(Qt::UserRole + 1).toString();
        info.obsOrder = orderItem->data(Qt::UserRole + 1).toInt();
        block.members.append(info);
        if (row == headerRow) {
          block.minOrder = info.obsOrder;
        } else if (block.minOrder == std::numeric_limits<int>::max()) {
          block.minOrder = info.obsOrder;
        } else if (headerRow < 0) {
          block.minOrder = std::min(block.minOrder, info.obsOrder);
        }
      }
      std::sort(block.members.begin(), block.members.end(),
                [](const RowInfo &a, const RowInfo &b) { return a.obsOrder < b.obsOrder; });
      blocks.insert(block.key, block);
    }

    QVector<GroupBlock> order;
    order.reserve(blocks.size());
    for (const GroupBlock &block : blocks) {
      order.push_back(block);
    }
    std::sort(order.begin(), order.end(),
              [](const GroupBlock &a, const GroupBlock &b) { return a.minOrder < b.minOrder; });

    int fromIdx = -1;
    for (int i = 0; i < order.size(); ++i) {
      if (order[i].key == fromKey) {
        fromIdx = i;
        break;
      }
    }
    if (fromIdx < 0) {
      if (error) *error = "Group not found for reorder.";
      return false;
    }
    if (fromIdx == 0) return true;

    GroupBlock moving = order.takeAt(fromIdx);
    order.insert(0, moving);

    QVector<QVariant> obsIds;
    QVector<QVariant> orderValues;
    int counter = 1;
    for (const GroupBlock &block : order) {
      for (const RowInfo &member : block.members) {
        obsIds << member.obsId;
        orderValues << counter++;
      }
    }

    QString err;
    if (!db_->updateObsOrderByObservationId(tableName_, obsIds, orderValues, &err)) {
      if (error) *error = err;
      return false;
    }
    return true;
  }

  bool deleteGroupForRow(int row, QString *error) {
    const QString key = groupKeyForRow(row);
    if (key.isEmpty()) {
      if (error) *error = "Group not found.";
      return false;
    }
    const QList<int> members = groupRowsByKey_.value(key);
    if (members.isEmpty()) return true;
    int setId = -1;
    const int setIdCol = columnIndex("SET_ID");
    if (setIdCol >= 0) {
      QStandardItem *setItem = model_->item(members.first(), setIdCol);
      if (setItem) setId = setItem->data(Qt::UserRole + 1).toInt();
    }
    QList<QVariantMap> keyList;
    keyList.reserve(members.size());
    for (int memberRow : members) {
      QVariantMap keyValues = keyValuesForRow(memberRow);
      if (!keyValues.isEmpty()) {
        keyList.append(keyValues);
      }
    }
    QString err;
    for (const QVariantMap &keyValues : keyList) {
      if (!db_->deleteRecordByKey(tableName_, keyValues, &err)) {
        if (error) *error = err;
        return false;
      }
    }
    if (setId >= 0) {
      QString normError;
      if (!normalizeObsOrderForSet(setId, &normError)) {
        if (error) *error = normError;
        return false;
      }
    }
    return true;
  }

  bool normalizeObsOrderForSet(int setId, QString *error) {
    if (!db_ || !db_->isOpen()) {
      if (error) *error = "Not connected";
      return false;
    }
    if (setId < 0) return true;
    QList<ColumnMeta> cols;
    if (!db_->loadColumns(tableName_, cols, error)) {
      return false;
    }
    QVector<QVector<QVariant>> rows;
    if (!db_->fetchRows(tableName_, cols, "SET_ID", QString::number(setId),
                        "", "", "OBS_ORDER", rows, error)) {
      return false;
    }
    int obsIdCol = -1;
    for (int i = 0; i < cols.size(); ++i) {
      if (cols[i].name.compare("OBSERVATION_ID", Qt::CaseInsensitive) == 0) {
        obsIdCol = i;
        break;
      }
    }
    if (obsIdCol < 0) return true;
    QVector<QVariant> obsIds;
    QVector<QVariant> orderValues;
    obsIds.reserve(rows.size());
    orderValues.reserve(rows.size());
    for (int i = 0; i < rows.size(); ++i) {
      if (obsIdCol >= rows[i].size()) continue;
      const QVariant obsId = rows[i].at(obsIdCol);
      obsIds << obsId;
      orderValues << (i + 1);
    }
    QString err;
    if (!db_->updateObsOrderByObservationId(tableName_, obsIds, orderValues, &err)) {
      if (error) *error = err;
      return false;
    }
    return true;
  }

  bool moveRowAfter(int from, int to, QString *errorOut) {
    if (!allowReorder_) return false;
    if (!searchEdit_->text().trimmed().isEmpty()) {
      showInfo(this, "Reorder disabled",
                               "Clear the search filter before reordering.");
      return false;
    }
    if (from < 0 || to < 0 || from >= model_->rowCount() || to >= model_->rowCount()) return false;
    if (from == to) return true;
    if (!db_ || !db_->isOpen()) {
      showWarning(this, "Reorder failed", "Not connected");
      return false;
    }

    const SwapInfo src = swapInfoForRow(from);
    const SwapInfo dst = swapInfoForRow(to);
    if (!src.valid || !dst.valid) {
      showWarning(this, "Reorder failed",
                           "Missing OBSERVATION_ID/OBS_ORDER values.");
      return false;
    }

    const int setIdCol = columnIndex("SET_ID");
    int setId = -1;
    if (setIdCol >= 0) {
      QStandardItem *setItem = model_->item(from, setIdCol);
      if (setItem) setId = setItem->data(Qt::UserRole + 1).toInt();
    }

    ViewState state = captureViewState();
    const bool groupMove = groupingEnabled_ && isGroupHeaderRow(from) &&
                           !expandedGroups_.contains(groupKeyForRow(from));

    QString error;
    if (groupMove) {
      if (!moveGroupAfterRow(from, to, &error)) {
        if (errorOut) *errorOut = error;
        showWarning(this, "Reorder failed", error.isEmpty() ? "Group move failed." : error);
        refresh();
        return false;
      }
    } else {
      if (!moveSingleAfterRow(from, to, setId, &error)) {
        if (errorOut) *errorOut = error;
        showWarning(this, "Reorder failed", error.isEmpty() ? "Move failed." : error);
        refresh();
        return false;
      }
    }

    refreshWithState(state);
    emit dataMutated();
    return true;
  }

  bool moveSingleAfterRowWithRefresh(int fromRow, int toRow, QString *errorOut) {
    if (!allowReorder_) return false;
    if (!searchEdit_->text().trimmed().isEmpty()) {
      showInfo(this, "Reorder disabled",
                               "Clear the search filter before reordering.");
      return false;
    }
    if (fromRow < 0 || toRow < 0 || fromRow >= model_->rowCount() || toRow >= model_->rowCount()) {
      return false;
    }
    if (fromRow == toRow) return true;
    if (!db_ || !db_->isOpen()) {
      showWarning(this, "Reorder failed", "Not connected");
      return false;
    }

    const int setIdCol = columnIndex("SET_ID");
    int setId = -1;
    if (setIdCol >= 0) {
      QStandardItem *setItem = model_->item(fromRow, setIdCol);
      if (setItem) setId = setItem->data(Qt::UserRole + 1).toInt();
    }

    ViewState state = captureViewState();
    QString error;
    if (!moveSingleAfterRow(fromRow, toRow, setId, &error)) {
      if (errorOut) *errorOut = error;
      showWarning(this, "Reorder failed", error.isEmpty() ? "Move failed." : error);
      refresh();
      return false;
    }
    refreshWithState(state);
    emit dataMutated();
    return true;
  }

  int previousVisibleRow(int row) const {
    for (int r = row - 1; r >= 0; --r) {
      if (!view_->isRowHidden(r)) return r;
    }
    return -1;
  }

  int nextVisibleRow(int row) const {
    for (int r = row + 1; r < model_->rowCount(); ++r) {
      if (!view_->isRowHidden(r)) return r;
    }
    return -1;
  }

  int firstVisibleRow() const {
    for (int r = 0; r < model_->rowCount(); ++r) {
      if (!view_->isRowHidden(r)) return r;
    }
    return -1;
  }

  int lastVisibleRow() const {
    for (int r = model_->rowCount() - 1; r >= 0; --r) {
      if (!view_->isRowHidden(r)) return r;
    }
    return -1;
  }

  void applyHiddenColumns() {
    if (!view_ || columns_.isEmpty()) return;
    for (int i = 0; i < columns_.size(); ++i) {
      const QString name = columns_.at(i).name.toUpper();
      const bool hide = hiddenColumns_.contains(name);
      view_->setColumnHidden(i, hide);
    }
  }

  void applyColumnOrderRules() {
    if (!view_ || columns_.isEmpty() || columnAfterRules_.isEmpty()) return;
    QHeaderView *header = view_->horizontalHeader();
    for (const auto &rule : columnAfterRules_) {
      const int anchor = columnIndex(rule.first);
      const int column = columnIndex(rule.second);
      if (anchor < 0 || column < 0) continue;
      const int anchorVisual = header->visualIndex(anchor);
      int columnVisual = header->visualIndex(column);
      if (anchorVisual < 0 || columnVisual < 0) continue;
      const int target = anchorVisual + 1;
      if (columnVisual == target) continue;
      header->moveSection(columnVisual, target);
    }
  }

  void updateGroupSequenceNumbers() {
    if (!view_ || !model_) return;
    if (!groupingEnabled_ || groupRowsByKey_.isEmpty()) {
      for (int row = 0; row < model_->rowCount(); ++row) {
        model_->setHeaderData(row, Qt::Vertical, QVariant());
      }
      return;
    }

    const int obsOrderCol = columnIndex("OBS_ORDER");
    struct GroupOrder {
      QString key;
      int order = 0;
    };
    QVector<GroupOrder> groupOrder;
    groupOrder.reserve(groupRowsByKey_.size());
    for (auto it = groupRowsByKey_.begin(); it != groupRowsByKey_.end(); ++it) {
      int minOrder = std::numeric_limits<int>::max();
      const int headerRow = groupHeaderRowByKey_.value(it.key(), -1);
      if (obsOrderCol >= 0) {
        for (int row : it.value()) {
          QStandardItem *orderItem = model_->item(row, obsOrderCol);
          if (!orderItem) continue;
          const int orderVal = orderItem->data(Qt::UserRole + 1).toInt();
          if (row == headerRow) {
            minOrder = orderVal;
            break;
          }
          if (orderVal < minOrder) minOrder = orderVal;
        }
      } else if (!it.value().isEmpty()) {
        minOrder = it.value().first();
      }
      if (minOrder == std::numeric_limits<int>::max()) minOrder = 0;
      groupOrder.append({it.key(), minOrder});
    }
    std::sort(groupOrder.begin(), groupOrder.end(),
              [](const GroupOrder &a, const GroupOrder &b) { return a.order < b.order; });

    QHash<QString, int> seqByKey;
    int seq = 1;
    for (const GroupOrder &group : groupOrder) {
      seqByKey.insert(group.key, seq++);
    }

    for (int row = 0; row < model_->rowCount(); ++row) {
      const QString key = groupKeyByRow_.value(row);
      if (key.isEmpty()) {
        model_->setHeaderData(row, Qt::Vertical, QVariant());
      } else {
        const int num = seqByKey.value(key, row + 1);
        model_->setHeaderData(row, Qt::Vertical, QString::number(num));
      }
    }
  }

  QString headerSettingsKey() const {
    if (tableName_.isEmpty()) return QString();
    return QString("tableHeaders/%1/state").arg(tableName_);
  }

  QString groupingSettingsKey(const QString &suffix) const {
    if (tableName_.isEmpty()) return QString();
    return QString("tableGrouping/%1/%2").arg(tableName_, suffix);
  }

  bool restoreHeaderState() {
    if (!view_ || tableName_.isEmpty()) return false;
    QSettings settings(kSettingsOrg, kSettingsApp);
    const QByteArray state = settings.value(headerSettingsKey()).toByteArray();
    if (state.isEmpty()) return false;
    headerStateUpdating_ = true;
    const bool ok = view_->horizontalHeader()->restoreState(state);
    headerStateUpdating_ = false;
    if (ok) {
      headerStateLoaded_ = true;
    }
    return ok;
  }

  void scheduleHeaderStateSave() {
    if (headerStateUpdating_ || !headerSaveTimer_) return;
    headerSaveTimer_->start(200);
  }

  void saveHeaderState() {
    if (headerStateUpdating_ || !view_ || tableName_.isEmpty()) return;
    QSettings settings(kSettingsOrg, kSettingsApp);
    settings.setValue(headerSettingsKey(), view_->horizontalHeader()->saveState());
    headerStateLoaded_ = true;
  }

  void loadGroupingState() {
    manualUngroupObsIds_.clear();
    manualGroupKeyByObsId_.clear();
    selectedObsIdByHeader_.clear();
    if (tableName_.isEmpty()) return;
    QSettings settings(kSettingsOrg, kSettingsApp);
    const QStringList ungrouped = settings.value(groupingSettingsKey("manualUngroup")).toStringList();
    for (const QString &obsId : ungrouped) {
      if (!obsId.trimmed().isEmpty()) manualUngroupObsIds_.insert(obsId.trimmed());
    }
    const QStringList groups = settings.value(groupingSettingsKey("manualGroups")).toStringList();
    for (const QString &entry : groups) {
      const int idx = entry.indexOf('=');
      if (idx <= 0) continue;
      const QString obsId = entry.left(idx).trimmed();
      const QString key = entry.mid(idx + 1).trimmed();
      if (obsId.isEmpty() || key.isEmpty()) continue;
      manualGroupKeyByObsId_.insert(obsId, key);
    }
    const QStringList selected = settings.value(groupingSettingsKey("selectedObs")).toStringList();
    for (const QString &entry : selected) {
      const int idx = entry.indexOf('=');
      if (idx <= 0) continue;
      const QString headerObsId = entry.left(idx).trimmed();
      const QString selectedObsId = entry.mid(idx + 1).trimmed();
      if (headerObsId.isEmpty() || selectedObsId.isEmpty()) continue;
      selectedObsIdByHeader_.insert(headerObsId, selectedObsId);
    }
  }

  void saveGroupingState() {
    if (tableName_.isEmpty()) return;
    QSettings settings(kSettingsOrg, kSettingsApp);
    QStringList ungrouped = manualUngroupObsIds_.values();
    ungrouped.sort();
    settings.setValue(groupingSettingsKey("manualUngroup"), ungrouped);
    QStringList groups;
    groups.reserve(manualGroupKeyByObsId_.size());
    for (auto it = manualGroupKeyByObsId_.begin(); it != manualGroupKeyByObsId_.end(); ++it) {
      groups << QString("%1=%2").arg(it.key(), it.value());
    }
    groups.sort();
    settings.setValue(groupingSettingsKey("manualGroups"), groups);

    QStringList selected;
    selected.reserve(selectedObsIdByHeader_.size());
    for (auto it = selectedObsIdByHeader_.begin(); it != selectedObsIdByHeader_.end(); ++it) {
      if (it.key().trimmed().isEmpty() || it.value().trimmed().isEmpty()) continue;
      selected << QString("%1=%2").arg(it.key(), it.value());
    }
    selected.sort();
    settings.setValue(groupingSettingsKey("selectedObs"), selected);
  }

  void deleteRow(int row) {
    if (!allowDelete_) return;
    if (row < 0 || row >= model_->rowCount()) return;
    if (groupingEnabled_ && isGroupHeaderRow(row) && !expandedGroups_.contains(groupKeyForRow(row))) {
      if (QMessageBox::question(this, "Delete Target Group",
                                "Delete all targets in this group?",
                                QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) {
        return;
      }
      QString error;
      if (!deleteGroupForRow(row, &error)) {
        showWarning(this, "Delete failed", error);
        return;
      }
      refreshWithState(captureViewState());
      emit dataMutated();
      return;
    }

    if (QMessageBox::question(this, "Delete Target", "Delete the selected target?",
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) {
      return;
    }
    QVariantMap keyValues = keyValuesForRow(row);
    if (keyValues.isEmpty()) {
      showWarning(this, "Delete failed", "Missing primary key values.");
      return;
    }
    QString error;
    if (!db_->deleteRecordByKey(tableName_, keyValues, &error)) {
      showWarning(this, "Delete failed", error);
      return;
    }
    const int setIdCol = columnIndex("SET_ID");
    const int obsOrderCol = columnIndex("OBS_ORDER");
    int setId = -1;
    int oldPos = -1;
    if (setIdCol >= 0 && obsOrderCol >= 0) {
      QStandardItem *setItem = model_->item(row, setIdCol);
      QStandardItem *orderItem = model_->item(row, obsOrderCol);
      if (setItem) setId = setItem->data(Qt::UserRole + 1).toInt();
      if (orderItem) oldPos = orderItem->data(Qt::UserRole + 1).toInt();
    }

    ViewState state = captureViewState();
    suppressItemChange_ = true;
    model_->removeRow(row);
    suppressItemChange_ = false;

    if (setId >= 0 && oldPos >= 0) {
      QString shiftError;
      if (!db_->shiftObsOrderAfterDelete(tableName_, setId, oldPos, &shiftError)) {
        showWarning(this, "Reorder failed", shiftError);
        refresh();
        return;
      }
    }
    refreshWithState(state);
    emit dataMutated();
  }

  void duplicateRow(int row) {
    if (!db_ || !db_->isOpen()) return;
    if (row < 0 || row >= model_->rowCount()) return;

    const int setIdCol = columnIndex("SET_ID");
    const int obsOrderCol = columnIndex("OBS_ORDER");
    if (setIdCol < 0 || obsOrderCol < 0) {
      // Fallback: simple duplicate to end if ordering columns missing.
      QVariantMap values;
      QSet<QString> nullColumns;
      for (int c = 0; c < columns_.size(); ++c) {
        const ColumnMeta &meta = columns_.at(c);
        if (meta.isAutoIncrement()) continue;
        QStandardItem *item = model_->item(row, c);
        if (!item) continue;
        const bool isNull = item->data(Qt::UserRole + 2).toBool();
        const QVariant value = item->data(Qt::UserRole + 1);
        if (isNull) {
          nullColumns.insert(meta.name);
        } else {
          values.insert(meta.name, value);
        }
      }
      QString error;
      if (!insertRecord(values, nullColumns, &error)) {
        showWarning(this, "Duplicate failed", error);
        return;
      }
      refreshWithState(captureViewState());
      emit dataMutated();
      return;
    }

    QStandardItem *setItem = model_->item(row, setIdCol);
    if (!setItem) return;
    const int setId = setItem->data(Qt::UserRole + 1).toInt();
    if (setId <= 0) return;

    const QString groupKey = groupKeyForRow(row);
    const bool groupDuplicate = groupingEnabled_ && isGroupHeaderRow(row) &&
                                !expandedGroups_.contains(groupKey);

    QVector<int> sourceRows;
    if (groupDuplicate && !groupKey.isEmpty()) {
      for (int memberRow : groupRowsByKey_.value(groupKey)) {
        sourceRows.append(memberRow);
      }
    } else {
      sourceRows.append(row);
    }
    if (sourceRows.isEmpty()) return;

    std::sort(sourceRows.begin(), sourceRows.end(), [&](int a, int b) {
      QStandardItem *orderA = model_->item(a, obsOrderCol);
      QStandardItem *orderB = model_->item(b, obsOrderCol);
      const int oa = orderA ? orderA->data(Qt::UserRole + 1).toInt() : 0;
      const int ob = orderB ? orderB->data(Qt::UserRole + 1).toInt() : 0;
      return oa < ob;
    });

    QStandardItem *orderItem = model_->item(sourceRows.last(), obsOrderCol);
    if (!orderItem) return;
    const int lastOrder = orderItem->data(Qt::UserRole + 1).toInt();
    const int insertPos = lastOrder + 1;
    const int count = sourceRows.size();

    QString error;
    if (!db_->shiftObsOrderForInsert(tableName_, setId, insertPos, count, &error)) {
      showWarning(this, "Duplicate failed", error.isEmpty() ? "Failed to insert target(s)." : error);
      return;
    }

    bool insertedAll = true;
    for (int i = 0; i < sourceRows.size(); ++i) {
      const int srcRow = sourceRows.at(i);
      QVariantMap values;
      QSet<QString> nullColumns;
      for (int c = 0; c < columns_.size(); ++c) {
        const ColumnMeta &meta = columns_.at(c);
        if (meta.isAutoIncrement()) continue;
        QStandardItem *item = model_->item(srcRow, c);
        if (!item) continue;
        const bool isNull = item->data(Qt::UserRole + 2).toBool();
        const QVariant value = item->data(Qt::UserRole + 1);
        if (isNull) {
          nullColumns.insert(meta.name);
        } else {
          values.insert(meta.name, value);
        }
      }
      values.insert("OBS_ORDER", insertPos + i);
      nullColumns.remove("OBS_ORDER");
      if (!insertRecord(values, nullColumns, &error)) {
        insertedAll = false;
        showWarning(this, "Duplicate failed", error);
        break;
      }
    }

    if (!insertedAll) {
      QString normErr;
      normalizeObsOrderForSet(setId, &normErr);
      refreshWithState(captureViewState());
      emit dataMutated();
      return;
    }

    ViewState state = captureViewState();
    refreshWithState(state);
    emit dataMutated();

    const QStringList newObsIds = obsIdsForObsOrderRange(setId, insertPos, count);
    if (!newObsIds.isEmpty()) {
      if (groupDuplicate) {
        const QString groupId =
            QString("MANUAL:%1").arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
        for (const QString &newObsId : newObsIds) {
          manualUngroupObsIds_.remove(newObsId);
          manualGroupKeyByObsId_.insert(newObsId, groupId);
        }
        saveGroupingState();
        applyGrouping();
      } else {
        int groupSize = 1;
        if (groupingEnabled_ && !groupKey.isEmpty()) {
          groupSize = groupRowsByKey_.value(groupKey).size();
        }
        if (groupSize <= 1) {
          for (const QString &newObsId : newObsIds) {
            manualUngroupObsIds_.insert(newObsId);
            manualGroupKeyByObsId_.remove(newObsId);
          }
          saveGroupingState();
          applyGrouping();
        }
      }
    }
  }

  QVariantMap keyValuesForRow(int row) const {
    QVariantMap keyValues;
    for (int c = 0; c < columns_.size(); ++c) {
      if (!columns_[c].isPrimaryKey()) continue;
      QStandardItem *item = model_->item(row, c);
      if (!item) continue;
      const QVariant keyValue = item->data(Qt::UserRole + 1);
      const bool keyIsNull = item->data(Qt::UserRole + 2).toBool();
      if (!keyValue.isValid() || keyIsNull) return QVariantMap();
      keyValues.insert(columns_[c].name, keyValue);
    }
    return keyValues;
  }

  int columnIndex(const QString &name) const {
    for (int i = 0; i < columns_.size(); ++i) {
      if (columns_[i].name.compare(name, Qt::CaseInsensitive) == 0) return i;
    }
    return -1;
  }

  bool isColumnBulkEditable(int col) const {
    if (col < 0 || col >= columns_.size()) return false;
    const ColumnMeta &meta = columns_.at(col);
    if (meta.isPrimaryKey() || meta.isAutoIncrement()) return false;
    if (hiddenColumns_.contains(meta.name.toUpper())) return false;
    return true;
  }

  QStringList editableColumnsForBulkEdit() const {
    QStringList result;
    for (const ColumnMeta &meta : columns_) {
      if (meta.isPrimaryKey() || meta.isAutoIncrement()) continue;
      if (hiddenColumns_.contains(meta.name.toUpper())) continue;
      result << meta.name;
    }
    return result;
  }

  QStringList obsIdsInView() const {
    QStringList obsIds;
    const int obsIdCol = columnIndex("OBSERVATION_ID");
    if (obsIdCol < 0) return obsIds;
    QSet<QString> seen;
    for (int row = 0; row < model_->rowCount(); ++row) {
      QStandardItem *item = model_->item(row, obsIdCol);
      if (!item) continue;
      const QString obsId = item->data(Qt::UserRole + 1).toString();
      if (obsId.isEmpty() || seen.contains(obsId)) continue;
      seen.insert(obsId);
      obsIds.append(obsId);
    }
    return obsIds;
  }

  QStringList obsIdsForObsOrderRange(int setId, int startOrder, int count) const {
    QStringList obsIds;
    if (count <= 0) return obsIds;
    const int obsIdCol = columnIndex("OBSERVATION_ID");
    const int obsOrderCol = columnIndex("OBS_ORDER");
    const int setIdCol = columnIndex("SET_ID");
    if (obsIdCol < 0 || obsOrderCol < 0 || setIdCol < 0) return obsIds;
    QMap<int, QString> ordered;
    const int endOrder = startOrder + count - 1;
    for (int row = 0; row < model_->rowCount(); ++row) {
      QStandardItem *setItem = model_->item(row, setIdCol);
      QStandardItem *orderItem = model_->item(row, obsOrderCol);
      QStandardItem *obsItem = model_->item(row, obsIdCol);
      if (!setItem || !orderItem || !obsItem) continue;
      const int rowSetId = setItem->data(Qt::UserRole + 1).toInt();
      if (rowSetId != setId) continue;
      const int orderVal = orderItem->data(Qt::UserRole + 1).toInt();
      if (orderVal < startOrder || orderVal > endOrder) continue;
      const QString obsId = obsItem->data(Qt::UserRole + 1).toString();
      if (!obsId.isEmpty()) ordered.insert(orderVal, obsId);
    }
    for (auto it = ordered.begin(); it != ordered.end(); ++it) {
      obsIds.append(it.value());
    }
    return obsIds;
  }

  void revertItem(QStandardItem *item, const QVariant &oldValue, bool oldIsNull) {
    suppressItemChange_ = true;
    item->setData(oldIsNull ? QVariant() : oldValue, Qt::EditRole);
    item->setData(oldIsNull ? QVariant() : oldValue, Qt::UserRole + 1);
    item->setData(oldIsNull, Qt::UserRole + 2);
    item->setText(displayForVariant(oldValue, oldIsNull));
    item->setForeground(QBrush(oldIsNull ? view_->palette().color(QPalette::Disabled, QPalette::Text)
                                         : view_->palette().color(QPalette::Text)));
    suppressItemChange_ = false;
  }

  bool insertRecord(const QVariantMap &values, const QSet<QString> &nullColumns, QString *error) {
    if (!db_) {
      if (error) *error = "Not connected";
      return false;
    }
    return db_->insertRecord(tableName_, columns_, values, nullColumns, error);
  }

  bool updateRecord(const QVariantMap &values, const QSet<QString> &nullColumns,
                    const QVariantMap &keyValues, QString *error) {
    if (!db_) {
      if (error) *error = "Not connected";
      return false;
    }
    return db_->updateRecord(tableName_, columns_, values, nullColumns, keyValues, error);
  }

  DbClient *db_ = nullptr;
  QString tableName_;
  QList<ColumnMeta> columns_;
  QStandardItemModel *model_ = nullptr;
  ReorderTableView *view_ = nullptr;

  QPushButton *refreshButton_ = nullptr;
  QPushButton *addButton_ = nullptr;
  QLabel *statusLabel_ = nullptr;

  QLabel *searchLabel_ = nullptr;
  QLineEdit *searchEdit_ = nullptr;
  QPushButton *searchApply_ = nullptr;
  QPushButton *searchClear_ = nullptr;
  QString searchColumn_;
  QString fixedFilterColumn_;
  QString fixedFilterValue_;
  QString orderByColumn_;
  bool sortingEnabled_ = true;
  bool allowReorder_ = false;
  bool allowDelete_ = false;
  bool allowColumnHeaderBulkEdit_ = false;
  bool quickAddEnabled_ = false;
  bool quickAddInsertAtTop_ = false;
  std::function<NormalizationResult(QVariantMap &, QSet<QString> &)> normalizer_;
  std::function<bool(QVariantMap &, QSet<QString> &, QString *)> quickAddBuilder_;
  QStringList hiddenColumns_;
  QVector<QPair<QString, QString>> columnAfterRules_;

  bool suppressItemChange_ = false;
  bool headerStateLoaded_ = false;
  bool headerStateUpdating_ = false;
  QTimer *headerSaveTimer_ = nullptr;
  bool headerRulesPending_ = false;

  bool groupingEnabled_ = false;
  QHash<QString, QList<int>> groupRowsByKey_;
  QHash<QString, int> groupHeaderRowByKey_;
  QHash<int, QString> groupKeyByRow_;
  QSet<QString> expandedGroups_;
  QSet<QString> manualUngroupObsIds_;
  QHash<QString, QString> manualGroupKeyByObsId_;
  QHash<QString, QString> selectedObsIdByHeader_;
  bool groupingStateLoaded_ = false;
  QPointer<QDialog> moveToDialog_;
};

class TimelineCanvas : public QWidget {
  Q_OBJECT
public:
  explicit TimelineCanvas(QWidget *parent = nullptr) : QWidget(parent) {
    setMouseTracking(true);
  }

  void setData(const TimelineData &data) {
    data_ = data;
    setMinimumHeight(sizeHint().height());
    updateGeometry();
    update();
  }

  void clear() {
    data_ = TimelineData();
    selectedObsId_.clear();
    setMinimumHeight(sizeHint().height());
    updateGeometry();
    update();
  }

  void setSelectedObsId(const QString &obsId) {
    if (selectedObsId_ == obsId) return;
    selectedObsId_ = obsId;
    setMinimumHeight(sizeHint().height());
    updateGeometry();
    update();
  }

  QSize sizeHint() const override {
    int height = topMargin_ + bottomMargin_;
    if (data_.targets.isEmpty()) {
      height += rowHeight_;
    } else {
      for (int i = 0; i < data_.targets.size(); ++i) {
        height += rowHeightForIndex(i);
      }
    }
    const int width = leftMargin_ + rightMargin_ + 720;
    return QSize(width, height);
  }

signals:
  void targetSelected(const QString &obsId);
  void targetReorderRequested(const QString &fromObsId, const QString &toObsId);
  void flagClicked(const QString &obsId, const QString &flagText);
  void contextMenuRequested(const QString &obsId, const QPoint &globalPos);
  void exptimeEditRequested(const QString &obsId);

protected:
  void mouseDoubleClickEvent(QMouseEvent *event) override {
    if (data_.targets.isEmpty()) return;
    const int index = rowAt(event->pos());
    if (index < 0 || index >= data_.targets.size()) return;
    const TimelineTarget &target = data_.targets.at(index);
    QVector<QPair<QDateTime, QDateTime>> segments = target.segments;
    if (segments.isEmpty() && target.startUtc.isValid() && target.endUtc.isValid()) {
      segments.append({target.startUtc, target.endUtc});
    }
    if (segments.isEmpty()) return;
    if (data_.timesUtc.isEmpty()) return;
    const qint64 t0 = data_.timesUtc.first().toMSecsSinceEpoch();
    const qint64 t1 = data_.timesUtc.last().toMSecsSinceEpoch();
    const QRect plotRect = plotArea();
    const QRect rowRect = rowArea(plotRect, index);
    for (const auto &segment : segments) {
      const qint64 s = segment.first.toMSecsSinceEpoch();
      const qint64 e = segment.second.toMSecsSinceEpoch();
      if (e <= t0 || s >= t1) continue;
      const double x1 = timeToX(std::max(s, t0), t0, t1, rowRect);
      const double x2 = timeToX(std::min(e, t1), t0, t1, rowRect);
      const double y = rowRect.center().y() - barHeight_ / 2.0;
      QRectF bar(x1, y, x2 - x1, barHeight_);
      if (bar.contains(event->pos())) {
        if (!target.obsId.isEmpty()) {
          emit exptimeEditRequested(target.obsId);
        }
        event->accept();
        return;
      }
    }
    QWidget::mouseDoubleClickEvent(event);
  }
  void contextMenuEvent(QContextMenuEvent *event) override {
    if (data_.targets.isEmpty()) return;
    const int index = rowAt(event->pos());
    if (index < 0 || index >= data_.targets.size()) return;
    const QString obsId = data_.targets.at(index).obsId;
    if (!obsId.isEmpty()) {
      emit contextMenuRequested(obsId, event->globalPos());
    }
  }

  void paintEvent(QPaintEvent *) override {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.fillRect(rect(), palette().base());

    if (data_.targets.isEmpty() || data_.timesUtc.isEmpty()) {
      painter.setPen(palette().color(QPalette::Disabled, QPalette::Text));
      painter.drawText(rect(), Qt::AlignCenter, "Run OTM to view timeline.");
      return;
    }

    const QRect plotRect = plotArea();
    painter.setPen(palette().color(QPalette::Mid));
    painter.drawRect(plotRect.adjusted(0, 0, -1, -1));

    const qint64 t0 = data_.timesUtc.first().toMSecsSinceEpoch();
    const qint64 t1 = data_.timesUtc.last().toMSecsSinceEpoch();
    if (t1 <= t0) return;

    drawTwilightLines(&painter, plotRect, t0, t1);
    drawTimeAxis(&painter, plotRect, t0, t1);

    drawRowSeparators(&painter, plotRect);

    const double airmassMin = 1.0;
    const double airmassMax = data_.airmassLimit > 0.0 ? data_.airmassLimit : 4.0;

    drawIdleGaps(&painter, plotRect, t0, t1);

    for (int i = 0; i < data_.targets.size(); ++i) {
      const TimelineTarget &target = data_.targets.at(i);
      const QRect rowRect = rowArea(plotRect, i);
      const bool selected = (!selectedObsId_.isEmpty() && target.obsId == selectedObsId_);
      const bool observed = target.observed || !target.segments.isEmpty() ||
                            (target.startUtc.isValid() && target.endUtc.isValid());

      QColor labelColor = palette().color(QPalette::Text);
      if (!observed) {
        labelColor = palette().color(QPalette::Disabled, QPalette::Text);
      } else if (target.severity == 2) {
        labelColor = QColor(220, 70, 70);
      } else if (target.severity == 1) {
        labelColor = QColor(255, 165, 0);
      }
      QString displayName = target.name.isEmpty() ? target.obsId : target.name;
      if (!displayName.isEmpty()) {
        QRegularExpression countSuffixRe("\\s*\\((\\d+)\\)\\s*$");
        displayName = displayName.left(countSuffixRe.match(displayName).capturedStart()).trimmed();
      }
      const int labelWidth = leftMargin_ - 6;
      const int orderWidth = 44;
      const int gap = 8;
      QRect orderRect(0, rowRect.top(), std::min(orderWidth, labelWidth), rowRect.height());
      QRect nameRect(orderRect.right() + gap, rowRect.top(),
                     std::max(0, labelWidth - orderRect.width() - gap), rowRect.height());
      if (target.obsOrder > 0) {
        QColor orderColor = labelColor.darker(140);
        painter.setPen(orderColor);
        painter.drawText(orderRect, Qt::AlignLeft | Qt::AlignVCenter,
                         QString::number(target.obsOrder));
      }
      painter.setPen(labelColor);
      painter.drawText(nameRect, Qt::AlignRight | Qt::AlignVCenter, displayName);

      drawExposureBar(&painter, rowRect, target, t0, t1, selected, i, observed);
      drawAirmassCurve(&painter, rowRect, target, t0, t1, airmassMin, airmassMax, selected, i, observed);
      if (selected) {
        drawAirmassCurveLabels(&painter, rowRect, target, t0, t1, airmassMin, airmassMax);
      }

      if (!target.flag.trimmed().isEmpty()) {
        QRect flagRect(plotRect.right() + 6, rowRect.top(), rightLabelWidth_, rowRect.height());
        QFontMetrics fm(painter.font());
        const QString label = fm.elidedText(target.flag.trimmed(), Qt::ElideRight, flagRect.width() - 4);
        painter.setPen(labelColor);
        painter.drawText(flagRect, Qt::AlignVCenter | Qt::AlignLeft, label);
      }
    }

    drawDropIndicator(&painter, plotRect);
  }

  void mousePressEvent(QMouseEvent *event) override {
    if (data_.targets.isEmpty()) return;
    pressPos_ = event->pos();
    pressIndex_ = rowAt(event->pos());
    dragging_ = false;
    if (pressIndex_ >= 0 && pressIndex_ < data_.targets.size()) {
      const QRect plotRect = plotArea();
      const QRect rowRect = rowArea(plotRect, pressIndex_);
      const QString obsId = data_.targets.at(pressIndex_).obsId;
      const QString flagText = data_.targets.at(pressIndex_).flag.trimmed();
      if (!flagText.isEmpty()) {
        QRect flagRect(plotRect.right() + 6, rowRect.top(), rightLabelWidth_, rowRect.height());
        if (flagRect.contains(event->pos())) {
          emit flagClicked(obsId, flagText);
        }
      }
      if (!obsId.isEmpty()) {
        selectedObsId_ = obsId;
        emit targetSelected(obsId);
        update();
      }
    }
  }

  void mouseMoveEvent(QMouseEvent *event) override {
    if (pressIndex_ < 0) return;
    if (!dragging_) {
      if ((event->pos() - pressPos_).manhattanLength() < QApplication::startDragDistance()) {
        return;
      }
      dragging_ = true;
      setCursor(Qt::ClosedHandCursor);
    }
    int hover = rowAt(event->pos());
    if (hover < 0 && !data_.targets.isEmpty()) {
      const QRect plotRect = plotArea();
      if (event->pos().y() < plotRect.top()) {
        hover = 0;
      } else if (event->pos().y() > plotRect.bottom()) {
        hover = data_.targets.size() - 1;
      }
    }
    if (hover >= 0 && hover < data_.targets.size()) {
      const QRect plotRect = plotArea();
      const QRect hoverRect = rowArea(plotRect, hover);
      dragInsertAbove_ = event->pos().y() < hoverRect.center().y();
    } else {
      dragInsertAbove_ = false;
    }
    if (hover != dragHoverIndex_) {
      dragHoverIndex_ = hover;
      update();
    }
  }

  void mouseReleaseEvent(QMouseEvent *event) override {
    if (dragging_) {
      int targetIndex = rowAt(event->pos());
      if (targetIndex < 0) targetIndex = dragHoverIndex_;
      if (pressIndex_ >= 0 && targetIndex >= 0 && pressIndex_ != targetIndex) {
        const QString fromObs = data_.targets.at(pressIndex_).obsId;
        if (!fromObs.isEmpty()) {
          const QRect plotRect = plotArea();
          const QRect targetRect = rowArea(plotRect, targetIndex);
          const bool insertAbove = event->pos().y() < targetRect.center().y();
          if (insertAbove) {
            if (targetIndex == 0) {
              emit targetReorderRequested(fromObs, QString());
            } else {
              const QString prevObs = data_.targets.at(targetIndex - 1).obsId;
              if (!prevObs.isEmpty()) {
                emit targetReorderRequested(fromObs, prevObs);
              }
            }
          } else {
            const QString toObs = data_.targets.at(targetIndex).obsId;
            if (!toObs.isEmpty()) {
              emit targetReorderRequested(fromObs, toObs);
            }
          }
        }
      }
    }
    dragging_ = false;
    pressIndex_ = -1;
    dragHoverIndex_ = -1;
    dragInsertAbove_ = false;
    unsetCursor();
    update();
  }

private:
  TimelineData data_;
  QString selectedObsId_;
  QPoint pressPos_;
  int pressIndex_ = -1;
  bool dragging_ = false;
  int dragHoverIndex_ = -1;
  bool dragInsertAbove_ = false;

  const int leftMargin_ = 200;
  const int rightLabelWidth_ = 160;
  const int rightMargin_ = rightLabelWidth_ + 12;
  const int topMargin_ = 24;
  const int bottomMargin_ = 28;
  const int rowHeight_ = 26;
  const int selectedRowExtra_ = 70;
  const int barHeight_ = 10;

  QRect plotArea() const {
    return QRect(leftMargin_, topMargin_,
                 width() - leftMargin_ - rightMargin_,
                 height() - topMargin_ - bottomMargin_);
  }

  int rowHeightForIndex(int index) const {
    if (index < 0 || index >= data_.targets.size()) return rowHeight_;
    const TimelineTarget &target = data_.targets.at(index);
    if (!selectedObsId_.isEmpty() && target.obsId == selectedObsId_) {
      return rowHeight_ + selectedRowExtra_;
    }
    return rowHeight_;
  }

  double timeToX(qint64 t, qint64 t0, qint64 t1, const QRect &plotRect) const {
    const double frac = double(t - t0) / double(t1 - t0);
    return plotRect.left() + frac * plotRect.width();
  }

  QColor colorForTarget(int index, const QString &obsId) const {
    const uint hash = qHash(obsId);
    const int base = (static_cast<int>(hash % 360) + index * 137) % 360;
    QColor color;
    color.setHsv(base, 110, 210);
    return color;
  }

  QColor displayColorForTarget(const TimelineTarget &target, int index, bool observed) const {
    if (!observed) {
      return palette().color(QPalette::Disabled, QPalette::Text);
    }
    if (target.severity == 2) return QColor(220, 70, 70);
    if (target.severity == 1) return QColor(255, 165, 0);
    return colorForTarget(index, target.obsId);
  }

  QRect rowArea(const QRect &plotRect, int index) const {
    int y = plotRect.top();
    for (int i = 0; i < index; ++i) {
      y += rowHeightForIndex(i);
    }
    return QRect(plotRect.left(),
                 y,
                 plotRect.width(),
                 rowHeightForIndex(index));
  }

  int rowAt(const QPoint &pos) const {
    const QRect plotRect = plotArea();
    if (pos.y() < plotRect.top() || pos.y() > plotRect.bottom()) return -1;
    int y = plotRect.top();
    for (int i = 0; i < data_.targets.size(); ++i) {
      const int h = rowHeightForIndex(i);
      if (pos.y() >= y && pos.y() < y + h) {
        return i;
      }
      y += h;
    }
    return -1;
  }

  void drawTwilightLines(QPainter *painter, const QRect &plotRect, qint64 t0, qint64 t1) {
    struct Line {
      QDateTime time;
      QString label;
    };
    QVector<Line> lines;
    if (data_.twilightEvening16.isValid()) lines.append({data_.twilightEvening16, "Twilight -16"});
    if (data_.twilightEvening12.isValid()) lines.append({data_.twilightEvening12, "Twilight -12"});
    if (data_.twilightMorning12.isValid()) lines.append({data_.twilightMorning12, "Twilight -12"});
    if (data_.twilightMorning16.isValid()) lines.append({data_.twilightMorning16, "Twilight -16"});

    QPen pen(QColor(60, 60, 60, 160));
    pen.setStyle(Qt::DashLine);
    painter->setPen(pen);
    for (const Line &line : lines) {
      const qint64 t = line.time.toMSecsSinceEpoch();
      if (t < t0 || t > t1) continue;
      const double x = timeToX(t, t0, t1, plotRect);
      painter->drawLine(QPointF(x, plotRect.top()), QPointF(x, plotRect.bottom()));
      painter->drawText(QPointF(x + 4, plotRect.top() + 12), line.label);
    }
  }

  void drawTimeAxis(QPainter *painter, const QRect &plotRect, qint64 t0, qint64 t1) {
    const int tickCount = 6;
    painter->setPen(palette().color(QPalette::Text));
    for (int i = 0; i <= tickCount; ++i) {
      const double frac = double(i) / tickCount;
      const qint64 t = t0 + qint64(frac * (t1 - t0));
      const double x = plotRect.left() + frac * plotRect.width();
      painter->drawLine(QPointF(x, plotRect.bottom()), QPointF(x, plotRect.bottom() + 4));
      painter->drawLine(QPointF(x, plotRect.top()), QPointF(x, plotRect.top() - 4));
      QDateTime dt = QDateTime::fromMSecsSinceEpoch(t, Qt::UTC).toLocalTime();
      const QString label = dt.toString("HH:mm");
      painter->drawText(QPointF(x - 14, plotRect.bottom() + 18), label);
      painter->drawText(QPointF(x - 14, plotRect.top() - 6), label);
    }
  }

  void drawIdleGaps(QPainter *painter, const QRect &plotRect, qint64 t0, qint64 t1) {
    if (data_.idleIntervals.isEmpty()) return;
    QColor gapColor(220, 50, 50, 40);
    painter->setPen(Qt::NoPen);
    painter->setBrush(gapColor);
    for (const auto &interval : data_.idleIntervals) {
      const qint64 s = interval.first.toMSecsSinceEpoch();
      const qint64 e = interval.second.toMSecsSinceEpoch();
      const qint64 gs = std::max(s, t0);
      const qint64 ge = std::min(e, t1);
      if (ge <= gs) continue;
      const double x1 = timeToX(gs, t0, t1, plotRect);
      const double x2 = timeToX(ge, t0, t1, plotRect);
      painter->drawRect(QRectF(x1, plotRect.top(), x2 - x1, plotRect.height()));
    }
  }

  void drawExposureBar(QPainter *painter, const QRect &rowRect, const TimelineTarget &target,
                       qint64 t0, qint64 t1, bool selected, int colorIndex, bool observed) {
    if (!observed) return;
    QVector<QPair<QDateTime, QDateTime>> segments = target.segments;
    if (segments.isEmpty() && target.startUtc.isValid() && target.endUtc.isValid()) {
      segments.append({target.startUtc, target.endUtc});
    }
    if (segments.isEmpty()) return;
    const QRect plotRect = rowRect;
    QColor color = displayColorForTarget(target, colorIndex, observed);
    color.setAlpha(selected ? 200 : 150);
    painter->setPen(Qt::NoPen);
    painter->setBrush(color);
    for (const auto &segment : segments) {
      const qint64 s = segment.first.toMSecsSinceEpoch();
      const qint64 e = segment.second.toMSecsSinceEpoch();
      if (e <= t0 || s >= t1) continue;
      const double x1 = timeToX(std::max(s, t0), t0, t1, plotRect);
      const double x2 = timeToX(std::min(e, t1), t0, t1, plotRect);
      const double y = rowRect.center().y() - barHeight_ / 2.0;
      QRectF bar(x1, y, x2 - x1, barHeight_);
      painter->drawRoundedRect(bar, 3, 3);
    }
  }

  void drawAirmassCurve(QPainter *painter, const QRect &rowRect, const TimelineTarget &target,
                        qint64 t0, qint64 t1, double minVal, double maxVal,
                        bool selected, int colorIndex, bool observed) {
    if (data_.timesUtc.isEmpty() || target.airmass.isEmpty()) return;
    const int n = std::min(data_.timesUtc.size(), target.airmass.size());
    if (n <= 1) return;
    QPainterPath path;
    bool started = false;
    for (int i = 0; i < n; ++i) {
      const double am = target.airmass.at(i);
      if (!std::isfinite(am) || am > maxVal || am < minVal) {
        if (started) started = false;
        continue;
      }
      const qint64 t = data_.timesUtc.at(i).toMSecsSinceEpoch();
      const double x = timeToX(t, t0, t1, rowRect);
      const double frac = (am - minVal) / (maxVal - minVal);
      const double clamped = std::min(1.0, std::max(0.0, frac));
      const double y = rowRect.top() + clamped * rowRect.height();
      if (!started) {
        path.moveTo(x, y);
        started = true;
      } else {
        path.lineTo(x, y);
      }
    }
    if (path.isEmpty()) return;

    QColor lineColor = displayColorForTarget(target, colorIndex, observed);
    if (!observed) lineColor = palette().color(QPalette::Disabled, QPalette::Text);
    QPen pen(lineColor, selected ? 2.0 : 1.2);
    pen.setStyle(Qt::DashLine);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawPath(path);
  }

  void drawAirmassCurveLabels(QPainter *painter, const QRect &rowRect, const TimelineTarget &target,
                              qint64 t0, qint64 t1, double minVal, double maxVal) {
    if (data_.timesUtc.isEmpty() || target.airmass.isEmpty()) return;
    const int n = std::min(data_.timesUtc.size(), target.airmass.size());
    if (n <= 0) return;

    QVector<int> visibleIdx;
    visibleIdx.reserve(n);
    for (int i = 0; i < n; ++i) {
      const double am = target.airmass.at(i);
      if (!std::isfinite(am) || am > maxVal || am < minVal) continue;
      visibleIdx.append(i);
    }
    if (visibleIdx.isEmpty()) return;

    int labelCount = std::min(10, static_cast<int>(visibleIdx.size()));
    QFont font = painter->font();
    font.setPointSize(std::max(8, font.pointSize() - 1));
    painter->setFont(font);
    QColor labelColor = palette().color(QPalette::Text);
    labelColor.setAlpha(170);
    painter->setPen(labelColor);
    QFontMetrics fm(font);

    // Determine consistent placement (above or below) based on peak location.
    double peak = -1.0;
    int peakIdx = -1;
    for (int i = 0; i < visibleIdx.size(); ++i) {
      const double am = target.airmass.at(visibleIdx.at(i));
      if (am > peak) {
        peak = am;
        peakIdx = visibleIdx.at(i);
      }
    }
    const bool placeAbove = (peakIdx >= 0) ? (peakIdx < n / 2) : true;

    const int labelHeight = fm.height();
    const int minSpacing = std::max(6, labelHeight + 2);
    if (labelCount > 1) {
      const int labelWidth = fm.horizontalAdvance(QString::number(peak > 0 ? peak : 1.0, 'f', 1));
      const int desiredSpacing = labelWidth + 6;
      const int maxLabelsByWidth = std::max(1, rowRect.width() / std::max(1, desiredSpacing));
      labelCount = std::min(labelCount, maxLabelsByWidth);
    }

    double lastX = -1e9;
    for (int k = 0; k < labelCount; ++k) {
      const int idx = visibleIdx.at((k * (visibleIdx.size() - 1)) / std::max(1, labelCount - 1));
      const double am = target.airmass.at(idx);
      const qint64 t = data_.timesUtc.at(idx).toMSecsSinceEpoch();
      const double x = timeToX(t, t0, t1, rowRect);
      const double frac = (am - minVal) / (maxVal - minVal);
      const double clamped = std::min(1.0, std::max(0.0, frac));
      const double y = rowRect.top() + clamped * rowRect.height();
      const QString label = QString::number(am, 'f', 1);
      const int labelWidth = fm.horizontalAdvance(label);
      if (x - lastX < minSpacing) continue;
      lastX = x;
      int lx = int(x - labelWidth / 2);
      if (lx < rowRect.left() + 2) lx = rowRect.left() + 2;
      if (lx + labelWidth > rowRect.right() - 2) lx = rowRect.right() - 2 - labelWidth;
      int ly = placeAbove ? int(y - labelHeight - 2) : int(y + 2);
      if (placeAbove && ly < rowRect.top() + 2) ly = int(y + 2);
      if (!placeAbove && ly + labelHeight > rowRect.bottom() - 2) ly = int(y - labelHeight - 2);
      painter->drawText(QRect(lx, ly, labelWidth, labelHeight),
                        Qt::AlignCenter, label);
    }
  }

  void drawRowSeparator(QPainter *painter, const QRect &plotRect, const QRect &rowRect, int index) {
    if (index >= data_.targets.size() - 1) return;
    QColor lineColor(90, 90, 90);
    lineColor.setAlpha(35);
    QPen pen(lineColor, 1.0);
    pen.setStyle(Qt::DashLine);
    painter->setPen(pen);
    const int y = rowRect.bottom();
    painter->drawLine(plotRect.left(), y, plotRect.right(), y);
  }

  void drawRowSeparators(QPainter *painter, const QRect &plotRect) {
    for (int i = 0; i < data_.targets.size(); ++i) {
      const QRect rowRect = rowArea(plotRect, i);
      drawRowSeparator(painter, plotRect, rowRect, i);
    }
  }

  void drawDropIndicator(QPainter *painter, const QRect &plotRect) {
    if (!dragging_ || dragHoverIndex_ < 0 || dragHoverIndex_ >= data_.targets.size()) return;
    const QRect rowRect = rowArea(plotRect, dragHoverIndex_);
    QColor lineColor(90, 160, 255);
    QPen pen(lineColor, 2.0);
    painter->setPen(pen);
    const int y = dragInsertAbove_ ? rowRect.top() : rowRect.bottom();
    painter->drawLine(plotRect.left(), y, plotRect.right(), y);
  }
};

class TimelinePanel : public QWidget {
  Q_OBJECT
public:
  explicit TimelinePanel(QWidget *parent = nullptr) : QWidget(parent) {
    QVBoxLayout *layout = new QVBoxLayout(this);
    scroll_ = new QScrollArea(this);
    scroll_->setWidgetResizable(true);
    canvas_ = new TimelineCanvas(this);
    scroll_->setWidget(canvas_);
    layout->addWidget(scroll_, 1);

    connect(canvas_, &TimelineCanvas::targetSelected, this, &TimelinePanel::targetSelected);
    connect(canvas_, &TimelineCanvas::targetReorderRequested, this, &TimelinePanel::targetReorderRequested);
    connect(canvas_, &TimelineCanvas::flagClicked, this, &TimelinePanel::flagClicked);
    connect(canvas_, &TimelineCanvas::contextMenuRequested, this, &TimelinePanel::contextMenuRequested);
    connect(canvas_, &TimelineCanvas::exptimeEditRequested, this, &TimelinePanel::exptimeEditRequested);
  }

  void setData(const TimelineData &data) { canvas_->setData(data); }
  void clear() { canvas_->clear(); }
  void setSelectedObsId(const QString &obsId) { canvas_->setSelectedObsId(obsId); }

signals:
  void targetSelected(const QString &obsId);
  void targetReorderRequested(const QString &fromObsId, const QString &toObsId);
  void flagClicked(const QString &obsId, const QString &flagText);
  void contextMenuRequested(const QString &obsId, const QPoint &globalPos);
  void exptimeEditRequested(const QString &obsId);

private:
  QScrollArea *scroll_ = nullptr;
  TimelineCanvas *canvas_ = nullptr;
};

class MainWindow : public QMainWindow {
  Q_OBJECT
public:
  MainWindow(QWidget *parent = nullptr) : QMainWindow(parent) {
    setWindowTitle("NGPS Target Set Editor");
    QWidget *central = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(central);

    QHBoxLayout *topBar = new QHBoxLayout();
    seqStart_ = new QPushButton("Seq Start", central);
    seqAbort_ = new QPushButton("Seq Abort", central);
    QPushButton *activateSetButton = new QPushButton("Activate target set", central);
    QPushButton *importCsvButton = new QPushButton("Import CSV", central);
    QPushButton *deleteSetButton = new QPushButton("Delete target set", central);
    runOtm_ = new QPushButton("Run OTM", central);
    showOtmLog_ = new QPushButton("Show OTM log", central);
    QLabel *otmStartLabel = new QLabel("OTM Start UTC:", central);
    otmStartEdit_ = new QLineEdit(central);
    otmUseNow_ = new QCheckBox("Use current time", central);
    QLabel *doTypeLabel = new QLabel("Seq Do:", central);
    QToolButton *doAllButton = new QToolButton(central);
    QToolButton *doOneButton = new QToolButton(central);
    QLabel *acqModeLabel = new QLabel("Acq Mode:", central);
    QToolButton *acqMode1Button = new QToolButton(central);
    QToolButton *acqMode2Button = new QToolButton(central);
    QToolButton *acqMode3Button = new QToolButton(central);
    connStatus_ = new QLabel("Not connected", central);

    doAllButton->setText("All");
    doAllButton->setCheckable(true);
    doOneButton->setText("One");
    doOneButton->setCheckable(true);
    doAllButton->setChecked(true);

    acqMode1Button->setText("1");
    acqMode1Button->setCheckable(true);
    acqMode2Button->setText("2");
    acqMode2Button->setCheckable(true);
    acqMode3Button->setText("3");
    acqMode3Button->setCheckable(true);
    acqMode1Button->setChecked(true);

    QButtonGroup *doTypeGroup = new QButtonGroup(this);
    doTypeGroup->setExclusive(true);
    doTypeGroup->addButton(doAllButton, 0);
    doTypeGroup->addButton(doOneButton, 1);

    QButtonGroup *acqModeGroup = new QButtonGroup(this);
    acqModeGroup->setExclusive(true);
    acqModeGroup->addButton(acqMode1Button, 1);
    acqModeGroup->addButton(acqMode2Button, 2);
    acqModeGroup->addButton(acqMode3Button, 3);

    topBar->addWidget(seqStart_);
    topBar->addWidget(seqAbort_);
    topBar->addWidget(activateSetButton);
    topBar->addWidget(importCsvButton);
    topBar->addWidget(deleteSetButton);
    topBar->addWidget(runOtm_);
    topBar->addWidget(showOtmLog_);
    topBar->addSpacing(12);
    topBar->addWidget(doTypeLabel);
    topBar->addWidget(doAllButton);
    topBar->addWidget(doOneButton);
    topBar->addSpacing(12);
    topBar->addWidget(acqModeLabel);
    topBar->addWidget(acqMode1Button);
    topBar->addWidget(acqMode2Button);
    topBar->addWidget(acqMode3Button);
    topBar->addStretch();
    topBar->addWidget(connStatus_);
    layout->addLayout(topBar);

    QHBoxLayout *otmBar = new QHBoxLayout();
    otmBar->addWidget(otmStartLabel);
    otmBar->addWidget(otmStartEdit_);
    otmBar->addWidget(otmUseNow_);
    otmBar->addStretch();
    layout->addLayout(otmBar);

    otmStartEdit_->setFixedWidth(90);
    otmStartEdit_->setPlaceholderText("YYYY-MM-DDTHH:MM:SS.sss");

    tabs_ = new QTabWidget(central);
    setsPanel_ = new TablePanel("Target Sets", tabs_);
    setTargetsPanel_ = new TablePanel("Targets (Set View)", tabs_);
    timelinePanel_ = new TimelinePanel(tabs_);

    setTargetsPanel_->setSearchColumn("NAME");
    setTargetsPanel_->setOrderByColumn("OBS_ORDER");
    setTargetsPanel_->setSortingEnabled(false);
    setTargetsPanel_->setAllowReorder(true);
    setTargetsPanel_->setAllowDelete(true);
    setTargetsPanel_->setAllowColumnHeaderBulkEdit(true);
    setTargetsPanel_->setRowNormalizer(normalizeTargetRow);
    setTargetsPanel_->setGroupingEnabled(true);
    setTargetsPanel_->setQuickAddEnabled(true);
    setTargetsPanel_->setQuickAddInsertAtTop(true);
    setTargetsPanel_->setHiddenColumns({"OBSERVATION_ID", "SET_ID", "OBS_ORDER",
                                        "TARGET_NUMBER", "SEQUENCE_NUMBER", "SLITOFFSET",
                                        "OBSMODE"});
    setTargetsPanel_->setColumnAfterRules({{"NEXP", "EXPTIME"},
                                           {"EXPTIME", "OTMexpt"},
                                           {"OTMEXPT", "SLITWIDTH"},
                                           {"SLITWIDTH", "OTMslitwidth"},
                                           {"AIRMASS_MAX", "MAGNITUDE"},
                                           {"MAGNITUDE", "MAGFILTER"}});
    setTargetsPanel_->setQuickAddBuilder([this](QVariantMap &values, QSet<QString> &nullColumns,
                                                QString *error) -> bool {
      Q_UNUSED(nullColumns);
      if (!setTargetsPanel_) {
        if (error) *error = "Target list not ready.";
        return false;
      }
      const QString setIdStr = setTargetsPanel_->fixedFilterValue();
      bool okSet = false;
      const int setId = setIdStr.toInt(&okSet);
      if (!okSet || setId <= 0) {
        if (error) *error = "Select a target set before adding targets.";
        return false;
      }

      QSet<QString> cols;
      for (const ColumnMeta &meta : setTargetsPanel_->columns()) {
        cols.insert(meta.name.toUpper());
      }
      auto hasCol = [&](const QString &name) { return cols.contains(name.toUpper()); };
      auto setVal = [&](const QString &name, const QVariant &val) {
        if (hasCol(name)) values.insert(name, val);
      };

      int nameSeq = 1;
      int insertOrder = 1;
      QString err;
      if (hasCol("OBS_ORDER")) {
        if (!dbClient_.nextObsOrderForSet(config_.tableTargets, setId, &nameSeq, &err)) {
          nameSeq = 1;
        }
      }

      if (insertOrder < 1) insertOrder = 1;
      const QString name = QString("NewTarget %1").arg(nameSeq);
      setVal("SET_ID", setId);
      setVal("OBS_ORDER", insertOrder);
      setVal("TARGET_NUMBER", 1);
      setVal("SEQUENCE_NUMBER", 1);
      setVal("STATE", kDefaultTargetState);
      setVal("NAME", name);
      setVal("RA", "0.0");
      setVal("DECL", "0.0");
      setVal("OFFSET_RA", "0.0");
      setVal("OFFSET_DEC", "0.0");
      setVal("DRA", "0.0");
      setVal("DDEC", "0.0");
      setVal("SLITANGLE", kDefaultSlitangle);
      setVal("SLITWIDTH", kDefaultSlitwidth);
      setVal("EXPTIME", kDefaultExptime);
      setVal("NEXP", "1");
      setVal("POINTMODE", kDefaultPointmode);
      setVal("CCDMODE", kDefaultCcdmode);
      setVal("AIRMASS_MAX", formatNumber(kDefaultAirmassMax));
      setVal("BINSPAT", QString::number(kDefaultBin));
      setVal("BINSPECT", QString::number(kDefaultBin));
      setVal("CHANNEL", kDefaultChannel);
      setVal("MAGNITUDE", formatNumber(kDefaultMagnitude));
      setVal("MAGSYSTEM", kDefaultMagsystem);
      setVal("MAGFILTER", kDefaultMagfilter);
      setVal("NOTBEFORE", kDefaultNotBefore);
      setVal("SRCMODEL", normalizeSrcmodelValue(QString()));

      double low = 0.0;
      double high = 0.0;
      const auto def = defaultWrangeForChannel(kDefaultChannel);
      low = def.first;
      high = def.second;
      setVal("WRANGE_LOW", formatNumber(low));
      setVal("WRANGE_HIGH", formatNumber(high));

      double exptimeNumeric = 0.0;
      if (extractSetNumeric(kDefaultExptime, &exptimeNumeric)) {
        setVal("OTMexpt", formatNumber(exptimeNumeric));
      }
      setVal("OTMslitwidth", formatNumber(kDefaultOtmSlitwidth));

      return true;
    });

    tabs_->addTab(setsPanel_, "Target Sets");
    tabs_->addTab(setTargetsPanel_, "Targets (Set View)");
    tabs_->addTab(timelinePanel_, "Timeline");
    layout->addWidget(tabs_, 5);

    setCentralWidget(central);

    connect(seqStart_, &QPushButton::clicked, this, &MainWindow::seqStart);
    connect(seqAbort_, &QPushButton::clicked, this, &MainWindow::seqAbort);
    connect(activateSetButton, &QPushButton::clicked, this, &MainWindow::activateSelectedSet);
    connect(importCsvButton, &QPushButton::clicked, this, &MainWindow::importTargetListCsv);
    connect(deleteSetButton, &QPushButton::clicked, this, &MainWindow::deleteSelectedSet);
    connect(runOtm_, &QPushButton::clicked, this, &MainWindow::runOtm);
    connect(showOtmLog_, &QPushButton::clicked, this, &MainWindow::showOtmLog);
    connect(otmUseNow_, &QCheckBox::toggled, this, &MainWindow::handleOtmUseNowToggle);
    connect(otmStartEdit_, &QLineEdit::editingFinished, this, [this]() {
      saveOtmStart();
      scheduleAutoOtmRun();
    });
    connect(setsPanel_, &TablePanel::selectionChanged, this, &MainWindow::updateSetViewFromSelection);
    connect(setTargetsPanel_, &TablePanel::dataMutated, this, &MainWindow::scheduleAutoOtmRun);
    connect(doTypeGroup, QOverload<int>::of(&QButtonGroup::idClicked), this,
            [this](int id) { runSeqCommand({"do", id == 0 ? "all" : "one"}); });
    connect(acqModeGroup, QOverload<int>::of(&QButtonGroup::idClicked), this,
            [this](int id) { runSeqCommand({"acqmode", QString::number(id)}); });

    otmAutoTimer_ = new QTimer(this);
    otmAutoTimer_->setSingleShot(true);
    connect(otmAutoTimer_, &QTimer::timeout, this, &MainWindow::runOtmAuto);

    connect(timelinePanel_, &TimelinePanel::targetSelected, this, [this](const QString &obsId) {
      if (!obsId.isEmpty()) {
        setTargetsPanel_->selectRowByColumnValue("OBSERVATION_ID", obsId);
      }
    });
    connect(timelinePanel_, &TimelinePanel::targetReorderRequested, this,
            [this](const QString &fromObsId, const QString &toObsId) {
              handleTimelineReorder(fromObsId, toObsId);
            });
    connect(timelinePanel_, &TimelinePanel::flagClicked, this,
            [this](const QString &obsId, const QString &flagText) {
              showOtmFlagDetails(obsId, flagText);
            });
    connect(timelinePanel_, &TimelinePanel::contextMenuRequested, this,
            [this](const QString &obsId, const QPoint &globalPos) {
              if (!setTargetsPanel_) return;
              setTargetsPanel_->showContextMenuForObsId(obsId, globalPos);
            });
    connect(timelinePanel_, &TimelinePanel::exptimeEditRequested, this,
            [this](const QString &obsId) { editExptimeForObsId(obsId); });

    connect(setTargetsPanel_, &TablePanel::selectionChanged, this, [this]() {
      const QVariantMap values = setTargetsPanel_->currentRowValues();
      const QString obsId = values.value("OBSERVATION_ID").toString();
      if (!obsId.isEmpty() && timelinePanel_) {
        timelinePanel_->setSelectedObsId(obsId);
      }
    });

    connectFromConfig();
  }

protected:
  void closeEvent(QCloseEvent *event) override {
    closing_ = true;
    if (otmAutoTimer_) otmAutoTimer_->stop();
    if (setsPanel_) setsPanel_->persistHeaderState();
    if (setTargetsPanel_) setTargetsPanel_->persistHeaderState();
    for (QProcess *proc : findChildren<QProcess *>()) {
      proc->disconnect();
      if (proc->state() != QProcess::NotRunning) {
        proc->kill();
        proc->waitForFinished(200);
      }
    }
    QMainWindow::closeEvent(event);
  }

private slots:
  void connectFromConfig() {
    const QString cfgPath = detectDefaultConfigPath();
    if (cfgPath.isEmpty()) {
      showWarning(this, "Config", "sequencerd.cfg not found.");
      return;
    }
    configPath_ = cfgPath;
    config_ = loadConfigFile(cfgPath);
    if (!config_.isComplete()) {
      showWarning(this, "Config", "Config file is missing DB settings.");
      return;
    }
    openDatabase();
  }

  void seqStart() { seqStartWithStartupCheck(); }
  void seqAbort() { runSeqCommand({"abort"}); }
  void activateSelectedSet() {
    const QVariantMap values = setsPanel_->currentRowValues();
    if (values.isEmpty()) {
      showInfo(this, "Target Sets", "Select a target set first.");
      return;
    }
    const QVariant setName = values.value("SET_NAME");
    const QString setNameText = setName.toString().trimmed();
    if (!setName.isValid() || setNameText.isEmpty()) {
      showWarning(this, "Target Sets", "SET_NAME not found.");
      return;
    }
    runSeqCommand({"targetset", setNameText});
  }

  void importTargetListCsv() {
    if (!dbClient_.isOpen()) {
      showWarning(this, "Import CSV", "Not connected to database.");
      return;
    }
    const QString filePath = QFileDialog::getOpenFileName(
        this, "Import Target List CSV", QDir::currentPath(), "CSV Files (*.csv)");
    if (filePath.isEmpty()) return;

    const QString defaultSetName = QFileInfo(filePath).baseName();
    bool ok = false;
    QString setName = QInputDialog::getText(
        this, "New Target Set", "Target set name:", QLineEdit::Normal, defaultSetName, &ok);
    if (!ok) return;
    setName = setName.trimmed();
    if (setName.isEmpty()) {
      showWarning(this, "Import CSV", "Target set name is required.");
      return;
    }

    int setId = -1;
    QString error;
    if (!createTargetSet(setName, &setId, &error)) {
      showWarning(this, "Import CSV", error.isEmpty() ? "Failed to create target set." : error);
      return;
    }

    QList<ColumnMeta> targetColumns;
    if (!dbClient_.loadColumns(config_.tableTargets, targetColumns, &error)) {
      showWarning(this, "Import CSV", error.isEmpty() ? "Failed to load target columns." : error);
      return;
    }

    QSet<QString> targetColumnNames;
    for (const ColumnMeta &meta : targetColumns) {
      targetColumnNames.insert(meta.name.toUpper());
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      showWarning(this, "Import CSV", "Unable to read CSV file.");
      return;
    }

    QTextStream in(&file);
    QString headerLine;
    while (!in.atEnd()) {
      headerLine = in.readLine();
      if (!headerLine.trimmed().isEmpty()) break;
    }
    if (headerLine.trimmed().isEmpty()) {
      showWarning(this, "Import CSV", "CSV file is empty.");
      return;
    }

    const QStringList headerFields = parseCsvLine(headerLine);
    QHash<QString, int> headerMap;
    for (int i = 0; i < headerFields.size(); ++i) {
      headerMap.insert(headerFields[i].trimmed().toUpper(), i);
    }

    auto getField = [&](const QString &name, const QStringList &fields) -> QString {
      const int idx = headerMap.value(name.toUpper(), -1);
      if (idx < 0 || idx >= fields.size()) return QString();
      return fields.at(idx).trimmed();
    };

    int obsOrder = 1;
    int inserted = 0;
    int rowIndex = 0;
    QStringList warnings;

    while (!in.atEnd()) {
      const QString line = in.readLine();
      if (line.trimmed().isEmpty()) continue;
      ++rowIndex;
      const QStringList fields = parseCsvLine(line);
      if (fields.size() == 1 && fields.at(0).trimmed().isEmpty()) continue;

      QVariantMap values;
      QSet<QString> nullColumns;

      const QString name = getField("NAME", fields);
      if (name.trimmed().isEmpty()) {
        warnings << QString("Skipped row %1: missing NAME").arg(rowIndex);
        continue;
      }
      const bool isCalib = name.toUpper().startsWith("CAL_");

      if (targetColumnNames.contains("SET_ID")) values.insert("SET_ID", setId);
      if (targetColumnNames.contains("OBS_ORDER")) values.insert("OBS_ORDER", obsOrder);
      if (targetColumnNames.contains("TARGET_NUMBER")) values.insert("TARGET_NUMBER", 1);
      if (targetColumnNames.contains("SEQUENCE_NUMBER")) values.insert("SEQUENCE_NUMBER", 1);
      if (targetColumnNames.contains("STATE")) values.insert("STATE", kDefaultTargetState);
      if (targetColumnNames.contains("NAME")) values.insert("NAME", name);

      const QString ra = getField("RA", fields);
      const QString dec = getField("DECL", fields);
      if (!isCalib && (ra.isEmpty() || dec.isEmpty())) {
        warnings << QString("Skipped row %1 (%2): missing RA/DECL").arg(rowIndex).arg(name);
        continue;
      }
      if (targetColumnNames.contains("RA") && !ra.isEmpty()) values.insert("RA", ra);
      if (targetColumnNames.contains("DECL") && !dec.isEmpty()) values.insert("DECL", dec);

      const QString offsetRa = getField("OFFSET_RA", fields);
      const QString offsetDec = getField("OFFSET_DEC", fields);
      if (targetColumnNames.contains("OFFSET_RA") && !offsetRa.isEmpty()) values.insert("OFFSET_RA", offsetRa);
      if (targetColumnNames.contains("OFFSET_DEC") && !offsetDec.isEmpty()) values.insert("OFFSET_DEC", offsetDec);

      const QString slitangle = getField("SLITANGLE", fields);
      const QString slitwidth = getField("SLITWIDTH", fields);
      const QString exptime = getField("EXPTIME", fields);
      if (targetColumnNames.contains("SLITANGLE")) values.insert("SLITANGLE", slitangle);
      if (targetColumnNames.contains("SLITWIDTH")) values.insert("SLITWIDTH", slitwidth);
      if (targetColumnNames.contains("EXPTIME")) values.insert("EXPTIME", exptime);

      const QString binspect = getField("BINSPECT", fields);
      const QString binspat = getField("BINSPAT", fields);
      if (targetColumnNames.contains("BINSPECT")) values.insert("BINSPECT", binspect);
      if (targetColumnNames.contains("BINSPAT")) values.insert("BINSPAT", binspat);

      const QString airmassMax = getField("AIRMASS_MAX", fields);
      if (targetColumnNames.contains("AIRMASS_MAX")) values.insert("AIRMASS_MAX", airmassMax);

      const QString wrangeLow = getField("WRANGE_LOW", fields);
      const QString wrangeHigh = getField("WRANGE_HIGH", fields);
      if (targetColumnNames.contains("WRANGE_LOW")) values.insert("WRANGE_LOW", wrangeLow);
      if (targetColumnNames.contains("WRANGE_HIGH")) values.insert("WRANGE_HIGH", wrangeHigh);

      const QString channel = getField("CHANNEL", fields);
      if (targetColumnNames.contains("CHANNEL")) values.insert("CHANNEL", channel);

      const QString magnitude = getField("MAGNITUDE", fields);
      const QString magfilter = getField("MAGFILTER", fields);
      const QString magsystem = getField("MAGSYSTEM", fields);
      if (targetColumnNames.contains("MAGNITUDE")) values.insert("MAGNITUDE", magnitude);
      if (targetColumnNames.contains("MAGFILTER")) values.insert("MAGFILTER", magfilter);
      if (targetColumnNames.contains("MAGSYSTEM")) values.insert("MAGSYSTEM", magsystem);

      if (targetColumnNames.contains("POINTMODE")) values.insert("POINTMODE", getField("POINTMODE", fields));
      if (targetColumnNames.contains("CCDMODE")) values.insert("CCDMODE", getField("CCDMODE", fields));
      if (targetColumnNames.contains("NOTBEFORE")) values.insert("NOTBEFORE", getField("NOTBEFORE", fields));

      QString comment = getField("COMMENT", fields);
      const QString priority = getField("PRIORITY", fields);
      if (!priority.trimmed().isEmpty()) {
        if (!comment.trimmed().isEmpty()) comment += " ";
        comment += QString("PRIORITY=%1").arg(priority.trimmed());
      }
      if (targetColumnNames.contains("COMMENT") && !comment.isEmpty()) values.insert("COMMENT", comment);

      if (targetColumnNames.contains("OTMSLITWIDTH")) values.insert("OTMslitwidth", QString());
      if (targetColumnNames.contains("OTMEXPT")) values.insert("OTMexpt", QString());

      normalizeTargetRow(values, nullColumns);

      if (targetColumnNames.contains("OTMSLITWIDTH")) {
        if (values.value("OTMslitwidth").toString().trimmed().isEmpty()) {
          values.insert("OTMslitwidth", formatNumber(kDefaultOtmSlitwidth));
        }
      }

      for (const ColumnMeta &meta : targetColumns) {
        if (values.contains(meta.name)) {
          const QString text = values.value(meta.name).toString().trimmed();
          if (text.isEmpty() && meta.nullable) {
            nullColumns.insert(meta.name);
          }
        } else if (meta.nullable) {
          nullColumns.insert(meta.name);
        }
      }

      QString rowError;
      if (!dbClient_.insertRecord(config_.tableTargets, targetColumns, values, nullColumns, &rowError)) {
        warnings << QString("Row %1: %2").arg(rowIndex).arg(rowError.isEmpty() ? "Insert failed" : rowError);
        continue;
      }
      ++inserted;
      ++obsOrder;
    }

    if (inserted > 0 && targetColumnNames.contains("SET_ID")) {
      QVariantMap keyValues;
      keyValues.insert("SET_ID", setId);
      QVariantMap updates;
      updates.insert("NUM_OBSERVATIONS", inserted);
      dbClient_.updateColumnsByKey(config_.tableSets, updates, keyValues, nullptr);
    }

    setsPanel_->refresh();
    setsPanel_->selectRowByColumnValue("SET_ID", setId);
    updateSetViewFromSelection();

    QString summary = QString("Imported %1 targets into set \"%2\".").arg(inserted).arg(setName);
    if (!warnings.isEmpty()) {
      summary += "\n\nWarnings:\n" + warnings.join("\n");
    }
    showInfo(this, "Import CSV", summary);
    scheduleAutoOtmRun();
  }

  void deleteSelectedSet() {
    if (!dbClient_.isOpen()) {
      showWarning(this, "Delete Target Set", "Not connected to database.");
      return;
    }
    const QVariantMap values = setsPanel_->currentRowValues();
    if (values.isEmpty()) {
      showInfo(this, "Delete Target Set", "Select a target set first.");
      return;
    }
    const QVariant setId = values.value("SET_ID");
    const QString setName = values.value("SET_NAME").toString();
    if (!setId.isValid()) {
      showWarning(this, "Delete Target Set", "SET_ID not found.");
      return;
    }

    const QString prompt = QString("Delete target set \"%1\" and all targets in it?")
                               .arg(setName.isEmpty() ? setId.toString() : setName);
    if (QMessageBox::question(this, "Delete Target Set", prompt,
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) {
      return;
    }

    QString error;
    if (!dbClient_.deleteRecordsByColumn(config_.tableTargets, "SET_ID", setId, &error)) {
      showWarning(this, "Delete Target Set", error.isEmpty() ? "Failed to delete targets." : error);
      return;
    }
    QVariantMap keyValues;
    keyValues.insert("SET_ID", setId);
    if (!dbClient_.deleteRecordByKey(config_.tableSets, keyValues, &error)) {
      showWarning(this, "Delete Target Set", error.isEmpty() ? "Failed to delete target set." : error);
      return;
    }
    setsPanel_->refresh();
    setTargetsPanel_->clearFixedFilter();
    setTargetsPanel_->refresh();
    if (timelinePanel_) timelinePanel_->clear();
  }

  void showOtmLog() {
    if (lastOtmLog_.trimmed().isEmpty()) {
      showInfo(this, "OTM Log", "No OTM output captured yet.");
      return;
    }
    showInfo(this, "OTM Log", lastOtmLog_);
  }

  void showOtmFlagDetails(const QString &obsId, const QString &flagText) {
    if (flagText.trimmed().isEmpty()) return;
    QString detail = explainOtmFlags(flagText);
    if (detail.isEmpty()) {
      detail = QString("Flags: %1").arg(flagText.trimmed());
    } else {
      detail = QString("Flags: %1\n\n%2").arg(flagText.trimmed(), detail);
    }
    if (!obsId.trimmed().isEmpty()) {
      detail.prepend(QString("Target: %1\n").arg(obsId.trimmed()));
    }
    showInfo(this, "OTM Flag Details", detail);
  }

  void editExptimeForObsId(const QString &obsId) {
    if (obsId.trimmed().isEmpty()) return;
    if (!dbClient_.isOpen()) {
      showWarning(this, "Edit EXPTIME", "Not connected to database.");
      return;
    }
    if (!setTargetsPanel_) return;

    const bool hasNexp = setTargetsPanel_->hasColumn("NEXP");
    QVariant currentVal = setTargetsPanel_->valueForColumnInRow("OBSERVATION_ID", obsId, "EXPTIME");
    QString currentText = currentVal.toString().trimmed();
    if (currentText.isEmpty()) currentText = kDefaultExptime;

    int currentNexp = 1;
    if (hasNexp) {
      QVariant nexpVal = setTargetsPanel_->valueForColumnInRow("OBSERVATION_ID", obsId, "NEXP");
      int parsed = 0;
      if (parseInt(nexpVal.toString(), &parsed) && parsed > 0) {
        currentNexp = parsed;
      }
    }

    QDialog dialog(this);
    dialog.setWindowTitle("Edit Exposure");
    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    QFormLayout *form = new QFormLayout();
    QLineEdit *exptimeEdit = new QLineEdit(currentText, &dialog);
    form->addRow("EXPTIME (SET <sec> or SNR <value>):", exptimeEdit);
    QSpinBox *nexpSpin = nullptr;
    if (hasNexp) {
      nexpSpin = new QSpinBox(&dialog);
      nexpSpin->setRange(1, 9999);
      nexpSpin->setValue(currentNexp);
      form->addRow("NEXP:", nexpSpin);
    }
    layout->addLayout(form);
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttons);

    if (dialog.exec() != QDialog::Accepted) return;
    QString newText = exptimeEdit->text().trimmed();
    if (newText.isEmpty()) return;
    int newNexp = currentNexp;
    if (hasNexp && nexpSpin) {
      newNexp = std::max(1, nexpSpin->value());
    }

    QStringList targetObsIds;
    QHash<QString, QStringList> groups = setTargetsPanel_->groupMembersByHeaderObsId();
    if (groups.contains(obsId)) {
      targetObsIds = groups.value(obsId);
    } else {
      for (auto it = groups.begin(); it != groups.end(); ++it) {
        if (it.value().contains(obsId)) {
          targetObsIds = it.value();
          break;
        }
      }
    }
    if (targetObsIds.isEmpty()) targetObsIds << obsId;

    QStringList errors;
    for (const QString &memberObsId : targetObsIds) {
      QVariant nameVal = setTargetsPanel_->valueForColumnInRow("OBSERVATION_ID", memberObsId, "NAME");
      const QString name = nameVal.toString().trimmed();
      const bool isCalib = name.toUpper().startsWith("CAL_");
      const QString normalized = normalizeExptimeValue(newText, isCalib);
      QVariantMap updates;
      updates.insert("EXPTIME", normalized);
      if (hasNexp) {
        updates.insert("NEXP", QString::number(newNexp));
      }
      QVariantMap keyValues;
      keyValues.insert("OBSERVATION_ID", memberObsId);
      QString error;
      if (!dbClient_.updateColumnsByKey(config_.tableTargets, updates, keyValues, &error)) {
        errors << QString("%1: %2").arg(memberObsId, error.isEmpty() ? "Update failed" : error);
      }
    }

    if (!errors.isEmpty()) {
      showWarning(this, "Edit EXPTIME", errors.join("\n"));
    } else {
      setTargetsPanel_->refresh();
      scheduleAutoOtmRun();
    }
  }

  void scheduleAutoOtmRun() {
    if (closing_) return;
    if (!dbClient_.isOpen()) return;
    if (otmRunning_) {
      otmAutoPending_ = true;
      return;
    }
    if (!otmAutoTimer_) return;
    otmAutoTimer_->start(500);
  }

  void runOtm() { runOtmInternal(true); }

  void runOtmAuto() { runOtmInternal(false); }

  void runOtmInternal(bool showDialog) {
    if (closing_) return;
    const bool quiet = !showDialog;
    if (!dbClient_.isOpen()) {
      if (!quiet) showWarning(this, "Run OTM", "Not connected to database.");
      return;
    }

    const QVariantMap setValues = setsPanel_->currentRowValues();
    if (setValues.isEmpty()) {
      if (!quiet) showInfo(this, "Run OTM", "Select a target set first.");
      return;
    }

    const QVariant setIdValue = setValues.value("SET_ID");
    bool ok = false;
    const int setId = setIdValue.toInt(&ok);
    if (!ok) {
      if (!quiet) showWarning(this, "Run OTM", "SET_ID not found.");
      return;
    }

    if (ngpsRoot_.isEmpty()) {
      if (!quiet) showWarning(this, "Run OTM", "NGPS root not detected.");
      return;
    }

    const QString scriptPath = QDir(ngpsRoot_).filePath("Python/OTM/OTM.py");
    if (!QFile::exists(scriptPath)) {
      if (!quiet) showWarning(this, "Run OTM", QString("OTM script not found: %1").arg(scriptPath));
      return;
    }

    if (otmRunning_) {
      otmAutoPending_ = true;
      return;
    }

    OtmSettings settings = loadOtmSettings();
    if (showDialog) {
      OtmSettingsDialog dialog(settings, this);
      if (dialog.exec() != QDialog::Accepted) {
        return;
      }
      settings = dialog.settings();
      saveOtmSettings(settings);
    }

    QString startUtc = otmStartEdit_ ? otmStartEdit_->text().trimmed() : QString();
    if (otmUseNow_ && otmUseNow_->isChecked()) {
      startUtc = currentUtcString();
      if (otmStartEdit_) {
        otmStartEdit_->setText(startUtc);
      }
    }
    if (startUtc.isEmpty()) {
      startUtc = estimateTwilightUtc();
      if (startUtc.isEmpty()) startUtc = currentUtcString();
      if (otmStartEdit_) {
        otmStartEdit_->setText(startUtc);
      }
    }
    startUtc = normalizeOtmStartText(startUtc, quiet);
    if (otmStartEdit_) saveOtmStart();

    QList<ColumnMeta> targetColumns;
    QString error;
    if (!dbClient_.loadColumns(config_.tableTargets, targetColumns, &error)) {
      if (!quiet) showWarning(this, "Run OTM", error.isEmpty() ? "Failed to load target columns." : error);
      return;
    }

    QVector<QVector<QVariant>> rows;
    if (!dbClient_.fetchRows(config_.tableTargets, targetColumns,
                             "SET_ID", QString::number(setId),
                             "", "", "OBS_ORDER",
                             rows, &error)) {
      if (!quiet) showWarning(this, "Run OTM", error.isEmpty() ? "Failed to load targets." : error);
      return;
    }

    if (rows.isEmpty()) {
      if (!quiet) showInfo(this, "Run OTM", "No targets found in the selected set.");
      return;
    }

    QSet<QString> targetColumnNames;
    for (int i = 0; i < targetColumns.size(); ++i) {
      targetColumnNames.insert(targetColumns[i].name.toUpper());
    }

    QSet<QString> manualUngroupObsIds;
    QHash<QString, QStringList> panelGroups;
    if (setTargetsPanel_) {
      manualUngroupObsIds = setTargetsPanel_->ungroupedObsIds();
      panelGroups = setTargetsPanel_->groupMembersByHeaderObsId();
    }
    QHash<QString, QString> obsIdToHeader;
    for (auto it = panelGroups.begin(); it != panelGroups.end(); ++it) {
      const QString header = it.key();
      for (const QString &member : it.value()) {
        if (!member.isEmpty()) obsIdToHeader.insert(member, header);
      }
      if (!header.isEmpty()) obsIdToHeader.insert(header, header);
    }

    const bool includeSrcmodel = targetColumnNames.contains("SRCMODEL");
    const bool includeNexp = targetColumnNames.contains("NEXP");

    QStringList header;
    header << "OBSERVATION_ID"
           << "name" << "RA" << "DECL"
           << "slitangle" << "slitwidth";
    if (includeNexp) header << "NEXP";
    header << "exptime"
           << "notbefore" << "pointmode" << "ccdmode"
           << "airmass_max" << "binspat" << "binspect"
           << "channel" << "wrange" << "mag" << "magsystem" << "magfilter";
    if (includeSrcmodel) header << "srcmodel";

    struct RowRecord {
      int rowIndex = 0;
      QVariantMap values;
      QSet<QString> nullColumns;
      QString obsId;
      QString name;
      bool isCalib = false;
      QString groupKey;
      bool isScience = false;
    };

    struct GroupInfo {
      QString scienceObsId;
      QStringList members;
    };

    QStringList inputWarnings;
    QStringList coordDiagnostics;
    QHash<QString, QVariant> oldSlitwidthByObsId;
    QHash<QString, int> obsOrderByObsId;
    QVector<RowRecord> records;
    QHash<QString, GroupInfo> groups;

    int rowIndex = 0;
    for (const QVector<QVariant> &row : rows) {
      ++rowIndex;
      QVariantMap values;
      QSet<QString> nullColumns;
      for (int i = 0; i < targetColumns.size(); ++i) {
        if (i >= row.size()) continue;
        const QVariant value = row.at(i);
        if (value.isValid() && !value.isNull()) {
          values.insert(targetColumns[i].name, value);
        } else {
          nullColumns.insert(targetColumns[i].name);
        }
      }

      normalizeTargetRow(values, nullColumns);

      const QString obsId = valueToStringCaseInsensitive(values, "OBSERVATION_ID");
      if (obsId.isEmpty()) {
        inputWarnings << QString("Row %1: missing OBSERVATION_ID").arg(rowIndex);
        continue;
      }

      int obsOrder = 0;
      bool orderOk = false;
      obsOrder = valueToStringCaseInsensitive(values, "OBS_ORDER").toInt(&orderOk);
      if (orderOk) {
        obsOrderByObsId.insert(obsId, obsOrder);
      }

      const QString name = valueToStringCaseInsensitive(values, "NAME");
      if (name.isEmpty()) {
        inputWarnings << QString("Row %1: missing NAME").arg(rowIndex);
        continue;
      }

      const bool isCalib = name.toUpper().startsWith("CAL_");
      const QString ra = valueToStringCaseInsensitive(values, "RA");
      const QString dec = valueToStringCaseInsensitive(values, "DECL");
      if (!isCalib && (ra.isEmpty() || dec.isEmpty())) {
        inputWarnings << QString("Row %1 (%2): missing RA/DECL").arg(rowIndex).arg(name);
        continue;
      }

      const QString groupKey = obsIdToHeader.value(obsId, obsId);

      {
        double raDeg = 0.0;
        double decDeg = 0.0;
        if (computeScienceCoordDegreesProjected(values, &raDeg, &decDeg)) {
          bool hasOffsetRa = false;
          bool hasOffsetDec = false;
          const double offsetRa = offsetArcsecFromValues(values, {"OFFSET_RA", "DRA"}, &hasOffsetRa);
          const double offsetDec = offsetArcsecFromValues(values, {"OFFSET_DEC", "DDEC"}, &hasOffsetDec);
          QString key = groupKey;
          coordDiagnostics << QString("%1\t%2\tRA=%3\tDEC=%4\tDRA=%5\tDDEC=%6\tKEY=%7")
                                   .arg(obsId,
                                        name,
                                        QString::number(raDeg, 'f', 6),
                                        QString::number(decDeg, 'f', 6),
                                        QString::number(offsetRa, 'f', 3),
                                        QString::number(offsetDec, 'f', 3),
                                        key);
        } else {
          coordDiagnostics << QString("%1\t%2\tRA/DEC parse failed").arg(obsId, name);
        }
      }

      const bool isScience = (obsId == groupKey);

      GroupInfo &group = groups[groupKey];
      group.members.append(obsId);
      if (isScience && group.scienceObsId.isEmpty()) {
        group.scienceObsId = obsId;
      }

      RowRecord record;
      record.rowIndex = rowIndex;
      record.values = values;
      record.nullColumns = nullColumns;
      record.obsId = obsId;
      record.name = name;
      record.isCalib = isCalib;
      record.groupKey = groupKey;
      record.isScience = isScience;
      records.append(record);

      const QVariant oldSlit = values.value(findKeyCaseInsensitive(values, "OTMslitwidth"));
      if (oldSlit.isValid() && !oldSlit.isNull()) {
        oldSlitwidthByObsId.insert(obsId, oldSlit);
      }
    }

    QHash<QString, QStringList> membersByScienceObsId;
    for (auto it = groups.begin(); it != groups.end(); ++it) {
      if (it->members.isEmpty()) continue;
      if (it->scienceObsId.isEmpty()) {
        it->scienceObsId = it->members.first();
        inputWarnings << QString("Group %1: missing header row, using OBSERVATION_ID %2")
                             .arg(it.key(), it->scienceObsId);
      }
      membersByScienceObsId.insert(it->scienceObsId, it->members);
    }

    QStringList lines;
    lines << header.join(",");
    for (const RowRecord &record : records) {
      const GroupInfo group = groups.value(record.groupKey);
      if (record.obsId != group.scienceObsId) {
        continue;
      }

      QVariantMap values = record.values;
      QSet<QString> nullColumns = record.nullColumns;
      const QString obsId = record.obsId;
      const QString name = record.name;
      const bool isCalib = record.isCalib;

      QString ra = valueToStringCaseInsensitive(values, "RA");
      QString dec = valueToStringCaseInsensitive(values, "DECL");
      if (!isCalib && (ra.isEmpty() || dec.isEmpty())) {
        inputWarnings << QString("Row %1 (%2): missing RA/DECL").arg(record.rowIndex).arg(name);
        continue;
      }

      QString channel = valueToStringCaseInsensitive(values, "CHANNEL");
      if (channel.isEmpty()) channel = kDefaultChannel;

      double low = 0.0;
      double high = 0.0;
      const bool lowOk = parseDouble(valueToStringCaseInsensitive(values, "WRANGE_LOW"), &low);
      const bool highOk = parseDouble(valueToStringCaseInsensitive(values, "WRANGE_HIGH"), &high);
      if (!lowOk || !highOk || high <= low) {
        const auto def = defaultWrangeForChannel(channel);
        low = def.first;
        high = def.second;
      }
      const QString wrange = QString("%1 %2").arg(formatNumber(low), formatNumber(high));

      QString slitangle = valueToStringCaseInsensitive(values, "SLITANGLE");
      if (slitangle.isEmpty()) slitangle = kDefaultSlitangle;
      QString slitwidth = valueToStringCaseInsensitive(values, "SLITWIDTH");
      if (slitwidth.isEmpty()) slitwidth = kDefaultSlitwidth;
      QString exptime = valueToStringCaseInsensitive(values, "EXPTIME");
      if (exptime.isEmpty()) exptime = kDefaultExptime;
      int nexp = 1;
      if (includeNexp) {
        int parsed = 0;
        if (parseInt(valueToStringCaseInsensitive(values, "NEXP"), &parsed) && parsed > 0) {
          nexp = parsed;
        }
      }
      QString notbefore = valueToStringCaseInsensitive(values, "NOTBEFORE");
      if (notbefore.isEmpty()) notbefore = kDefaultNotBefore;
      QString pointmode = valueToStringCaseInsensitive(values, "POINTMODE");
      if (pointmode.isEmpty()) pointmode = kDefaultPointmode;
      QString ccdmode = valueToStringCaseInsensitive(values, "CCDMODE");
      if (ccdmode.isEmpty()) ccdmode = kDefaultCcdmode;
      QString airmassMax = valueToStringCaseInsensitive(values, "AIRMASS_MAX");
      if (airmassMax.isEmpty()) airmassMax = formatNumber(settings.airmassMax);
      QString binspat = valueToStringCaseInsensitive(values, "BINSPAT");
      if (binspat.isEmpty()) binspat = QString::number(kDefaultBin);
      QString binspect = valueToStringCaseInsensitive(values, "BINSPECT");
      if (binspect.isEmpty()) binspect = QString::number(kDefaultBin);
      QString mag = valueToStringCaseInsensitive(values, "MAGNITUDE");
      if (mag.isEmpty()) mag = formatNumber(kDefaultMagnitude);
      QString magsystem = valueToStringCaseInsensitive(values, "MAGSYSTEM");
      if (magsystem.isEmpty()) magsystem = kDefaultMagsystem;
      QString magfilter = valueToStringCaseInsensitive(values, "MAGFILTER");
      if (magfilter.isEmpty()) magfilter = kDefaultMagfilter;
      QString srcmodel = valueToStringCaseInsensitive(values, "SRCMODEL");
      srcmodel = normalizeSrcmodelValue(srcmodel);

      QStringList fields;
      fields << obsId << name << ra << dec
             << slitangle << slitwidth;
      if (includeNexp) fields << QString::number(nexp);
      fields << exptime
             << notbefore << pointmode << ccdmode
             << airmassMax << binspat << binspect
             << channel << wrange << mag << magsystem << magfilter;
      if (includeSrcmodel) fields << srcmodel;

      for (QString &field : fields) {
        field = csvEscape(field);
      }
      for (int rep = 0; rep < nexp; ++rep) {
        lines << fields.join(",");
      }
    }

    if (lines.size() <= 1) {
      if (!quiet) showInfo(this, "Run OTM", "No valid targets to send to OTM.");
      return;
    }

    const QString timestamp = QDateTime::currentDateTimeUtc().toString("yyyyMMdd_HHmmss_zzz");
    const QString inputPath = QDir::temp().filePath(
        QString("ngps_otm_input_%1_%2.csv").arg(setId).arg(timestamp));
    const QString outputPath = QDir::temp().filePath(
        QString("ngps_otm_output_%1_%2.csv").arg(setId).arg(timestamp));

    QFile inputFile(inputPath);
    if (!inputFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
      if (!quiet) showWarning(this, "Run OTM", "Unable to write OTM input file.");
      return;
    }
    QTextStream out(&inputFile);
    for (const QString &line : lines) {
      out << line << "\n";
    }
    inputFile.close();

    if (runOtm_) runOtm_->setEnabled(false);
    otmRunning_ = true;

    struct OtmRunContext {
      int setId = -1;
      QString setName;
      QString inputPath;
      QString outputPath;
      QString timelinePath;
      QString timestamp;
      double airmassMax = 0.0;
      QHash<QString, QVariant> oldSlitwidth;
      QHash<QString, QStringList> membersByScienceObsId;
    QSet<QString> targetColumnNames;
    QStringList inputWarnings;
    QHash<QString, int> obsOrderByObsId;
    QString startUtc;
    QStringList coordDiagnostics;
    QSet<QString> manualUngroupObsIds;
  };
    auto context = std::make_shared<OtmRunContext>();
    context->setId = setId;
    context->setName = setValues.value("SET_NAME").toString();
    context->inputPath = inputPath;
    context->outputPath = outputPath;
    context->timestamp = timestamp;
    context->oldSlitwidth = oldSlitwidthByObsId;
    context->membersByScienceObsId = membersByScienceObsId;
    context->targetColumnNames = targetColumnNames;
    context->inputWarnings = inputWarnings;
    context->obsOrderByObsId = obsOrderByObsId;
    context->startUtc = startUtc;
    context->airmassMax = settings.airmassMax;
    context->coordDiagnostics = coordDiagnostics;
    context->manualUngroupObsIds = manualUngroupObsIds;

    const QString pythonCmd = resolveOtmPython();
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    if (env.contains("PYTHONHOME")) env.remove("PYTHONHOME");
    if (env.contains("PYTHONPATH")) env.remove("PYTHONPATH");
    const QString addPath = QDir(ngpsRoot_).filePath("Python");
    env.insert("PYTHONPATH", addPath);
    env.insert("PYTHONNOUSERSITE", "1");

    QProcess *proc = new QProcess(this);
    proc->setProcessEnvironment(env);
    proc->setWorkingDirectory(ngpsRoot_);

    QStringList args;
    args << scriptPath
         << inputPath
         << startUtc
         << "-seeing" << formatNumber(settings.seeingFwhm) << formatNumber(settings.seeingPivot)
         << "-airmass_max" << formatNumber(settings.airmassMax)
         << "-out" << outputPath;
    if (!settings.useSkySim) args << "-noskysim";

    auto output = std::make_shared<QString>();
    statusBar()->showMessage(QString("Running: %1 %2").arg(pythonCmd, args.join(' ')), 5000);
    auto withDiagnostics = [context](const QString &base) {
      QString log = base;
      if (!context->coordDiagnostics.isEmpty()) {
        log += "\n\nFinal coordinates after offsets (deg):\n";
        log += context->coordDiagnostics.join("\n");
      }
      return log;
    };
    connect(proc, &QProcess::readyReadStandardOutput, this, [proc, output]() {
      *output += QString::fromUtf8(proc->readAllStandardOutput());
    });
    connect(proc, &QProcess::readyReadStandardError, this, [proc, output]() {
      *output += QString::fromUtf8(proc->readAllStandardError());
    });
    connect(proc, &QProcess::errorOccurred, this,
            [this, proc, output, context, quiet, withDiagnostics](QProcess::ProcessError) {
              if (runOtm_) runOtm_->setEnabled(true);
              otmRunning_ = false;
              const bool rerun = otmAutoPending_;
              otmAutoPending_ = false;
              lastOtmLog_ = withDiagnostics(*output);
              QString detail = *output;
              if (detail.trimmed().isEmpty()) detail = "Failed to start OTM process.";
              if (!quiet) {
                showWarning(this, "Run OTM", detail);
              } else {
                statusBar()->showMessage("OTM failed to start. See OTM log.", 8000);
              }
              QFile::remove(context->inputPath);
              QFile::remove(context->outputPath);
              proc->deleteLater();
              if (rerun) scheduleAutoOtmRun();
            });
    connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
            [this, proc, output, context, pythonCmd, env, quiet, withDiagnostics](int code, QProcess::ExitStatus status) {
              if (runOtm_) runOtm_->setEnabled(true);
              otmRunning_ = false;
              const bool rerun = otmAutoPending_;
              otmAutoPending_ = false;
              lastOtmLog_ = withDiagnostics(*output);
              auto cleanupFiles = [context]() {
                QFile::remove(context->inputPath);
                QFile::remove(context->outputPath);
                if (!context->timelinePath.isEmpty()) {
                  QFile::remove(context->timelinePath);
                }
              };

              const QString msg = QString("OTM exit %1 (%2)")
                                      .arg(code)
                                      .arg(status == QProcess::NormalExit ? "normal" : "crash");
              statusBar()->showMessage(msg, 5000);

              if (status != QProcess::NormalExit || code != 0) {
                QString detail = *output;
                if (detail.trimmed().isEmpty()) detail = msg;
                if (!quiet) {
                  showWarning(this, "Run OTM", detail);
                } else {
                  statusBar()->showMessage("OTM failed. See OTM log.", 8000);
                }
                cleanupFiles();
                proc->deleteLater();
                if (rerun) scheduleAutoOtmRun();
                return;
              }

              QFile outFile(context->outputPath);
              if (!outFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                if (!quiet) {
                  showWarning(this, "Run OTM", "Unable to read OTM output file.");
                } else {
                  statusBar()->showMessage("OTM output missing.", 8000);
                }
                cleanupFiles();
                proc->deleteLater();
                return;
              }

              QTextStream in(&outFile);
              QString headerLine;
              while (!in.atEnd()) {
                headerLine = in.readLine();
                if (!headerLine.trimmed().isEmpty()) break;
              }
              if (headerLine.trimmed().isEmpty()) {
                if (!quiet) {
                  showWarning(this, "Run OTM", "OTM output file is empty.");
                } else {
                  statusBar()->showMessage("OTM output empty.", 8000);
                }
                cleanupFiles();
                proc->deleteLater();
                return;
              }

              const QStringList headers = parseCsvLine(headerLine);
              QHash<QString, int> headerMap;
              for (int i = 0; i < headers.size(); ++i) {
                headerMap.insert(headers[i].trimmed().toUpper(), i);
              }

              auto getField = [&](const QString &name, const QStringList &fields) -> QString {
                const int idx = headerMap.value(name.toUpper(), -1);
                if (idx < 0 || idx >= fields.size()) return QString();
                return fields.at(idx).trimmed();
              };

              auto addUpdate = [&](QVariantMap &updates,
                                   const QStringList &fields,
                                   const QString &outCol,
                                   const QString &dbCol,
                                   bool isTimestamp,
                                   bool allowEmptyString,
                                   bool skipIfEmpty) {
                if (!context->targetColumnNames.contains(dbCol.toUpper())) return;
                QString raw = getField(outCol, fields);
                if (isTimestamp) {
                  raw = normalizeOtmTimestamp(raw);
                } else {
                  raw = raw.trimmed();
                }
                if (raw.compare("None", Qt::CaseInsensitive) == 0) {
                  raw.clear();
                }
                if (raw.isEmpty()) {
                  if (skipIfEmpty) {
                    return;
                  }
                  if (allowEmptyString) {
                    updates.insert(dbCol, QString());
                  } else {
                    updates.insert(dbCol, QVariant());
                  }
                } else {
                  updates.insert(dbCol, raw);
                }
              };

              struct AggUpdates {
                QVariantMap updates;
                QDateTime startMin;
                QDateTime endMax;
                QString startStr;
                QString endStr;
                bool hasStart = false;
                bool hasEnd = false;
              };

              int rowIndex = 0;
              QStringList warnings = context->inputWarnings;
              QHash<QString, AggUpdates> aggByObsId;

              while (!in.atEnd()) {
                const QString line = in.readLine();
                if (line.trimmed().isEmpty()) continue;
                ++rowIndex;
                const QStringList fields = parseCsvLine(line);
                if (fields.size() == 1 && fields.at(0).trimmed().isEmpty()) continue;

                const QString obsId = getField("OBSERVATION_ID", fields);
                if (obsId.isEmpty()) {
                  warnings << QString("Output row %1: missing OBSERVATION_ID").arg(rowIndex);
                  continue;
                }

                QVariantMap updates;
                addUpdate(updates, fields, "OTMstart", "OTMexp_start", true, false, false);
                addUpdate(updates, fields, "OTMend", "OTMexp_end", true, false, false);
                addUpdate(updates, fields, "OTMexptime", "OTMexpt", false, false, false);
                addUpdate(updates, fields, "OTMslitwidth", "OTMslitwidth", false, false, true);
                addUpdate(updates, fields, "OTMpa", "OTMpa", false, false, false);
                addUpdate(updates, fields, "OTMslitangle", "OTMslitangle", false, false, false);
                addUpdate(updates, fields, "OTMcass", "OTMcass", false, false, false);
                addUpdate(updates, fields, "OTMwait", "OTMwait", false, false, false);
                addUpdate(updates, fields, "OTMflag", "OTMflag", false, true, false);
                addUpdate(updates, fields, "OTMlast", "OTMlast", false, true, false);
                addUpdate(updates, fields, "OTMslewgo", "OTMslewgo", true, false, false);
                addUpdate(updates, fields, "OTMslew", "OTMslew", false, false, false);
                addUpdate(updates, fields, "OTMdead", "OTMdead", false, false, false);
                addUpdate(updates, fields, "OTMairmass_start", "OTMairmass_start", false, false, false);
                addUpdate(updates, fields, "OTMairmass_end", "OTMairmass_end", false, false, false);
                addUpdate(updates, fields, "OTMsky", "OTMsky", false, false, false);
                addUpdate(updates, fields, "OTMmoon", "OTMmoon", false, false, false);
                addUpdate(updates, fields, "OTMSNR", "OTMSNR", false, false, false);
                addUpdate(updates, fields, "OTMres", "OTMres", false, false, false);
                addUpdate(updates, fields, "OTMseeing", "OTMseeing", false, false, false);
                if (updates.isEmpty()) continue;

                AggUpdates &agg = aggByObsId[obsId];
                agg.updates = updates;

                const QString startStr = updates.value("OTMexp_start").toString().trimmed();
                if (!startStr.isEmpty()) {
                  const QDateTime dt = parseUtcIso(startStr);
                  if (dt.isValid() && (!agg.hasStart || dt < agg.startMin)) {
                    agg.startMin = dt;
                    agg.startStr = startStr;
                    agg.hasStart = true;
                  }
                }
                const QString endStr = updates.value("OTMexp_end").toString().trimmed();
                if (!endStr.isEmpty()) {
                  const QDateTime dt = parseUtcIso(endStr);
                  if (dt.isValid() && (!agg.hasEnd || dt > agg.endMax)) {
                    agg.endMax = dt;
                    agg.endStr = endStr;
                    agg.hasEnd = true;
                  }
                }
              }

              int updated = 0;
              for (auto it = aggByObsId.begin(); it != aggByObsId.end(); ++it) {
                const QString obsId = it.key();
                AggUpdates agg = it.value();
                if (agg.hasStart) agg.updates.insert("OTMexp_start", agg.startStr);
                if (agg.hasEnd) agg.updates.insert("OTMexp_end", agg.endStr);

                QVariantMap keyValues;
                QStringList members = context->membersByScienceObsId.value(obsId);
                if (members.isEmpty()) {
                  members << obsId;
                }
                for (const QString &memberObsId : members) {
                  keyValues.clear();
                  keyValues.insert("OBSERVATION_ID", memberObsId);
                  if (context->oldSlitwidth.contains(memberObsId) &&
                      context->targetColumnNames.contains("OTMSLITWIDTH")) {
                    keyValues.insert("OTMslitwidth", context->oldSlitwidth.value(memberObsId));
                  }

                  QString rowError;
                  if (!dbClient_.updateColumnsByKey(config_.tableTargets, agg.updates, keyValues, &rowError)) {
                    warnings << QString("Output (OBSERVATION_ID %1): %2")
                                    .arg(memberObsId)
                                    .arg(rowError.isEmpty() ? "Update failed" : rowError);
                    continue;
                  }
                  ++updated;
                }
              }

              setTargetsPanel_->refresh();
              auto finishSummary = [this, context, updated, warnings, quiet]() {
                if (!quiet) {
                  QString summary = QString("OTM updated %1 targets.").arg(updated);
                  if (!context->setName.trimmed().isEmpty()) {
                    summary = QString("OTM updated %1 targets in set \"%2\".")
                                  .arg(updated)
                                  .arg(context->setName.trimmed());
                  }
                  if (!warnings.isEmpty()) {
                    summary += "\n\nWarnings:\n" + warnings.join("\n");
                  }
                  showInfo(this, "Run OTM", summary);
                } else {
                  statusBar()->showMessage(QString("OTM updated %1 targets.").arg(updated), 5000);
                }
              };

              auto runTimeline = [this, context, pythonCmd, env, quiet, cleanupFiles]() {
                if (!timelinePanel_) {
                  cleanupFiles();
                  return;
                }
                const QString timelineScript = QDir(ngpsRoot_).filePath("Python/OTM/otm_timeline.py");
                if (!QFile::exists(timelineScript)) {
                  if (!quiet) {
                    showWarning(this, "Run OTM", QString("Timeline script not found: %1")
                                                .arg(timelineScript));
                  } else {
                    statusBar()->showMessage("Timeline script missing.", 8000);
                  }
                  cleanupFiles();
                  return;
                }

                context->timelinePath = QDir::temp().filePath(
                    QString("ngps_otm_timeline_%1_%2.json").arg(context->setId).arg(context->timestamp));

                QProcess *timelineProc = new QProcess(this);
                timelineProc->setProcessEnvironment(env);
                if (!ngpsRoot_.isEmpty()) timelineProc->setWorkingDirectory(ngpsRoot_);

                QStringList targs;
                targs << timelineScript
                      << "--input" << context->inputPath
                      << "--output" << context->outputPath
                      << "--json" << context->timelinePath;
                if (!context->startUtc.trimmed().isEmpty()) {
                  targs << "--start" << context->startUtc.trimmed();
                }

                auto timelineOutput = std::make_shared<QString>();
                connect(timelineProc, &QProcess::readyReadStandardOutput, this,
                        [timelineProc, timelineOutput]() {
                          *timelineOutput += QString::fromUtf8(timelineProc->readAllStandardOutput());
                        });
                connect(timelineProc, &QProcess::readyReadStandardError, this,
                        [timelineProc, timelineOutput]() {
                          *timelineOutput += QString::fromUtf8(timelineProc->readAllStandardError());
                        });
                connect(timelineProc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
                        [this, timelineProc, timelineOutput, context, quiet, cleanupFiles](int code,
                                                                                           QProcess::ExitStatus status) {
                          if (status != QProcess::NormalExit || code != 0) {
                            if (!quiet) {
                              showWarning(this, "Run OTM", "Timeline generation failed:\n" + *timelineOutput);
                            } else {
                              statusBar()->showMessage("Timeline generation failed.", 8000);
                            }
                            timelineProc->deleteLater();
                            cleanupFiles();
                            return;
                          }

                          TimelineData data;
                          QString parseError;
                          if (!loadTimelineJson(context->timelinePath, &data, &parseError)) {
                            if (!quiet) {
                              showWarning(this, "Run OTM", "Failed to load timeline data:\n" + parseError);
                            } else {
                              statusBar()->showMessage("Timeline data unreadable.", 8000);
                            }
                            timelineProc->deleteLater();
                            cleanupFiles();
                            return;
                          }

                          if (!context->obsOrderByObsId.isEmpty() && !data.targets.isEmpty()) {
                            QVector<QPair<int, QString>> order;
                            order.reserve(data.targets.size());
                            for (const TimelineTarget &target : data.targets) {
                              const int raw = context->obsOrderByObsId.value(
                                  target.obsId, std::numeric_limits<int>::max());
                              order.append({raw, target.obsId});
                            }
                            std::sort(order.begin(), order.end(),
                                      [](const auto &a, const auto &b) {
                                        if (a.first != b.first) return a.first < b.first;
                                        return a.second < b.second;
                                      });
                            QHash<QString, int> seqByObsId;
                            int seq = 1;
                            for (const auto &entry : order) {
                              if (!seqByObsId.contains(entry.second)) {
                                seqByObsId.insert(entry.second, seq++);
                              }
                            }
                            for (TimelineTarget &target : data.targets) {
                              if (seqByObsId.contains(target.obsId)) {
                                target.obsOrder = seqByObsId.value(target.obsId);
                              }
                            }
                          }

                          if (!context->obsOrderByObsId.isEmpty()) {
                            for (TimelineTarget &target : data.targets) {
                              if (context->obsOrderByObsId.contains(target.obsId)) {
                                if (target.obsOrder <= 0) {
                                  target.obsOrder = context->obsOrderByObsId.value(target.obsId);
                                }
                              }
                            }
                            std::stable_sort(data.targets.begin(), data.targets.end(),
                                             [](const TimelineTarget &a, const TimelineTarget &b) {
                                               if (a.obsOrder > 0 && b.obsOrder > 0) {
                                                 return a.obsOrder < b.obsOrder;
                                               }
                                               if (a.obsOrder > 0 || b.obsOrder > 0) {
                                                 return a.obsOrder > 0;
                                               }
                                               if (a.startUtc.isValid() && b.startUtc.isValid()) {
                                                 return a.startUtc < b.startUtc;
                                               }
                                               return a.name < b.name;
                                             });
                          }

                          data.airmassLimit = context->airmassMax;
                          timelinePanel_->setData(data);
                          const QVariantMap current = setTargetsPanel_->currentRowValues();
                          const QString obsId = current.value("OBSERVATION_ID").toString();
                          if (!obsId.isEmpty()) {
                            timelinePanel_->setSelectedObsId(obsId);
                          }

                          timelineProc->deleteLater();
                          cleanupFiles();
                        });

                timelineProc->start(pythonCmd, targs);
              };

              finishSummary();
              runTimeline();
              proc->deleteLater();
              if (rerun) scheduleAutoOtmRun();
            });

    proc->start(pythonCmd, args);
  }

  void updateSetViewFromSelection() {
    const QVariantMap values = setsPanel_->currentRowValues();
    const QVariant setId = values.value("SET_ID");
    if (!setId.isValid()) {
      setTargetsPanel_->clearFixedFilter();
      setTargetsPanel_->refresh();
      if (timelinePanel_) timelinePanel_->clear();
      return;
    }
    setTargetsPanel_->setFixedFilter("SET_ID", setId.toString());
    setTargetsPanel_->refresh();
    if (timelinePanel_) timelinePanel_->clear();
    scheduleAutoOtmRun();
  }

  void handleTimelineReorder(const QString &fromObsId, const QString &toObsId) {
    if (fromObsId.isEmpty()) return;
    if (!dbClient_.isOpen()) {
      statusBar()->showMessage("Reorder failed: not connected.", 5000);
      return;
    }

    QString error;
    if (toObsId.isEmpty()) {
      if (!setTargetsPanel_->moveGroupToTopObsId(fromObsId, &error)) {
        statusBar()->showMessage(error.isEmpty() ? "Reorder failed." : error, 5000);
        return;
      }
    } else {
      if (!setTargetsPanel_->moveGroupAfterObsId(fromObsId, toObsId, &error)) {
        statusBar()->showMessage(error.isEmpty() ? "Reorder failed." : error, 5000);
        return;
      }
    }

    scheduleAutoOtmRun();
  }

private:
  struct SeqProcessConfig {
    QString cmd;
    QProcessEnvironment env;
    QString workDir;
  };

  SeqProcessConfig seqProcessConfig() const {
    QSettings settings(kSettingsOrg, kSettingsApp);
    QString cmd = settings.value("seqCommand").toString();
    if (cmd.isEmpty()) {
      if (!ngpsRoot_.isEmpty()) {
        const QString candidate = QDir(ngpsRoot_).filePath("run/seq");
        if (QFile::exists(candidate)) {
          cmd = candidate;
        }
      }
      if (cmd.isEmpty()) cmd = "seq";
    }

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    if (!seqConfigPath_.isEmpty()) {
      env.insert("SEQUENCERD_CONFIG", seqConfigPath_);
    }
    if (!ngpsRoot_.isEmpty()) {
      env.insert("NGPS_ROOT", ngpsRoot_);
    }

    SeqProcessConfig cfg;
    cfg.cmd = cmd;
    cfg.env = env;
    cfg.workDir = ngpsRoot_;
    return cfg;
  }

  static bool outputHasState(const QString &output, const QString &state) {
    const QString pattern = QString("\\b%1\\b").arg(QRegularExpression::escape(state));
    const QRegularExpression re(pattern, QRegularExpression::CaseInsensitiveOption);
    return re.match(output).hasMatch();
  }

  bool createTargetSet(const QString &setName, int *setIdOut, QString *error) {
    QList<ColumnMeta> setColumns;
    if (!dbClient_.loadColumns(config_.tableSets, setColumns, error)) {
      return false;
    }

    QVariantMap values;
    QSet<QString> nullColumns;
    values.insert("SET_NAME", setName);

    if (!dbClient_.insertRecord(config_.tableSets, setColumns, values, nullColumns, error)) {
      return false;
    }

    QVariant idValue;
    if (dbClient_.fetchSingleValue("SELECT LAST_INSERT_ID()", {}, &idValue, error)) {
      bool ok = false;
      int id = idValue.toInt(&ok);
      if (ok) {
        if (setIdOut) *setIdOut = id;
        return true;
      }
    }

    if (!dbClient_.fetchSingleValue(
            QString("SELECT `SET_ID` FROM `%1` WHERE `SET_NAME`=? ORDER BY `SET_ID` DESC LIMIT 1")
                .arg(config_.tableSets),
            {setName}, &idValue, error)) {
      return false;
    }
    bool ok = false;
    int id = idValue.toInt(&ok);
    if (!ok) {
      if (error) *error = "Unable to determine new SET_ID";
      return false;
    }
    if (setIdOut) *setIdOut = id;
    return true;
  }

  void openDatabase() {
    dbClient_.close();
    QString error;
    if (!dbClient_.connect(config_, &error)) {
      connStatus_->setText("Connection failed");
      showError(this, "DB Connection", error.isEmpty() ? "Unable to connect" : error);
      return;
    }

    connStatus_->setText(QString("Connected: %1@%2:%3/%4")
                             .arg(config_.user)
                             .arg(config_.host)
                             .arg(config_.port)
                             .arg(config_.schema));

    setsPanel_->setDatabase(&dbClient_, config_.tableSets);
    setTargetsPanel_->setDatabase(&dbClient_, config_.tableTargets);

    settingsForSeq();
  }

  void settingsForSeq() {
    seqConfigPath_ = configPath_;
    ngpsRoot_ = inferNgpsRootFromConfig(seqConfigPath_);
    initializeOtmStart();
  }

  QString resolveOtmPython() const {
    QSettings settings(kSettingsOrg, kSettingsApp);
    QString cmd = settings.value("otmPython").toString().trimmed();
    if (!cmd.isEmpty()) {
      return cmd;
    }

    const QString envCmd = qEnvironmentVariable("NGPS_PYTHON");
    if (!envCmd.isEmpty()) return envCmd;

    const QString venv = qEnvironmentVariable("VIRTUAL_ENV");
    if (!venv.isEmpty()) {
      const QString candidate = QDir(venv).filePath("bin/python");
      if (QFileInfo::exists(candidate)) return candidate;
    }

    const QString homeCandidate = QDir::home().filePath("venvs/ngps/bin/python");
    if (QFileInfo::exists(homeCandidate)) return homeCandidate;

    if (!ngpsRoot_.isEmpty()) {
      const QString localVenv = QDir(ngpsRoot_).filePath("venv/bin/python");
      if (QFileInfo::exists(localVenv)) return localVenv;
    }

    return "python3";
  }

  OtmSettings loadOtmSettings() const {
    QSettings settings(kSettingsOrg, kSettingsApp);
    OtmSettings s;
    s.seeingFwhm = settings.value("otmSeeingFwhm", 1.1).toDouble();
    s.seeingPivot = settings.value("otmSeeingPivot", 500.0).toDouble();
    s.airmassMax = settings.value("otmAirmassMax", 4.0).toDouble();
    s.useSkySim = settings.value("otmUseSkySim", true).toBool();
    s.pythonCmd = settings.value("otmPython").toString();
    if (s.pythonCmd.trimmed().isEmpty()) {
      const QString defaultPython = QDir::home().filePath("venvs/ngps/bin/python");
      if (QFileInfo::exists(defaultPython)) {
        s.pythonCmd = defaultPython;
      }
    }
    return s;
  }

  void saveOtmSettings(const OtmSettings &settings) {
    QSettings cfg(kSettingsOrg, kSettingsApp);
    cfg.setValue("otmSeeingFwhm", settings.seeingFwhm);
    cfg.setValue("otmSeeingPivot", settings.seeingPivot);
    cfg.setValue("otmAirmassMax", settings.airmassMax);
    cfg.setValue("otmUseSkySim", settings.useSkySim);
    if (settings.pythonCmd.trimmed().isEmpty()) {
      cfg.remove("otmPython");
    } else {
      cfg.setValue("otmPython", settings.pythonCmd.trimmed());
    }
  }

  QString loadOtmStart() const {
    QSettings settings(kSettingsOrg, kSettingsApp);
    return settings.value("otmStart").toString().trimmed();
  }

  void saveOtmStart() {
    if (!otmStartEdit_) return;
    if (otmUseNow_ && otmUseNow_->isChecked()) {
      return;
    }
    QString text = otmStartEdit_->text().trimmed();
    if (text.isEmpty()) {
      text = estimateTwilightUtc();
      if (text.isEmpty()) text = currentUtcString();
      otmStartEdit_->setText(text);
    }
    text = normalizeOtmStartText(text, true);
    lastOtmStartManual_ = text;
    QSettings settings(kSettingsOrg, kSettingsApp);
    settings.setValue("otmStart", text);
  }

  QString currentUtcString() const {
    return QDateTime::currentDateTimeUtc().toString("yyyy-MM-ddTHH:mm:ss.zzz");
  }

  QString normalizeOtmStartText(const QString &text, bool quiet, bool *changed = nullptr) {
    QString normalized = text.trimmed();
    if (normalized.contains(' ') && !normalized.contains('T')) {
      normalized.replace(' ', 'T');
    }
    if (!parseUtcIso(normalized).isValid()) {
      QString fallback = estimateTwilightUtc();
      if (fallback.isEmpty()) fallback = currentUtcString();
      if (changed) *changed = true;
      if (otmStartEdit_) {
        otmStartEdit_->setText(fallback);
      }
      if (!quiet) {
        statusBar()->showMessage(
            QString("Invalid OTM start time; using %1").arg(fallback), 8000);
      }
      return fallback;
    }
    if (changed) *changed = (normalized != text.trimmed());
    return normalized;
  }

  QString estimateTwilightUtc() const {
    const QString pythonCmd = resolveOtmPython();
    if (pythonCmd.isEmpty()) return QString();

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    if (env.contains("PYTHONHOME")) env.remove("PYTHONHOME");
    if (env.contains("PYTHONPATH")) env.remove("PYTHONPATH");
    const QString addPath = QDir(ngpsRoot_).filePath("Python");
    env.insert("PYTHONPATH", addPath);
    env.insert("PYTHONNOUSERSITE", "1");

    QProcess proc;
    proc.setProcessEnvironment(env);
    if (!ngpsRoot_.isEmpty()) proc.setWorkingDirectory(ngpsRoot_);

    const QString code = R"PY(
from astropy.time import Time
from astropy.coordinates import EarthLocation, AltAz, get_sun
import astropy.units as u
import numpy as np
import sys

loc = EarthLocation.of_site('Palomar')
t0 = Time.now()
target = -12.0
times = t0 + np.linspace(0, 1, 289) * u.day  # 5-min steps
alts = get_sun(times).transform_to(AltAz(obstime=times, location=loc)).alt.deg

for i in range(len(alts) - 1):
    if alts[i] > target and alts[i+1] <= target:
        frac = (target - alts[i]) / (alts[i+1] - alts[i]) if alts[i+1] != alts[i] else 0.0
        t = times[i] + frac * (times[i+1] - times[i])
        print(t.utc.iso)
        sys.exit(0)

# fallback: any crossing
for i in range(len(alts) - 1):
    if (alts[i] - target) * (alts[i+1] - target) <= 0:
        frac = (target - alts[i]) / (alts[i+1] - alts[i]) if alts[i+1] != alts[i] else 0.0
        t = times[i] + frac * (times[i+1] - times[i])
        print(t.utc.iso)
        sys.exit(0)

print(t0.utc.iso)
)PY";

    proc.start(pythonCmd, {"-c", code});
    if (!proc.waitForFinished(6000)) {
      return QString();
    }
    if (proc.exitStatus() != QProcess::NormalExit || proc.exitCode() != 0) {
      return QString();
    }
    const QString out = QString::fromUtf8(proc.readAllStandardOutput()).trimmed();
    return out;
  }

  void initializeOtmStart() {
    if (!otmStartEdit_) return;
    const QString saved = loadOtmStart();
    if (!saved.isEmpty()) {
      otmStartEdit_->setText(saved);
      lastOtmStartManual_ = saved;
      return;
    }

    const QString twilight = estimateTwilightUtc();
    const QString useVal = twilight.isEmpty() ? currentUtcString() : twilight;
    otmStartEdit_->setText(useVal);
    lastOtmStartManual_ = useVal;
    saveOtmStart();
  }

  void handleOtmUseNowToggle(bool checked) {
    if (!otmStartEdit_) return;
    if (checked) {
      lastOtmStartManual_ = otmStartEdit_->text().trimmed();
      otmStartEdit_->setText(currentUtcString());
      otmStartEdit_->setEnabled(false);
    } else {
      otmStartEdit_->setEnabled(true);
      if (!lastOtmStartManual_.isEmpty()) {
        otmStartEdit_->setText(lastOtmStartManual_);
      } else {
        initializeOtmStart();
      }
    }
    saveOtmStart();
    scheduleAutoOtmRun();
  }

  void runSeqCommandAndCapture(const QStringList &args,
                               const std::function<void(int, const QString &)> &onFinished) {
    SeqProcessConfig cfg = seqProcessConfig();
    QProcess *proc = new QProcess(this);
    proc->setProcessEnvironment(cfg.env);
    if (!cfg.workDir.isEmpty()) {
      proc->setWorkingDirectory(cfg.workDir);
    }

    auto output = std::make_shared<QString>();
    statusBar()->showMessage(QString("Running: %1 %2").arg(cfg.cmd, args.join(' ')), 5000);
    connect(proc, &QProcess::readyReadStandardOutput, this, [proc, output]() {
      *output += QString::fromUtf8(proc->readAllStandardOutput());
    });
    connect(proc, &QProcess::readyReadStandardError, this, [proc, output]() {
      *output += QString::fromUtf8(proc->readAllStandardError());
    });
    connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
            [this, proc, output, onFinished](int code, QProcess::ExitStatus status) {
              const QString msg = QString("Seq exit %1 (%2)")
                                      .arg(code)
                                      .arg(status == QProcess::NormalExit ? "normal" : "crash");
              statusBar()->showMessage(msg, 5000);
              if (code != 0 && !output->isEmpty()) {
                showWarning(this, "Sequencer", *output);
              }
              if (onFinished) {
                onFinished(code, *output);
              }
              proc->deleteLater();
            });

    proc->start(cfg.cmd, args);
  }

  void seqStartWithStartupCheck() {
    runSeqCommandAndCapture({"state"}, [this](int code, const QString &output) {
      if (code == 0) {
        const bool ready = outputHasState(output, "READY");
        const bool running = outputHasState(output, "RUNNING");
        const bool starting = outputHasState(output, "STARTING");
        if (running || starting) {
          statusBar()->showMessage("Sequencer already running/starting.", 5000);
          return;
        }
        if (!ready && !running && !starting) {
          runSeqCommandAndCapture({"startup"},
                                  [this](int, const QString &) { runSeqCommand({"start"}); });
          return;
        }
        runSeqCommand({"start"});
        return;
      }
      runSeqCommand({"start"});
    });
  }

  void runSeqCommand(const QStringList &args) {
    SeqProcessConfig cfg = seqProcessConfig();
    QProcess *proc = new QProcess(this);
    proc->setProcessEnvironment(cfg.env);
    if (!cfg.workDir.isEmpty()) {
      proc->setWorkingDirectory(cfg.workDir);
    }

    auto output = std::make_shared<QString>();
    statusBar()->showMessage(QString("Running: %1 %2").arg(cfg.cmd, args.join(' ')), 5000);
    connect(proc, &QProcess::readyReadStandardOutput, this, [proc, output]() {
      *output += QString::fromUtf8(proc->readAllStandardOutput());
    });
    connect(proc, &QProcess::readyReadStandardError, this, [proc, output]() {
      *output += QString::fromUtf8(proc->readAllStandardError());
    });
    connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
            [this, proc, output](int code, QProcess::ExitStatus status) {
              const QString msg = QString("Seq exit %1 (%2)")
                                      .arg(code)
                                      .arg(status == QProcess::NormalExit ? "normal" : "crash");
              statusBar()->showMessage(msg, 5000);
              if (code != 0 && !output->isEmpty()) {
                showWarning(this, "Sequencer", *output);
              }
              proc->deleteLater();
            });

    proc->start(cfg.cmd, args);
  }

  QTabWidget *tabs_ = nullptr;
  TablePanel *setsPanel_ = nullptr;
  TablePanel *setTargetsPanel_ = nullptr;
  TimelinePanel *timelinePanel_ = nullptr;

  QLabel *connStatus_ = nullptr;

  QPushButton *seqStart_ = nullptr;
  QPushButton *seqAbort_ = nullptr;
  QPushButton *runOtm_ = nullptr;
  QPushButton *showOtmLog_ = nullptr;
  QLineEdit *otmStartEdit_ = nullptr;
  QCheckBox *otmUseNow_ = nullptr;
  QString lastOtmStartManual_;
  QString lastOtmLog_;
  QTimer *otmAutoTimer_ = nullptr;
  bool otmRunning_ = false;
  bool otmAutoPending_ = false;
  bool closing_ = false;

  DbConfig config_;
  DbClient dbClient_;

  QString configPath_;
  QString seqConfigPath_;
  QString ngpsRoot_;
};

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  MainWindow window;
  window.resize(1200, 800);
  window.show();
  return app.exec();
}

#include "main.moc"
