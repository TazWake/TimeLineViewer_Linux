#include "FieldDetailWindow.h"
#include <QFont>
#include <QFontMetrics>

FieldDetailWindow::FieldDetailWindow(const QString& fieldName, const QString& content, QWidget* parent)
    : QDialog(parent), layout(nullptr), titleLabel(nullptr), contentEdit(nullptr), buttonBox(nullptr)
{
    setWindowTitle(QString("Field Details: %1").arg(fieldName));
    setModal(false); // Allow multiple windows to be open
    resize(600, 400); // Default size
    
    setupUI();
    
    // Set the content
    contentEdit->setPlainText(content);
    
    // Make the text read-only but selectable
    contentEdit->setReadOnly(true);
    
    // Use monospace font for better formatting of XML/JSON
    QFont monoFont("Consolas, Monaco, monospace");
    monoFont.setPointSize(10);
    contentEdit->setFont(monoFont);
    
    // Enable word wrap for long lines
    contentEdit->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
}

FieldDetailWindow::~FieldDetailWindow()
{
    // Qt handles cleanup automatically
}

void FieldDetailWindow::setupUI()
{
    layout = new QVBoxLayout(this);
    
    // Title label showing field name
    titleLabel = new QLabel(this);
    QFont titleFont = titleLabel->font();
    titleFont.setBold(true);
    titleFont.setPointSize(titleFont.pointSize() + 1);
    titleLabel->setFont(titleFont);
    layout->addWidget(titleLabel);
    
    // Text edit for content
    contentEdit = new QTextEdit(this);
    layout->addWidget(contentEdit);
    
    // Button box with Close button
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttonBox);
    
    setLayout(layout);
}