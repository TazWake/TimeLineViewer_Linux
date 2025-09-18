#pragma once
#include <QWidget>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>

/**
 * @brief FilterBar provides a UI for selecting a column and entering a search term.
 */
class FilterBar : public QWidget {
    Q_OBJECT
public:
    explicit FilterBar(QWidget* parent = nullptr);
    void setColumns(const QStringList& columns);

signals:
    void searchRequested(const QString& column, const QString& term);
    // Future: void fontSizeChanged(int pointSize);
    // Future: void lineHeightChanged(int px);

private slots:
    void onSearchClicked();

private:
    QComboBox* columnPicker;
    QLineEdit* input;
    QPushButton* searchButton;
}; 