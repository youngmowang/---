#pragma once

#include <QObject>
#include <QApplication>
#include <QStyleHints>

class ThemeManager : public QObject {
    Q_OBJECT

public:
    enum class Theme {
        Light,
        Dark,
        Auto
    };

    static ThemeManager& instance();
    void setTheme(Theme theme);
    Theme currentTheme() const;

signals:
    void themeChanged(Theme theme);

private:
    ThemeManager();
    void applyTheme(Theme theme);
    void updateSystemTheme();

    Theme m_currentTheme;
    Theme m_manualTheme; 
};
