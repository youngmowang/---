#include "ThemeManager.h"
#include <QPalette>
#include <QStyle>
#include <QStyleFactory>
#include <QDebug>
#include <QFont>
#include <QFontDatabase>

ThemeManager& ThemeManager::instance() {
    static ThemeManager instance;
    return instance;
}

ThemeManager::ThemeManager() : m_currentTheme(Theme::Light), m_manualTheme(Theme::Auto) {
    connect(QApplication::styleHints(), &QStyleHints::colorSchemeChanged, this, &ThemeManager::updateSystemTheme);
    
    QFont font;
    if (QFontDatabase::addApplicationFont(":/fonts/Inter-Regular.ttf") != -1) {
        font.setFamily("Inter");
    } else {
        font.setFamily("Segoe UI"); 
    }
    font.setPointSize(10);
    QApplication::setFont(font);

    updateSystemTheme(); 
}

void ThemeManager::setTheme(Theme theme) {
    m_manualTheme = theme;
    if (theme == Theme::Auto) {
        updateSystemTheme();
    } else {
        applyTheme(theme);
    }
}

ThemeManager::Theme ThemeManager::currentTheme() const {
    return m_currentTheme;
}

void ThemeManager::updateSystemTheme() {
    if (m_manualTheme != Theme::Auto) return;

    Qt::ColorScheme scheme = QApplication::styleHints()->colorScheme();
    if (scheme == Qt::ColorScheme::Dark) {
        applyTheme(Theme::Dark);
    } else {
        applyTheme(Theme::Light);
    }
}

void ThemeManager::applyTheme(Theme theme) {
    if (m_currentTheme == theme) return;

    m_currentTheme = theme;
    QApplication::setStyle(QStyleFactory::create("Fusion"));
    
    QPalette p;
    QString qss;

    if (theme == Theme::Dark) {
        QColor bg(17, 24, 39);
        QColor surface(31, 41, 55);
        QColor text(249, 250, 251);
        QColor primary(59, 130, 246);
        QColor border(55, 65, 81);

        p.setColor(QPalette::Window, bg);
        p.setColor(QPalette::WindowText, text);
        p.setColor(QPalette::Base, surface);
        p.setColor(QPalette::AlternateBase, bg);
        p.setColor(QPalette::ToolTipBase, surface);
        p.setColor(QPalette::ToolTipText, text);
        p.setColor(QPalette::Text, text);
        p.setColor(QPalette::Button, surface);
        p.setColor(QPalette::ButtonText, text);
        p.setColor(QPalette::BrightText, Qt::red);
        p.setColor(QPalette::Link, primary);
        p.setColor(QPalette::Highlight, primary);
        p.setColor(QPalette::HighlightedText, Qt::white);

        qss = R"(
            QMainWindow, QDialog { background-color: #111827; color: #F9FAFB; }
            QWidget { color: #F9FAFB; }
            
            QListView {
                background-color: #1F2937;
                border: 1px solid #374151;
                border-radius: 8px;
                color: #F9FAFB;
                padding: 5px;
                font-size: 16px;
                outline: none;
            }
            QListView::item {
                padding: 12px;
                border-bottom: 1px solid #374151;
            }
            QListView::item:selected {
                background-color: #374151;
                color: #60A5FA;
                border-radius: 6px;
            }
            
            QPushButton {
                background-color: #1F2937;
                border: 1px solid #374151;
                border-radius: 8px;
                padding: 10px 20px;
                color: #F9FAFB;
                font-weight: 600;
                font-size: 14px;
            }
            QPushButton:hover { background-color: #374151; border-color: #60A5FA; }
            QPushButton:pressed { background-color: #111827; }
            QPushButton:disabled { background-color: #111827; color: #6B7280; border-color: #374151; }
            
            QLineEdit, QComboBox, QSpinBox {
                background-color: #1F2937;
                border: 1px solid #374151;
                border-radius: 8px;
                padding: 8px 12px;
                color: #F9FAFB;
                font-size: 14px;
                selection-background-color: #3B82F6;
            }
            QLineEdit:focus, QComboBox:focus, QSpinBox:focus {
                border: 1px solid #60A5FA;
            }
            
            QGroupBox {
                border: 1px solid #374151;
                border-radius: 12px;
                margin-top: 12px;
                padding-top: 16px;
                font-weight: bold;
            }
            QGroupBox::title { subcontrol-origin: margin; left: 12px; padding: 0 5px; color: #9CA3AF; }
            
            QScrollBar:vertical {
                border: none;
                background: #111827;
                width: 10px;
                margin: 0px;
            }
            QScrollBar::handle:vertical {
                background: #4B5563;
                min-height: 20px;
                border-radius: 5px;
            }
            QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }
        )";
    } else {
        QColor bg(243, 244, 246);
        QColor surface(255, 255, 255);
        QColor text(31, 41, 55);
        QColor primary(37, 99, 235);
        QColor border(229, 231, 235);

        p.setColor(QPalette::Window, bg);
        p.setColor(QPalette::WindowText, text);
        p.setColor(QPalette::Base, surface);
        p.setColor(QPalette::AlternateBase, bg);
        p.setColor(QPalette::ToolTipBase, surface);
        p.setColor(QPalette::ToolTipText, text);
        p.setColor(QPalette::Text, text);
        p.setColor(QPalette::Button, surface);
        p.setColor(QPalette::ButtonText, text);
        p.setColor(QPalette::BrightText, Qt::red);
        p.setColor(QPalette::Link, primary);
        p.setColor(QPalette::Highlight, primary);
        p.setColor(QPalette::HighlightedText, Qt::white);

        qss = R"(
            QMainWindow, QDialog { background-color: #F3F4F6; color: #1F2937; }
            QWidget { color: #1F2937; }
            
            QListView {
                background-color: #FFFFFF;
                border: 1px solid #E5E7EB;
                border-radius: 8px;
                color: #1F2937;
                padding: 5px;
                font-size: 16px;
                outline: none;
            }
            QListView::item {
                padding: 12px;
                border-bottom: 1px solid #F3F4F6;
            }
            QListView::item:selected {
                background-color: #EFF6FF;
                color: #2563EB;
                border-radius: 6px;
            }
            
            QPushButton {
                background-color: #FFFFFF;
                border: 1px solid #E5E7EB;
                border-radius: 8px;
                padding: 10px 20px;
                color: #374151;
                font-weight: 600;
                font-size: 14px;
            }
            QPushButton:hover { background-color: #F9FAFB; border-color: #2563EB; }
            QPushButton:pressed { background-color: #F3F4F6; }
            QPushButton:disabled { background-color: #F3F4F6; color: #9CA3AF; border-color: #E5E7EB; }
            
            QLineEdit, QComboBox, QSpinBox {
                background-color: #FFFFFF;
                border: 1px solid #E5E7EB;
                border-radius: 8px;
                padding: 8px 12px;
                color: #1F2937;
                font-size: 14px;
                selection-background-color: #2563EB;
            }
            QLineEdit:focus, QComboBox:focus, QSpinBox:focus {
                border: 1px solid #2563EB;
            }
            
            QGroupBox {
                border: 1px solid #E5E7EB;
                border-radius: 12px;
                margin-top: 12px;
                padding-top: 16px;
                font-weight: bold;
            }
            QGroupBox::title { subcontrol-origin: margin; left: 12px; padding: 0 5px; color: #6B7280; }
            
            QScrollBar:vertical {
                border: none;
                background: #F3F4F6;
                width: 10px;
                margin: 0px;
            }
            QScrollBar::handle:vertical {
                background: #D1D5DB;
                min-height: 20px;
                border-radius: 5px;
            }
            QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }
        )";
    }

    QApplication::setPalette(p);
    qApp->setStyleSheet(qss);
    
    emit themeChanged(theme);
}
