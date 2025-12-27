# Book Database <!-- omit in toc -->
Шаблонный класс для хранения и обработки записей о книгах с набором анализирующих функций.

## Обзор класса и вспомогательных сущностей
* Пространство имён -- bookdb;
* `Genre` - `enum class`, перечень жанров: `Fiction` (художественная литература), `NonFiction` (нехудожественная литература), `SciFi` (научная фантастика), `Biography` (жизнеописания), `Mystery` (загадочное и таинственное), `Unknown` (неизвестный жанр);
* `Book` - класс, с чьей помощью хранятся сведения о книге: писатель, название, жанр (см. выше), год написания, рейтинг и количество прочтений;
* `BookDatabase` - основной класс, чьим шаблоном `BookContainer` выступает используемый контейнер для хранения данных о книгах (по умолчанию std::vector, но может использоваться любой другой, поддерживающий методы `size()`, `front()`, `back()`, `begin()`, `end()`, `cbegin()`, `cend()`).  Предлагает следующие методы:
  * `const std::unordered_set<std::string_view>& GetAuthors()` -- перечень писателей, чьи работы были внесены в класс;
  * `const BookContainer& GetBooks()` -- перечень внесённых книг;
    
  * `template <typename... Args> Book& EmplaceBack()` -- непосредственно инициализирует книгу в хранилище, избегая накладных расходов по копированию хранимых в ней сведений;
  * `void PushBack()` -- добавляет ранее проинициализированную книгу в хранилище;
  * `void Clear()` -- очищает содержимое хранилища;
    
* `BookDatabase` также предоставляет методы, позволяющие использовать класс в алгоритмах STL:
  * `size_t size() const` -- количество хранимых книг;
  * `BookDatabase::iterator begin()` -- возвращает итератор, соответствующий первой хранимой книге;
  * `BookDatabase::iterator end()` -- возвращает итератор, указывающий за пределы множества хранимых книг;
  * `BookDatabase::const_iterator begin() const`, `BookDatabase::const_iterator cbegin() const` -- возвращает `const`-итератор, соответствующий первой хранимой книге;
  * `BookDatabase::const_iterator begin() const`, `BookDatabase::const_iterator cbegin() const` -- возвращает `const`-итератор, указывающий за пределы множества хранимых книг;
  
* В `Genre`, `Book` и `BookDatabase` включена поддержка `std::format` и `std::print`;

## Обзор анализирующих функций
* Пространство имён -- bookdb;
* `template <BookContainerLike T, 
    typename Cmp = TransparentStringLess>
  std::flat_map<std::string_view, unsigned, Cmp>  
  BuildAuthorHistogramFlat(const BookDatabase<T>& db, Cmp cmp = {})` -- производит гистограмму количества включённых в `db` книг по писателям с использованием сравнивающей функции (компаратора) `cmp`;
* `template <BookContainerLike T>
  std::flat_map<Genre, double> 
  CalculateGenreRatings(const BookDatabase<T>& db)` -- подсчитывает распределение рейтингов, взвешенных по количеству прочтений, по представленным в хранилище жанрам; 
* `template <BookContainerLike T>
  constexpr double CalculateAverageRating(const BookDatabase<T>& db)` -- подсчитывает средний рейтинг, взвешенный по количеству прочтений;
* `template <BookContainerLike T>
  BookData SampleRandomBooks(const BookDatabase<T>& db, 
    size_t sample_size)` -- выводит случайную выборку из `sample_size` книг в виде вектора ссылок на книги `BookData = std::vector<std::reference_wrapper<const Book>>`;
* `template <BookContainerLike T, 
    BookComparator Cmp = comp::LessByRating<>>
  BookData GetTopNBy(BookDatabase<T>& db, size_t top_size, 
    Cmp cmp = Cmp{})` -- выводит `top_size` наилучших книг в соответствии со сравнивающей функцией `Cmp = comp::LessByRating<>>` (книги с наилучшим рейтингом по умолчанию);
  
* `auto YearBetween(int from, int to)` -- производит фильтрующую сущность - лямбда-функцию, отбирающую книги по попаданию во временной отрезок `[from; to]`;
* `auto RatingAbove(double threshold)` -- аналогично предыдущему, отсев по рейтингу выше, чем `threshold`;
* `auto GenreIs(Genre genre)` -- аналогично предыдущему, отсев по соответствию требуемому жанру `genre`;
  
* `template <BookPredicate... Ps> auto all_of(Ps... ps)` -- комбинирующая отсеивающая сущность, выбирает книги, соответствующие каждому из условий-предикатов `Ps...`, для каждого из которых определена возможность поставить книге `Book` в соответстие логическое значение (удовлетворяется тремя предыдущими фильтрами);  
* `template <BookPredicate... Ps> auto any_of(Ps... ps)` -- аналогично предыдущему, но отбираются книги удовлетворяющие любому из условий `Ps...`; 
* `template <BookIterator It, BookPredicate P>
  BookData FilterBooks(It from, It to, P&& p)` -- выбирает книги в пределах, заданных итераторами `from` и `to`, соответствующие условию-предикату `p`;
