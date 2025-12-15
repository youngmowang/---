#pragma once
#include <QAbstractListModel>
#include "Word.h"

class WordModel : public QAbstractListModel {
    Q_OBJECT

public:
    QHash<int, QByteArray> roleNames() const override;
    
    enum WordRoles {
        IdRole = Qt::UserRole + 1,
        SpellingRole,
        PhoneticRole,
        DefinitionRole,
        ExampleRole,
        FavoriteRole
    };

    explicit WordModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void loadWords(int bookId = -1);
    void addWord(const Word& word);
    void toggleFavorite(int row);

    enum SortOrder {
        Alphabetical,
        Random
    };
    Q_ENUM(SortOrder)

    void sortWords(SortOrder order);

    void setFilter(const QString &text);

private:
    QList<Word> m_words;     // Displayed words
    QList<Word> m_allWords;  // All loaded words
};
