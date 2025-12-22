#pragma once

#include "book.hpp"
#include "concepts.hpp"
#include "heterogeneous_lookup.hpp"

#include <print>

#include <string>
#include <string_view>

#include <vector>
#include <list>
#include <unordered_set>

namespace bookdb
{
    template <BookContainerLike BookContainer = std::vector<Book>>
    class BookDatabase
    {
    public:
        // Псевдонимы
        using AuthorContainer = std::list<std::string>;
        using AuthorIndex = std::unordered_set<std::string_view>;

        using iterator = typename BookContainer::iterator;
        using const_iterator = typename BookContainer::const_iterator;

        constexpr BookDatabase() = default;
        constexpr BookDatabase(std::initializer_list<Book> books);
        
        size_t size() const noexcept { return books_.size(); }

        iterator begin() noexcept { return books_.begin(); }
        const_iterator begin() const noexcept { return books_.begin(); }
        const_iterator cbegin() const noexcept { return books_.cbegin(); }

        iterator end() noexcept { return books_.end(); }
        const_iterator end() const noexcept { return books_.end(); }
        const_iterator cend() const noexcept { return books_.cend(); }

        // Ваш код здесь
        const AuthorIndex& GetAuthors() const noexcept { return author_index_; }
        const BookContainer& GetBooks() const noexcept { return books_; }

        template <typename... Args>
        Book& EmplaceBack(Args&&... args);
        
        void PushBack(const Book& book);
        
        void Clear()
        {
            books_.clear();
            authors_.clear();
            author_index_.clear();
        }

    private:
        BookContainer books_;

        AuthorContainer authors_;
        AuthorIndex author_index_;
    };

    template <BookContainerLike BookContainer>
    constexpr BookDatabase<BookContainer>::BookDatabase(std::initializer_list<Book> books)
    {
        for (const Book& book : books)
        {
            EmplaceBack(book);
        }
    }

    template <BookContainerLike BookContainer>
    template <typename... Args>
    Book& BookDatabase<BookContainer>::EmplaceBack(Args&&... args)
    {
        books_.emplace_back(std::forward<Args>(args)...);

        // Проверка, значится ли писатель в БД
        if (AuthorIndex::iterator pos = 
                author_index_.find(books_.back().author); 
            pos != author_index_.end())
        {
            /* Если да, то книга просто привязывается к 
            имеющейся записи о писателе */
            books_.back().author = *pos;
        }
        else
        {
            // Иначе новый писатель вносится в БД
            authors_.emplace_back(books_.back().author);
            author_index_.emplace(authors_.back());
            books_.back().author = authors_.back();
        }
        

        return books_.back();
    }

    template <BookContainerLike BookContainer>
    void BookDatabase<BookContainer>::PushBack(const Book& book)
    {
        EmplaceBack(book);
    }
}  // namespace bookdb

namespace std
{
    using namespace bookdb;

    template <>
    struct formatter<BookDatabase<std::vector<Book>>>
    {
        template <typename FormatCtx>
        auto format(const BookDatabase<std::vector<Book>>& db, 
            FormatCtx& fc) const
        {
            format_to(fc.out(), "BookDatabase (size = {}):\n", db.size());

            format_to(fc.out(), "Books:\n");
            for (const Book& book : db.GetBooks())
            {
                format_to(fc.out(), "- {}\n", book);
            }

            format_to(fc.out(), "\n");

            format_to(fc.out(), "Authors:\n");
            for (string_view author : db.GetAuthors())
            {
                format_to(fc.out(), "- {}\n", author);
            }
            return fc.out();
        }

        constexpr std::format_parse_context::const_iterator 
        parse(format_parse_context& ctx)
        {
            return ctx.begin(); // Пользовательский формат пренебрегается
        }
    };
}  // namespace std