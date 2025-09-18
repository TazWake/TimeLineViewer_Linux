#include "AppWindow.h"
#include "TimelineTab.h"
#include <QMessageBox>
#include <QDebug>
#include <QFileInfo>

AppWindow::AppWindow(QWidget* parent)
    : QMainWindow(parent)
{
    tabs = new QTabWidget(this);
    setCentralWidget(tabs);
    setupMenu();
    setWindowTitle("Linux Timeline Viewer");
    resize(1200, 800);
    connect(tabs, &QTabWidget::currentChanged, this, &AppWindow::onTabChanged);
}

AppWindow::~AppWindow() {}

void AppWindow::setupMenu()
{
    QMenuBar* menuBar = this->menuBar();
    QMenu* fileMenu = menuBar->addMenu("&File");
    openAction = new QAction("&Open", this);
    saveAction = new QAction("&Save", this);
    saveAction->setShortcut(QKeySequence::Save);
    saveAction->setEnabled(false);
    exitAction = new QAction("E&xit", this);
    fileMenu->addAction(openAction);
    fileMenu->addAction(saveAction);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAction);
    connect(openAction, &QAction::triggered, this, &AppWindow::openFile);
    connect(saveAction, &QAction::triggered, this, &AppWindow::saveFile);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);

    QMenu* viewMenu = menuBar->addMenu("&View");
    fontIncAction = new QAction("Increase Font Size", this);
    fontDecAction = new QAction("Decrease Font Size", this);
    resetFontAction = new QAction("Reset Font", this);
    viewMenu->addAction(fontIncAction);
    viewMenu->addAction(fontDecAction);
    viewMenu->addSeparator();
    viewMenu->addAction(resetFontAction);
    connect(fontIncAction, &QAction::triggered, this, &AppWindow::increaseFontSize);
    connect(fontDecAction, &QAction::triggered, this, &AppWindow::decreaseFontSize);
    connect(resetFontAction, &QAction::triggered, this, &AppWindow::resetFontAndLineHeight);

    QMenu* searchMenu = menuBar->addMenu("&Search");
    searchCurrentTabAction = new QAction("Search in Current Tab...", this);
    searchAllTabsAction = new QAction("Search in All Tabs...", this);
    clearSearchAction = new QAction("Clear Search", this);
    searchMenu->addAction(searchCurrentTabAction);
    searchMenu->addAction(searchAllTabsAction);
    searchMenu->addSeparator();
    searchMenu->addAction(clearSearchAction);
    connect(searchCurrentTabAction, &QAction::triggered, this, &AppWindow::searchInCurrentTab);
    connect(searchAllTabsAction, &QAction::triggered, this, &AppWindow::searchInAllTabs);
    connect(clearSearchAction, &QAction::triggered, this, &AppWindow::clearSearch);
}

void AppWindow::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open Timeline File", QString(), "Timeline Files (*.csv *.txt)");
    if (fileName.isEmpty())
        return;
    
    // Validate file before attempting to open
    QFileInfo fileInfo(fileName);
    if (!fileInfo.exists()) {
        QMessageBox::warning(this, "File Error", "The selected file does not exist.");
        return;
    }
    
    if (!fileInfo.isReadable()) {
        QMessageBox::warning(this, "File Error", "The selected file is not readable. Check file permissions.");
        return;
    }
    
    if (fileInfo.size() == 0) {
        QMessageBox::warning(this, "File Error", "The selected file is empty.");
        return;
    }
    
    // Additional security checks
    if (fileInfo.size() > 2LL * 1024 * 1024 * 1024) { // 2GB limit
        QMessageBox::warning(this, "File Error", "The selected file is too large (over 2GB limit).");
        return;
    }
    
    try {
        TimelineTab* tab = new TimelineTab(fileName, this);
        tab->setFontSize(currentFontSize);
        tabs->addTab(tab, fileInfo.fileName());
        tabs->setCurrentWidget(tab);
        updateWindowTitle();
        
        // Connect to model's dataChanged signal to update save action and window title
        connect(tab->getModel(), &TimelineModel::dataChanged, this, [this](bool hasUnsaved) {
            updateWindowTitle();
            saveAction->setEnabled(hasUnsaved);
        });
        
        statusBar()->showMessage("File loaded successfully", 2000);
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Error Loading File", 
            QString("Failed to load the timeline file: %1").arg(e.what()));
    } catch (...) {
        QMessageBox::critical(this, "Error Loading File", 
            "An unexpected error occurred while loading the file.");
    }
}

void AppWindow::saveFile()
{
    TimelineTab* tab = qobject_cast<TimelineTab*>(tabs->currentWidget());
    if (!tab) return;
    
    if (tab->saveChanges()) {
        statusBar()->showMessage("Tags saved successfully.", 2000);
        updateWindowTitle();
        saveAction->setEnabled(false);
    } else {
        QMessageBox::warning(this, "Save Error", "Failed to save tags. Please check file permissions.");
    }
}

void AppWindow::increaseFontSize() { currentFontSize = qMin(currentFontSize + 1, 32); applyFontAndLineHeight(); }
void AppWindow::decreaseFontSize() { currentFontSize = qMax(currentFontSize - 1, 6); applyFontAndLineHeight(); }
void AppWindow::resetFontAndLineHeight() { currentFontSize = 10; applyFontAndLineHeight(); }
void AppWindow::applyFontAndLineHeight() {
    for (int i = 0; i < tabs->count(); ++i) {
        TimelineTab* tab = qobject_cast<TimelineTab*>(tabs->widget(i));
        if (tab) {
            tab->setFontSize(currentFontSize);
        }
    }
}

void AppWindow::searchInCurrentTab() { showSearchDialog(false); }
void AppWindow::searchInAllTabs() { showSearchDialog(true); }

void AppWindow::clearSearch() {
    int cleared = 0;
    for (int i = 0; i < tabs->count(); ++i) {
        TimelineTab* tab = qobject_cast<TimelineTab*>(tabs->widget(i));
        if (tab) {
            tab->search("All Columns", "");
            ++cleared;
        }
    }
    statusBar()->showMessage(QString("Cleared search in %1 tab(s)." ).arg(cleared));
}

void AppWindow::showSearchDialog(bool allTabs)
{
    // Gather columns
    QStringList allColumns;
    if (allTabs) {
        QSet<QString> colSet;
        for (int i = 0; i < tabs->count(); ++i) {
            TimelineTab* tab = qobject_cast<TimelineTab*>(tabs->widget(i));
            if (!tab) {
                qWarning() << "Null TimelineTab at index" << i;
                continue;
            }
            QStringList tabCols = tab->columnNames();
            for (const QString& col : tabCols) {
                if (!col.isNull() && !col.trimmed().isEmpty())
                    colSet.insert(col.trimmed());
                else
                    qWarning() << "Skipping empty or null column in tab" << i;
            }
        }
        allColumns = QStringList(colSet.begin(), colSet.end());
    } else {
        TimelineTab* tab = qobject_cast<TimelineTab*>(tabs->currentWidget());
        if (tab) {
            QStringList tabCols = tab->columnNames();
            for (const QString& col : tabCols) {
                if (!col.isNull() && !col.trimmed().isEmpty())
                    allColumns << col.trimmed();
                else
                    qWarning() << "Skipping empty or null column in current tab";
            }
        }
    }
    allColumns.removeAll("");
    allColumns.removeDuplicates();
    allColumns.sort();
    if (allColumns.isEmpty()) {
        statusBar()->showMessage("No columns available for search.");
        return;
    }
    allColumns.prepend("All Columns");

    // Dialog
    QDialog dialog(this);
    dialog.setWindowTitle(allTabs ? "Search in All Tabs" : "Search in Current Tab");
    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    QLabel* label = new QLabel("Search term:", &dialog);
    QLineEdit* input = new QLineEdit(&dialog);
    QLabel* colLabel = new QLabel("Column:", &dialog);
    QComboBox* colPicker = new QComboBox(&dialog);
    colPicker->addItems(allColumns);
    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    layout->addWidget(label);
    layout->addWidget(input);
    layout->addWidget(colLabel);
    layout->addWidget(colPicker);
    layout->addWidget(buttons);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    if (dialog.exec() != QDialog::Accepted) return;
    QString term = input->text();
    QString col = colPicker->currentText();
    if (term.isEmpty()) return;

    int totalMatches = 0;
    int firstTabWithMatch = -1;
    int firstRow = -1;
    for (int i = 0; i < tabs->count(); ++i) {
        TimelineTab* tab = qobject_cast<TimelineTab*>(tabs->widget(i));
        if (!tab) {
            qWarning() << "Null TimelineTab at index" << i;
            continue;
        }
        bool found = false;
        try {
            found = tab->search(col, term);
        } catch (...) {
            qWarning() << "Exception during search in tab" << i;
            continue;
        }
        if (found) {
            if (firstTabWithMatch == -1) {
                firstTabWithMatch = i;
                firstRow = 0; // Could be improved to select first match
            }
            totalMatches++;
        }
    }
    if (totalMatches > 0 && firstTabWithMatch != -1) {
        tabs->setCurrentIndex(firstTabWithMatch);
        statusBar()->showMessage(QString("%1 tab(s) matched for '%2'.").arg(totalMatches).arg(term));
    } else {
        statusBar()->showMessage("No matches found.");
    }
}

void AppWindow::onTabChanged(int index)
{
    updateWindowTitle();
    if (index >= 0) {
        TimelineTab* tab = qobject_cast<TimelineTab*>(tabs->widget(index));
        if (tab) {
            saveAction->setEnabled(tab->hasUnsavedChanges());
        }
    } else {
        saveAction->setEnabled(false);
    }
}

void AppWindow::closeEvent(QCloseEvent* event)
{
    if (checkUnsavedChanges()) {
        event->accept();
    } else {
        event->ignore();
    }
}

bool AppWindow::checkUnsavedChanges()
{
    QStringList unsavedTabs;
    for (int i = 0; i < tabs->count(); ++i) {
        TimelineTab* tab = qobject_cast<TimelineTab*>(tabs->widget(i));
        if (tab && tab->hasUnsavedChanges()) {
            unsavedTabs << tabs->tabText(i);
        }
    }
    
    if (unsavedTabs.isEmpty()) {
        return true; // No unsaved changes, OK to close
    }
    
    QString message = "You have unsaved changes in the following tabs:\n\n";
    message += unsavedTabs.join("\n");
    message += "\n\nDo you want to save your changes before exiting?";
    
    QMessageBox::StandardButton result = QMessageBox::question(
        this, "Unsaved Changes", message,
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
        QMessageBox::Save
    );
    
    if (result == QMessageBox::Save) {
        // Save all unsaved tabs
        bool allSaved = true;
        for (int i = 0; i < tabs->count(); ++i) {
            TimelineTab* tab = qobject_cast<TimelineTab*>(tabs->widget(i));
            if (tab && tab->hasUnsavedChanges()) {
                if (!tab->saveChanges()) {
                    allSaved = false;
                    QMessageBox::warning(this, "Save Error", 
                        QString("Failed to save tags for %1. Please check file permissions.")
                        .arg(tabs->tabText(i)));
                }
            }
        }
        return allSaved;
    } else if (result == QMessageBox::Discard) {
        return true; // Discard changes and close
    } else {
        return false; // Cancel close
    }
}

void AppWindow::updateWindowTitle()
{
    QString title = "Linux Timeline Viewer";
    
    TimelineTab* currentTab = qobject_cast<TimelineTab*>(tabs->currentWidget());
    if (currentTab) {
        QString fileName = QFileInfo(currentTab->getFilePath()).fileName();
        title += " - " + fileName;
        if (currentTab->hasUnsavedChanges()) {
            title += " *";
        }
    }
    
    setWindowTitle(title);
} 