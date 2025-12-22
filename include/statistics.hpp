#pragma once

#include "book_database.hpp"
#include "comparators.hpp"

#include <print>

#include <string_view>
#include <vector>
#include <flat_map>

#include <algorithm>
#include <random>

namespace bookdb
{
    template <BookContainerLike T, 
        typename Cmp = TransparentStringLess>
    std::flat_map<std::string_view, unsigned, Cmp> 
    BuildAuthorHistogramFlat(const BookDatabase<T>& db, 
        Cmp cmp = {})
    {
        std::vector<std::string_view> 
        keys(db.GetAuthors().begin(), db.GetAuthors().end());

        std::vector<unsigned> 
        vals(db.GetAuthors().size(), 0);

        std::flat_map out(keys, vals, cmp);
        for (const Book& book : db)
        {
            ++out.at(book);
        }
        
        return out;
    }

    // Подсчёт средних рейтингов по жанрам, взвешенных по количеству прочтений
    template <BookContainerLike T>
    std::flat_map<Genre, double> 
    CalculateGenreRatings(const BookDatabase<T>& db)
    {
        std::vector<Genre> genres;
        std::flat_map counts(std::vector<Genre>{}, 
            std::vector<unsigned>{});
        std::flat_map out(std::vector<Genre>{}, 
            std::vector<double>{});
        
        for (const Book& book : db)
        {
            counts[book.genre] += book.read_count;
            out[book.genre] += book.rating * book.read_count;
        }

        for (size_t i = 0; i < counts.size(); ++i)
        {
            size_t count = counts.begin()[i].second;
            out.at(counts.begin()[i].first) /= 
                count ? count : 1;
        }
        
        return out;
    }

    // Подсчёт среднего рейтинга, взвешенного по количеству прочтений
    template <BookContainerLike T>
    constexpr double CalculateAverageRating(const BookDatabase<T>& db)
    {
        double out = std::accumulate(db.begin(), db.end(), 0.0, 
                [](double val, const Book& book)
                {
                    return val + book.rating * book.read_count;
                });
        
        unsigned count = 
            std::accumulate(db.begin(), db.end(), 0, 
                [](unsigned val, const Book& book)
                {
                    return val + book.read_count;
                });
        
        return out / (count ? count : 1);
    }

    using BookData = 
        std::vector<std::reference_wrapper<const Book>>;

    template <BookContainerLike T>
    BookData SampleRandomBooks(const BookDatabase<T>& db, 
        size_t sample_size)
    {
        if (sample_size > db.size())
        {
            throw std::logic_error("Sample size cannot exceed "
                "the book storage's volume");
        }

        // Выборка исчерпывает хранилище
        if (sample_size == db.size())
        {
            return {db.cbegin(), db.cend()};
        }

        // Тривиальный случай пустой выборки
        if (!sample_size) return {};
        
        BookData out;
        out.reserve(sample_size);

        std::sample(db.cbegin(), db.cend(), 
            std::back_inserter(out), sample_size, 
            std::mt19937(std::random_device{}()));
        
        return out;
    }

    template <BookContainerLike T, 
        BookComparator Cmp = comp::LessByRating<>>
    BookData GetTopNBy(BookDatabase<T>& db, size_t top_size, 
        Cmp cmp = Cmp{})
    {
        if (top_size > db.size())
        {
            throw std::logic_error("Cannot choose more books than "
                "the book storage's volume");
        }

        // Тривиальный случай топ-0
        if (!top_size) return {};

        std::partial_sort(db.begin(), 
            std::prev(db.end(), top_size), db.end(), cmp);
        
        BookData out(std::prev(db.end(), top_size), db.end());
        return out;
    }

    // Фильтрующие функции
    auto YearBetween(int from, int to)
    {
        return [from_ = from, to_ = to](const Book& book)
            {
                return book.year >= from_ && 
                    book.year <= to_;
            };
    }

    auto RatingAbove(double threshold)
    {
        return [threshold_ = threshold]<double eps = 1e-7>(const Book& book)
            {
                return std::abs(book.rating - threshold_) < eps || 
                    book.rating >= threshold_;
            };
    }
    
    auto GenreIs(Genre genre)
    {
        return [genre_ = genre](const Book& book)
            {
                return book.genre == genre_;
            };
    }

    // Комбинаторы фильтрующих функций
    // Выполнение всех условий ps
    template <BookPredicate... Ps>
    auto all_of(Ps... ps)
    {
        return [...ps_ = std::forward<Ps>(ps)](const Book& book)
            {
                return (... && ps_(book));
            };
    }

    // Выполнение хотя бы одного из условий ps
    template <BookPredicate... Ps>
    auto any_of(Ps... ps)
    {
        return [...ps_ = std::forward<Ps>(ps)](const Book& book)
        {
            return (... || ps_(book));
        };
    }

    template <BookIterator It, BookPredicate P>
    BookData FilterBooks(It from, It to, P&& p)
    {
        // Тривиальный случай совпадающих итераторов
        if (from == to) return {};
        
        BookData out;
        std::copy_if(from, to, 
            std::back_inserter(out), 
            std::forward<P>(p));
        return out;
    }
}  // namespace bookdb