#include "TimelineModel.h"
#include "utils/JsonXmlFormatter.h"
#include "utils/FileUtils.h"
#include <QTextStream>
#include <QFileInfo>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include <QElapsedTimer>
#include <QCoreApplication>
#include <QColor>

TimelineModel::TimelineModel(const QString& filePath, QObject* parent)
    : QAbstractTableModel(parent), filePath(filePath), timelineType(Unknown), file(filePath), unsavedChanges(false)
{
    // Validate file size before processing
    QFileInfo fileInfo(filePath);
    if (fileInfo.size() > MAX_FILE_SIZE) {
        throw std::runtime_error("File size exceeds maximum limit (2GB)");
    }
    
    if (!fileInfo.exists()) {
        throw std::runtime_error("File does not exist");
    }
    
    if (!fileInfo.isReadable()) {
        throw std::runtime_error("File is not readable");
    }
    
    detectFormat();
    buildLineIndex();
    loadTaggedRows();
}

TimelineModel::~TimelineModel() { file.close(); }

void TimelineModel::detectFormat()
{
    QMutexLocker locker(&fileMutex);
    QElapsedTimer timer;
    timer.start();

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        timelineType = Unknown;
        return;
    }
    QTextStream in(&file);
    QString headerLine = in.readLine();
    
    try {
        headers = FileUtils::parseCsvLine(headerLine);
    } catch (const std::exception& e) {
        qWarning() << "Error parsing CSV header:" << e.what();
        timelineType = Unknown;
        return;
    }
    
    if (headers == QStringList({"Date","Size","Type","Mode","UID","GID","Meta","File Name"}))
        timelineType = Filesystem;
    else if (headers == QStringList({"datetime","timestamp_desc","source","source_long","message","parser","display_name","tag"}))
        timelineType = Super;
    else
        timelineType = Unknown;
    file.seek(0);
    qDebug() << "TimelineModel: format detection took" << timer.elapsed() << "ms, type:"
             << (timelineType == Filesystem ? "Filesystem" :
                 timelineType == Super      ? "Super"      : "Unknown");
}

void TimelineModel::buildLineIndex()
{
    QMutexLocker locker(&fileMutex);
    QElapsedTimer timer;
    timer.start();
    qDebug() << "TimelineModel: building line index for" << filePath;

    lineOffsets.clear();
    if (!file.isOpen()) {
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            throw std::runtime_error("Failed to open file for reading");
        }
    }

    qint64 offset = 0;
    QTextStream in(&file);
    QString line;

    line = in.readLine();
    if (line.isNull())
        throw std::runtime_error("File appears to be empty or corrupted");
    offset += line.toUtf8().size() + 1;

    int lineCount = 0;
    while (!in.atEnd()) {
        if (lineCount >= MAX_LINE_COUNT)
            throw std::runtime_error("File exceeds maximum line count limit (10 million lines)");

        if (lineOffsets.size() * static_cast<qint64>(sizeof(qint64)) > MAX_INDEX_MEMORY)
            throw std::runtime_error("File index exceeds memory limit (500MB)");

        lineOffsets.append(offset);
        line = in.readLine();
        if (line.isNull())
            break;

        offset += line.toUtf8().size() + 1;
        ++lineCount;

        // Every 50 000 lines, briefly yield to the event loop so the UI stays
        // responsive. ExcludeUserInputEvents prevents re-entrancy issues.
        if (lineCount % 50000 == 0) {
            locker.unlock();
            QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
            locker.relock();
            qDebug() << "TimelineModel: indexed" << lineCount << "lines so far...";
        }
    }

    file.seek(0);
    qDebug() << "TimelineModel: indexed" << lineCount << "lines in" << timer.elapsed()
             << "ms, index memory:" << (lineOffsets.size() * static_cast<qint64>(sizeof(qint64))) << "bytes";
}

int TimelineModel::rowCount(const QModelIndex&) const
{
    return m_isFiltered ? m_filteredRows.size() : lineOffsets.size();
}

int TimelineModel::toSourceRow(int viewRow) const
{
    if (m_isFiltered && viewRow >= 0 && viewRow < m_filteredRows.size())
        return m_filteredRows[viewRow];
    return viewRow;
}

int TimelineModel::columnCount(const QModelIndex&) const
{
    return headers.size();
}

QVariant TimelineModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= rowCount())
        return QVariant();

    const int srcRow = toSourceRow(index.row());

    // Handle tag column for Super timelines
    if (timelineType == Super && index.column() == 7) {
        if (role == Qt::CheckStateRole)
            return taggedRows.contains(srcRow) ? Qt::Checked : Qt::Unchecked;
        if (role == Qt::DisplayRole)
            return QVariant();
    }

    // Background tint for tagged rows
    if (role == Qt::BackgroundRole && taggedRows.contains(srcRow))
        return QColor(240, 240, 240);

    if (role != Qt::DisplayRole)
        return QVariant();

    QMutexLocker locker(&fileMutex);

    if (!file.isOpen()) {
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << "Failed to open file for reading";
            return QVariant();
        }
    }

    if (srcRow < 0 || srcRow >= lineOffsets.size())
        return QVariant();

    if (!file.seek(lineOffsets[srcRow])) {
        qWarning() << "Failed to seek to file position";
        return QVariant();
    }
    
    QString line = QString::fromUtf8(file.readLine().trimmed());
    QStringList fields;
    
    try {
        fields = FileUtils::parseCsvLine(line);
    } catch (const std::exception& e) {
        qWarning() << "Error parsing CSV line:" << e.what();
        return QVariant();
    }
    
    if (index.column() < 0 || index.column() >= fields.size())
        return QVariant();
    
    QString cellData = fields[index.column()];
    
    // Apply JSON/XML formatting for message field in Super timelines
    if (timelineType == Super && index.column() == 4) { // message field
        return JsonXmlFormatter::formatIfApplicable(cellData);
    }
    
    return cellData;
}

QVariant TimelineModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
        return QVariant();
    if (section < 0 || section >= headers.size())
        return QVariant();
    return headers[section];
}

int TimelineModel::columnIndex(const QString& name) const
{
    return headers.indexOf(name);
}

bool TimelineModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || timelineType != Super || index.column() != 7)
        return false;

    if (role == Qt::CheckStateRole) {
        setRowTagged(toSourceRow(index.row()), value.toInt() == Qt::Checked);
        return true;
    }
    return false;
}

Qt::ItemFlags TimelineModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
        
    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    
    // Make tag column checkable for Super timelines
    if (timelineType == Super && index.column() == 7) {
        flags |= Qt::ItemIsUserCheckable;
    }
    
    return flags;
}

TimelineModel::TimelineType TimelineModel::type() const
{
    return timelineType;
}

bool TimelineModel::isRowTagged(int row) const
{
    return taggedRows.contains(row);
}

void TimelineModel::setRowTagged(int sourceRow, bool tagged)
{
    bool changed = false;
    if (tagged && !taggedRows.contains(sourceRow)) {
        taggedRows.insert(sourceRow);
        changed = true;
    } else if (!tagged && taggedRows.contains(sourceRow)) {
        taggedRows.remove(sourceRow);
        changed = true;
    }

    if (changed) {
        unsavedChanges = true;
        // Notify the view using the view-row index, not the source row.
        int viewRow = m_isFiltered ? m_filteredRows.indexOf(sourceRow) : sourceRow;
        if (viewRow >= 0)
            emit dataChanged(createIndex(viewRow, 0), createIndex(viewRow, columnCount() - 1));
        emit tagsModified(unsavedChanges);
    }
}

bool TimelineModel::hasUnsavedChanges() const
{
    return unsavedChanges;
}

bool TimelineModel::saveTaggedRows()
{
    QString tagFilePath = getTagFilePath();
    if (tagFilePath.isEmpty()) {
        qWarning() << "Failed to determine tag file path";
        return false;
    }
    
    QFile tagFile(tagFilePath);
    
    if (!tagFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Failed to open tag file for writing (check permissions)";
        return false;
    }
    
    QTextStream out(&tagFile);
    for (int row : taggedRows) {
        out << row << "\n";
    }
    
    if (out.status() != QTextStream::Ok) {
        qWarning() << "Error occurred while writing tag file";
        return false;
    }
    
    unsavedChanges = false;
    emit tagsModified(unsavedChanges);
    return true;
}

QString TimelineModel::getFilePath() const
{
    return filePath;
}

void TimelineModel::loadTaggedRows()
{
    QString tagFilePath = getTagFilePath();
    if (tagFilePath.isEmpty()) {
        return; // No valid tag file path
    }
    
    QFile tagFile(tagFilePath);
    
    if (!tagFile.exists()) {
        return; // No tag file exists, start with empty set
    }
    
    if (!tagFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open tag file for reading (check permissions)";
        return;
    }
    
    QTextStream in(&tagFile);
    int loadedCount = 0;
    const int MAX_TAGS = 1000000; // Limit number of tags to prevent memory issues
    
    while (!in.atEnd() && loadedCount < MAX_TAGS) {
        QString line = in.readLine().trimmed();
        if (line.length() > 20) { // Sanity check - row numbers shouldn't be this long
            qWarning() << "Invalid tag data detected, skipping";
            continue;
        }
        
        bool ok;
        int row = line.toInt(&ok);
        if (ok && row >= 0 && row < lineOffsets.size()) {
            taggedRows.insert(row);
            loadedCount++;
        } else if (ok) {
            qWarning() << "Tag references invalid row number, skipping";
        }
    }
    
    if (loadedCount >= MAX_TAGS) {
        qWarning() << "Tag file contains too many entries, some may not be loaded";
    }
}

QString TimelineModel::sanitizeFileName(const QString& fileName) const
{
    QString sanitized = fileName;
    
    sanitized.remove(QRegularExpression(R"([.]{2,})"));   // Remove .. sequences
    sanitized.remove(QRegularExpression(R"([/\\])"));    // Remove path separators
    sanitized.remove(QRegularExpression(R"([<>:"|?*])"));// Remove invalid filename chars
    
    // Limit filename length
    if (sanitized.length() > 200) {
        sanitized = sanitized.left(200);
    }
    
    // Ensure we have a valid filename
    if (sanitized.isEmpty()) {
        sanitized = "timeline";
    }
    
    return sanitized;
}

bool TimelineModel::ensureTagDirectory() const
{
    QString tagDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir;
    if (!dir.exists(tagDir)) {
        return dir.mkpath(tagDir);
    }
    return true;
}

bool TimelineModel::isFiltered() const { return m_isFiltered; }

int TimelineModel::filteredRowCount() const
{
    return m_isFiltered ? m_filteredRows.size() : -1;
}

void TimelineModel::clearFilter()
{
    if (!m_isFiltered)
        return;
    beginResetModel();
    m_filteredRows.clear();
    m_isFiltered = false;
    endResetModel();
}

void TimelineModel::applyFilter(const QString& column, const QString& term)
{
    if (term.isEmpty()) {
        clearFilter();
        return;
    }

    const int colIdx = (column == "All Columns") ? -1 : columnIndex(column);
    const QString lowerTerm = term.toLower();
    const int total = lineOffsets.size();
    QVector<int> matches;

    QMutexLocker locker(&fileMutex);
    if (!file.isOpen()) {
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            return;
    }

    for (int i = 0; i < total; ++i) {
        file.seek(lineOffsets[i]);
        const QString line = QString::fromUtf8(file.readLine().trimmed());

        try {
            const QStringList fields = FileUtils::parseCsvLine(line);
            bool match = false;
            if (colIdx < 0) {
                for (const QString& f : fields) {
                    if (f.toLower().contains(lowerTerm)) { match = true; break; }
                }
            } else if (colIdx < fields.size()) {
                match = fields[colIdx].toLower().contains(lowerTerm);
            }
            if (match)
                matches.append(i);
        } catch (...) {}

        if (i % 10000 == 0) {
            locker.unlock();
            emit searchProgress(i, total);
            QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
            locker.relock();
        }
    }

    beginResetModel();
    m_filteredRows = matches;
    m_isFiltered = true;
    endResetModel();
}

QString TimelineModel::getTagFilePath() const
{
    if (!ensureTagDirectory()) {
        qWarning() << "Failed to create application data directory";
        return QString(); // Return empty string on failure
    }
    
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        qWarning() << "Source timeline file no longer exists";
        return QString();
    }
    
    QString baseName = sanitizeFileName(fileInfo.completeBaseName());
    if (baseName.isEmpty()) {
        qWarning() << "Unable to generate valid tag filename";
        return QString();
    }
    
    QString tagFileName = baseName + ".tags";
    QString tagDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    
    return tagDir + QDir::separator() + tagFileName;
} 