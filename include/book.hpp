#pragma once

#include <format>
#include <string_view>
#include <stdexcept>

namespace bookdb
{
    enum class Genre : char
    {
        Fiction = 0, 
        NonFiction, 
        SciFi, 
        Biography, 
        Mystery, 
        Unknown
    };

    constexpr Genre GenreFromString(std::string_view sv)
    {
        using namespace std::string_view_literals;

        struct CaseNeutralCmp
        {
            // String_view equality check
            bool operator()(std::string_view sv1, 
                std::string_view sv2) const noexcept
            {
                if (sv1.size() != sv2.size()) return false;
                
                for (size_t i = 0; i < sv1.size(); ++i)
                {
                    if (std::tolower(sv1[i]) != 
                        std::tolower(sv2[i])) return false;
                }

                return true;
            };
        };

        CaseNeutralCmp cmp = CaseNeutralCmp{};
        if (cmp(sv, "Fiction"sv)) return Genre::Fiction;
        if (cmp(sv, "NonFiction"sv)) return Genre::NonFiction;
        if (cmp(sv, "SciFi"sv)) return Genre::SciFi;
        if (cmp(sv, "Biography"sv)) return Genre::Biography;
        if (cmp(sv, "Mystery"sv)) return Genre::Mystery;
        if (cmp(sv, "Unknown"sv)) return Genre::Unknown;
        
        throw std::logic_error("Unsupported bookdb::Genre");
    }

    template <typename T>
    constexpr Genre AsGenre(T t)
    {
        static_assert(false, "Unsupported input:"
            "only bookdb::Genre and std::string_view are permitted");
        return Genre::Unknown;
    }

    template <>
    constexpr Genre AsGenre<Genre>(Genre g)
    {
        return g;
    }

    template <>
    constexpr Genre AsGenre<std::string_view>(std::string_view sv)
    {
        return GenreFromString(sv);
    }

    struct Book
    {
        /* string_view для сбережения памяти, чтобы ссылаться на 
        полновесную строку в хранилище */
        std::string_view author;
        std::string title;

        int year;
        Genre genre;
        double rating;
        unsigned read_count;
        
        constexpr Book() : 
            author{}, 
            title{}, 
            year{0}, 
            genre{Genre::Unknown}, 
            rating{0}, 
            read_count{0}
        {}

        template <typename T, 
            std::enable_if<std::is_same_v<T, Genre> || 
                std::is_same_v<T, std::string_view>>* = nullptr>
        constexpr Book(std::string_view title, std::string_view author, 
            int year, T genre, 
            double rating, unsigned read_count) noexcept : 
            title{title}, 
            author{author}, 
            year{year}, 
            genre{AsGenre<T>(genre)}, 
            rating{rating}, 
            read_count{read_count}
        {}
    };
}  // namespace bookdb

namespace std
{
    using namespace bookdb;

    template <>
    struct formatter<Genre, char>
    {
        template <typename FormatCtx>
        FormatCtx::iterator format(Genre g, FormatCtx& fc) const
        {
            std::string genre_str;
            
            switch (g)
            {
                case Genre::Fiction:    genre_str = "Fiction"; break;
                case Genre::Mystery:    genre_str = "Mystery"; break;
                case Genre::NonFiction: genre_str = "NonFiction"; break;
                case Genre::SciFi:      genre_str = "SciFi"; break;
                case Genre::Biography:  genre_str = "Biography"; break;
                case Genre::Unknown:    genre_str = "Unknown"; break;
                default:
                    throw logic_error{"Unsupported bookdb::Genre"};
            }
            
            return format_to(fc.out(), "{}", genre_str);
        }

        constexpr format_parse_context::iterator 
        parse(format_parse_context& ctx)
        {
            return ctx.begin(); // Пользовательский формат пренебрегается
        }
    };

    // Ваш код для std::formatter<Book> здесь
    
    template <>
    struct formatter<Book, char>
    {
        template <typename FormatCtx>
        FormatCtx::iterator format(const Book& book, FormatCtx& fc) const
        {
            return format_to(fc.out(), "{} -- ({}) {}, "
                "genre: {}, rating: {:.2f}, reads: {}", 
                book.author, book.year, book.title, 
                book.genre, book.rating, book.read_count);
        }

        constexpr format_parse_context::iterator
        parse(format_parse_context& ctx)
        {
            return ctx.begin(); // Пользовательский формат пренебрегается
        }
    };
}  // namespace std
