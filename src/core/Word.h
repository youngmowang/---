#pragma once
#include <QString>
#include <QStringList>
#include <QDateTime>

struct Word {
    int id = -1;
    QString spelling;
    QString phonetic;
    QString definition;
    QString example;
    QStringList tags;
    int bookId = 0;
    bool isFavorite = false;
    QDateTime createdAt;

    bool isValid() const {
        return !spelling.isEmpty() && !definition.isEmpty();
    }
};
