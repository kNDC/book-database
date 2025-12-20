#include "book_database.hpp"
#include "statistics.hpp"

#include <gtest/gtest.h>
#include <sstream>

//==== Tests for Genre ====//
TEST(Genre_Tests, Conversion)
{
    using namespace bookdb;
    using namespace std::string_view_literals;

    EXPECT_EQ(GenreFromString("fiction"sv), 
        GenreFromString("Fiction"sv));
    EXPECT_EQ(GenreFromString("NonFiction"sv), 
        GenreFromString("nonfiction"sv));
    EXPECT_EQ(GenreFromString("SciFi"sv), 
        GenreFromString("sCiFi"sv));
    EXPECT_EQ(GenreFromString("Biography"sv), 
        GenreFromString("bIoGraPhy"sv));
    EXPECT_EQ(GenreFromString("Mystery"sv), 
        GenreFromString("mystery"sv));
    EXPECT_EQ(GenreFromString("Unknown"sv), 
        GenreFromString("unknown"sv));
    
    EXPECT_THROW(GenreFromString("Fan fiction"sv), 
        std::logic_error);
}

TEST(Genre_Tests, CompositeConversion)
{
    using namespace bookdb;
    using namespace std::string_view_literals;

    std::string_view sv = "Biography"sv;
    std::stringstream ss;

    EXPECT_EQ(AsGenre(AsGenre(sv)), 
        Genre::Biography);
    
    std::print(ss, "{}", GenreFromString(sv));
    EXPECT_EQ(ss.str(), sv);
}

//==== Tests for Book ====//
TEST(Book_Tests, AuthorStorage)
{
    using namespace bookdb;

    std::string author = "abcde";

    Book book{};
    book.author = author;

    author = "01234";
    EXPECT_EQ(book.author, author);
}

//==== Tests for BookDatabase ====//
TEST(BookDatabase_Tests, LocalAuthorStorage)
{
    using namespace bookdb;

    std::string author = "abcde";
    std::string author_orig = author;
    BookDatabase db;

    for (size_t i = 0; i < 500; ++i)
    {
        Book book;
        book.title = std::to_string(i);
        book.author = author;

        db.EmplaceBack(book);

        // Книг теперь больше...
        ASSERT_EQ(db.GetBooks().size(), i + 1);
        ASSERT_EQ(std::distance(db.begin(), db.end()), i + 1);

        // ...а количество писателей прежнее
        ASSERT_EQ(db.GetAuthors().size(), 1);
    }
    
    author = "01234";

    /* Имя писателя хранится в БД, 
    а не вытягивается из author */
    for (auto it = db.begin(); it != db.end(); ++it)
    {
        ASSERT_EQ(it->author, author_orig);
    }
}

TEST(BookDatabase_Tests, AuthorMerging)
{
    using namespace bookdb;

    std::string author = "abcde";
    BookDatabase db;
    
    for (size_t i = 0; i < 500; ++i)
    {
        Book book;
        book.title = std::to_string(i);
        book.author = author;

        db.EmplaceBack(book);

        // Книга добавлена...
        ASSERT_EQ(db.GetBooks().back().title, 
            std::to_string(i));

        // ...книг теперь больше...
        ASSERT_EQ(db.GetBooks().size(), i + 1);
        ASSERT_EQ(std::distance(db.begin(), db.end()), i + 1);

        // ...а количество писателей прежнее
        ASSERT_EQ(db.GetAuthors().size(), 1);
    }
}

TEST(BookDatabase_Tests, Histogram)
{
    using namespace bookdb;

    BookDatabase db;
    size_t n_authors = 100;

    for (size_t i = 0; i < n_authors; ++i)
    {
        for (size_t j = i; j < n_authors; ++j)
        {
            Book book{};
            book.title = std::to_string(j);
            book.author = std::to_string(i);

            db.EmplaceBack(book);
        }
    }
    
    auto histogram = BuildAuthorHistogramFlat(db);
    for (size_t i = 0; i < n_authors; ++i)
    {
        ASSERT_EQ(histogram.at(std::to_string(i)), 
            n_authors - i);
    }

    // Случай пустого хранилища
    ASSERT_EQ(BuildAuthorHistogramFlat(BookDatabase{}).size(), 
        0);
}

TEST(BookDatabase_Tests, AverageRatings)
{
    using namespace bookdb;

    BookDatabase db;
    const std::vector<Genre> genres{Genre::Biography, 
        Genre::Mystery, Genre::NonFiction};
    const std::vector<size_t> book_genre_sizes{100, 200, 75};
    std::vector<double> avg_ratings(3, 0);

    double avg_rating = 0;

    {
        size_t i = 0;
        size_t upper_bound = 0;
        size_t genre_read_count = 0;
        size_t total_read_count = 0;

        for (size_t j = 0; j < genres.size(); ++j)
        {
            upper_bound += book_genre_sizes[j];

            for (; i < upper_bound; ++i)
            {
                Book book{};
                book.author = std::format("{0}, son of {0}", 
                    std::to_string(i));
                book.title = std::to_string(i);
                book.genre = genres[j];
                book.rating = i;
                book.read_count = i + 1;

                db.EmplaceBack(book);

                avg_ratings[j] += i * (i + 1);
                avg_rating += i * (i + 1);

                genre_read_count += i + 1;
                total_read_count += i + 1;
            }
            
            avg_ratings[j] /= genre_read_count;
            genre_read_count = 0;
        }

        avg_rating /= total_read_count;
    }
    
    // Взвешенные пожанровые рейтинги
    {
        auto genre_ratings = CalculateGenreRatings(db);
        for (size_t i = 0; i < genres.size(); ++i)
        {
            ASSERT_DOUBLE_EQ(genre_ratings.at(genres[i]), 
                avg_ratings[i]);
        }
    }

    // Общий средневзвешенный рейтинг
    {
        ASSERT_DOUBLE_EQ(CalculateAverageRating(db), 
            avg_rating);
    }
}

TEST(BookDatabase_Tests, AverageRatingsCornerCases)
{
    using namespace bookdb;

    BookDatabase db0;

    // Случай пустого хранилища
    ASSERT_EQ(CalculateGenreRatings(db0).size(), 0);
    ASSERT_DOUBLE_EQ(CalculateAverageRating(db0), 0);
    
    // Случай отсутствия прочтений
    db0.EmplaceBack();
    auto genre_ratings = CalculateGenreRatings(db0);
    ASSERT_DOUBLE_EQ(genre_ratings.at(Genre::Unknown), 0);
    ASSERT_DOUBLE_EQ(CalculateAverageRating(db0), 0);
}

TEST(BookDatabase_Tests, RandomSample)
{
    using namespace bookdb;

    BookDatabase db;
    const size_t n_books = 100;

    for (size_t i = 0; i < n_books; ++i)
    {
        Book book{};
        book.author = std::to_string(i + 1);
        book.title = std::to_string(i);
        db.EmplaceBack();
    }

    for (size_t i = 0; i < n_books; ++i)
    {
        BookData sample = SampleRandomBooks(db, i);
        ASSERT_EQ(sample.size(), i);
    }
    
    ASSERT_THROW(SampleRandomBooks(db, n_books + 1), 
        std::logic_error);
}

TEST(BookDatabase_Tests, TopN)
{
    using namespace bookdb;

    BookDatabase db;
    const size_t n_books = 100;

    for (size_t i = 0; i < n_books; ++i)
    {
        Book book{};
        book.author = std::to_string(i + 1);
        book.title = std::to_string(i);
        book.rating = n_books - i;
        book.year = i;
        book.read_count = 2 * i + 5;
        db.EmplaceBack(book);
    }

    // Проверка отбора по рейтингу
    for (size_t i = 0; i < n_books; ++i)
    {
        BookData top_books_def = GetTopNBy(db, i);
        BookData top_books = 
            GetTopNBy(db, i, comp::LessByRating{});
        ASSERT_EQ(top_books.size(), i);

        std::sort(top_books.begin(), top_books.end(), 
            comp::LessByRating{});
        std::sort(top_books_def.begin(), top_books_def.end(), 
            comp::LessByRating{});
        for (size_t j = n_books - i; j < n_books; ++j)
        {
            ASSERT_EQ(top_books[j + i - n_books].get().title, 
                top_books_def[j + i - n_books].get().title);
            ASSERT_DOUBLE_EQ(top_books[j + i - n_books].get().rating, 
                j + 1);
        }
    }

    // Проверка отбора по году
    for (size_t i = 0; i < n_books; ++i)
    {
        BookData top_books = 
            GetTopNBy(db, i, comp::LessByPopularity{});
        ASSERT_EQ(top_books.size(), i);

        std::sort(top_books.begin(), top_books.end(), 
            comp::LessByYear{});
        for (size_t j = n_books - i; j < n_books; ++j)
        {
            ASSERT_EQ(top_books[j + i - n_books].get().year, j);
        }
    }

    // Проверка отбора по прочтениям
    for (size_t i = 0; i < n_books; ++i)
    {
        BookData top_books = GetTopNBy(db, i, comp::LessByPopularity{});
        ASSERT_EQ(top_books.size(), i);

        std::sort(top_books.begin(), top_books.end(), 
            comp::LessByPopularity{});
        for (size_t j = n_books - i; j < n_books; ++j)
        {
            ASSERT_EQ(top_books[j + i - n_books].get().read_count, 
                2 * j + 5);
        }
    }
    
    ASSERT_THROW(SampleRandomBooks(db, n_books + 1), 
        std::logic_error);
}

TEST(BookDatabase_Tests, CompoundedFiltering)
{
    using namespace bookdb;

    BookDatabase db;
    const size_t n_books = 200;

    for (size_t i = 0; i < n_books; ++i)
    {
        Book book{};
        book.author = std::to_string(i + 1);
        book.title = std::to_string(i);
        book.rating = n_books - i;
        book.year = i;
        book.genre = (i % 2) 
            ? Genre::Fiction 
            : Genre::NonFiction;
        book.read_count = 3 * i + 5;
        db.EmplaceBack(book);
    }

    // Комбинирующий фильтр
    BookData books_comp = FilterBooks(db.begin(), db.end(), 
        all_of(YearBetween(50, 149), 
            GenreIs(Genre::Fiction)));
    std::sort(books_comp.begin(), books_comp.end(), 
        comp::LessByRating{});
    
    // Пошаговый фильтр
    BookData books_incr = FilterBooks(db.begin(), db.end(), 
        YearBetween(50, 149));
    ASSERT_EQ(books_incr.size(), 100);

    std::sort(books_incr.begin(), books_incr.end(), 
        comp::LessByYear{});
    for (size_t i = 0; i < books_incr.size(); ++i)
    {
        ASSERT_EQ(books_incr[i].get().year, i + 50);
    }

    {
        BookData::iterator pos = 
            std::partition(books_incr.begin(), books_incr.end(), 
                [](std::reference_wrapper<const Book> book_ref)
                {
                    return book_ref.get().genre == Genre::Fiction;
                });
        books_incr = { books_incr.begin(), pos };

        std::sort(books_incr.begin(), books_incr.end(), 
            comp::LessByRating{});
    }

    ASSERT_EQ(books_comp.size(), books_incr.size());
    for (size_t i = 0; i < books_comp.size(); ++i)
    {
        ASSERT_EQ(books_comp[i].get().title, 
            books_incr[i].get().title);
    }
}

TEST(BookDatabase_Tests, FilteringCompositions)
{
    using namespace bookdb;

    BookDatabase db;
    const size_t n_books = 200;

    for (size_t i = 0; i < n_books; ++i)
    {
        Book book{};
        book.author = std::to_string(i + 1);
        book.title = std::to_string(i);
        book.rating = n_books - i;
        book.year = i;
        book.genre = (i % 2) 
            ? Genre::Fiction 
            : Genre::NonFiction;
        book.read_count = 3 * i + 5;
        db.EmplaceBack(book);
    }

    // Фильтр с пересечением условий-альтернатив
    {
        BookData books = FilterBooks(db.begin(), db.end(), 
            all_of(GenreIs(Genre::Fiction), 
                GenreIs(Genre::NonFiction)));
        ASSERT_EQ(books.size(), 0);
    }

    // Фильтр с объединением условий-альтернатив
    {
        BookData books = FilterBooks(db.begin(), db.end(), 
            any_of(GenreIs(Genre::Fiction), 
                GenreIs(Genre::NonFiction)));
        ASSERT_EQ(books.size(), db.size());

        std::sort(books.begin(), books.end(), 
            [](std::reference_wrapper<const Book> book_ref1, 
                std::reference_wrapper<const Book> book_ref2)
            {
                return book_ref1.get().year < 
                    book_ref2.get().year;
            });
        
        for (size_t i = 0; i < db.size(); ++i)
        {
            ASSERT_EQ(books[i].get().title, 
                db.GetBooks()[i].title);
        }
    }
}

TEST(BookDatabase_Tests, RatingFiltering)
{
    using namespace bookdb;

    BookDatabase db;
    const size_t n_books = 200;
    const double min_rating = 30;

    for (size_t i = 0; i < n_books; ++i)
    {
        Book book{};
        book.author = std::to_string(i + 1);
        book.title = std::to_string(i);
        book.rating = n_books - i;
        book.year = i;
        book.genre = (i % 2) 
            ? Genre::Fiction 
            : Genre::NonFiction;
        book.read_count = 3 * i + 5;
        db.EmplaceBack(book);
    }

    // Простой фильтр
    BookData books = FilterBooks(db.begin(), db.end(), 
        RatingAbove(min_rating));
    
    // Правильный размер
    ASSERT_EQ(books.size(), 
        n_books - (unsigned)min_rating + 1);
    
    // Правильные рейтинги
    for (size_t i = 0; i < books.size(); ++i)
    {
        ASSERT_DOUBLE_EQ(books[i].get().rating, 
            n_books - i);
    }
}