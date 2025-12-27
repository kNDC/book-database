#pragma once

#include "book.hpp"

#include <concepts>
#include <iterator>

namespace bookdb
{
    template <typename T, typename... Args>
    concept BookContainerLike = requires(Args&&... args)
    {
        // В сущности исполнен метод emplace_back
        { T{}.emplace_back(std::forward<Args>(args)...) } ->
            std::same_as<Book&>;

        // Можно определить количество содержащихся элементов
        T{}.size();

        // Сущность должна поддерживать итераторы
        typename T::iterator;
        typename T::const_iterator;

        // В сущности должно быть возможным обращаться к началу и концу
        { T{}.front() } -> std::same_as<Book&>;
        { T{}.back() } -> std::same_as<Book&>;

        // Сущность должна обладать методами begin() и end()
        { T{}.begin() } -> std::same_as<typename T::iterator>;
        { T{}.cbegin() } -> std::same_as<typename T::const_iterator>;
        { T{}.end() } -> std::same_as<typename T::iterator>;
        { T{}.cend() } -> std::same_as<typename T::const_iterator>;
    };

    template <typename It>
    concept BookIterator = requires
    {
        { *It{} } -> std::same_as<Book&>; // Dereferencing
        { It{}.operator->() } -> std::same_as<Book*>; // Using '->'
    };

    template <typename S, typename It>
    concept BookSentinel = BookIterator<It> && requires
    {
        S{}.operator==(It{});
    };

    template <typename P>
    concept BookPredicate = requires(P p)
    {
        /* Предикат принимает книгу, 
        выдаёт логическую переменную */
        { p(Book{}) } -> std::same_as<bool>;
    };

    template <typename C>
    concept BookComparator = requires(C c)
    {
        /* Сравниватель принимает две книги, 
        выдаёт логическую переменную */
        { c(Book{}, Book{}) } -> std::same_as<bool>;
    };
}  // namespace bookdb