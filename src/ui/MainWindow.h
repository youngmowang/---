#pragma once

#include <QMainWindow>
#include <QStackedWidget>
#include "DashboardView.h"
#include "PreviewView.h"
#include "StudyView.h"
#include "TestView.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onNavDashboard();
    void onNavPreview();
    void onNavStudy();

private:
    void setupUi();
    void switchPage(QWidget *page);
    
    QStackedWidget *m_stackedWidget;
    DashboardView *m_dashboardView;
    PreviewView *m_previewView;
    StudyView *m_studyView;
    TestView *m_testView;
};
