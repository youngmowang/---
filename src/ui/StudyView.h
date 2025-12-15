#pragma once
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QComboBox>
#include "../core/FsrsScheduler.h"
#include "../core/Word.h"
#include "../core/Book.h"
#include "ThemeManager.h"

class StudyView : public QWidget {
    Q_OBJECT

public:
    explicit StudyView(QWidget *parent = nullptr);
    void startSession();
    void updateTheme(ThemeManager::Theme theme);
    void refreshBooks();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void onShowAnswer();
    void onRateAgain();
    void onRateHard();
    void onRateGood();
    void onRateEasy();

private:
    void setupUi();
    void showNextCard();
    void showQuestion();
    void showAnswer();
    void processRating(int rating);

    QLabel *m_lblWord;
    QLabel *m_lblPhonetic;
    QLabel *m_lblDefinition;
    QLabel *m_lblExample;
    QPushButton *m_btnShowAnswer;
    QWidget *m_ratingWidget;
    QWidget *m_answerContainer;
    QComboBox *m_comboBook;
    
    QList<Word> m_sessionQueue;
    int m_currentIndex;
    FsrsScheduler m_scheduler;
};
