#pragma once

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QStackedWidget>
#include <QComboBox>
#include <QButtonGroup>
#include "../core/WordModel.h"

class TestView : public QWidget {
    Q_OBJECT

public:
    explicit TestView(QWidget *parent = nullptr);
    void startTest();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void onCheckAnswer();
    void onNextQuestion();
    void onModeChanged(int index);
    void onChoiceClicked(int index);

private:
    void setupUi();
    void showQuestion();
    void updateTheme(int theme); 
    void refreshBooks();
    void generateDistractors(const Word& correctWord, QList<Word>& distractors);

    WordModel *m_model;
    QList<Word> m_testQueue;
    int m_currentIndex;
    int m_correctCount;
    int m_correctOptionIndex; 
    
    enum TestMode { Spelling, Recalling };
    TestMode m_currentMode;
    
    QComboBox *m_comboBook;
    QSpinBox *m_spinCount;
    QLabel *m_lblProgress;
    QLabel *m_lblQuestionType;
    QLabel *m_lblQuestion;
    

    QWidget *m_spellingContainer;
    QLineEdit *m_inputAnswer;
    QPushButton *m_btnSubmit;
    

    QWidget *m_choiceContainer;
    QButtonGroup *m_choiceGroup;
    QList<QPushButton*> m_choiceButtons;
    QPushButton *m_btnNextChoice; 

    QStackedWidget *m_answerStack;
    QLabel *m_lblResult;
};
