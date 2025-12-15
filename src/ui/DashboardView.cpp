#include "DashboardView.h"
#include "../db/DatabaseManager.h"
#include <QVBoxLayout>
#include <QFrame>
#include <QFrame>
#include <QDateTime>
#include <QPaintEvent>

DashboardView::DashboardView(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 40, 40, 40);
    mainLayout->setSpacing(30);


    QLabel *lblWelcome = new QLabel(tr("欢迎回来, 学习者!"), this);
    lblWelcome->setStyleSheet("font-size: 32px; font-weight: bold; color: palette(text);");
    mainLayout->addWidget(lblWelcome);

    QLabel *lblDate = new QLabel(QDateTime::currentDateTime().toString("yyyy年MM月dd日 dddd"), this);
    lblDate->setStyleSheet("font-size: 18px; color: palette(midlight); margin-bottom: 20px;");
    mainLayout->addWidget(lblDate);


    QGridLayout *gridLayout = new QGridLayout();
    gridLayout->setSpacing(20);


    QWidget *cardTotal = createStatCard(QStringLiteral("总单词数"), "0", "#3B82F6", m_lblTotalWords);
    gridLayout->addWidget(cardTotal, 0, 0);


    QWidget *cardLearned = createStatCard(QStringLiteral("已学单词"), "0", "#10B981", m_lblLearnedWords);
    gridLayout->addWidget(cardLearned, 0, 1);


    QWidget *cardDue = createStatCard(QStringLiteral("今日待复习"), "0", "#F59E0B", m_lblDueWords);
    gridLayout->addWidget(cardDue, 0, 2);

    mainLayout->addLayout(gridLayout);
    
    m_chartWidget = new LearningChart(this);
    mainLayout->addWidget(m_chartWidget);
    
    mainLayout->addStretch();
}

QWidget* DashboardView::createStatCard(const QString& title, const QString& value, const QString& color, QLabel*& outLabel) {
    QFrame *card = new QFrame(this);
    card->setObjectName("statCard");
    card->setStyleSheet(QString("QFrame#statCard { background-color: palette(base); border: 1px solid palette(mid); border-radius: 16px; border-left: 5px solid %1; }").arg(color));
    
    QVBoxLayout *layout = new QVBoxLayout(card);
    layout->setContentsMargins(20, 20, 20, 20);
    
    QLabel *lblTitle = new QLabel(title, card);
    lblTitle->setStyleSheet("font-size: 16px; color: palette(midlight);");
    
    QLabel *lblValue = new QLabel(value, card);
    lblValue->setStyleSheet(QString("font-size: 36px; font-weight: bold; color: %1;").arg(color));
    
    outLabel = lblValue;

    layout->addWidget(lblTitle);
    layout->addWidget(lblValue);
    
    return card;
}

void DashboardView::showEvent(QShowEvent *event) {
    QWidget::showEvent(event);
    refreshStats();
}

void DashboardView::refreshStats() {
    int total = DatabaseManager::instance().getTotalWordCount();
    int learned = DatabaseManager::instance().getLearnedWordCount();
    int due = DatabaseManager::instance().getDueWordCount();

    m_lblTotalWords->setText(QString::number(total));
    m_lblLearnedWords->setText(QString::number(learned));
    m_lblDueWords->setText(QString::number(due));

    m_chartWidget->setData(DatabaseManager::instance().getReviewHistory());
}


