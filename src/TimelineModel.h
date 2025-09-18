#pragma once
#include <QAbstractTableModel>
#include <QString>
#include <QVector>
#include <QFile>
#include <QTextStream>
#include <QSet>
#include <QStandardPaths>
#include <QDir>
#include <QRegExp>
#include <QMutex>
#include <QMutexLocker>
#include <memory>

/**
 * @brief TimelineModel is a QAbstractTableModel backed by a timeline CSV file.
 */
class TimelineModel : public QAbstractTableModel {
    Q_OBJECT
public:
    enum TimelineType {
        Filesystem,
        Super,
        Unknown
    };
    TimelineModel(const QString& filePath, QObject* parent = nullptr);
    ~TimelineModel();
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    int columnIndex(const QString& name) const;
    TimelineType type() const;
    bool isRowTagged(int row) const;
    void setRowTagged(int row, bool tagged);
    bool hasUnsavedChanges() const;
    bool saveTaggedRows();
    QString getFilePath() const;

signals:
    void dataChanged(bool hasUnsavedChanges);

private:
    // Security limits
    static constexpr qint64 MAX_FILE_SIZE = 2LL * 1024 * 1024 * 1024;  // 2GB limit
    static constexpr int MAX_LINE_COUNT = 10000000;  // 10 million lines
    static constexpr qint64 MAX_INDEX_MEMORY = 500LL * 1024 * 1024;  // 500MB for index
    QString filePath;
    TimelineType timelineType;
    QStringList headers;
    QVector<qint64> lineOffsets; // File offsets for each line
    mutable QFile file;
    mutable QMutex fileMutex; // Protect file operations
    QSet<int> taggedRows; // Set of tagged row indices
    bool unsavedChanges;
    void detectFormat();
    void buildLineIndex();
    void loadTaggedRows();
    QString getTagFilePath() const;
    QString sanitizeFileName(const QString& fileName) const;
    bool ensureTagDirectory() const;
}; 