#pragma once
#include <QDialog>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>

/**
 * @brief FieldDetailWindow displays the full content of a timeline field in a resizable dialog.
 */
class FieldDetailWindow : public QDialog {
    Q_OBJECT
public:
    explicit FieldDetailWindow(const QString& fieldName, const QString& content, QWidget* parent = nullptr);
    ~FieldDetailWindow();

private:
    QVBoxLayout* layout;
    QLabel* titleLabel;
    QTextEdit* contentEdit;
    QDialogButtonBox* buttonBox;
    
    void setupUI();
};