#include "FileUtils.h"
#include <QFileInfo>
#include <QFile>
#include <QTextStream>
#include <QDebug>

namespace FileUtils {

QString baseName(const QString& path)
{
    return QFileInfo(path).baseName();
}

QStringList sniffCsvHeader(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return QStringList();
    QTextStream in(&file);
    QString headerLine = in.readLine();
    return parseCsvLine(headerLine);
}

void validateCsvLine(const QString& line)
{
    if (line.length() > MAX_LINE_LENGTH) {
        throw std::runtime_error("CSV line exceeds maximum length limit");
    }
}

void validateFieldLength(const QString& field)
{
    if (field.length() > MAX_FIELD_LENGTH) {
        throw std::runtime_error("CSV field exceeds maximum length limit");
    }
}

QStringList parseCsvLine(const QString& line)
{
    // Validate input line length
    validateCsvLine(line);
    
    QStringList fields;
    QString current;
    bool inQuotes = false;
    bool escapeNext = false;
    
    for (int i = 0; i < line.length(); ++i) {
        QChar c = line[i];
        
        if (escapeNext) {
            current += c;
            escapeNext = false;
        } else if (c == '\\') {
            escapeNext = true;
        } else if (c == '"') {
            inQuotes = !inQuotes;
        } else if (c == ',' && !inQuotes) {
            // Validate field length before adding
            validateFieldLength(current);
            fields.append(current);
            current.clear();
            
            // Check maximum number of fields
            if (fields.size() >= MAX_FIELDS_PER_LINE) {
                throw std::runtime_error("CSV line exceeds maximum field count limit");
            }
        } else {
            current += c;
            
            // Check field length during construction to fail early
            if (current.length() > MAX_FIELD_LENGTH) {
                throw std::runtime_error("CSV field exceeds maximum length limit");
            }
        }
    }
    
    // Validate the last field
    validateFieldLength(current);
    fields.append(current);
    
    return fields;
}

} // namespace FileUtils 