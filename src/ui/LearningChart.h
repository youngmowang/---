#pragma once
#include <QWidget>
#include <QPaintEvent>
#include <QMap>
#include <QString>

class LearningChart : public QWidget {
    Q_OBJECT
public:
    explicit LearningChart(QWidget *parent = nullptr);
    void setData(const QMap<QString, int>& data);
protected:
    void paintEvent(QPaintEvent *event) override;
private:
    QMap<QString, int> m_data;
};
