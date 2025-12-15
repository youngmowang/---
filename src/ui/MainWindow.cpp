#include "MainWindow.h"
#include "DashboardView.h"
#include "PreviewView.h"
#include "StudyView.h"
#include "SettingsDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QStackedWidget>
#include <QLabel>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QParallelAnimationGroup>
#include <QIcon>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setupUi();
}

MainWindow::~MainWindow() {
}

void MainWindow::setupUi() {
    setWindowTitle(tr("AutoWord - 英语单词记忆"));
    resize(1024, 768);
    setWindowIcon(QIcon("src/resources/logo.jpg"));

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    

    QWidget *navBar = new QWidget(this);
    navBar->setObjectName("navBar");
    navBar->setStyleSheet("QWidget#navBar { background-color: palette(base); border-bottom: 1px solid palette(mid); }");
    QHBoxLayout *navLayout = new QHBoxLayout(navBar);
    navLayout->setContentsMargins(20, 10, 20, 10);
    navLayout->setSpacing(15);

    QPushButton *btnDashboard = new QPushButton(tr("首页"), this);
    QPushButton *btnPreview = new QPushButton(tr("词库预览"), this);
    QPushButton *btnStudy = new QPushButton(tr("今日学习"), this);
    QPushButton *btnTest = new QPushButton(tr("小测验"), this);
    QPushButton *btnSettings = new QPushButton(tr("设置"), this);
    
    btnDashboard->setCursor(Qt::PointingHandCursor);
    btnPreview->setCursor(Qt::PointingHandCursor);
    btnStudy->setCursor(Qt::PointingHandCursor);
    btnTest->setCursor(Qt::PointingHandCursor);
    btnSettings->setCursor(Qt::PointingHandCursor);

    navLayout->addWidget(btnDashboard);
    navLayout->addWidget(btnPreview);
    navLayout->addWidget(btnStudy);
    navLayout->addWidget(btnTest);
    navLayout->addStretch();
    navLayout->addWidget(btnSettings);
    
    mainLayout->addWidget(navBar);

    m_stackedWidget = new QStackedWidget(this);
    
    m_dashboardView = new DashboardView(this);
    m_previewView = new PreviewView(this);
    m_studyView = new StudyView(this);
    m_testView = new TestView(this);
    
    m_stackedWidget->addWidget(m_dashboardView);
    m_stackedWidget->addWidget(m_previewView);
    m_stackedWidget->addWidget(m_studyView);
    m_stackedWidget->addWidget(m_testView);
    
    mainLayout->addWidget(m_stackedWidget);

    connect(btnDashboard, &QPushButton::clicked, this, &MainWindow::onNavDashboard);
    connect(btnPreview, &QPushButton::clicked, this, &MainWindow::onNavPreview);
    connect(btnStudy, &QPushButton::clicked, this, &MainWindow::onNavStudy);
    connect(btnTest, &QPushButton::clicked, this, [this](){
        switchPage(m_testView);
        m_testView->startTest();
    });
    connect(btnSettings, &QPushButton::clicked, this, [this](){
        SettingsDialog dlg(this);
        if (dlg.exec() == QDialog::Accepted) {
            m_studyView->refreshBooks();
        }
    });
}

void MainWindow::switchPage(QWidget *page) {
    if (m_stackedWidget->currentWidget() == page) return;


    QWidget *current = m_stackedWidget->currentWidget();
    QWidget *next = page;
    

    m_stackedWidget->setCurrentWidget(next);
    
    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(next);
    next->setGraphicsEffect(effect);
    
    QPropertyAnimation *anim = new QPropertyAnimation(effect, "opacity");
    anim->setDuration(300);
    anim->setStartValue(0.0);
    anim->setEndValue(1.0);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    
    connect(anim, &QPropertyAnimation::finished, [effect](){
        effect->deleteLater();
    });
    
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void MainWindow::onNavDashboard() {
    switchPage(m_dashboardView);
    m_dashboardView->refreshStats();
}

void MainWindow::onNavPreview() {
    switchPage(m_previewView);
}

void MainWindow::onNavStudy() {
    switchPage(m_studyView);
    m_studyView->startSession();
}
