#include "DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QDebug>
#include <QStandardPaths>
#include <QDir>
#include <QDateTime>
#include <QTime>

DatabaseManager& DatabaseManager::instance() {
    static DatabaseManager instance;
    return instance;
}

DatabaseManager::DatabaseManager() {}

DatabaseManager::~DatabaseManager() {
    if (m_db.isOpen()) {
        m_db.close();
    }
}

bool DatabaseManager::connect(const QString& path) {
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(path);

    if (!m_db.open()) {
        qCritical() << "Error: connection with database failed" << m_db.lastError();
        return false;
    }
    qDebug() << "Database: connection ok";
    return initTables();
}

bool DatabaseManager::initTables() {
    QSqlQuery query;
    
    if (!query.exec("CREATE TABLE IF NOT EXISTS books ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "name TEXT NOT NULL UNIQUE, "
                    "created_at DATETIME DEFAULT CURRENT_TIMESTAMP"
                    ")")) {
        qCritical() << "Error creating books table:" << query.lastError();
        return false;
    }

    if (!query.exec("CREATE TABLE IF NOT EXISTS words ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "book_id INTEGER DEFAULT 0, "
                    "spelling TEXT NOT NULL, "
                    "phonetic TEXT, "
                    "definition TEXT, "
                    "example TEXT, "
                    "tags TEXT, "
                    "is_favorite INTEGER DEFAULT 0, "
                    "created_at DATETIME DEFAULT CURRENT_TIMESTAMP, "
                    "FOREIGN KEY(book_id) REFERENCES books(id)"
                    ")")) {
        qCritical() << "Error creating words table:" << query.lastError();
        return false;
    }
    
    if (!m_db.record("words").contains("book_id")) {
        query.exec("ALTER TABLE words ADD COLUMN book_id INTEGER DEFAULT 0");
    }

    if (!query.exec("CREATE TABLE IF NOT EXISTS cards ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "word_id INTEGER NOT NULL, "
                    "state INTEGER DEFAULT 0, " 
                    "due DATETIME, "
                    "stability REAL DEFAULT 0, "
                    "difficulty REAL DEFAULT 0, "
                    "elapsed_days INTEGER DEFAULT 0, "
                    "scheduled_days INTEGER DEFAULT 0, "
                    "reps INTEGER DEFAULT 0, "
                    "lapses INTEGER DEFAULT 0, "
                    "last_review DATETIME, "
                    "FOREIGN KEY(word_id) REFERENCES words(id)"
                    ")")) {
        qCritical() << "Error creating cards table:" << query.lastError();
        return false;
    }

    return true;
}

QSqlDatabase DatabaseManager::database() const {
    return m_db;
}

int DatabaseManager::createBook(const QString& name) {
    QSqlQuery query;
    query.prepare("INSERT INTO books (name) VALUES (:name)");
    query.bindValue(":name", name);
    if (query.exec()) {
        return query.lastInsertId().toInt();
    }
    query.prepare("SELECT id FROM books WHERE name = :name");
    query.bindValue(":name", name);
    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    return 0; 
}

QList<Book> DatabaseManager::getAllBooks() const {
    QList<Book> books;
    QSqlQuery query("SELECT * FROM books ORDER BY created_at DESC");
    while (query.next()) {
        Book b;
        b.id = query.value("id").toInt();
        b.name = query.value("name").toString();
        b.createdAt = query.value("created_at").toDateTime();
        
        QSqlQuery countQuery;
        countQuery.prepare("SELECT COUNT(*) FROM words WHERE book_id = :id");
        countQuery.bindValue(":id", b.id);
        if (countQuery.exec() && countQuery.next()) {
            b.count = countQuery.value(0).toInt();
        }
        
        books.append(b);
    }
    return books;
}

int DatabaseManager::getUncategorizedWordCount() const {
    QSqlQuery query("SELECT COUNT(*) FROM words WHERE book_id = 0 OR book_id IS NULL");
    if (query.next()) {
        return query.value(0).toInt();
    }
    return 0;
}

bool DatabaseManager::deleteBook(int bookId) {
    QSqlQuery query;
    query.prepare("DELETE FROM words WHERE book_id = :id");
    query.bindValue(":id", bookId);
    if (!query.exec()) return false;

    query.prepare("DELETE FROM books WHERE id = :id");
    query.bindValue(":id", bookId);
    return query.exec();
}

int DatabaseManager::getTotalWordCount() const {
    QSqlQuery query("SELECT COUNT(*) FROM words");
    if (query.next()) return query.value(0).toInt();
    return 0;
}

int DatabaseManager::getLearnedWordCount() const {

    QSqlQuery query("SELECT COUNT(*) FROM cards WHERE state > 0");
    if (query.next()) return query.value(0).toInt();
    return 0;
}

int DatabaseManager::getDueWordCount() const {

    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM cards WHERE due <= :now");
    query.bindValue(":now", QDateTime::currentDateTime());
    if (query.exec() && query.next()) return query.value(0).toInt();
    return 0;
}

QMap<QString, int> DatabaseManager::getReviewHistory() {
    QMap<QString, int> history;
    QSqlQuery query;

    QDateTime sevenDaysAgoDt = QDateTime::currentDateTime().addDays(-6);
    sevenDaysAgoDt.setTime(QTime(0, 0));
    qint64 sevenDaysAgo = sevenDaysAgoDt.toSecsSinceEpoch();
    

    

    query.prepare("SELECT last_review FROM cards WHERE last_review >= :date");
    query.bindValue(":date", sevenDaysAgoDt);
    
    if (query.exec()) {
        while (query.next()) {
            QDateTime dt = query.value(0).toDateTime();
            if (dt.isValid()) {
                QString dateStr = dt.toString("yyyy-MM-dd");
                history[dateStr]++;
            }
        }
    }
    

    for (int i = 0; i < 7; ++i) {
        QString dateStr = QDateTime::currentDateTime().addDays(-i).toString("yyyy-MM-dd");
        if (!history.contains(dateStr)) {
            history[dateStr] = 0;
        }
    }
    
    return history;
}

bool DatabaseManager::deleteWord(int wordId) {
    QSqlQuery query;
    query.prepare("DELETE FROM words WHERE id = :id");
    query.bindValue(":id", wordId);
    return query.exec();
}

bool DatabaseManager::addWord(const Word& word) {
    QSqlQuery query;
    query.prepare("INSERT INTO words (book_id, spelling, phonetic, definition, example, tags, is_favorite) "
                  "VALUES (:book_id, :spelling, :phonetic, :definition, :example, :tags, :is_favorite)");
    query.bindValue(":book_id", word.bookId);
    query.bindValue(":spelling", word.spelling);
    query.bindValue(":phonetic", word.phonetic);
    query.bindValue(":definition", word.definition);
    query.bindValue(":example", word.example);
    query.bindValue(":tags", word.tags.join(";"));
    query.bindValue(":is_favorite", word.isFavorite);

    if (!query.exec()) {
        qWarning() << "Failed to add word:" << query.lastError();
        return false;
    }
    return true;
}

bool DatabaseManager::setFavorite(int wordId, bool favorite) {
    QSqlQuery query;
    query.prepare("UPDATE words SET is_favorite = :fav WHERE id = :id");
    query.bindValue(":fav", favorite);
    query.bindValue(":id", wordId);
    return query.exec();
}

QList<Word> DatabaseManager::getAllWords(int bookId) const {
    QList<Word> words;
    QString sql = "SELECT * FROM words";
    if (bookId != -1) {
        sql += QString(" WHERE book_id = %1").arg(bookId);
    }
    sql += " ORDER BY spelling ASC";
    
    QSqlQuery query(sql);
    while (query.next()) {
        Word w;
        w.id = query.value("id").toInt();
        w.bookId = query.value("book_id").toInt();
        w.spelling = query.value("spelling").toString();
        w.phonetic = query.value("phonetic").toString();
        w.definition = query.value("definition").toString();
        w.example = query.value("example").toString();
        w.tags = query.value("tags").toString().split(';', Qt::SkipEmptyParts);
        w.isFavorite = query.value("is_favorite").toBool();
        w.createdAt = query.value("created_at").toDateTime();
        words.append(w);
    }
    return words;
}

QList<Word> DatabaseManager::getDueWords(int bookId, int limit) const {
    QList<Word> words;
    QString sql = "SELECT w.* FROM words w JOIN cards c ON w.id = c.word_id WHERE c.due <= :now";
    if (bookId != -1) {
        sql += QString(" AND w.book_id = %1").arg(bookId);
    }
    sql += " ORDER BY c.due ASC LIMIT :limit";
    
    QSqlQuery query;
    query.prepare(sql);
    query.bindValue(":now", QDateTime::currentDateTime());
    query.bindValue(":limit", limit);
    
    if (query.exec()) {
        while (query.next()) {
            Word w;
            w.id = query.value("id").toInt();
            w.bookId = query.value("book_id").toInt();
            w.spelling = query.value("spelling").toString();
            w.phonetic = query.value("phonetic").toString();
            w.definition = query.value("definition").toString();
            w.example = query.value("example").toString();
            w.tags = query.value("tags").toString().split(';', Qt::SkipEmptyParts);
            w.isFavorite = query.value("is_favorite").toBool();
            w.createdAt = query.value("created_at").toDateTime();
            words.append(w);
        }
    }
    return words;
}

FsrsCard DatabaseManager::getCard(int wordId) {
    FsrsCard card;
    card.wordId = wordId;
    
    QSqlQuery query;
    query.prepare("SELECT * FROM cards WHERE word_id = :id");
    query.bindValue(":id", wordId);
    
    if (query.exec() && query.next()) {
        card.id = query.value("id").toInt();
        card.state = query.value("state").toInt();
        card.due = query.value("due").toDateTime();
        card.stability = query.value("stability").toDouble();
        card.difficulty = query.value("difficulty").toDouble();
        card.elapsedDays = query.value("elapsed_days").toInt();
        card.scheduledDays = query.value("scheduled_days").toInt();
        card.reps = query.value("reps").toInt();
        card.lapses = query.value("lapses").toInt();
        card.lastReview = query.value("last_review").toDateTime();
    } else {
        QSqlQuery insert;
        insert.prepare("INSERT INTO cards (word_id, due) VALUES (:id, :due)");
        insert.bindValue(":id", wordId);
        insert.bindValue(":due", QDateTime::currentDateTime());
        insert.exec();
        
        card.id = insert.lastInsertId().toInt();
        card.due = QDateTime::currentDateTime();
    }
    return card;
}

bool DatabaseManager::updateCard(const FsrsCard& card) {
    QSqlQuery query;
    query.prepare("UPDATE cards SET state=:state, due=:due, stability=:stability, "
                  "difficulty=:difficulty, elapsed_days=:elapsed, scheduled_days=:scheduled, "
                  "reps=:reps, lapses=:lapses, last_review=:last WHERE id=:id");
    
    query.bindValue(":state", card.state);
    query.bindValue(":due", card.due);
    query.bindValue(":stability", card.stability);
    query.bindValue(":difficulty", card.difficulty);
    query.bindValue(":elapsed", card.elapsedDays);
    query.bindValue(":scheduled", card.scheduledDays);
    query.bindValue(":reps", card.reps);
    query.bindValue(":lapses", card.lapses);
    query.bindValue(":last", card.lastReview);
    query.bindValue(":id", card.id);
    
    if (!query.exec()) {
        qCritical() << "Error updating card:" << query.lastError();
        return false;
    }
    return true;
}
