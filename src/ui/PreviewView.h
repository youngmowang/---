#pragma once
#include <QWidget>
#include <QListView>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QLineEdit>
#include "../core/WordModel.h"

class PreviewView : public QWidget {
    Q_OBJECT

public:
    explicit PreviewView(QWidget *parent = nullptr);
    WordModel* model() const;

private slots:
    void onSortAZ();
    void onSortRandom();
    void onSearchTextChanged(const QString &text);

private:
    void setupUi();
    
    QListView *m_listView;
    WordModel *m_model;
    QLineEdit *m_searchBar;
    QPushButton *m_btnSortAZ;
    QPushButton *m_btnSortRandom;
    QComboBox *m_comboBook;
    QPushButton *m_btnDeleteBook;
    
    void refreshBooks();
    void onDeleteWord();
    void onDeleteBook();
};
