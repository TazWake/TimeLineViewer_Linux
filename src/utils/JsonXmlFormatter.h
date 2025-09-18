#pragma once
#include <QString>

/**
 * @brief JsonXmlFormatter provides pretty-printing for embedded JSON or XML.
 */
class JsonXmlFormatter {
public:
    // Security limits for parsing
    static constexpr int MAX_PARSE_SIZE = 1024 * 1024;  // 1MB limit
    static constexpr int MAX_XML_DEPTH = 100;            // XML nesting limit
    
    static QString formatIfApplicable(const QString& rawText);
    static bool isValidForParsing(const QString& text);
}; 