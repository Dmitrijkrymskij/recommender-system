#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include <cmath>
#include <locale>

using std::cout;
using std::cin;
using std::endl;
using std::string;
using std::vector;
using std::map;
using std::pair;

// Структура элемента контента (фильм/сериал/книга/музыка)
struct ContentItem
{
    string title;        // Название
    string type;         // "film", "series", "book", "music"
    string genre;        // Жанр (комедия, драма, рок ...)
    int ageRating;       // Возрастной рейтинг (0,6,12,16,18)
    string targetGroup;  // Целевая группа: child, teen, young, adult, middle, senior, elderly
    int popularity;      // Популярность от 1 до 10 (для алгоритма популярности)
};

// Структура оценки пользователя (для коллаборативной фильтрации)
struct Rating
{
    int userId;          // ID пользователя (0 - текущий, 1..N - другие)
    int itemId;          // ID элемента контента (индекс в database)
    int score;           // Оценка от 1 до 5 (1 - ужасно, 5 - отлично)
};

// Класс рекомендательной системы
class Recommender
{
private:
    // Основная база контента
    vector<ContentItem> database;
    // Пользователи и их оценки (для коллаборативной фильтрации)
    vector<Rating> allRatings;
    // Возраст текущего пользователя
    int userAge;
    // Предпочитаемые жанры текущего пользователя
    vector<string> preferredGenres;
    // Группа текущего пользователя (вычисляется)
    string userGroup;
    // Максимальный рейтинг по возрасту (0,6,12,16,18)
    int maxAllowedRating;


    // Определение группы пользователя и максимального рейтинга по возрасту
    string determineUserGroup(int age, int& maxRating)
    {
        if (age <= 12)
        {
            maxRating = 6;
            return "child";
        }
        else if (age <= 17)
        {
            maxRating = 12;
            return "teen";
        }
        else if (age <= 25)
        {
            maxRating = 18;
            return "young";
        }
        else if (age <= 35)
        {
            maxRating = 18;
            return "adult";
        }
        else if (age <= 45)
        {
            maxRating = 18;
            return "middle";
        }
        else if (age <= 60)
        {
            maxRating = 18;
            return "senior";
        }
        else
        {
            maxRating = 18;
            return "elderly";
        }
    }

    // Обновление полей userGroup и maxAllowedRating на основе userAge
    void updateUserGroupAndRating()
    {
        userGroup = determineUserGroup(userAge, maxAllowedRating);
    }

    // Проверка, подходит ли элемент контента по возрастному рейтингу
    bool isAgeAppropriate(const ContentItem& item)
    {
        return (item.ageRating <= maxAllowedRating);
    }

    // Проверка, подходит ли элемент по целевой группе
    bool isTargetGroupMatch(const ContentItem& item)
    {
        return (item.targetGroup == userGroup);
    }

    // Проверка, совпадает ли жанр элемента с предпочтениями пользователя
    bool isGenreMatch(const ContentItem& item)
    {
        return (std::find(preferredGenres.begin(), preferredGenres.end(), item.genre) != preferredGenres.end());
    }

public:
    // Конструктор: заполняет базу контента и синтетические оценки пользователей
    Recommender()
    {
        // Заполнение базы данных
        // (Фильмы, сериалы, книги, музыка)
        // Индексы элементов будут от 0 до N-1 (для рейтингов)

        // ---- Фильмы ----
        database.push_back({ "Шрек", "film", "комедия", 0, "child", 9 });
        database.push_back({ "Король Лев", "film", "мультфильм", 0, "child", 10 });
        database.push_back({ "Гарри Поттер и узник Азкабана", "film", "фэнтези", 12, "teen", 9 });
        database.push_back({ "Человек-паук", "film", "фантастика", 12, "teen", 8 });
        database.push_back({ "Брат", "film", "драма", 16, "young", 7 });
        database.push_back({ "Ночной дозор", "film", "фантастика", 12, "young", 6 });
        database.push_back({ "Стиляги", "film", "музыкальный", 12, "young", 7 });
        database.push_back({ "Начало", "film", "фантастика", 16, "young", 9 });
        database.push_back({ "Брат 2", "film", "драма", 16, "adult", 6 });
        database.push_back({ "Холоп", "film", "комедия", 12, "adult", 7 });
        database.push_back({ "Т-34", "film", "военный", 12, "adult", 8 });
        database.push_back({ "Москва слезам не верит", "film", "драма", 12, "middle", 9 });
        database.push_back({ "Иван Васильевич меняет профессию", "film", "комедия", 6, "middle", 10 });
        database.push_back({ "Легенда №17", "film", "спорт", 6, "middle", 8 });
        database.push_back({ "Движение вверх", "film", "спорт", 6, "senior", 7 });
        database.push_back({ "Адмирал", "film", "исторический", 12, "senior", 6 });
        database.push_back({ "Судьба человека", "film", "драма", 12, "elderly", 8 });
        database.push_back({ "Офицеры", "film", "драма", 12, "elderly", 9 });

        // ---- Сериалы ----
        database.push_back({ "Метод", "series", "детектив", 18, "adult", 6 });
        database.push_back({ "Триггер", "series", "драма", 16, "young", 7 });
        database.push_back({ "Полицейский с Рублёвки", "series", "комедия", 16, "adult", 8 });
        database.push_back({ "Ольга", "series", "комедия", 16, "middle", 7 });
        database.push_back({ "Эпидемия", "series", "фантастика", 16, "young", 6 });
        database.push_back({ "Перевал Дятлова", "series", "детектив", 16, "adult", 6 });
        database.push_back({ "Друзья", "series", "комедия", 12, "teen", 9 });
        database.push_back({ "Игра престолов", "series", "фэнтези", 18, "young", 10 });
        database.push_back({ "Корона", "series", "драма", 16, "senior", 8 });

        // ---- Книги ----
        database.push_back({ "Гарри Поттер и филос. камень", "book", "фэнтези", 6, "child", 9 });
        database.push_back({ "Метро 2033", "book", "фантастика", 16, "young", 7 });
        database.push_back({ "Generation П", "book", "сатира", 18, "adult", 5 });
        database.push_back({ "Война и мир", "book", "роман", 12, "middle", 8 });
        database.push_back({ "Мастер и Маргарита", "book", "мистика", 16, "adult", 9 });
        database.push_back({ "Двенадцать стульев", "book", "комедия", 12, "senior", 8 });
        database.push_back({ "Азазель", "book", "детектив", 12, "middle", 7 });
        database.push_back({ "Пикник на обочине", "book", "фантастика", 12, "young", 7 });
        database.push_back({ "1984", "book", "антиутопия", 16, "young", 9 });
        database.push_back({ "Убить пересмешника", "book", "драма", 12, "middle", 8 });

        // ---- Музыка ----
        database.push_back({ "Детские песни", "music", "детская", 0, "child", 7 });
        database.push_back({ "Кино - Группа крови", "music", "рок", 12, "young", 9 });
        database.push_back({ "Oxxxymiron - Горгород", "music", "хип-хоп", 18, "young", 7 });
        database.push_back({ "Face - Юморист", "music", "хип-хоп", 16, "young", 6 });
        database.push_back({ "Ленинград - WWW", "music", "рок", 18, "adult", 8 });
        database.push_back({ "Монеточка - Раскраски", "music", "поп", 12, "adult", 6 });
        database.push_back({ "ДДТ - Что такое осень", "music", "рок", 0, "middle", 9 });
        database.push_back({ "Сплин - Выхода нет", "music", "рок", 0, "middle", 8 });
        database.push_back({ "Чайковский - Лебединое озеро", "music", "классика", 0, "middle", 7 });
        database.push_back({ "Толкунова - Нежность", "music", "эстрада", 0, "senior", 8 });
        database.push_back({ "Зыкина - Течёт река Волга", "music", "народная", 0, "senior", 8 });
        database.push_back({ "Высоцкий - Кони", "music", "авторская", 12, "elderly", 9 });
        database.push_back({ "Кобзон - День Победы", "music", "эстрада", 0, "elderly", 9 });
        database.push_back({ "Queen - Bohemian Rhapsody", "music", "рок", 0, "young", 10 });
        database.push_back({ "The Beatles - Yesterday", "music", "рок", 0, "middle", 10 });
        database.push_back({ "Sinatra - My Way", "music", "джаз", 0, "senior", 9 });

        // Синтетические оценки пользователей
        // Создаём 6 фейковых пользователей (userId = 1..6) с разными предпочтениями
        // Каждый пользователь оценил несколько элементов (itemId – индекс в database)
        // Оценка от 1 до 5.
        // Для простоты используем реальные индексы, которые видны при добавлении (0,1,2,...)

        // Пользователь 1: подросток, любит фэнтези и комедии
        allRatings.push_back({ 1, 2, 5 });  // Гарри Поттер (индекс 2)
        allRatings.push_back({ 1, 3, 4 });  // Человек-паук
        allRatings.push_back({ 1, 22, 4 }); // Друзья
        allRatings.push_back({ 1, 28, 5 }); // Гарри Поттер книга (индекс 28)

    }

    // Второй этап конструктора: после того как database полностью заполнена,
    // добавим оценки (вызывается отдельно после заполнения базы)
    void initRatings()
    {
        // Теперь индексы точно известны. Создадим отображение "название -> индекс" для удобства.
        map<string, int> titleToIndex;
        for (size_t i = 0; i < database.size(); ++i)
        {
            titleToIndex[database[i].title] = i;
        }

        // Пользователь 1 (подросток 15 лет, любит фэнтези)
        allRatings.push_back({ 1, titleToIndex["Гарри Поттер и узник Азкабана"], 5 });
        allRatings.push_back({ 1, titleToIndex["Человек-паук"], 4 });
        allRatings.push_back({ 1, titleToIndex["Друзья"], 5 });
        allRatings.push_back({ 1, titleToIndex["Гарри Поттер и филос. камень"], 5 });

        // Пользователь 2 (молодёжь 20 лет, любит драму и рок)
        allRatings.push_back({ 2, titleToIndex["Брат"], 5 });
        allRatings.push_back({ 2, titleToIndex["Начало"], 4 });
        allRatings.push_back({ 2, titleToIndex["Кино - Группа крови"], 5 });
        allRatings.push_back({ 2, titleToIndex["Игра престолов"], 4 });

        // Пользователь 3 (взрослый 30 лет, комедии и поп)
        allRatings.push_back({ 3, titleToIndex["Холоп"], 4 });
        allRatings.push_back({ 3, titleToIndex["Иван Васильевич меняет профессию"], 5 });
        allRatings.push_back({ 3, titleToIndex["Монеточка - Раскраски"], 4 });

        // Пользователь 4 (средний возраст 40 лет, классика, драма)
        allRatings.push_back({ 4, titleToIndex["Москва слезам не верит"], 5 });
        allRatings.push_back({ 4, titleToIndex["Война и мир"], 5 });
        allRatings.push_back({ 4, titleToIndex["Чайковский - Лебединое озеро"], 4 });

        // Пользователь 5 (старшее поколение 55 лет, историческое, народная)
        allRatings.push_back({ 5, titleToIndex["Адмирал"], 5 });
        allRatings.push_back({ 5, titleToIndex["Зыкина - Течёт река Волга"], 5 });

        // Пользователь 6 (пожилой 70 лет, военное, авторская песня)
        allRatings.push_back({ 6, titleToIndex["Судьба человека"], 5 });
        allRatings.push_back({ 6, titleToIndex["Офицеры"], 5 });
        allRatings.push_back({ 6, titleToIndex["Высоцкий - Кони"], 5 });
    }


    // Установка возраста и жанров текущего пользователя
    void setUserAge(int age)
    {
        userAge = age;
        updateUserGroupAndRating();
    }

    void setPreferredGenres(const vector<string>& genres)
    {
        preferredGenres = genres;
    }

    // Алгоритм 1: На основе правил (возраст + жанры + целевая группа)
    // Возвращает вектор названий 

    vector<string> getRuleBasedRecommendations(const string& contentType)
    {
        vector<string> results;
        for (size_t i = 0; i < database.size(); ++i)
        {
            const ContentItem& item = database[i];
            if (item.type == contentType && isAgeAppropriate(item) && isTargetGroupMatch(item) && isGenreMatch(item))
            {
                results.push_back(item.title);
                if (results.size() >= 5) break;
            }
        }
        return results;
    }

    // Алгоритм 2: Коллаборативная фильтрация 
    // Находит топ-2 пользователей, наиболее похожих на текущего (по возрасту и жанрам),
    // затем рекомендует элементы, которые они высоко оценили (>=4), но текущий ещё не оценивал.
    // Возвращает вектор названий, отсортированных по сумме оценок соседей.
    vector<string> getCollaborativeRecommendations(const string& contentType)
    {
        // Используем предопределённые характеристики фейковых пользователей.
        // Создадим структуру "синтетический пользователь" с возрастом, жанрами и оценками.
        struct SyntheticUser
        {
            int id;
            int age;
            vector<string> genres;
            vector<Rating> ratings; // оценки только этого пользователя
        };

        // Определим 6 фейковых пользователей с возрастом и любимыми жанрами
        vector<SyntheticUser> synthUsers;
        // Пользователь 1: подросток 15 лет, жанры: фэнтези, комедия
        synthUsers.push_back({ 1, 15, {"фэнтези", "комедия"}, {} });
        // Пользователь 2: 20 лет, жанры: драма, рок
        synthUsers.push_back({ 2, 20, {"драма", "рок"}, {} });
        // Пользователь 3: 30 лет, жанры: комедия, поп
        synthUsers.push_back({ 3, 30, {"комедия", "поп"}, {} });
        // Пользователь 4: 40 лет, жанры: драма, классика
        synthUsers.push_back({ 4, 40, {"драма", "классика"}, {} });
        // Пользователь 5: 55 лет, жанры: исторический, народная
        synthUsers.push_back({ 5, 55, {"исторический", "народная"}, {} });
        // Пользователь 6: 70 лет, жанры: военный, авторская
        synthUsers.push_back({ 6, 70, {"военный", "авторская"}, {} });

        // Заполним оценки для каждого синтетического пользователя из allRatings
        for (const Rating& r : allRatings)
        {
            for (SyntheticUser& su : synthUsers)
            {
                if (su.id == r.userId)
                {
                    su.ratings.push_back(r);
                    break;
                }
            }
        }

        // Вычислим сходство текущего пользователя с каждым синтетическим.
        // Метрика: разница в возрасте (чем меньше, тем лучше) + количество общих жанров.
        vector<pair<int, double>> similarities; // (userId, score)
        for (const SyntheticUser& su : synthUsers)
        {
            double ageDiff = std::abs(userAge - su.age) / 10.0; // нормализуем
            int commonGenres = 0;
            for (const string& g : su.genres)
            {
                if (std::find(preferredGenres.begin(), preferredGenres.end(), g) != preferredGenres.end())
                {
                    commonGenres++;
                }
            }
            double genreSimilarity = commonGenres / (double)std::max(su.genres.size(), preferredGenres.size());
            // Итоговая близость: чем меньше ageDiff и больше genreSimilarity, тем лучше.
            // Преобразуем в оценку: score = genreSimilarity - ageDiff*0.1.
            double score = genreSimilarity - ageDiff * 0.1;
            similarities.push_back({ su.id, score });
        }

        // Сортируем по убыванию score
        std::sort(similarities.begin(), similarities.end(),
            [](const pair<int, double>& a, const pair<int, double>& b) { return a.second > b.second; });

        // Берём двух ближайших соседей
        vector<int> neighborIds;
        for (int i = 0; i < 2 && i < (int)similarities.size(); ++i)
        {
            neighborIds.push_back(similarities[i].first);
        }

        // Собираем все оценки соседей для элементов заданного contentType
        map<string, int> itemScore; // название -> суммарная оценка
        for (int nid : neighborIds)
        {
            for (const SyntheticUser& su : synthUsers)
            {
                if (su.id == nid)
                {
                    for (const Rating& r : su.ratings)
                    {
                        if (r.score >= 4) // берём только высокие оценки
                        {
                            const ContentItem& item = database[r.itemId];
                            if (item.type == contentType && isAgeAppropriate(item)) // также проверяем возраст
                            {
                                itemScore[item.title] += r.score;
                            }
                        }
                    }
                    break;
                }
            }
        }

        // Преобразуем в вектор и сортируем по убыванию суммы
        vector<pair<string, int>> sortedItems(itemScore.begin(), itemScore.end());
        std::sort(sortedItems.begin(), sortedItems.end(),
            [](const pair<string, int>& a, const pair<string, int>& b) { return a.second > b.second; });

        vector<string> results;
        for (size_t i = 0; i < sortedItems.size() && results.size() < 5; ++i)
        {
            results.push_back(sortedItems[i].first);
        }
        return results;
    }


    // Алгоритм 3: По популярности (с учетом возрастного рейтинга)
    // Возвращает топ-5 самых популярных элементов типа contentType,
    // подходящих по возрасту (без учёта жанров и целевой группы)
    vector<string> getPopularityRecommendations(const string& contentType)
    {
        // Собираем все элементы нужного типа, подходящие по возрасту
        vector<const ContentItem*> candidates;
        for (size_t i = 0; i < database.size(); ++i)
        {
            const ContentItem& item = database[i];
            if (item.type == contentType && isAgeAppropriate(item))
            {
                candidates.push_back(&item);
            }
        }
        // Сортируем по убыванию popularity
        std::sort(candidates.begin(), candidates.end(),
            [](const ContentItem* a, const ContentItem* b) { return a->popularity > b->popularity; });

        vector<string> results;
        for (size_t i = 0; i < candidates.size() && results.size() < 5; ++i)
        {
            results.push_back(candidates[i]->title);
        }
        return results;
    }

    
    // Вывод всех рекомендаций 
    void printAllRecommendations()
    {
        cout << "\n==================== РЕКОМЕНДАЦИИ ====================\n";
        cout << "Ваша возрастная группа: " << userGroup << "\n";
        // Преобразование группы в читаемый вид
        if (userGroup == "child") cout << " (0-12 лет)\n";
        else if (userGroup == "teen") cout << " (13-17 лет)\n";
        else if (userGroup == "young") cout << " (18-25 лет)\n";
        else if (userGroup == "adult") cout << " (26-35 лет)\n";
        else if (userGroup == "middle") cout << " (36-45 лет)\n";
        else if (userGroup == "senior") cout << " (46-60 лет)\n";
        else cout << " (60+ лет)\n";

        vector<string> types = { "film", "series", "book", "music" };
        vector<string> typeNames = { "Фильмы", "Сериалы", "Книги", "Музыка" };

        // Для каждого типа контента выводим результаты трёх алгоритмов
        for (size_t t = 0; t < types.size(); ++t)
        {
            cout << "\n======== " << typeNames[t] << " ========\n";

            // Алгоритм 1: Правила
            cout << "--- Алгоритм 1 (Правила) ---\n";
            vector<string> recs1 = getRuleBasedRecommendations(types[t]);
            if (recs1.empty()) cout << "  (нет рекомендаций)\n";
            else for (const string& s : recs1) cout << "  - " << s << "\n";

            // Алгоритм 2: Коллаборативная фильтрация
            cout << "--- Алгоритм 2 (Коллаборативная) ---\n";
            vector<string> recs2 = getCollaborativeRecommendations(types[t]);
            if (recs2.empty()) cout << "  (нет рекомендаций)\n";
            else for (const string& s : recs2) cout << "  - " << s << "\n";

            // Алгоритм 3: Популярность
            cout << "--- Алгоритм 3 (Популярность) ---\n";
            vector<string> recs3 = getPopularityRecommendations(types[t]);
            if (recs3.empty()) cout << "  (нет рекомендаций)\n";
            else for (const string& s : recs3) cout << "  - " << s << "\n";
        }

        // Сравнительные комментарии
        cout << "\n========== Описание алгоритмов ==========\n";
        cout << "Алгоритм 1 (Правила): Учитывает ваш возраст, жанры и целевую группу контента.\n";
        cout << "Алгоритм 2 (Коллаборативный): Использует оценки похожих пользователей (синтетическая база).\n";
        cout << "Алгоритм 3 (Популярный): Рекомендует самое популярное, без персонализации.\n";
    }
};

// Функция вывода меню жанров
void showGenreMenu()
{
    cout << "\nДоступные жанры (выберите номера через пробел, 0 - конец):\n";
    cout << "1. комедия   2. драма     3. фантастика   4. ужасы\n";
    cout << "5. детектив  6. рок       7. поп          8. хип-хоп\n";
    cout << "9. классика 10. спорт    11. военный     12. мистика\n";
    cout << "13. фэнтези 14. антиутопия 15. сатира    16. мультфильм\n";
    cout << "17. исторический 18. народная  19. авторская  20. джаз\n";
    cout << "21. эстрада 22. музыкальный 23. роман\n";
    cout << "Ваш выбор: ";
}

int main()
{
    setlocale(LC_ALL, "Russian"); //Подключение русского языка

    Recommender recommender;
    // Инициализируем синтетические оценки после заполнения базы
    recommender.initRatings();

    int age = 0;
    cout << "Добро пожаловать в рекомендательную систему!\n";
    cout << "Введите ваш возраст: ";
    cin >> age;
    while (age < 0 || age > 120)
    {
        cout << "Некорректный возраст: ";
        cin >> age;
    }
    recommender.setUserAge(age);

    // Список всех возможных жанров (для проверки)
    vector<string> allGenres = {
        "комедия", "драма", "фантастика", "ужасы", "детектив",
        "рок", "поп", "хип-хоп", "классика", "спорт", "военный",
        "мистика", "фэнтези", "антиутопия", "сатира", "мультфильм",
        "исторический", "народная", "авторская", "джаз", "эстрада",
        "музыкальный", "роман"
    };
    vector<string> chosenGenres;

    showGenreMenu();
    int choice;
    while (true)
    {
        cin >> choice;
        if (choice == 0) break;
        if (choice >= 1 && choice <= (int)allGenres.size())
        {
            string genre = allGenres[choice - 1];
            if (std::find(chosenGenres.begin(), chosenGenres.end(), genre) == chosenGenres.end())
            {
                chosenGenres.push_back(genre);
                cout << "Добавлен жанр: " << genre << "\n";
            }
            else cout << "Уже выбран.\n";
        }
        else cout << "Неверный номер.\n";
        cout << "Следующий (0 - закончить): ";
    }
    if (chosenGenres.empty())
    {
        chosenGenres.push_back("комедия");
        cout << "Добавлена комедия по умолчанию.\n";
    }
    recommender.setPreferredGenres(chosenGenres);

    recommender.printAllRecommendations();

    cout << "\nНажмите Enter для выхода.";
    cin.ignore();
    cin.get();
    return 0; //Завершение работы программы
}