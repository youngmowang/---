#include "PreviewView.h"
#include "../core/TtsEngine.h"
#include "../db/DatabaseManager.h"
#include <QMenu>
#include <QMessageBox>
#include <QAction>
#include <QLabel>

PreviewView::PreviewView(QWidget *parent) : QWidget(parent) {
    m_model = new WordModel(this);
    setupUi();
    refreshBooks();
    m_model->loadWords(); 
}

WordModel* PreviewView::model() const {
    return m_model;
}

void PreviewView::setupUi() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QHBoxLayout *toolbarLayout = new QHBoxLayout();
    
    m_comboBook = new QComboBox(this);
    m_comboBook->addItem(tr("全部单词"), -1);
    connect(m_comboBook, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index){
        int bookId = m_comboBook->currentData().toInt();
        m_model->loadWords(bookId);
        m_btnDeleteBook->setEnabled(bookId != -1);
    });
    
    m_btnDeleteBook = new QPushButton(tr("删除当前词书"), this);
    m_btnDeleteBook->setEnabled(false);
    connect(m_btnDeleteBook, &QPushButton::clicked, this, &PreviewView::onDeleteBook);

    m_btnSortAZ = new QPushButton(tr("A-Z 排序"), this);
    m_btnSortRandom = new QPushButton(tr("随机乱序"), this);
    
    m_searchBar = new QLineEdit(this);
    m_searchBar->setPlaceholderText(tr("搜索单词..."));
    connect(m_searchBar, &QLineEdit::textChanged, this, &PreviewView::onSearchTextChanged);

    toolbarLayout->addWidget(new QLabel(tr("词书:"), this));
    toolbarLayout->addWidget(m_comboBook);
    toolbarLayout->addWidget(m_searchBar);
    toolbarLayout->addWidget(m_btnDeleteBook);
    toolbarLayout->addWidget(m_btnDeleteBook);
    toolbarLayout->addWidget(m_btnSortAZ);
    toolbarLayout->addWidget(m_btnSortRandom);
    toolbarLayout->addStretch();

    mainLayout->addLayout(toolbarLayout);

    m_listView = new QListView(this);
    m_listView->setModel(m_model);
    m_listView->setAlternatingRowColors(true);
    m_listView->setContextMenuPolicy(Qt::CustomContextMenu);
    
    mainLayout->addWidget(m_listView);

    connect(m_btnSortAZ, &QPushButton::clicked, this, &PreviewView::onSortAZ);
    connect(m_btnSortRandom, &QPushButton::clicked, this, &PreviewView::onSortRandom);
    connect(m_listView, &QListView::clicked, this, [this](const QModelIndex &index) {
        QString spelling = index.data(WordModel::SpellingRole).toString();
        TtsEngine::instance().speak(spelling);
    });

    connect(m_listView, &QListView::doubleClicked, this, [this](const QModelIndex &index) {
        m_model->toggleFavorite(index.row());
    });
    
    connect(m_listView, &QListView::customContextMenuRequested, this, [this](const QPoint &pos){
        QModelIndex index = m_listView->indexAt(pos);
        if (index.isValid()) {
            QMenu menu(this);
            QAction *delAction = menu.addAction(tr("删除单词"));
            connect(delAction, &QAction::triggered, this, &PreviewView::onDeleteWord);
            menu.exec(m_listView->mapToGlobal(pos));
        }
    });
}

void PreviewView::refreshBooks() {
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

void PreviewView::onDeleteWord() {
    QModelIndex index = m_listView->currentIndex();
    if (!index.isValid()) return;
    
    QString spelling = index.data(WordModel::SpellingRole).toString();
    int wordId = index.data(WordModel::IdRole).toInt();
    
    if (QMessageBox::question(this, tr("确认删除"), tr("确定要删除单词 \"%1\" 吗？").arg(spelling)) == QMessageBox::Yes) {
        if (DatabaseManager::instance().deleteWord(wordId)) {
            int bookId = m_comboBook->currentData().toInt();
            m_model->loadWords(bookId);
            refreshBooks(); 
        }
    }
}

void PreviewView::onDeleteBook() {
    int bookId = m_comboBook->currentData().toInt();
    if (bookId == -1) return;
    
    QString bookName = m_comboBook->currentText();
    
    if (QMessageBox::warning(this, tr("确认删除"), 
        tr("确定要删除词书 \"%1\" 及其所有单词吗？\n此操作不可恢复！").arg(bookName),
        QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
            
        if (DatabaseManager::instance().deleteBook(bookId)) {
            refreshBooks();
            m_comboBook->setCurrentIndex(0); 
            m_model->loadWords(-1);
        }
    }
}

void PreviewView::onSortAZ() {
    m_model->sortWords(WordModel::Alphabetical);
}

void PreviewView::onSortRandom() {
    m_model->sortWords(WordModel::Random);
}

void PreviewView::onSearchTextChanged(const QString &text) {
    m_model->setFilter(text);
}
