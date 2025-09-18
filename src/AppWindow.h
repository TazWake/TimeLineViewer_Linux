#pragma once
#include <QMainWindow>
#include <QTabWidget>
#include <QMenuBar>
#include <QAction>
#include <QFileDialog>
#include <QInputDialog>
#include <QComboBox>
#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QDialogButtonBox>
#include <QCloseEvent>

class TimelineTab;

/**
 * @brief The AppWindow class is the main application window with tabbed interface and menu.
 */
class AppWindow : public QMainWindow {
    Q_OBJECT
public:
    AppWindow(QWidget* parent = nullptr);
    ~AppWindow();

private slots:
    void openFile();
    void saveFile();
    void increaseFontSize();
    void decreaseFontSize();
    void resetFontAndLineHeight();
    void searchInCurrentTab();
    void searchInAllTabs();
    void clearSearch();
    void onTabChanged(int index);

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    QTabWidget* tabs;
    QAction* openAction;
    QAction* saveAction;
    QAction* exitAction;
    QAction* fontIncAction;
    QAction* fontDecAction;
    QAction* resetFontAction;
    QAction* searchCurrentTabAction;
    QAction* searchAllTabsAction;
    QAction* clearSearchAction;
    void setupMenu();
    void applyFontAndLineHeight();
    int currentFontSize = 10;
    int currentLineHeight = 20;
    void showSearchDialog(bool allTabs);
    bool checkUnsavedChanges();
    void updateWindowTitle();
}; 