#include "SettingsDialog.h"
#include "ThemeManager.h"
#include "../network/WebDavClient.h"
#include "../core/DictionaryParser.h"
#include "../db/DatabaseManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QFileDialog>
#include <QSettings>
#include <QMessageBox>

SettingsDialog::SettingsDialog(QWidget *parent) : QDialog(parent) {
    setupUi();
    loadSettings();
}

void SettingsDialog::setupUi() {
    setWindowTitle(tr("设置"));
    resize(400, 500);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QGroupBox *grpTheme = new QGroupBox(tr("外观"), this);
    QVBoxLayout *themeLayout = new QVBoxLayout(grpTheme);
    m_comboTheme = new QComboBox(this);
    m_comboTheme->addItem(tr("跟随系统"), QVariant::fromValue(ThemeManager::Theme::Auto));
    m_comboTheme->addItem(tr("浅色模式"), QVariant::fromValue(ThemeManager::Theme::Light));
    m_comboTheme->addItem(tr("深色模式"), QVariant::fromValue(ThemeManager::Theme::Dark));
    connect(m_comboTheme, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsDialog::onThemeChanged);
    themeLayout->addWidget(new QLabel(tr("主题模式:"), this));
    themeLayout->addWidget(m_comboTheme);
    mainLayout->addWidget(grpTheme);
    
    QGroupBox *grpDict = new QGroupBox(tr("词库管理"), this);
    QVBoxLayout *dictLayout = new QVBoxLayout(grpDict);
    m_btnImport = new QPushButton(tr("导入词库 (CSV)"), this);
    connect(m_btnImport, &QPushButton::clicked, this, &SettingsDialog::onImportDictionary);
    dictLayout->addWidget(m_btnImport);
    mainLayout->addWidget(grpDict);

    QGroupBox *grpSync = new QGroupBox(tr("WebDAV 同步"), this);
    QVBoxLayout *syncLayout = new QVBoxLayout(grpSync);
    
    m_editWebDavUrl = new QLineEdit(this);
    m_editWebDavUser = new QLineEdit(this);
    m_editWebDavPass = new QLineEdit(this);
    m_editWebDavPass->setEchoMode(QLineEdit::Password);
    
    syncLayout->addWidget(new QLabel(tr("服务器地址:"), this));
    syncLayout->addWidget(m_editWebDavUrl);
    syncLayout->addWidget(new QLabel(tr("用户名:"), this));
    syncLayout->addWidget(m_editWebDavUser);
    syncLayout->addWidget(new QLabel(tr("密码:"), this));
    syncLayout->addWidget(m_editWebDavPass);
    
    m_btnSync = new QPushButton(tr("立即同步"), this);
    connect(m_btnSync, &QPushButton::clicked, this, &SettingsDialog::onSyncNow);
    syncLayout->addWidget(m_btnSync);
    
    mainLayout->addWidget(grpSync);

    QPushButton *btnAbout = new QPushButton(tr("关于作者"), this);
    connect(btnAbout, &QPushButton::clicked, this, [this](){
        QMessageBox::information(this, tr("关于作者"), tr("安徽理工大学 计算机24-10 高玮博"));
    });
    mainLayout->addWidget(btnAbout);

    mainLayout->addStretch();

    QHBoxLayout *btnLayout = new QHBoxLayout();
    m_btnSave = new QPushButton(tr("保存"), this);
    connect(m_btnSave, &QPushButton::clicked, this, &SettingsDialog::onSave);
    QPushButton *btnClose = new QPushButton(tr("关闭"), this);
    connect(btnClose, &QPushButton::clicked, this, &QDialog::accept);
    
    btnLayout->addStretch();
    btnLayout->addWidget(m_btnSave);
    btnLayout->addWidget(btnClose);
    mainLayout->addLayout(btnLayout);
}

void SettingsDialog::loadSettings() {
    QSettings settings("AutoWord", "Config");
    m_editWebDavUrl->setText(settings.value("WebDav/Url").toString());
    m_editWebDavUser->setText(settings.value("WebDav/User").toString());
    m_editWebDavPass->setText(settings.value("WebDav/Pass").toString());
    
    ThemeManager::Theme theme = (ThemeManager::Theme)settings.value("Theme", (int)ThemeManager::Theme::Auto).toInt();
    int index = m_comboTheme->findData(QVariant::fromValue(theme));
    if (index >= 0) m_comboTheme->setCurrentIndex(index);
}

void SettingsDialog::saveSettings() {
    QSettings settings("AutoWord", "Config");
    settings.setValue("WebDav/Url", m_editWebDavUrl->text());
    settings.setValue("WebDav/User", m_editWebDavUser->text());
    settings.setValue("WebDav/Pass", m_editWebDavPass->text());
    settings.setValue("Theme", m_comboTheme->currentData().toInt());
    QMessageBox::information(this, tr("保存"), tr("设置已保存"));
}

void SettingsDialog::onSave() {
    saveSettings();
    accept();
}

void SettingsDialog::onImportDictionary() {
    QString fileName = QFileDialog::getOpenFileName(this, tr("导入词库"), "", tr("Dictionary Files (*.csv *.txt *.json);;CSV Files (*.csv);;Text Files (*.txt);;JSON Files (*.json)"));
    if (fileName.isEmpty()) return;

    QFileInfo fileInfo(fileName);
    QString bookName = fileInfo.completeBaseName();
    int bookId = DatabaseManager::instance().createBook(bookName);

    QList<Word> words = DictionaryParser::parseFile(fileName);
    int count = 0;
    
    for (Word& word : words) {
        word.bookId = bookId; 
        if (DatabaseManager::instance().addWord(word)) {
            count++;
        }
    }

    QMessageBox::information(this, tr("导入完成"), tr("成功导入 %1 个单词到词书《%2》").arg(count).arg(bookName));
    accept();
}

void SettingsDialog::onThemeChanged(int index) {
    ThemeManager::Theme theme = m_comboTheme->itemData(index).value<ThemeManager::Theme>();
    ThemeManager::instance().setTheme(theme);
}

void SettingsDialog::onSyncNow() {
    WebDavClient *client = new WebDavClient(this);
    client->setCredentials(m_editWebDavUrl->text(), m_editWebDavUser->text(), m_editWebDavPass->text());
    
    QString dbPath = DatabaseManager::instance().database().databaseName();
    client->uploadDatabase(dbPath, "autoword.db");
    
    connect(client, &WebDavClient::uploadFinished, this, [this](bool success, QString msg){
        if (success) {
            QMessageBox::information(this, tr("同步"), tr("同步成功"));
        } else {
            QMessageBox::warning(this, tr("同步失败"), msg);
        }
    });
}
