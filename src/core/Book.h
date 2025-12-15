#pragma once
#include <QString>
#include <QDateTime>

struct Book {
    int id = -1;
    QString name;
    int count = 0;
    QDateTime createdAt;
};
