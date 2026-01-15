/**
 * @file test_model_indexing.cpp
 * @brief Example: Multiple ways to index/query the same Qt model
 * 
 * Demonstrates:
 * 1. QStandardItemModel with custom roles (flat storage)
 * 2. Two QSortFilterProxyModel instances providing different "views"
 * 3. Direct querying by custom role data
 * 
 * Domain: Book Library
 * - Genre (Fiction/Technical/History) maps to ResourceType
 * - Publisher (Academic/Local/SelfPub) maps to ResourceTier
 */

#include <QCoreApplication>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QDebug>
#include <QList>

// Custom roles for our data
enum BookRole {
    TitleRole = Qt::UserRole + 1,
    GenreRole = Qt::UserRole + 2,
    PublisherRole = Qt::UserRole + 3,
    YearRole = Qt::UserRole + 4,
    AuthorRole = Qt::UserRole + 5,
    ISBNRole = Qt::UserRole + 6
};

enum class Genre {
    Unknown = 0,
    Fiction,
    Technical,
    History,
    SciFi
};

enum class Publisher {
    Unknown = 0,
    Academic,
    Local,
    SelfPublished
};

// Helper to convert enums to strings
QString genreToString(Genre g) {
    switch(g) {
        case Genre::Fiction: return "Fiction";
        case Genre::Technical: return "Technical";
        case Genre::History: return "History";
        case Genre::SciFi: return "Sci-Fi";
        default: return "Unknown";
    }
}

QString publisherToString(Publisher p) {
    switch(p) {
        case Publisher::Academic: return "Academic Press";
        case Publisher::Local: return "Local Publisher";
        case Publisher::SelfPublished: return "Self-Published";
        default: return "Unknown";
    }
}

// Sample book data
struct Book {
    QString title;
    Genre genre;
    Publisher publisher;
    int year;
    QString author;
    QString isbn;
};

QList<Book> getSampleBooks() {
    return {
        {"C++ Programming", Genre::Technical, Publisher::Academic, 2020, "Bjarne Stroustrup", "978-0321563842"},
        {"Qt for Beginners", Genre::Technical, Publisher::Local, 2019, "John Smith", "978-1234567890"},
        {"The Great Gatsby", Genre::Fiction, Publisher::Academic, 1925, "F. Scott Fitzgerald", "978-0743273565"},
        {"1984", Genre::Fiction, Publisher::Local, 1949, "George Orwell", "978-0451524935"},
        {"Dune", Genre::SciFi, Publisher::Academic, 1965, "Frank Herbert", "978-0441013593"},
        {"Foundation", Genre::SciFi, Publisher::SelfPublished, 1951, "Isaac Asimov", "978-0553293357"},
        {"World War II", Genre::History, Publisher::Academic, 2005, "Antony Beevor", "978-0316023634"},
        {"OpenSCAD Guide", Genre::Technical, Publisher::SelfPublished, 2021, "Marius Kintel", "978-9876543210"},
        {"The Hobbit", Genre::Fiction, Publisher::Academic, 1937, "J.R.R. Tolkien", "978-0345339683"},
        {"Linux Kernel", Genre::Technical, Publisher::Academic, 2018, "Linus Torvalds", "978-0321637369"}
    };
}

// Proxy model that filters by Genre
class GenreFilterProxy : public QSortFilterProxyModel {
public:
    explicit GenreFilterProxy(QObject* parent = nullptr) : QSortFilterProxyModel(parent) {}
    
    void setFilterGenre(Genre g) {
        m_filterGenre = g;
        invalidateFilter();
    }
    
protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override {
        QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
        int genre = sourceModel()->data(index, GenreRole).toInt();
        return (Genre)genre == m_filterGenre;
    }
    
private:
    Genre m_filterGenre = Genre::Unknown;
};

// Proxy model that filters by Publisher
class PublisherFilterProxy : public QSortFilterProxyModel {
public:
    explicit PublisherFilterProxy(QObject* parent = nullptr) : QSortFilterProxyModel(parent) {}
    
    void setFilterPublisher(Publisher p) {
        m_filterPublisher = p;
        invalidateFilter();
    }
    
protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override {
        QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
        int pub = sourceModel()->data(index, PublisherRole).toInt();
        return (Publisher)pub == m_filterPublisher;
    }
    
private:
    Publisher m_filterPublisher = Publisher::Unknown;
};

// Populate model with flat storage (all books as rows)
void populateModel(QStandardItemModel* model, const QList<Book>& books) {
    for (const auto& book : books) {
        auto* item = new QStandardItem(book.title);
        item->setData(book.title, TitleRole);
        item->setData(static_cast<int>(book.genre), GenreRole);
        item->setData(static_cast<int>(book.publisher), PublisherRole);
        item->setData(book.year, YearRole);
        item->setData(book.author, AuthorRole);
        item->setData(book.isbn, ISBNRole);
        
        model->appendRow(item);
    }
}

// Query model directly by role
QList<QStandardItem*> findByGenre(QStandardItemModel* model, Genre genre) {
    QList<QStandardItem*> results;
    for (int i = 0; i < model->rowCount(); ++i) {
        QStandardItem* item = model->item(i);
        if (static_cast<Genre>(item->data(GenreRole).toInt()) == genre) {
            results.append(item);
        }
    }
    return results;
}

QList<QStandardItem*> findByPublisher(QStandardItemModel* model, Publisher pub) {
    QList<QStandardItem*> results;
    for (int i = 0; i < model->rowCount(); ++i) {
        QStandardItem* item = model->item(i);
        if (static_cast<Publisher>(item->data(PublisherRole).toInt()) == pub) {
            results.append(item);
        }
    }
    return results;
}

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    
    qDebug() << "=== Qt Model Indexing Example ===\n";
    
    // 1. Create base model with flat storage
    QStandardItemModel model;
    QList<Book> books = getSampleBooks();
    populateModel(&model, books);
    
    qDebug() << "Total books in model:" << model.rowCount() << "\n";
    
    // =========================================================================
    // Approach 1: Direct querying by custom roles
    // =========================================================================
    qDebug() << "--- APPROACH 1: Direct Role Queries ---";
    
    auto technicalBooks = findByGenre(&model, Genre::Technical);
    qDebug() << "\nTechnical books (by direct query):";
    for (auto* item : technicalBooks) {
        qDebug() << "  -" << item->data(TitleRole).toString()
                 << "by" << item->data(AuthorRole).toString()
                 << "(" << item->data(YearRole).toInt() << ")";
    }
    
    auto academicBooks = findByPublisher(&model, Publisher::Academic);
    qDebug() << "\nAcademic Press books (by direct query):";
    for (auto* item : academicBooks) {
        qDebug() << "  -" << item->data(TitleRole).toString()
                 << "[" << genreToString(static_cast<Genre>(item->data(GenreRole).toInt())) << "]";
    }
    
    // =========================================================================
    // Approach 2: QSortFilterProxyModel "indexes" (views)
    // =========================================================================
    qDebug() << "\n--- APPROACH 2: QSortFilterProxyModel Views ---";
    
    // Create Genre-based proxy
    GenreFilterProxy genreProxy;
    genreProxy.setSourceModel(&model);
    genreProxy.setFilterGenre(Genre::SciFi);
    
    qDebug() << "\nSci-Fi books (via GenreFilterProxy):";
    for (int i = 0; i < genreProxy.rowCount(); ++i) {
        QModelIndex idx = genreProxy.index(i, 0);
        qDebug() << "  -" << idx.data(TitleRole).toString()
                 << "by" << idx.data(AuthorRole).toString();
    }
    
    // Create Publisher-based proxy
    PublisherFilterProxy pubProxy;
    pubProxy.setSourceModel(&model);
    pubProxy.setFilterPublisher(Publisher::SelfPublished);
    
    qDebug() << "\nSelf-Published books (via PublisherFilterProxy):";
    for (int i = 0; i < pubProxy.rowCount(); ++i) {
        QModelIndex idx = pubProxy.index(i, 0);
        qDebug() << "  -" << idx.data(TitleRole).toString()
                 << "[" << genreToString(static_cast<Genre>(idx.data(GenreRole).toInt())) << "]";
    }
    
    // =========================================================================
    // Approach 3: Multiple simultaneous proxies (different "indexes")
    // =========================================================================
    qDebug() << "\n--- APPROACH 3: Multiple Simultaneous Views ---";
    
    GenreFilterProxy fictionView;
    fictionView.setSourceModel(&model);
    fictionView.setFilterGenre(Genre::Fiction);
    
    PublisherFilterProxy localView;
    localView.setSourceModel(&model);
    localView.setFilterPublisher(Publisher::Local);
    
    qDebug() << "\nFiction View has" << fictionView.rowCount() << "books";
    qDebug() << "Local Publisher View has" << localView.rowCount() << "books";
    
    // Show they're accessing the SAME underlying data
    qDebug() << "\nChanging base model...";
    auto* newBook = new QStandardItem("New Technical Book");
    newBook->setData("New Technical Book", TitleRole);
    newBook->setData(static_cast<int>(Genre::Technical), GenreRole);
    newBook->setData(static_cast<int>(Publisher::Local), PublisherRole);
    newBook->setData(2025, YearRole);
    newBook->setData("Agent Smith", AuthorRole);
    newBook->setData("978-1111111111", ISBNRole);
    model.appendRow(newBook);
    
    qDebug() << "Fiction View now has" << fictionView.rowCount() << "books (unchanged)";
    qDebug() << "Local Publisher View now has" << localView.rowCount() << "books (increased by 1!)";
    
    // =========================================================================
    // Summary
    // =========================================================================
    qDebug() << "\n=== CONCLUSION ===";
    qDebug() << "✓ Can query same model by different criteria (Genre, Publisher)";
    qDebug() << "✓ QSortFilterProxyModel acts as a dynamic 'view' or 'index'";
    qDebug() << "✓ Multiple proxies can exist simultaneously";
    qDebug() << "✓ All proxies share the same underlying data";
    qDebug() << "\nThis maps to your resource inventory:";
    qDebug() << "  Genre → ResourceType (Templates, Examples, Fonts)";
    qDebug() << "  Publisher → ResourceTier (Installation, Machine, User)";
    
    return 0;
}
