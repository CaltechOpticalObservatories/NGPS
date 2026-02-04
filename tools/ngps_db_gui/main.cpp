#include <QtWidgets>
#include <QCoreApplication>
#include <QRegularExpression>
#include <QStandardItemModel>
#include <QDebug>
#include <mysqlx/xdevapi.h>
#include <cctype>
#include <memory>

namespace {
const char kSettingsOrg[] = "NGPS";
const char kSettingsApp[] = "ngps_db_gui";
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

class ReorderTableView : public QTableView {
  Q_OBJECT
public:
  explicit ReorderTableView(QWidget *parent = nullptr) : QTableView(parent) {}

signals:
  void dragSwapRequested(int sourceRow, int targetRow);
  void deleteRequested();

protected:
  void mousePressEvent(QMouseEvent *event) override {
    if (event->button() == Qt::LeftButton) {
      pressPos_ = event->pos();
      pressRow_ = indexAt(pressPos_).row();
      dragging_ = false;
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
    unsetCursor();
    QTableView::mouseReleaseEvent(event);
  }

  void keyPressEvent(QKeyEvent *event) override {
    if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace) {
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
      const QVariant raw = values.value(meta.name);
      const QString text = raw.toString();
      const bool hasText = !text.isEmpty();

      if (meta.isAutoIncrement() && !hasText && !nullColumns.contains(meta.name)) {
        continue;
      }

      if (nullColumns.contains(meta.name)) {
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
          vals << "?";
          binds << QString();
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
      const QVariant raw = values.value(meta.name);
      const QString text = raw.toString();
      const bool hasText = !text.isEmpty();

      if (nullColumns.contains(meta.name)) {
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
    view_->horizontalHeader()->setStretchLastSection(true);
    view_->setSortingEnabled(sortingEnabled_);
    view_->setContextMenuPolicy(Qt::CustomContextMenu);
    layout->addWidget(view_, 1);

    statusLabel_ = new QLabel("Not connected", this);
    layout->addWidget(statusLabel_);

    connect(refreshButton_, &QPushButton::clicked, this, &TablePanel::refresh);
    connect(addButton_, &QPushButton::clicked, this, &TablePanel::addRecord);
    connect(searchApply_, &QPushButton::clicked, this, &TablePanel::refresh);
    connect(searchClear_, &QPushButton::clicked, this, &TablePanel::clearSearch);
    connect(view_, &QWidget::customContextMenuRequested, this, &TablePanel::showContextMenu);
    connect(view_, &ReorderTableView::dragSwapRequested, this, &TablePanel::handleDragSwap);
    connect(view_, &ReorderTableView::deleteRequested, this, &TablePanel::handleDeleteShortcut);
    connect(model_, &QStandardItemModel::itemChanged, this, &TablePanel::handleItemChanged);
    connect(view_->selectionModel(), &QItemSelectionModel::currentRowChanged, this,
            [this](const QModelIndex &, const QModelIndex &) { emit selectionChanged(); });
  }

  void setDatabase(DbClient *db, const QString &tableName) {
    db_ = db;
    tableName_ = tableName;
    columns_.clear();
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

  void setAllowDelete(bool enabled) { allowDelete_ = enabled; }

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

  QList<ColumnMeta> columns() const { return columns_; }

public slots:
  void refresh() {
    refreshWithState(captureViewState());
  }

  void addRecord() {
    if (!db_ || !db_->isOpen()) return;
    RecordEditorDialog dialog(tableName_, columns_, QVariantMap(), true, this);
    if (dialog.exec() == QDialog::Accepted) {
      QString error;
      if (!insertRecord(dialog.values(), dialog.nullColumns(), &error)) {
        QMessageBox::warning(this, "Insert failed", error);
      }
    }
    refresh();
  }

  void clearSearch() {
    searchEdit_->clear();
    refresh();
  }

signals:
  void selectionChanged();

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
      QMessageBox::warning(this, "Update failed",
                           QString("%1 cannot be NULL").arg(meta.name));
      revertItem(item, oldValue, oldIsNull);
      return;
    }
    if (!newIsNull && text.isEmpty() && !meta.nullable) {
      QMessageBox::warning(this, "Update failed",
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

    QVariantMap keyValues;
    for (int c = 0; c < columns_.size(); ++c) {
      if (!columns_[c].isPrimaryKey()) continue;
      QStandardItem *rowItem = model_->item(row, c);
      if (!rowItem) continue;
      const QVariant keyValue = rowItem->data(Qt::UserRole + 1);
      const bool keyIsNull = rowItem->data(Qt::UserRole + 2).toBool();
      if (!keyValue.isValid() || keyIsNull) {
        QMessageBox::warning(this, "Update failed",
                             QString("Primary key %1 is NULL").arg(columns_[c].name));
        revertItem(item, oldValue, oldIsNull);
        return;
      }
      keyValues.insert(columns_[c].name, keyValue);
    }

    QString error;
    if (!db_->updateRecord(tableName_, columns_, values, nullColumns, keyValues, &error)) {
      QMessageBox::warning(this, "Update failed", error);
      revertItem(item, oldValue, oldIsNull);
      return;
    }

    suppressItemChange_ = true;
    item->setData(newIsNull ? QVariant() : newValue, Qt::EditRole);
    item->setData(newIsNull ? QVariant() : newValue, Qt::UserRole + 1);
    item->setData(newIsNull, Qt::UserRole + 2);
    item->setText(displayForVariant(newValue, newIsNull));
    item->setForeground(QBrush(newIsNull ? view_->palette().color(QPalette::Disabled, QPalette::Text)
                                         : view_->palette().color(QPalette::Text)));
    suppressItemChange_ = false;
  }

  void handleDragSwap(int sourceRow, int targetRow) {
    swapRows(sourceRow, targetRow);
  }

  void handleDeleteShortcut() {
    if (!allowDelete_) return;
    const int row = view_->currentIndex().row();
    if (row < 0) return;
    deleteRow(row);
  }

  void showContextMenu(const QPoint &pos) {
    const QModelIndex index = view_->indexAt(pos);
    if (!index.isValid()) return;
    if (!allowDelete_ && !allowReorder_) return;
    const bool searchActive = !searchEdit_->text().trimmed().isEmpty();

    QMenu menu(this);
    QAction *deleteAction = nullptr;
    QAction *duplicateAction = nullptr;
    QAction *moveUp = nullptr;
    QAction *moveDown = nullptr;
    QAction *moveTop = nullptr;
    QAction *moveBottom = nullptr;
    if (allowReorder_ && !searchActive) {
      moveUp = menu.addAction("Move Up");
      moveDown = menu.addAction("Move Down");
      moveTop = menu.addAction("Move to Top");
      moveBottom = menu.addAction("Move to Bottom");
      menu.addSeparator();
    }
    if (allowReorder_) {
      duplicateAction = menu.addAction("Duplicate");
    }
    if (allowDelete_) {
      deleteAction = menu.addAction("Delete");
    }
    QAction *chosen = menu.exec(view_->viewport()->mapToGlobal(pos));
    if (!chosen) return;

    const int row = index.row();
    if (chosen == deleteAction) {
      deleteRow(row);
    } else if (chosen == duplicateAction) {
      duplicateRow(row);
    } else if (chosen == moveUp) {
      moveRow(row, row - 1);
    } else if (chosen == moveDown) {
      moveRow(row, row + 1);
    } else if (chosen == moveTop) {
      moveRow(row, 0);
    } else if (chosen == moveBottom) {
      moveRow(row, model_->rowCount() - 1);
    }
  }

private:
  struct SwapInfo {
    bool valid = false;
    QVariant obsId;
    QVariant obsOrder;
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
    view_->resizeColumnsToContents();
    const QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    statusLabel_->setText(QString("Last refresh: %1").arg(timestamp));
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

  void moveRow(int from, int to) {
    swapRows(from, to);
  }

  void swapRows(int from, int to) {
    if (!allowReorder_) return;
    if (!searchEdit_->text().trimmed().isEmpty()) {
      QMessageBox::information(this, "Reorder disabled",
                               "Clear the search filter before reordering.");
      return;
    }
    if (from < 0 || to < 0 || from >= model_->rowCount() || to >= model_->rowCount()) return;
    if (from == to) return;
    if (!db_ || !db_->isOpen()) {
      QMessageBox::warning(this, "Reorder failed", "Not connected");
      return;
    }

    const SwapInfo src = swapInfoForRow(from);
    const SwapInfo dst = swapInfoForRow(to);
    if (!src.valid || !dst.valid) {
      QMessageBox::warning(this, "Reorder failed",
                           "Missing OBSERVATION_ID/OBS_ORDER values.");
      return;
    }

    ViewState state = captureViewState();
    if (view_->currentIndex().row() == from) {
      QVariantMap desiredKeys = keyValuesForRow(from);
      if (!desiredKeys.isEmpty()) {
        for (const ColumnMeta &meta : columns_) {
          if (!meta.isPrimaryKey()) continue;
          if (meta.name.compare("OBSERVATION_ID", Qt::CaseInsensitive) == 0) {
            desiredKeys[meta.name] = dst.obsId;
            break;
          }
        }
        state.keyValues = desiredKeys;
      }
    }

    QString error;
    if (!db_->swapTargets(tableName_,
                          src.obsId, src.obsOrder,
                          dst.obsId, dst.obsOrder,
                          &error)) {
      QMessageBox::warning(this, "Reorder failed", error);
      refresh();
      return;
    }
    refreshWithState(state);
  }

  void deleteRow(int row) {
    if (!allowDelete_) return;
    if (row < 0 || row >= model_->rowCount()) return;
    if (QMessageBox::question(this, "Delete Target", "Delete the selected target?",
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) {
      return;
    }
    QVariantMap keyValues = keyValuesForRow(row);
    if (keyValues.isEmpty()) {
      QMessageBox::warning(this, "Delete failed", "Missing primary key values.");
      return;
    }
    QString error;
    if (!db_->deleteRecordByKey(tableName_, keyValues, &error)) {
      QMessageBox::warning(this, "Delete failed", error);
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
        QMessageBox::warning(this, "Reorder failed", shiftError);
        refresh();
        return;
      }
    }
    refreshWithState(state);
  }

  void duplicateRow(int row) {
    if (!db_ || !db_->isOpen()) return;
    if (row < 0 || row >= model_->rowCount()) return;

    QVariantMap values;
    QSet<QString> nullColumns;
    for (int c = 0; c < columns_.size(); ++c) {
      const ColumnMeta &meta = columns_.at(c);
      if (meta.isAutoIncrement()) {
        continue;
      }
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

    const int setIdCol = columnIndex("SET_ID");
    const int obsOrderCol = columnIndex("OBS_ORDER");
    if (setIdCol >= 0 && obsOrderCol >= 0) {
      QStandardItem *setItem = model_->item(row, setIdCol);
      if (setItem) {
        const int setId = setItem->data(Qt::UserRole + 1).toInt();
        int nextOrder = 0;
        QString error;
        if (!db_->nextObsOrderForSet(tableName_, setId, &nextOrder, &error)) {
          QMessageBox::warning(this, "Duplicate failed", error);
          return;
        }
        values.insert("OBS_ORDER", nextOrder);
        nullColumns.remove("OBS_ORDER");
      }
    }

    QString error;
    if (!insertRecord(values, nullColumns, &error)) {
      QMessageBox::warning(this, "Duplicate failed", error);
      return;
    }
    refreshWithState(captureViewState());
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

  bool suppressItemChange_ = false;
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
    connStatus_ = new QLabel("Not connected", central);

    topBar->addWidget(seqStart_);
    topBar->addWidget(seqAbort_);
    topBar->addWidget(activateSetButton);
    topBar->addStretch();
    topBar->addWidget(connStatus_);
    layout->addLayout(topBar);

    tabs_ = new QTabWidget(central);
    setsPanel_ = new TablePanel("Target Sets", tabs_);
    targetsPanel_ = new TablePanel("Targets", tabs_);
    setTargetsPanel_ = new TablePanel("Targets (Set View)", tabs_);

    targetsPanel_->setSearchColumn("NAME");
    targetsPanel_->setOrderByColumn("OBS_ORDER");
    targetsPanel_->setSortingEnabled(false);
    targetsPanel_->setAllowReorder(true);
    targetsPanel_->setAllowDelete(true);

    setTargetsPanel_->setSearchColumn("NAME");
    setTargetsPanel_->setSortingEnabled(true);

    tabs_->addTab(setsPanel_, "Target Sets");
    tabs_->addTab(targetsPanel_, "Targets (Active)");
    tabs_->addTab(setTargetsPanel_, "Targets (Set View)");
    layout->addWidget(tabs_, 5);

    setCentralWidget(central);

    connect(seqStart_, &QPushButton::clicked, this, &MainWindow::seqStart);
    connect(seqAbort_, &QPushButton::clicked, this, &MainWindow::seqAbort);
    connect(activateSetButton, &QPushButton::clicked, this, &MainWindow::activateSelectedSet);
    connect(setsPanel_, &TablePanel::selectionChanged, this, &MainWindow::updateSetViewFromSelection);

    connectFromConfig();
  }

private slots:
  void connectFromConfig() {
    const QString cfgPath = detectDefaultConfigPath();
    if (cfgPath.isEmpty()) {
      QMessageBox::warning(this, "Config", "sequencerd.cfg not found.");
      return;
    }
    configPath_ = cfgPath;
    config_ = loadConfigFile(cfgPath);
    if (!config_.isComplete()) {
      QMessageBox::warning(this, "Config", "Config file is missing DB settings.");
      return;
    }
    openDatabase();
  }

  void seqStart() { runSeqCommand({"start"}); }
  void seqAbort() { runSeqCommand({"abort"}); }
  void activateSelectedSet() {
    const QVariantMap values = setsPanel_->currentRowValues();
    if (values.isEmpty()) {
      QMessageBox::information(this, "Target Sets", "Select a target set first.");
      return;
    }
    const QVariant setId = values.value("SET_ID");
    if (!setId.isValid()) {
      QMessageBox::warning(this, "Target Sets", "SET_ID not found.");
      return;
    }
    runSeqCommand({"targetset", setId.toString()});
  }

  void updateSetViewFromSelection() {
    const QVariantMap values = setsPanel_->currentRowValues();
    const QVariant setId = values.value("SET_ID");
    if (!setId.isValid()) {
      setTargetsPanel_->clearFixedFilter();
      setTargetsPanel_->refresh();
      return;
    }
    setTargetsPanel_->setFixedFilter("SET_ID", setId.toString());
    setTargetsPanel_->refresh();
  }

private:
  void openDatabase() {
    dbClient_.close();
    QString error;
    if (!dbClient_.connect(config_, &error)) {
      connStatus_->setText("Connection failed");
      QMessageBox::critical(this, "DB Connection", error.isEmpty() ? "Unable to connect" : error);
      return;
    }

    connStatus_->setText(QString("Connected: %1@%2:%3/%4")
                             .arg(config_.user)
                             .arg(config_.host)
                             .arg(config_.port)
                             .arg(config_.schema));

    setsPanel_->setDatabase(&dbClient_, config_.tableSets);
    targetsPanel_->setDatabase(&dbClient_, config_.tableTargets);
    setTargetsPanel_->setDatabase(&dbClient_, config_.tableTargets);

    settingsForSeq();
  }

  void settingsForSeq() {
    seqConfigPath_ = configPath_;
    ngpsRoot_ = inferNgpsRootFromConfig(seqConfigPath_);
  }

  void runSeqCommand(const QStringList &args) {
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

    QProcess *proc = new QProcess(this);
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    if (!seqConfigPath_.isEmpty()) {
      env.insert("SEQUENCERD_CONFIG", seqConfigPath_);
    }
    if (!ngpsRoot_.isEmpty()) {
      env.insert("NGPS_ROOT", ngpsRoot_);
    }
    proc->setProcessEnvironment(env);
    if (!ngpsRoot_.isEmpty()) {
      proc->setWorkingDirectory(ngpsRoot_);
    }

    auto output = std::make_shared<QString>();
    statusBar()->showMessage(QString("Running: %1 %2").arg(cmd, args.join(' ')), 5000);
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
                QMessageBox::warning(this, "Sequencer", *output);
              }
              proc->deleteLater();
            });

    proc->start(cmd, args);
  }

  QTabWidget *tabs_ = nullptr;
  TablePanel *setsPanel_ = nullptr;
  TablePanel *targetsPanel_ = nullptr;
  TablePanel *setTargetsPanel_ = nullptr;

  QLabel *connStatus_ = nullptr;

  QPushButton *seqStart_ = nullptr;
  QPushButton *seqAbort_ = nullptr;

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
