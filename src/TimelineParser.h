#pragma once
#include <QStringList>

/**
 * @brief TimelineParser provides static utilities for timeline format detection.
 */
class TimelineParser {
public:
    enum TimelineType {
        Filesystem,
        Super,
        Unknown
    };
    static TimelineType detectFormat(const QStringList& headerFields);
}; 