#pragma once
#include "Word.h"
#include <QList>
#include <QString>

class DictionaryParser {
public:
    static QList<Word> parseFile(const QString& filePath);

private:
    static QList<Word> parseCsv(const QString& filePath);
    static QList<Word> parseJson(const QString& filePath);
    static QList<Word> parseTxt(const QString& filePath);
};
