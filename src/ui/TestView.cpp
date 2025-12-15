#include "TestView.h"
#include "../db/DatabaseManager.h"
#include "../core/TtsEngine.h"
#include "ThemeManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QSpinBox>
#include <QMessageBox>
#include <QRandomGenerator>
#include <algorithm>
#include <random>
#include <chrono>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QGraphicsOpacityEffect>
#include <QKeyEvent>
#include <QApplication>
#ifdef Q_OS_WIN
#include <windows.h>
#endif

TestView::TestView(QWidget *parent) : QWidget(parent), m_currentIndex(0), m_correctCount(0), m_currentMode(Spelling) {
    m_model = new WordModel(this);
    setupUi();
    
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this](ThemeManager::Theme theme){

    });
}

void TestView::setupUi() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 40, 40, 40);
    mainLayout->setSpacing(20);

    QHBoxLayout *topLayout = new QHBoxLayout();
    
    QComboBox *comboMode = new QComboBox(this);
    comboMode->addItem(tr("拼写测试 (看义写词)"), Spelling);
    comboMode->addItem(tr("回忆测试 (看词选义)"), Recalling);
    connect(comboMode, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TestView::onModeChanged);
    
    m_lblProgress = new QLabel(tr("进度: 0/0"), this);
    
    m_spinCount = new QSpinBox(this);
    m_spinCount->setRange(5, 50);
    m_spinCount->setValue(10);
    m_spinCount->setPrefix(tr("题目数: "));
    
    m_comboBook = new QComboBox(this);
    m_comboBook->addItem(tr("全部单词"), -1);
    refreshBooks();

    topLayout->addWidget(new QLabel(tr("测试模式:"), this));
    topLayout->addWidget(m_comboBook);
    topLayout->addWidget(comboMode);
    topLayout->addWidget(m_spinCount);
    topLayout->addStretch();
    topLayout->addWidget(m_lblProgress);
    
    mainLayout->addLayout(topLayout);

    QWidget *questionContainer = new QWidget(this);
    questionContainer->setObjectName("questionContainer");
    questionContainer->setStyleSheet("QWidget#questionContainer { background-color: palette(base); border: 1px solid palette(mid); border-radius: 16px; }");
    
    QVBoxLayout *qLayout = new QVBoxLayout(questionContainer);
    qLayout->setContentsMargins(40, 60, 40, 60);
    
    m_lblQuestionType = new QLabel(tr("请拼写出以下单词:"), this);
    m_lblQuestionType->setAlignment(Qt::AlignCenter);
    m_lblQuestionType->setStyleSheet("color: palette(midlight); font-size: 16px;");
    
    m_lblQuestion = new QLabel("", this);
    m_lblQuestion->setWordWrap(true);
    m_lblQuestion->setAlignment(Qt::AlignCenter);
    m_lblQuestion->setStyleSheet("font-size: 32px; font-weight: bold; color: palette(text); margin: 20px;");

    qLayout->addStretch();
    qLayout->addWidget(m_lblQuestionType);
    qLayout->addWidget(m_lblQuestion);
    qLayout->addStretch();
    
    mainLayout->addWidget(questionContainer);

    m_answerStack = new QStackedWidget(this);


    m_spellingContainer = new QWidget(this);
    QHBoxLayout *inputLayout = new QHBoxLayout(m_spellingContainer);
    m_inputAnswer = new QLineEdit(this);
    m_inputAnswer->setPlaceholderText(tr("在此输入答案..."));
    m_inputAnswer->setMinimumHeight(50);
    m_inputAnswer->setFont(QFont("Arial", 16));
    connect(m_inputAnswer, &QLineEdit::returnPressed, this, &TestView::onCheckAnswer);

    m_btnSubmit = new QPushButton(tr("提交"), this);
    m_btnSubmit->setMinimumHeight(50);
    m_btnSubmit->setCursor(Qt::PointingHandCursor);
    m_btnSubmit->setStyleSheet("background-color: #2563EB; color: white; font-weight: bold; border-radius: 8px; padding: 0 30px;");
    connect(m_btnSubmit, &QPushButton::clicked, this, &TestView::onCheckAnswer);

    inputLayout->addWidget(m_inputAnswer);
    inputLayout->addWidget(m_btnSubmit);
    

    m_choiceContainer = new QWidget(this);
    QVBoxLayout *choiceLayout = new QVBoxLayout(m_choiceContainer);
    choiceLayout->setSpacing(10);
    m_choiceGroup = new QButtonGroup(this);
    
    for (int i = 0; i < 4; ++i) {
        QPushButton *btn = new QPushButton(this);
        btn->setMinimumHeight(60);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setStyleSheet("text-align: left; padding-left: 20px; font-size: 16px; border-radius: 8px; border: 1px solid palette(mid);");
        m_choiceGroup->addButton(btn, i);
        m_choiceButtons.append(btn);
        choiceLayout->addWidget(btn);
        connect(btn, &QPushButton::clicked, this, [this, i](){ onChoiceClicked(i); });
    }

    m_btnNextChoice = new QPushButton(tr("下一题"), this);
    m_btnNextChoice->setMinimumHeight(50);
    m_btnNextChoice->setCursor(Qt::PointingHandCursor);
    m_btnNextChoice->setStyleSheet("background-color: #2563EB; color: white; font-weight: bold; border-radius: 8px; margin-top: 10px;");
    m_btnNextChoice->hide();
    connect(m_btnNextChoice, &QPushButton::clicked, this, &TestView::onNextQuestion);
    choiceLayout->addWidget(m_btnNextChoice);

    m_answerStack->addWidget(m_spellingContainer);
    m_answerStack->addWidget(m_choiceContainer);
    
    mainLayout->addWidget(m_answerStack);

    m_lblResult = new QLabel("", this);
    m_lblResult->setAlignment(Qt::AlignCenter);
    m_lblResult->setStyleSheet("font-size: 18px; font-weight: bold; margin-top: 10px;");
    mainLayout->addWidget(m_lblResult);
    
    mainLayout->addStretch();
}

void TestView::startTest() {
    int bookId = m_comboBook->currentData().toInt();
    m_testQueue = DatabaseManager::instance().getAllWords(bookId);
    
    if (m_testQueue.isEmpty()) {
        QMessageBox::warning(this, tr("开始测试"), tr("当前词书没有单词，无法开始测试。"));
        return;
    }
    
    auto rng = std::default_random_engine(std::chrono::system_clock::now().time_since_epoch().count());
    std::shuffle(m_testQueue.begin(), m_testQueue.end(), rng);
    
    int count = m_spinCount->value();
    if (m_testQueue.size() > count) {
        m_testQueue = m_testQueue.mid(0, count);
    }

    m_currentIndex = 0;
    m_correctCount = 0;
    showQuestion();
}

void TestView::showQuestion() {
    if (m_currentIndex >= m_testQueue.size()) {
        m_lblQuestion->setText(tr("测试结束！"));
        m_lblQuestionType->setText("");
        m_answerStack->hide();
        m_lblResult->setText(tr("最终得分: %1 / %2").arg(m_correctCount).arg(m_testQueue.size()));
        return;
    }

    m_answerStack->show();
    m_lblResult->clear();

    const Word &word = m_testQueue[m_currentIndex];
    m_lblProgress->setText(tr("进度: %1 / %2").arg(m_currentIndex + 1).arg(m_testQueue.size()));

    if (m_currentMode == Spelling) {
        m_answerStack->setCurrentWidget(m_spellingContainer);
        m_inputAnswer->clear();
        m_inputAnswer->setFocus();
        m_btnSubmit->setText(tr("提交"));
        m_btnSubmit->disconnect();
        connect(m_btnSubmit, &QPushButton::clicked, this, &TestView::onCheckAnswer);
        
        m_lblQuestionType->setText(tr("请拼写出以下单词:"));
        m_lblQuestion->setText(word.definition);
    } else {
        m_answerStack->setCurrentWidget(m_choiceContainer);
        m_btnNextChoice->hide();
        m_lblQuestionType->setText(tr("请选择正确的中文含义:"));
        m_lblQuestion->setText(word.spelling);
        TtsEngine::instance().speak(word.spelling);
        

        this->setFocus();

        QList<Word> distractors;
        generateDistractors(word, distractors);
        
        QList<Word> options = distractors;
        options.append(word);
        
        auto rng = std::default_random_engine(std::chrono::system_clock::now().time_since_epoch().count());
        std::shuffle(options.begin(), options.end(), rng);
        
        for (int i = 0; i < 4; ++i) {
            m_choiceButtons[i]->setText(options[i].definition);
            m_choiceButtons[i]->setEnabled(true);
            m_choiceButtons[i]->setStyleSheet("text-align: left; padding-left: 20px; font-size: 16px; border-radius: 8px; border: 1px solid palette(mid);");
            if (options[i].id == word.id) {
                m_correctOptionIndex = i;
            }
        }
    }
}

void TestView::generateDistractors(const Word& correctWord, QList<Word>& distractors) {
    QList<Word> allWords = DatabaseManager::instance().getAllWords(m_comboBook->currentData().toInt());
    
    for (int i = 0; i < allWords.size(); ++i) {
        if (allWords[i].id == correctWord.id) {
            allWords.removeAt(i);
            break;
        }
    }

    if (allWords.size() < 3) {
        distractors = allWords;
        while(distractors.size() < 3) {
            Word w;
            w.id = -1;
            w.spelling = "N/A";
            w.definition = "选项不足";
            distractors.append(w);
        }
    } else {
        auto rng = std::default_random_engine(std::chrono::system_clock::now().time_since_epoch().count());
        std::shuffle(allWords.begin(), allWords.end(), rng);
        distractors = allWords.mid(0, 3);
    }
}

void TestView::onChoiceClicked(int index) {
    bool isCorrect = (index == m_correctOptionIndex);
    
    for (int i = 0; i < 4; ++i) {
        m_choiceButtons[i]->setEnabled(false);
        if (i == m_correctOptionIndex) {
            m_choiceButtons[i]->setStyleSheet("text-align: left; padding-left: 20px; font-size: 16px; border-radius: 8px; background-color: #10B981; color: white; border: none;");
        } else if (i == index && !isCorrect) {
            m_choiceButtons[i]->setStyleSheet("text-align: left; padding-left: 20px; font-size: 16px; border-radius: 8px; background-color: #EF4444; color: white; border: none;");
            

            QPropertyAnimation *anim = new QPropertyAnimation(m_choiceButtons[i], "pos");
            anim->setDuration(300);
            anim->setLoopCount(2);
            QPoint pos = m_choiceButtons[i]->pos();
            anim->setKeyValueAt(0, pos);
            anim->setKeyValueAt(0.25, pos + QPoint(5, 0));
            anim->setKeyValueAt(0.75, pos + QPoint(-5, 0));
            anim->setKeyValueAt(1, pos);
            anim->start(QAbstractAnimation::DeleteWhenStopped);
        }
    }

    if (isCorrect) {
        m_lblResult->setText(tr("回答正确！"));
        m_lblResult->setStyleSheet("color: #10B981; font-size: 18px; font-weight: bold;");
        m_correctCount++;
        TtsEngine::instance().speak("Correct");
#ifdef Q_OS_WIN
        MessageBeep(MB_OK);
#else
#ifdef Q_OS_WIN
        MessageBeep(MB_OK);
#else
        QApplication::beep();
#endif
#endif
    } else {
        m_lblResult->setText(tr("回答错误。"));
        m_lblResult->setStyleSheet("color: #EF4444; font-size: 18px; font-weight: bold;");
        TtsEngine::instance().speak("Wrong");
#ifdef Q_OS_WIN
        MessageBeep(MB_OK);
#else
#ifdef Q_OS_WIN
        MessageBeep(MB_OK);
#else
        QApplication::beep();
#endif
#endif
    }

    m_btnNextChoice->show();
    m_btnNextChoice->setFocus();
}

void TestView::onCheckAnswer() {
    const Word &word = m_testQueue[m_currentIndex];
    QString userAnswer = m_inputAnswer->text().trimmed();
    bool correct = false;

    if (userAnswer.compare(word.spelling, Qt::CaseInsensitive) == 0) {
        correct = true;
    }

    if (correct) {
        m_lblResult->setText(tr("回答正确！"));
        m_lblResult->setStyleSheet("color: #10B981; font-size: 18px; font-weight: bold;");
        m_correctCount++;
        TtsEngine::instance().speak("Correct");
#ifdef Q_OS_WIN
        MessageBeep(MB_OK);
#else
#ifdef Q_OS_WIN
        MessageBeep(MB_OK);
#else
        QApplication::beep();
#endif
#endif
    } else {
        m_lblResult->setText(tr("回答错误。正确答案: %1").arg(word.spelling));
        m_lblResult->setStyleSheet("color: #EF4444; font-size: 18px; font-weight: bold;");
        TtsEngine::instance().speak("Wrong");
#ifdef Q_OS_WIN
        MessageBeep(MB_OK);
#else
#ifdef Q_OS_WIN
        MessageBeep(MB_OK);
#else
        QApplication::beep();
#endif
#endif
        

        QPropertyAnimation *anim = new QPropertyAnimation(m_inputAnswer, "pos");
        anim->setDuration(300);
        anim->setLoopCount(2);
        QPoint pos = m_inputAnswer->pos();
        anim->setKeyValueAt(0, pos);
        anim->setKeyValueAt(0.25, pos + QPoint(5, 0));
        anim->setKeyValueAt(0.75, pos + QPoint(-5, 0));
        anim->setKeyValueAt(1, pos);
        anim->start(QAbstractAnimation::DeleteWhenStopped);
    }

    m_btnSubmit->setText(tr("下一题"));
    m_btnSubmit->disconnect();
    connect(m_btnSubmit, &QPushButton::clicked, this, &TestView::onNextQuestion);
}

void TestView::onNextQuestion() {
    m_currentIndex++;
    showQuestion();
}

void TestView::onModeChanged(int index) {
    m_currentMode = static_cast<TestMode>(index);
    startTest();
}

void TestView::refreshBooks() {
    m_comboBook->blockSignals(true);
    m_comboBook->clear();
    m_comboBook->addItem(tr("全部单词"), -1);
    
    int uncategorizedCount = DatabaseManager::instance().getUncategorizedWordCount();
    if (uncategorizedCount > 0) {
        m_comboBook->addItem(tr("未分类单词 (%1词)").arg(uncategorizedCount), 0);
    }

    QList<Book> books = DatabaseManager::instance().getAllBooks();
    for (const Book& book : books) {
        m_comboBook->addItem(QString("%1 (%2词)").arg(book.name).arg(book.count), book.id);
    }
    m_comboBook->blockSignals(false);
}

void TestView::keyPressEvent(QKeyEvent *event) {
    if (m_currentMode == Recalling) {
        if (m_btnNextChoice->isVisible()) {
            if (event->key() == Qt::Key_Space || event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
                onNextQuestion();
            }
        } else {
            switch (event->key()) {
                case Qt::Key_1: if(m_choiceButtons[0]->isEnabled()) onChoiceClicked(0); break;
                case Qt::Key_2: if(m_choiceButtons[1]->isEnabled()) onChoiceClicked(1); break;
                case Qt::Key_3: if(m_choiceButtons[2]->isEnabled()) onChoiceClicked(2); break;
                case Qt::Key_4: if(m_choiceButtons[3]->isEnabled()) onChoiceClicked(3); break;
                default: QWidget::keyPressEvent(event);
            }
        }
    } else {

        QWidget::keyPressEvent(event);
    }
}
