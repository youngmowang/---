#include "WordModel.h"
#include "../db/DatabaseManager.h"

WordModel::WordModel(QObject *parent) : QAbstractListModel(parent) {
}

int WordModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) return 0;
    return m_words.count();
}

QVariant WordModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_words.count())
        return QVariant();

    const Word &word = m_words[index.row()];

    switch (role) {
    case IdRole:
        return word.id;
    case SpellingRole:
        return word.spelling;
    case Qt::DisplayRole:
        return QString("%1  -  %2").arg(word.spelling, word.definition);
    case PhoneticRole:
        return word.phonetic;
    case DefinitionRole:
        return word.definition;
    case ExampleRole:
        return word.example;
    case FavoriteRole:
        return word.isFavorite;
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> WordModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[SpellingRole] = "spelling";
    roles[PhoneticRole] = "phonetic";
    roles[DefinitionRole] = "definition";
    roles[ExampleRole] = "example";
    roles[FavoriteRole] = "isFavorite";
    return roles;
}

void WordModel::loadWords(int bookId) {
    beginResetModel();
    m_allWords = DatabaseManager::instance().getAllWords(bookId);
    m_words = m_allWords;
    endResetModel();
}

void WordModel::setFilter(const QString &text) {
    beginResetModel();
    if (text.isEmpty()) {
        m_words = m_allWords;
    } else {
        m_words.clear();
        for (const Word& word : m_allWords) {
            if (word.spelling.contains(text, Qt::CaseInsensitive)) {
                m_words.append(word);
            }
        }
    }
    endResetModel();
}

void WordModel::addWord(const Word& word) {
    if (DatabaseManager::instance().addWord(word)) {
        beginInsertRows(QModelIndex(), m_words.count(), m_words.count());
        m_words.append(word);
        m_allWords.append(word);
        endInsertRows();
    }
}

void WordModel::toggleFavorite(int row) {
    if (row < 0 || row >= m_words.count()) return;
    
    Word& word = m_words[row];
    bool newStatus = !word.isFavorite;
    
    if (DatabaseManager::instance().setFavorite(word.id, newStatus)) {
        word.isFavorite = newStatus;
        emit dataChanged(index(row), index(row), {FavoriteRole});
    }
}

void WordModel::sortWords(SortOrder order) {
    beginResetModel();
    if (order == Alphabetical) {
        std::sort(m_words.begin(), m_words.end(), [](const Word& a, const Word& b) {
            return a.spelling.compare(b.spelling, Qt::CaseInsensitive) < 0;
        });
    } else if (order == Random) {
        std::random_shuffle(m_words.begin(), m_words.end()); 
    }
    endResetModel();
}
