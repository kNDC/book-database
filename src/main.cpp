#include "book_database.hpp"
#include "statistics.hpp"

using namespace bookdb;

int main()
{
    // Create a book database
    BookDatabase<std::vector<Book>> db;
    
    // Add some books
    db.EmplaceBack("1984", "George Orwell", 1949, Genre::SciFi, 5.1, 190);
    db.EmplaceBack("Animal Farm", "George Orwell", 1945, Genre::Fiction, 4.4, 143);
    db.EmplaceBack("The Great Gatsby", "F. Scott Fitzgerald", 1925, Genre::Fiction, 4.5, 120);
    db.EmplaceBack("To Kill a Mockingbird", "Harper Lee", 1960, Genre::Fiction, 4.8, 156);
    db.EmplaceBack("Pride and Prejudice", "Jane Austen", 1813, Genre::Fiction, 4.7, 178);
    db.EmplaceBack("The Catcher in the Rye", "J.D. Salinger", 1951, Genre::Fiction, 4.3, 112);
    db.EmplaceBack("Brave New World", "Aldous Huxley", 1932, Genre::SciFi, 4.5, 98);
    db.EmplaceBack("Jane Eyre", "Charlotte Brontë", 1847, Genre::Fiction, 4.6, 110);
    db.EmplaceBack("The Hobbit", "J.R.R. Tolkien", 1937, Genre::Fiction, 4.9, 203);
    db.EmplaceBack("Lord of the Flies", "William Golding", 1954, Genre::Fiction, 4.2, 89);
    std::println("{}", db);
    
    // Sorts
    std::sort(db.begin(), db.end(), comp::LessByAuthor{});
    std::print("Books sorted by author: {}==================\n", db);

    std::sort(db.begin(), db.end(), comp::LessByPopularity{});
    std::print("Books sorted by popularity: {}==================\n", db);

    // Author histogram
    auto histogram = BuildAuthorHistogramFlat(db);
    std::print("Author histogram: {}\n\n", histogram);

    // Ratings
    auto genre_ratings = CalculateGenreRatings(db);

    std::println("Average ratings by genre:");
    for (size_t i = 0; i < genre_ratings.size(); ++i)
    {
        std::println("{}: {:.2f}", 
            genre_ratings.begin()[i].first, 
            genre_ratings.begin()[i].second);
    }
    std::println("");

    double avg_rating = CalculateAverageRating(db);
    std::println("Average book rating in the library: {:.2f}\n", avg_rating);
    
    // Filters
    BookData filtered = FilterBooks(db.begin(), db.end(), 
        all_of(YearBetween(1900, 1999), RatingAbove(4.5)));
    
    std::print("Books from the XXth century with rating ≥ 4.5:\n");
    for(auto book : filtered)
    {
        std::println("{}", book.get());
    }
    std::println("");
    
    // A random book sample
    BookData sample = SampleRandomBooks(db, 4);
    std::print("{} random books from the database:\n", 4);
    std::for_each(sample.cbegin(), sample.cend(), 
        [](const auto& v)
        {
            std::println("{}", v.get());
        });
    std::println("");
    
    // Top N = 3 books
    BookData top_books = GetTopNBy(db, 3, comp::LessByRating{});
    std::print("Top 3 books by rating:\n");
    std::for_each(top_books.cbegin(), top_books.cend(), 
        [](const auto& v)
        {
            std::print("{}\n", v.get());
        });
    std::println("");

    using BookIt = BookDatabase<std::vector<Book>>::iterator;
    BookIt OrwellBookIt = 
        std::find_if(db.begin(), db.end(), 
        [](const Book& book)
        {
            return book.author == "George Orwell";
        });
    
    if (OrwellBookIt != db.end())
    {
        std::println("Transparent look-up by authors\n"
            "First found book by George Orwell: {}", 
            *OrwellBookIt);
    }
}