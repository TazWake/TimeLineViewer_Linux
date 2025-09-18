#include "FilterBar.h"

FilterBar::FilterBar(QWidget* parent)
    : QWidget(parent)
{
    columnPicker = new QComboBox(this);
    columnPicker->setToolTip("Select a column to search, or choose 'All Columns' to search the entire table.");
    input = new QLineEdit(this);
    searchButton = new QPushButton("Search", this);
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->addWidget(columnPicker);
    layout->addWidget(input);
    layout->addWidget(searchButton);
    setLayout(layout);
    connect(searchButton, &QPushButton::clicked, this, &FilterBar::onSearchClicked);
}

void FilterBar::setColumns(const QStringList& columns)
{
    columnPicker->clear();
    columnPicker->addItems(columns);
}

void FilterBar::onSearchClicked()
{
    emit searchRequested(columnPicker->currentText(), input->text());
} 