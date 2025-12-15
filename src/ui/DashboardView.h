#pragma once

#include <QWidget>
#include <QLabel>
#include <QGridLayout>
#include <QMap>

#include "LearningChart.h"

class DashboardView : public QWidget {
    Q_OBJECT

public:
    explicit DashboardView(QWidget *parent = nullptr);
    void refreshStats();

protected:
    void showEvent(QShowEvent *event) override;

private:
    QWidget* createStatCard(const QString& title, const QString& value, const QString& color, QLabel*& outLabel);
    
    QLabel *m_lblTotalWords;
    QLabel *m_lblLearnedWords;
    QLabel *m_lblDueWords;
    LearningChart *m_chartWidget;
};
