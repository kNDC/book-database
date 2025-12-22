#pragma once

#include "book.hpp"

namespace bookdb::comp
{
    struct LessByAuthor
    {
        bool operator()(const Book& book1, 
            const Book& book2) const noexcept
        {
            return (book1.author == book2.author) 
                ? book1.year < book2.year 
                : book1.author < book2.author;
        }
    };

    struct LessByPopularity
    {
        bool operator()(const Book& book1, 
            const Book& book2) const noexcept
        {
            return book1.read_count < book2.read_count;
        }
    };

    struct LessByYear
    {
        bool operator()(const Book& book1, 
            const Book& book2) const noexcept
        {
            return book1.year < book2.year;
        }
    };

    template <double eps = 1e-7>
    struct LessByRating
    {
        bool operator()(const Book& book1, 
            const Book& book2) const noexcept
        {
            return (std::abs(book1.rating - book2.rating) > eps) && 
                (book1.rating < book2.rating);
        }
    };

    struct LessByReads
    {
        bool operator()(const Book& book1, 
            const Book& book2) const noexcept
        {
            return book1.read_count < book2.read_count;
        }
    };
}  // namespace bookdb::comp