#pragma once

#include <string>
#include <string_view>

namespace bookdb
{
    struct TransparentStringLess
    {
        using is_transparent = void;

        bool operator()(std::string_view author1, 
            std::string_view author2) const noexcept
        {
            return author1 < author2;
        }

        bool operator()(const Book& book, 
            std::string_view author) const noexcept
        {
            return book.author < author;
        }

        bool operator()(std::string_view author, 
            const Book& book) const noexcept
        {
            return author < book.author;
        }
    };
    
    struct TransparentStringEqual
    {
        using is_transparent = void;

        bool operator()(std::string_view author1, 
            std::string_view author2) const noexcept
        {
            return author1 == author2;
        }

        bool operator()(const Book& book, 
            std::string_view author) const noexcept
        {
            return book.author == author;
        }

        bool operator()(std::string_view author, 
            const Book& book) const noexcept
        {
            return author == book.author;
        }
    };
    
    struct TransparentStringHash
    {
        using is_transparent = void;

        size_t operator()(const Book& book) const noexcept
        {
            return std::hash<std::string_view>{}(book.author);
        }
    };
}  // namespace bookdb
