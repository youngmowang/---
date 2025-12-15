#include "DictionaryParser.h"
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDateTime>
#include <QDebug>
#include <QRegularExpression>

QList<Word> DictionaryParser::parseFile(const QString& filePath) {
    QFileInfo info(filePath);
    QString suffix = info.suffix().toLower();

    if (suffix == "json") {
        return parseJson(filePath);
    } else if (suffix == "txt") {
        return parseTxt(filePath);
    } else {
        return parseCsv(filePath);
    }
}

QList<Word> DictionaryParser::parseCsv(const QString& filePath) {
    QList<Word> words;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return words;
    }

    QTextStream in(&file);
    
    QByteArray data = file.readAll();
    file.close();
    
    QString content;
    if (data.startsWith("\xEF\xBB\xBF")) {
        content = QString::fromUtf8(data);
    } else {
        QString utf8Str = QString::fromUtf8(data);
        if (utf8Str.contains(QChar(0xFFFD))) {
             content = QString::fromLocal8Bit(data);
        } else {
             content = utf8Str;
        }
    }
    
    QTextStream stream(&content);
    while (!stream.atEnd()) {
        QString line = stream.readLine();
        QStringList parts = line.split(",");
        
        if (parts.size() >= 1) {
            Word w;
            w.spelling = parts[0].trimmed();
            
            if (parts.size() == 2) {
                QString secondPart = parts[1].trimmed();
                static QRegularExpression reChinese("[\\u4e00-\\u9fa5]");
                if (secondPart.contains(reChinese)) {
                    w.definition = secondPart;
                } else {
                    w.phonetic = secondPart;
                }
            } else {
                if (parts.size() > 1) w.phonetic = parts[1].trimmed();
                if (parts.size() > 2) w.definition = parts[2].trimmed();
                if (parts.size() > 3) w.example = parts[3].trimmed();
                if (parts.size() > 4) w.tags = parts[4].trimmed().split(";", Qt::SkipEmptyParts);
            }
            w.createdAt = QDateTime::currentDateTime();
            w.isFavorite = false;
            if (!w.spelling.isEmpty()) words.append(w);
        }
    }
    return words;
}

QList<Word> DictionaryParser::parseJson(const QString& filePath) {
    QList<Word> words;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return words;
    }

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    
    if (doc.isNull()) {
        QString content = QString::fromLocal8Bit(data);
        doc = QJsonDocument::fromJson(content.toUtf8());
    }

    if (doc.isNull()) {
        return words;
    }
    
    QJsonArray array;
    if (doc.isArray()) {
        array = doc.array();
    } else if (doc.isObject()) {
        QJsonObject obj = doc.object();
        if (obj.contains("words") && obj["words"].isArray()) {
            array = obj["words"].toArray();
        } else if (obj.contains("data") && obj["data"].isArray()) {
            array = obj["data"].toArray();
        }
    }

    for (const QJsonValue &val : array) {
        if (val.isObject()) {
            QJsonObject obj = val.toObject();
            Word w;
            if (obj.contains("spelling")) w.spelling = obj["spelling"].toString();
            else if (obj.contains("word")) w.spelling = obj["word"].toString();
            else if (obj.contains("headWord")) w.spelling = obj["headWord"].toString();

            if (obj.contains("phonetic")) w.phonetic = obj["phonetic"].toString();
            else if (obj.contains("phone")) w.phonetic = obj["phone"].toString();
            else if (obj.contains("usphone")) w.phonetic = obj["usphone"].toString(); 

            if (obj.contains("definition")) w.definition = obj["definition"].toString();
            else if (obj.contains("trans")) w.definition = obj["trans"].toString();
            else if (obj.contains("mean")) w.definition = obj["mean"].toString();

            if (obj.contains("example")) w.example = obj["example"].toString();
            else if (obj.contains("sentences")) {
                if (obj["sentences"].isArray()) {
                    QJsonArray sents = obj["sentences"].toArray();
                    if (!sents.isEmpty()) {
                        QJsonObject s = sents[0].toObject();
                        w.example = s["sContent"].toString() + "\n" + s["sCn"].toString();
                    }
                }
            }

            w.spelling = w.spelling.trimmed();
            w.createdAt = QDateTime::currentDateTime();
            w.isFavorite = false;
            
            if (!w.spelling.isEmpty()) {
                words.append(w);
            }
        }
    }
    return words;
}

QList<Word> DictionaryParser::parseTxt(const QString& filePath) {
    QList<Word> words;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return words;
    }

    QTextStream in(&file);
    
    QByteArray data = file.readAll();
    file.close();
    
    QString content;
    
    if (data.startsWith("\xEF\xBB\xBF")) {
        content = QString::fromUtf8(data);
    } else {
        QString utf8Str = QString::fromUtf8(data);
        if (utf8Str.contains(QChar(0xFFFD))) {
             content = QString::fromLocal8Bit(data);
        } else {
             content = utf8Str;
        }
    }
    
    QTextStream stream(&content);
    while (!stream.atEnd()) {
        QString line = stream.readLine().trimmed();
        if (line.isEmpty()) continue;

        Word w;
        if (line.contains("|")) {
            QStringList parts = line.split("|");
            w.spelling = parts[0].trimmed();
            if (parts.size() > 1) w.definition = parts[1].trimmed();
        } else if (line.contains("\t")) {
            QStringList parts = line.split("\t");
            w.spelling = parts[0].trimmed();
            if (parts.size() > 1) w.definition = parts[1].trimmed();
        } else {
            w.spelling = line;
        }
        
        w.createdAt = QDateTime::currentDateTime();
        w.isFavorite = false;
        
        if (!w.spelling.isEmpty()) {
            words.append(w);
        }
    }
    return words;
}
