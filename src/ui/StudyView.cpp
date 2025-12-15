#include "StudyView.h"
#include "../db/DatabaseManager.h"
#include "../core/TtsEngine.h"
#include <QMessageBox>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QKeyEvent>
#include <random>
#include <chrono>
#include <algorithm>

StudyView::StudyView(QWidget *parent) : QWidget(parent), m_currentIndex(0) {
    setupUi();
}

void StudyView::setupUi() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(40, 40, 40, 40);


    QHBoxLayout *topLayout = new QHBoxLayout();
    m_comboBook = new QComboBox(this);
    m_comboBook->setMinimumWidth(200);
    refreshBooks();
    
    QPushButton *btnStart = new QPushButton(tr("开始/重置"), this);
    connect(btnStart, &QPushButton::clicked, this, &StudyView::startSession);
    
    topLayout->addWidget(new QLabel(tr("当前词书:"), this));
    topLayout->addWidget(m_comboBook);
    topLayout->addWidget(btnStart);
    topLayout->addStretch();
    
    mainLayout->addLayout(topLayout);


    QWidget *cardContainer = new QWidget(this);
    cardContainer->setObjectName("cardContainer");
    cardContainer->setStyleSheet("QWidget#cardContainer { background-color: palette(base); border: 1px solid palette(mid); border-radius: 16px; }");
    QVBoxLayout *cardLayout = new QVBoxLayout(cardContainer);
    cardLayout->setContentsMargins(40, 60, 40, 60);
    cardLayout->setSpacing(30);

    m_lblWord = new QLabel(this);
    m_lblWord->setAlignment(Qt::AlignCenter);
    m_lblWord->setStyleSheet("font-size: 48px; font-weight: bold; color: palette(text);");
    
    m_lblPhonetic = new QLabel(this);
    m_lblPhonetic->setAlignment(Qt::AlignCenter);
    m_lblPhonetic->setStyleSheet("font-size: 20px; color: palette(midlight); font-family: 'Segoe UI', sans-serif;");

    m_answerContainer = new QWidget(this);
    QVBoxLayout *answerLayout = new QVBoxLayout(m_answerContainer);
    answerLayout->setContentsMargins(0, 0, 0, 0);
    answerLayout->setSpacing(15);
    
    m_lblDefinition = new QLabel(this);
    m_lblDefinition->setAlignment(Qt::AlignCenter);
    m_lblDefinition->setWordWrap(true);
    m_lblDefinition->setStyleSheet("font-size: 24px; color: palette(text); margin-top: 20px;");
    
    m_lblExample = new QLabel(this);
    m_lblExample->setAlignment(Qt::AlignCenter);
    m_lblExample->setWordWrap(true);
    m_lblExample->setStyleSheet("font-size: 18px; color: palette(link); font-style: italic;");

    answerLayout->addWidget(m_lblDefinition);
    answerLayout->addWidget(m_lblExample);

    cardLayout->addWidget(m_lblWord);
    cardLayout->addWidget(m_lblPhonetic);
    cardLayout->addWidget(m_answerContainer);
    cardLayout->addStretch();

    mainLayout->addWidget(cardContainer);


    m_btnShowAnswer = new QPushButton(tr("显示答案"), this);
    m_btnShowAnswer->setMinimumHeight(50);
    m_btnShowAnswer->setCursor(Qt::PointingHandCursor);
    m_btnShowAnswer->setStyleSheet("font-size: 18px; font-weight: bold; background-color: #3B82F6; color: white; border: none; border-radius: 8px;");
    connect(m_btnShowAnswer, &QPushButton::clicked, this, &StudyView::onShowAnswer);
    mainLayout->addWidget(m_btnShowAnswer);

    m_ratingWidget = new QWidget(this);
    QHBoxLayout *ratingLayout = new QHBoxLayout(m_ratingWidget);
    ratingLayout->setSpacing(15);
    ratingLayout->setContentsMargins(0, 0, 0, 0);

    struct RateBtn { QString text; QString color; void (StudyView::*slot)(); };
    RateBtn btns[] = {
        {tr("重来 (Again)"), "#EF4444", &StudyView::onRateAgain},
        {tr("困难 (Hard)"), "#F59E0B", &StudyView::onRateHard},
        {tr("良好 (Good)"), "#10B981", &StudyView::onRateGood},
        {tr("简单 (Easy)"), "#3B82F6", &StudyView::onRateEasy}
    };

    for (const auto& b : btns) {
        QPushButton *btn = new QPushButton(b.text, this);
        btn->setMinimumHeight(50);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setStyleSheet(QString("QPushButton { background-color: %1; color: white; font-weight: bold; border: none; border-radius: 8px; } QPushButton:hover { background-color: %1; opacity: 0.9; }").arg(b.color));
        connect(btn, &QPushButton::clicked, this, b.slot);
        ratingLayout->addWidget(btn);
    }
    
    mainLayout->addWidget(m_ratingWidget);
}

void StudyView::refreshBooks() {
    m_comboBook->clear();
    int uncategorizedCount = DatabaseManager::instance().getUncategorizedWordCount();
    if (uncategorizedCount > 0) {
        m_comboBook->addItem(tr("未分类单词 (%1词)").arg(uncategorizedCount), 0);
    }
    QList<Book> books = DatabaseManager::instance().getAllBooks();
    for (const Book& book : books) {
        m_comboBook->addItem(QString("%1 (%2词)").arg(book.name).arg(book.count), book.id);
    }
}

void StudyView::startSession() {
    int bookId = m_comboBook->currentData().toInt();
    

    m_sessionQueue = DatabaseManager::instance().getDueWords(bookId, 20);
    

    if (m_sessionQueue.size() < 20) {
        QList<Word> allWords = DatabaseManager::instance().getAllWords(bookId);

        auto rng = std::default_random_engine(std::chrono::system_clock::now().time_since_epoch().count());
        std::shuffle(allWords.begin(), allWords.end(), rng);
        
        for (const Word& w : allWords) {
            if (m_sessionQueue.size() >= 20) break;
            

            bool exists = false;
            for (const Word& existing : m_sessionQueue) {
                if (existing.id == w.id) { exists = true; break; }
            }
            if (!exists) m_sessionQueue.append(w);
        }
    } 
    m_currentIndex = 0;
    
    if (m_sessionQueue.isEmpty()) {
        QMessageBox::information(this, tr("提示"), tr("当前词书没有单词。"));
        return;
    }
    
    showNextCard();
}

void StudyView::showNextCard() {
    if (m_currentIndex >= m_sessionQueue.size()) {
        QMessageBox::information(this, tr("完成"), tr("今日学习任务已完成！"));
        return;
    }
    
    showQuestion();
}

void StudyView::showQuestion() {
    const Word &word = m_sessionQueue[m_currentIndex];
    m_lblWord->setText(word.spelling);
    m_lblPhonetic->setText(word.phonetic);
    m_lblDefinition->setText(word.definition);
    m_lblExample->setText(word.example);
    
    m_answerContainer->hide();
    m_ratingWidget->hide();
    m_btnShowAnswer->show();
    
    TtsEngine::instance().speak(word.spelling);
    

    this->setFocus();
}

void StudyView::showAnswer() {
    m_btnShowAnswer->hide();
    m_answerContainer->show();
    m_ratingWidget->show();
    

    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(m_answerContainer);
    m_answerContainer->setGraphicsEffect(effect);
    
    QPropertyAnimation *animOpacity = new QPropertyAnimation(effect, "opacity");
    animOpacity->setDuration(400);
    animOpacity->setStartValue(0.0);
    animOpacity->setEndValue(1.0);
    animOpacity->setEasingCurve(QEasingCurve::OutQuad);
    
    QPropertyAnimation *animPos = new QPropertyAnimation(m_answerContainer, "pos");

    
    animOpacity->start(QAbstractAnimation::DeleteWhenStopped);
}

void StudyView::onShowAnswer() {
    showAnswer();
}

void StudyView::processRating(int rating) {
    if (m_currentIndex >= m_sessionQueue.size()) return;
    
    const Word &word = m_sessionQueue[m_currentIndex];
    

    FsrsCard card = DatabaseManager::instance().getCard(word.id);
    

    FsrsRating::Rating fsrsRating = static_cast<FsrsRating::Rating>(rating);
    FsrsCard nextCard = m_scheduler.schedule(card, fsrsRating);
    

    DatabaseManager::instance().updateCard(nextCard);
    

    m_currentIndex++;
    showNextCard();
}

void StudyView::onRateAgain() { processRating(1); }
void StudyView::onRateHard() { processRating(2); }
void StudyView::onRateGood() { processRating(3); }
void StudyView::onRateEasy() { processRating(4); }

void StudyView::updateTheme(ThemeManager::Theme theme) {

}

void StudyView::keyPressEvent(QKeyEvent *event) {
    if (m_btnShowAnswer->isVisible()) {
        if (event->key() == Qt::Key_Space) {
            onShowAnswer();
        }
    } else if (m_ratingWidget->isVisible()) {
        switch (event->key()) {
            case Qt::Key_1: onRateAgain(); break;
            case Qt::Key_2: onRateHard(); break;
            case Qt::Key_3: onRateGood(); break;
            case Qt::Key_4: onRateEasy(); break;
            default: QWidget::keyPressEvent(event);
        }
    } else {
        QWidget::keyPressEvent(event);
    }
}
