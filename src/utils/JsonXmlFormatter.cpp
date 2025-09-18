#include "JsonXmlFormatter.h"
#include <QJsonDocument>
#include <QJsonParseError>
#include <QDomDocument>
#include <QDebug>

bool JsonXmlFormatter::isValidForParsing(const QString& text)
{
    // Check size limit
    if (text.size() > MAX_PARSE_SIZE) {
        return false;
    }
    
    // Basic check for XML bomb patterns
    if (text.contains("<!ENTITY", Qt::CaseInsensitive) || 
        text.contains("<!DOCTYPE", Qt::CaseInsensitive)) {
        return false;
    }
    
    // Check for excessive nesting in XML-like content
    int nestingLevel = 0;
    int maxNesting = 0;
    for (const QChar& c : text) {
        if (c == '<') {
            nestingLevel++;
            maxNesting = qMax(maxNesting, nestingLevel);
            if (maxNesting > MAX_XML_DEPTH) {
                return false;
            }
        } else if (c == '>') {
            nestingLevel = qMax(0, nestingLevel - 1);
        }
    }
    
    return true;
}

QString JsonXmlFormatter::formatIfApplicable(const QString& rawText)
{
    // Security validation first
    if (!isValidForParsing(rawText)) {
        qDebug() << "Content too large or potentially malicious for parsing, returning as-is";
        return rawText;
    }
    
    // Try JSON
    QJsonParseError jsonError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(rawText.toUtf8(), &jsonError);
    if (jsonError.error == QJsonParseError::NoError && !jsonDoc.isNull()) {
        QString formatted = jsonDoc.toJson(QJsonDocument::Indented);
        // Additional size check on output
        if (formatted.size() > MAX_PARSE_SIZE * 2) {
            qDebug() << "Formatted JSON too large, returning original";
            return rawText;
        }
        return formatted;
    }
    
    // Try XML (with entity resolution disabled for security)
    QDomDocument xmlDoc;
    QString xmlErrMsg;
    int xmlErrLine, xmlErrCol;
    
    // Configure XML parser to be more secure
    xmlDoc.setContent(rawText, false, &xmlErrMsg, &xmlErrLine, &xmlErrCol);  // Disable namespace processing
    
    if (xmlErrMsg.isEmpty()) {
        QString formatted = xmlDoc.toString(2);
        // Additional size check on output
        if (formatted.size() > MAX_PARSE_SIZE * 2) {
            qDebug() << "Formatted XML too large, returning original";
            return rawText;
        }
        return formatted;
    }
    
    // Not JSON or XML, or parsing failed
    return rawText;
} 