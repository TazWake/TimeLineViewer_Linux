#include "TimelineModel.h"
#include "utils/JsonXmlFormatter.h"
#include "utils/FileUtils.h"
#include <QTextStream>
#include <QFileInfo>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>

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
}

void TimelineModel::buildLineIndex()
{
    QMutexLocker locker(&fileMutex);
    
    lineOffsets.clear();
    if (!file.isOpen()) {
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            throw std::runtime_error("Failed to open file for reading");
        }
    }
    
    qint64 offset = 0;
    QTextStream in(&file);
    QString line;
    
    // Skip header
    line = in.readLine();
    if (line.isNull()) {
        throw std::runtime_error("File appears to be empty or corrupted");
    }
    offset += line.toUtf8().size() + 1;
    
    int lineCount = 0;
    while (!in.atEnd()) {
        // Check line count limit
        if (lineCount >= MAX_LINE_COUNT) {
            throw std::runtime_error("File exceeds maximum line count limit (10 million lines)");
        }
        
        // Check memory usage for index
        qint64 indexMemory = lineOffsets.size() * sizeof(qint64);
        if (indexMemory > MAX_INDEX_MEMORY) {
            throw std::runtime_error("File index exceeds memory limit (500MB)");
        }
        
        lineOffsets.append(offset);
        line = in.readLine();
        
        if (line.isNull()) {
            break; // End of file
        }
        
        offset += line.toUtf8().size() + 1;
        lineCount++;
    }
    
    file.seek(0);
    qDebug() << "Indexed" << lineCount << "lines, index memory usage:" << (lineOffsets.size() * sizeof(qint64)) << "bytes";
}

int TimelineModel::rowCount(const QModelIndex&) const
{
    return lineOffsets.size();
}

int TimelineModel::columnCount(const QModelIndex&) const
{
    return headers.size();
}

QVariant TimelineModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();
    
    // Handle tag column for Super timelines
    if (timelineType == Super && index.column() == 7) { // tag column
        if (role == Qt::CheckStateRole) {
            return taggedRows.contains(index.row()) ? Qt::Checked : Qt::Unchecked;
        }
        if (role == Qt::DisplayRole) {
            return QVariant(); // Don't show text for checkbox column
        }
    }
    
    // Handle background color for tagged rows
    if (role == Qt::BackgroundRole && taggedRows.contains(index.row())) {
        return QColor(240, 240, 240); // Light gray background for tagged rows
    }
    
    if (role != Qt::DisplayRole)
        return QVariant();
    
    QMutexLocker locker(&fileMutex);
    
    if (!file.isOpen()) {
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << "Failed to open file for reading";
            return QVariant();
        }
    }
    
    if (index.row() < 0 || index.row() >= lineOffsets.size())
        return QVariant();
        
    if (!file.seek(lineOffsets[index.row()])) {
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
        bool checked = value.toInt() == Qt::Checked;
        setRowTagged(index.row(), checked);
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

void TimelineModel::setRowTagged(int row, bool tagged)
{
    if (tagged) {
        if (!taggedRows.contains(row)) {
            taggedRows.insert(row);
            unsavedChanges = true;
            emit dataChanged(createIndex(row, 0), createIndex(row, columnCount() - 1));
            emit dataChanged(unsavedChanges);
        }
    } else {
        if (taggedRows.contains(row)) {
            taggedRows.remove(row);
            unsavedChanges = true;
            emit dataChanged(createIndex(row, 0), createIndex(row, columnCount() - 1));
            emit dataChanged(unsavedChanges);
        }
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
    emit dataChanged(unsavedChanges);
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
    
    // Remove path traversal sequences
    sanitized.remove(QRegExp("[.]{2,}"));  // Remove .. sequences
    sanitized.remove(QRegExp("[\\/]"));   // Remove path separators
    sanitized.remove(QRegExp("[<>:\"|?*]"));  // Remove invalid filename chars
    
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