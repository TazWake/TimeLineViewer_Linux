#pragma once
#include <QWidget>
#include <QTableView>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QSortFilterProxyModel>
#include "FilterBar.h"
#include "TimelineModel.h"
#include "FieldDetailWindow.h"

/**
 * @brief TimelineTab represents a single tab with a loaded timeline file.
 */
class TimelineTab : public QWidget {
    Q_OBJECT
public:
    TimelineTab(const QString& filePath, QWidget* parent = nullptr);
    ~TimelineTab();
    void setFontSize(int pointSize);
    void setLineHeight(int px);
    QStringList columnNames() const;
    bool search(const QString& column, const QString& term);
    bool hasUnsavedChanges() const;
    bool saveChanges();
    TimelineModel* getModel() const;
    QString getFilePath() const;

private slots:
    void onSearchRequested(const QString& column, const QString& term);
    void onTableDoubleClicked(const QModelIndex& index);

private:
    FilterBar* filterBar;
    QTableView* tableView;
    QStatusBar* statusBar;
    TimelineModel* model;
    QSortFilterProxyModel* proxyModel;
    int fontSize = 10;
    int lineHeight = 20;
    void updateStatus(const QString& msg = QString());
    void updateFilterBarColumns();
}; 