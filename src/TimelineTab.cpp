#include "TimelineTab.h"
#include <QHeaderView>
#include <QFont>
#include <QMenu>

TimelineTab::TimelineTab(const QString& filePath, QWidget* parent)
    : QWidget(parent)
{
    filterBar = new FilterBar(this);
    model = new TimelineModel(filePath, this);
    tableView = new QTableView(this);
    tableView->setModel(model);
    tableView->setSortingEnabled(false); // full sort requires reading all rows; disabled for large files
    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    tableView->horizontalHeader()->setSectionsMovable(true);
    tableView->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(tableView->horizontalHeader(), &QHeaderView::customContextMenuRequested,
            this, &TimelineTab::onHeaderContextMenu);
    statusBar = new QStatusBar(this);
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(filterBar);
    layout->addWidget(tableView);
    layout->addWidget(statusBar);
    setLayout(layout);
    connect(filterBar, &FilterBar::searchRequested, this, &TimelineTab::onSearchRequested);
    connect(tableView, &QTableView::doubleClicked, this, &TimelineTab::onTableDoubleClicked);
    connect(model, &TimelineModel::searchProgress, this, [this](int done, int total) {
        statusBar->showMessage(QString("Searching… %1 / %2 rows scanned").arg(done).arg(total));
    });
    updateFilterBarColumns();
    updateStatus();
}

TimelineTab::~TimelineTab() {}

void TimelineTab::updateFilterBarColumns()
{
    QStringList cols = model->headerData(0, Qt::Horizontal, Qt::DisplayRole).toStringList();
    if (cols.isEmpty())
        cols = model->headerData(0, Qt::Horizontal, Qt::DisplayRole).toString().split(",");
    if (cols.isEmpty())
        cols = model->columnCount() > 0 ? QStringList() : QStringList();
    for (int i = 0; i < model->columnCount(); ++i)
        cols << model->headerData(i, Qt::Horizontal, Qt::DisplayRole).toString();
    cols.removeDuplicates();
    cols.removeAll("");
    cols.prepend("All Columns");
    filterBar->setColumns(cols);
}

QStringList TimelineTab::columnNames() const
{
    QStringList cols;
    for (int i = 0; i < model->columnCount(); ++i)
        cols << model->headerData(i, Qt::Horizontal, Qt::DisplayRole).toString();
    return cols;
}

void TimelineTab::onSearchRequested(const QString& column, const QString& term)
{
    bool found = search(column, term);
    if (term.isEmpty()) {
        updateStatus();
    } else if (!found) {
        updateStatus("No matches found.");
    } else {
        updateStatus(QString("Matches: %1").arg(model->filteredRowCount()));
    }
}

bool TimelineTab::search(const QString& column, const QString& term)
{
    if (term.isEmpty()) {
        model->clearFilter();
        return false;
    }
    statusBar->showMessage("Searching…");
    model->applyFilter(column, term);
    return model->filteredRowCount() > 0;
}

void TimelineTab::setFontSize(int pointSize)
{
    fontSize = pointSize;
    QFont f = tableView->font();
    f.setPointSize(pointSize);
    tableView->setFont(f);
}

void TimelineTab::setLineHeight(int px)
{
    lineHeight = px;
    tableView->verticalHeader()->setDefaultSectionSize(px);
}

void TimelineTab::onTableDoubleClicked(const QModelIndex& index)
{
    if (!index.isValid()) return;
    
    // Get the column name for the title
    QString columnName = model->headerData(index.column(), Qt::Horizontal, Qt::DisplayRole).toString();
    
    // Get the cell content (this will include XML/JSON formatting if applicable)
    QString content = index.data(Qt::DisplayRole).toString();
    
    // Create and show the detail window
    FieldDetailWindow* detailWindow = new FieldDetailWindow(columnName, content, this);
    detailWindow->setAttribute(Qt::WA_DeleteOnClose); // Auto-delete when closed
    detailWindow->show();
}

void TimelineTab::onHeaderContextMenu(const QPoint& pos)
{
    QMenu menu(this);
    menu.setTitle("Show / hide columns");

    for (int col = 0; col < model->columnCount(); ++col) {
        QString name = model->headerData(col, Qt::Horizontal, Qt::DisplayRole).toString();
        QAction* action = menu.addAction(name);
        action->setCheckable(true);
        action->setChecked(!tableView->isColumnHidden(col));
        // Capture col by value; QMenu::exec() is synchronous so the lambda
        // lifetime is safe.
        connect(action, &QAction::toggled, this, [this, col](bool visible) {
            tableView->setColumnHidden(col, !visible);
        });
    }

    menu.exec(tableView->horizontalHeader()->mapToGlobal(pos));
}

void TimelineTab::updateStatus(const QString& msg)
{
    if (!msg.isEmpty()) {
        statusBar->showMessage(msg);
    } else {
        statusBar->showMessage(QString("Rows: %1").arg(model->rowCount()));
    }
}

bool TimelineTab::hasUnsavedChanges() const
{
    return model->hasUnsavedChanges();
}

bool TimelineTab::saveChanges()
{
    return model->saveTaggedRows();
}

TimelineModel* TimelineTab::getModel() const
{
    return model;
}

QString TimelineTab::getFilePath() const
{
    return model->getFilePath();
} 