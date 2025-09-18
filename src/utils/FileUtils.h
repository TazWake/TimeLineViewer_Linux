#pragma once
#include <QString>
#include <QStringList>
#include <stdexcept>

/**
 * @brief FileUtils provides file-related helper functions.
 */
namespace FileUtils {
    // Security constants
    constexpr int MAX_FIELD_LENGTH = 65536;     // 64KB per field
    constexpr int MAX_FIELDS_PER_LINE = 256;    // Maximum fields per CSV line
    constexpr int MAX_LINE_LENGTH = 1048576;    // 1MB per line
    
    QString baseName(const QString& path);
    QStringList sniffCsvHeader(const QString& filePath);
    QStringList parseCsvLine(const QString& line);
    
    // Security validation functions
    void validateCsvLine(const QString& line);
    void validateFieldLength(const QString& field);
} 