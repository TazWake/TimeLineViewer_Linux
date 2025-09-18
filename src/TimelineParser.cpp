#include "TimelineParser.h"

TimelineParser::TimelineType TimelineParser::detectFormat(const QStringList& headerFields)
{
    if (headerFields == QStringList({"Date","Size","Type","Mode","UID","GID","Meta","File Name"}))
        return Filesystem;
    if (headerFields == QStringList({"datetime","timestamp_desc","source","source_long","message","parser","display_name","tag"}))
        return Super;
    return Unknown;
} 