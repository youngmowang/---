#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>

class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);

private slots:
    void onImportDictionary();
    void onSyncNow();
    void onThemeChanged(int index);
    void onSave();

private:
    void setupUi();
    void loadSettings();
    void saveSettings();

    QLineEdit *m_editWebDavUrl;
    QLineEdit *m_editWebDavUser;
    QLineEdit *m_editWebDavPass;
    QComboBox *m_comboTheme;
    QPushButton *m_btnImport;
    QPushButton *m_btnSync;
    QPushButton *m_btnSave;
};
