#pragma once
#include <QSqlDatabase>
#include <QString>

#include "../core/Word.h"
#include "../core/Book.h"
#include "../core/FsrsScheduler.h"
#include <QList>
#include <QMap>

class DatabaseManager {
public:
    static DatabaseManager& instance();
    bool connect(const QString& path);
    bool initTables();
    QSqlDatabase database() const;

    int createBook(const QString& name);
    QList<Book> getAllBooks() const;
    int getUncategorizedWordCount() const;
    bool deleteBook(int bookId);


    int getTotalWordCount() const;
    int getLearnedWordCount() const;
    int getDueWordCount() const;
    QMap<QString, int> getReviewHistory();

    bool addWord(const Word& word);
    bool deleteWord(int wordId);
    bool setFavorite(int wordId, bool favorite);
    
    QList<Word> getAllWords(int bookId = -1) const; 
    QList<Word> getDueWords(int bookId = -1, int limit = 20) const;

    FsrsCard getCard(int wordId);
    bool updateCard(const FsrsCard& card);

private:
    DatabaseManager();
    ~DatabaseManager();
    QSqlDatabase m_db;
};
