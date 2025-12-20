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
        { T().emplace_back(std::forward<Args>(args)...) } ->
            std::same_as<Book&>;

        // Можно определить количество содержащихся элементов
        T().size();

        // Сущность должна поддерживать итераторы
        typename T::iterator;
        typename T::const_iterator;

        // В сущности должно быть возможным обращаться к началу и концу
        { T().front() } -> std::same_as<Book&>;
        { T().back() } -> std::same_as<Book&>;

        // Сущность должна обладать методами begin() и end()
        { T().begin() } -> std::same_as<typename T::iterator>;
        { T().cbegin() } -> std::same_as<typename T::const_iterator>;
        { T().end() } -> std::same_as<typename T::iterator>;
        { T().cend() } -> std::same_as<typename T::const_iterator>;
    };

    template <typename It>
    concept BookIterator = requires(It it)
    {
        { *it } -> std::same_as<Book&>;
        { it.operator->() } -> std::same_as<Book*>;
    };

    template <typename S, typename I>
    concept BookSentinel = true;

    template <typename P>
    concept BookPredicate = requires(P p)
    {
        { p(Book{}) } -> std::same_as<bool>;
    };

    template <typename C>
    concept BookComparator = requires(C c)
    {
        { c(Book{}, Book{}) } -> std::same_as<bool>;
    };
}  // namespace bookdb