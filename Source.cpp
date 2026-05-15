#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include <cmath>
#include <set>
#include <windows.h>

using std::cout;
using std::cin;
using std::endl;
using std::string;
using std::vector;
using std::map;
using std::pair;
using std::set;
using std::ifstream;
using std::ofstream;
using std::getline;

// Структура для хранения элемента контента (фильм, сериал, книга, музыка)
struct ContentItem
{
    string title;         // Название
    string type;          // Тип: film, series, book, music
    string genre;         // Жанр из списка 20 жанров
    int ageRating;        // Возрастной рейтинг: 0,6,12,16,18
    string targetGroup;   // Целевая группа: child, teen, young, adult, middle, senior, elderly
    int popularity;       // Популярность от 1 до 10
};

// Структура для хранения оценки пользователя
struct UserRating
{
    string userName;      // Имя пользователя
    string itemTitle;     // Название оцениваемого элемента
    int score;            // Оценка от 1 до 5
};

// Класс рекомендательной системы
class Recommender
{
private:
    vector<ContentItem> database;     // База контента в памяти
    vector<UserRating> allRatings;    // Все оценки всех пользователей
    string databaseFilename;          // Имя файла с базой контента
    string ratingsFilename;           // Имя файла с оценками
    string currentUserName;           // Имя текущего пользователя
    int userAge;                      // Возраст текущего пользователя
    vector<string> preferredGenres;   // Предпочитаемые жанры текущего пользователя
    string userGroup;                 // Группа текущего пользователя (child, teen,...)
    int maxAllowedRating;             // Максимальный возрастной рейтинг, доступный пользователю

    // Загрузка базы контента из текстового файла
    // Формат строки: название|тип|жанр|возраст|группа|популярность
    bool loadDatabase(const string& filename)
    {
        databaseFilename = filename;                  
        ifstream file(filename);                      
        if (!file.is_open()) return false;            // Если файл не открылся, вернуть ошибку
        database.clear();                             // Очистить текущую базу
        string line;                                  // Буфер для чтения строки
        while (getline(file, line))                   // Читать построчно до конца файла
        {
            if (line.empty()) continue;               // Пропустить пустые строки
            std::stringstream ss(line);               // Строковый поток для разбора
            string title, type, genre, targetGroup, ageRatingStr, popularityStr;
            //Извлечение названия, типа, жанра, возрастного рейтинга, целевой группы, популярности
            getline(ss, title, '|');                 
            getline(ss, type, '|');                
            getline(ss, genre, '|');                 
            getline(ss, ageRatingStr, '|');          
            getline(ss, targetGroup, '|');            
            getline(ss, popularityStr, '|');          
            int ageRating = std::stoi(ageRatingStr);  // Преобразовать рейтинг в число
            int popularity = std::stoi(popularityStr);// Преобразовать популярность в число
            // Добавить новый элемент в вектор database
            database.push_back({ title, type, genre, ageRating, targetGroup, popularity });
        }
        file.close();                                 
        return true;                                  
    }

    // Загрузка всех оценок из файла
    // Формат строки: имя_пользователя|название_элемента|оценка
    bool loadRatings(const string& filename)
    {
        ratingsFilename = filename;                   
        ifstream file(filename);                     
        if (!file.is_open()) return true;             
        allRatings.clear();                           // Очистить текущий список оценок
        string line;                                  
        while (getline(file, line))                  
        {
            if (line.empty()) continue;               // Пропустить пустые строки
            std::stringstream ss(line);               // Строковый поток
            string userName, itemTitle, scoreStr;
            //Извлечение имени пользователя, названия элемента, оценки
            getline(ss, userName, '|');               
            getline(ss, itemTitle, '|');             
            getline(ss, scoreStr, '|');              
            int score = std::stoi(scoreStr);          // Преобразовать оценку в число
            allRatings.push_back({ userName, itemTitle, score }); // Добавить в вектор
        }
        file.close();                                
        return true;                                  
    }

    // Добавить одну оценку в файл 
    bool appendRating(const string& userName, const string& itemTitle, int score)
    {
        ofstream file(ratingsFilename, std::ios::app); // Открыть файл для дописывания
        if (!file.is_open()) return false;             
        file << userName << "|" << itemTitle << "|" << score << "\n"; // Записать строку
        file.close();                                  
        return true;                                   
    }

    // Определить группу пользователя и максимальный рейтинг по возрасту
    string determineUserGroup(int age, int& maxRating)
    {
        if (age <= 12) { maxRating = 6;  return "child"; }      // Дети 0-12 лет
        else if (age <= 17) { maxRating = 12; return "teen"; }       // Подростки 13-17
        else if (age <= 25) { maxRating = 18; return "young"; }      // Молодёжь 18-25
        else if (age <= 35) { maxRating = 18; return "adult"; }      // Взрослые 26-35
        else if (age <= 45) { maxRating = 18; return "middle"; }     // Средний возраст 36-45
        else if (age <= 60) { maxRating = 18; return "senior"; }     // Старшее поколение 46-60
        else { maxRating = 18; return "elderly"; }    // Пожилые 60+
    }

    // Обновить поля userGroup и maxAllowedRating в соответствии с текущим userAge
    void updateUserGroupAndRating()
    {
        userGroup = determineUserGroup(userAge, maxAllowedRating);
    }

    // Проверить, подходит ли элемент по возрасту
    bool isAgeAppropriate(const ContentItem& item)
    {
        return (item.ageRating <= maxAllowedRating);
    }

    // Проверить, совпадает ли целевая группа элемента с группой пользователя
    bool isTargetGroupMatch(const ContentItem& item)
    {
        return (item.targetGroup == userGroup);
    }

    // Проверить, есть ли жанр элемента в списке предпочтений пользователя
    bool isGenreMatch(const ContentItem& item)
    {
        return (std::find(preferredGenres.begin(), preferredGenres.end(), item.genre) != preferredGenres.end());
    }

    // Получить ассоциативный массив оценок заданного пользователя (название -> оценка)
    map<string, int> getUserRatingsMap(const string& userName)
    {
        map<string, int> ratings;                     // Пустой контейнер
        for (const UserRating& r : allRatings)        // Перебрать все оценки
        {
            if (r.userName == userName)               // Если оценка принадлежит нужному пользователю
            {
                ratings[r.itemTitle] = r.score;       // Добавить в map
            }
        }
        return ratings;                               // Вернуть результат
    }

    // Вычислить косинусное сходство между двумя пользователями на основе их оценок
    double cosineSimilarity(const map<string, int>& ratingsA, const map<string, int>& ratingsB)
    {
        double dot = 0.0;       // Скалярное произведение векторов оценок
        double normA = 0.0;     // Квадрат нормы вектора A
        double normB = 0.0;     // Квадрат нормы вектора B
        // Вычислить скалярное произведение и квадрат нормы для A
        for (const auto& pairA : ratingsA)
        {
            string item = pairA.first;
            int scoreA = pairA.second;
            auto it = ratingsB.find(item);
            if (it != ratingsB.end())
            {
                dot += scoreA * it->second;           // Добавить вклад в скалярное произведение
            }
            normA += scoreA * scoreA;                 // Накопить квадрат нормы A
        }
        // Вычислить квадрат нормы для B
        for (const auto& pairB : ratingsB)
        {
            normB += pairB.second * pairB.second;
        }
        // Если хотя бы один вектор нулевой, сходство равно 0
        if (normA == 0 || normB == 0) return 0.0;
        return dot / (sqrt(normA) * sqrt(normB));     // Косинус угла между векторами
    }

    // Получить список всех пользователей, кроме текущего
    vector<string> getAllOtherUsers()
    {
        set<string> users;                            // Множество для уникальных имён
        for (const UserRating& r : allRatings)        // Перебрать все оценки
        {
            if (r.userName != currentUserName)        // Исключить текущего пользователя
            {
                users.insert(r.userName);             // Добавить имя в множество
            }
        }
        return vector<string>(users.begin(), users.end()); // Преобразовать множество в вектор
    }

    // Метрика точности: доля рекомендаций, жанр которых совпадает с предпочтениями
    double precision(const vector<string>& recommendations)
    {
        if (recommendations.empty()) return 0.0;      // Если нет рекомендаций, точность 0
        int matches = 0;                              // Счётчик совпадений
        for (const string& title : recommendations)   // Для каждой рекомендованной позиции
        {
            for (const ContentItem& item : database)  // Найти её в базе
            {
                if (item.title == title && isGenreMatch(item))
                {
                    matches++;                        // Жанр совпал, увеличить счётчик
                    break;
                }
            }
        }
        return (double)matches / recommendations.size(); // Вернуть долю
    }

    // Вычислить среднюю популярность рекомендованных элементов
    double avgPopularity(const vector<string>& recommendations)
    {
        if (recommendations.empty()) return 0.0;      // Если нет рекомендаций, вернуть 0
        double sum = 0.0;                             // Сумма популярностей
        int count = 0;                                // Количество найденных элементов
        for (const string& title : recommendations)   // Для каждой позиции
        {
            for (const ContentItem& item : database)  // Найти элемент в базе
            {
                if (item.title == title)
                {
                    sum += item.popularity;           // Добавить популярность
                    count++;                          // Увеличить счётчик
                    break;
                }
            }
        }
        return sum / count;                           // Вернуть среднее
    }

public:
    // Инициализация: загрузить базу контента и оценки из файлов
    bool init(const string& dbFile, const string& ratingsFile)
    {
        if (!loadDatabase(dbFile)) return false;      
        if (!loadRatings(ratingsFile)) return false;  
        return true;                                 
    }

    void setCurrentUser(const string& name) { currentUserName = name; }
    void setUserAge(int age) { userAge = age; updateUserGroupAndRating(); }
    void setPreferredGenres(const vector<string>& genres) { preferredGenres = genres; }

    // Добавление нового контента пользователем
    bool addContentItem()
    {
        ContentItem newItem;                          // Создать временный объект
        cout << "\n--- Добавление нового контента ---\n";
        cout << "Название: ";
        cin.ignore();                                 // Очистить буфер ввода
        getline(cin, newItem.title);                  // Ввести название
        if (newItem.title.empty())                    // Проверить, что название не пустое
        {
            cout << "Название не может быть пустым.\n";
            return false;
        }
        // Проверка на существование такого же названия в базе
        for (const ContentItem& item : database)
        {
            if (item.title == newItem.title)
            {
                cout << "Ошибка: элемент \"" << newItem.title << "\" уже есть в базе.\n";
                return false;
            }
        }
        // Выбор типа контента
        cout << "Тип контента:\n";
        cout << "1 - Фильм\n2 - Сериал\n3 - Книга\n4 - Музыка\n";
        cout << "Ваш выбор (1-4): ";
        int typeCode;
        cin >> typeCode;
        if (typeCode == 1) newItem.type = "film";
        else if (typeCode == 2) newItem.type = "series";
        else if (typeCode == 3) newItem.type = "book";
        else if (typeCode == 4) newItem.type = "music";
        else
        {
            cout << "Неверный тип.\n";
            return false;
        }
        // Список жанров (20 штук) для отображения и выбора
        vector<string> genresList = {
            "комедия", "драма", "фантастика", "ужасы", "детектив",
            "рок", "поп", "хип-хоп", "классика", "спорт", "военный",
            "мистика", "фэнтези", "антиутопия", "сатира", "мультфильм",
            "исторический", "народная", "джаз", "эстрада"
        };
        cout << "Жанр:\n";
        // Вывести список с номерами, по 5 штук в строке
        for (size_t i = 0; i < genresList.size(); ++i)
        {
            cout << i + 1 << ". " << genresList[i] << (i % 5 == 4 ? "\n" : "  ");
        }
        cout << "Введите номер жанра: ";
        int genreIdx;
        cin >> genreIdx;
        if (genreIdx < 1 || genreIdx >(int)genresList.size())
        {
            cout << "Неверный номер.\n";
            return false;
        }
        newItem.genre = genresList[genreIdx - 1];     // Установить жанр из списка
        // Ввод возрастного рейтинга
        cout << "Возрастной рейтинг (0, 6, 12, 16, 18): ";
        int rating;
        cin >> rating;
        if (rating != 0 && rating != 6 && rating != 12 && rating != 16 && rating != 18)
        {
            cout << "Неверный рейтинг.\n";
            return false;
        }
        newItem.ageRating = rating;
        // Выбор целевой группы (цифрой 1..7)
        cout << "Целевая группа:\n";
        cout << "1 - дети (0-12)\n2 - подростки (13-17)\n3 - молодёжь (18-25)\n";
        cout << "4 - взрослые (26-35)\n5 - средний возраст (36-45)\n";
        cout << "6 - старшее поколение (46-60)\n7 - пожилые (60+)\n";
        cout << "Ваш выбор (1-7): ";
        int groupCode;
        cin >> groupCode;
        switch (groupCode)
        {
        case 1: newItem.targetGroup = "child"; break;
        case 2: newItem.targetGroup = "teen"; break;
        case 3: newItem.targetGroup = "young"; break;
        case 4: newItem.targetGroup = "adult"; break;
        case 5: newItem.targetGroup = "middle"; break;
        case 6: newItem.targetGroup = "senior"; break;
        case 7: newItem.targetGroup = "elderly"; break;
        default:
            cout << "Неверный номер.\n";
            return false;
        }
        // Ввод популярности
        cout << "Популярность (1 - очень низкая, 10 - очень высокая): ";
        int pop;
        cin >> pop;
        if (pop < 1 || pop > 10)
        {
            cout << "Популярность от 1 до 10.\n";
            return false;
        }
        newItem.popularity = pop;
        // Добавить элемент в вектор базы данных
        database.push_back(newItem);
        // Дописать элемент в файл database.txt
        ofstream file(databaseFilename, std::ios::app);
        if (!file.is_open()) return false;
        file << newItem.title << "|" << newItem.type << "|" << newItem.genre << "|"
            << newItem.ageRating << "|" << newItem.targetGroup << "|" << newItem.popularity << "\n";
        file.close();
        cout << "Контент успешно добавлен!\n";
        return true;
    }

    // Оценка контента пользователем
    void rateContent()
    {
        while (true)                                  // Бесконечный цикл для повторных оценок
        {
            cout << "\n--- Оценка контента (от 1 до 5) ---\n";
            cout << "Введите часть названия для поиска: ";
            cin.ignore();                             // Очистить буфер перед getline
            string query;
            getline(cin, query);                      // Ввести поисковую строку
            vector<int> matches;                      // Индексы найденных элементов
            for (size_t i = 0; i < database.size(); ++i)
            {
                // Поиск подстроки в названии 
                if (database[i].title.find(query) != string::npos)
                {
                    matches.push_back(i);
                }
            }
            if (matches.empty())
            {
                cout << "Ничего не найдено.\n";
            }
            else
            {
                // Вывести найденные элементы
                cout << "Найдено:\n";
                for (size_t i = 0; i < matches.size() && i < 10; ++i)
                {
                    cout << i + 1 << ". " << database[matches[i]].title
                        << " (" << database[matches[i]].type << ")\n";
                }
                cout << "Выберите номер (0 - отмена): ";
                int idx;
                cin >> idx;
                if (idx != 0 && idx >= 1 && idx <= (int)matches.size())
                {
                    string itemTitle = database[matches[idx - 1]].title;
                    cout << "Ваша оценка (1-5): ";
                    int score;
                    cin >> score;
                    if (score >= 1 && score <= 5)
                    {
                        // Сохранить оценку в память и в файл
                        allRatings.push_back({ currentUserName, itemTitle, score });
                        appendRating(currentUserName, itemTitle, score);
                        cout << "Спасибо! Оценка сохранена.\n";
                    }
                    else
                    {
                        cout << "Оценка должна быть от 1 до 5.\n";
                    }
                }
                else if (idx != 0)
                {
                    cout << "Неверный номер.\n";
                }
            }
            // Спросить, продолжать ли оценивание
            int cont;
            cout << "Оценить ещё? (1 - Да, 0 - Нет): ";
            cin >> cont;
            if (cont != 1) break;                     // Выйти из цикла, если ответ не 1
        }
    }

    // Алгоритм 1: Правила (возраст, целевая группа, жанры)
    vector<string> getRuleBasedRecommendations(const string& contentType)
    {
        vector<string> results;                       // Вектор для результатов
        for (size_t i = 0; i < database.size(); ++i) // Перебрать всю базу
        {
            const ContentItem& item = database[i];
            // Проверить тип, возраст, целевую группу и жанр
            if (item.type == contentType && isAgeAppropriate(item) &&
                isTargetGroupMatch(item) && isGenreMatch(item))
            {
                results.push_back(item.title);        // Добавить название
                if (results.size() >= 5) break;       // Ограничить пятью рекомендациями
            }
        }
        return results;                               // Вернуть список
    }

    // Алгоритм 2: Коллаборативная фильтрация (косинусное сходство)
    vector<string> getCollaborativeRecommendations(const string& contentType)
    {
        // Получить оценки текущего пользователя
        map<string, int> currentRatings = getUserRatingsMap(currentUserName);
        // Если пользователь ничего не оценил, алгоритм не может работать
        if (currentRatings.empty())
        {
            cout << "  [Совет: поставьте несколько оценок, чтобы заработала коллаборативная фильтрация]\n";
            return {};
        }
        // Получить список других пользователей
        vector<string> otherUsers = getAllOtherUsers();
        if (otherUsers.empty()) return {};            // Нет других пользователей
        // Вычислить сходство с каждым другим пользователем
        vector<pair<string, double>> similarities;    // Имя пользователя -> оценка сходства
        for (const string& other : otherUsers)
        {
            map<string, int> otherRatings = getUserRatingsMap(other);
            double sim = cosineSimilarity(currentRatings, otherRatings);
            if (sim > 0.0)                            // Учитывать только положительное сходство
            {
                similarities.push_back({ other, sim });
            }
        }
        // Отсортировать по убыванию сходства
        std::sort(similarities.begin(), similarities.end(),
            [](const pair<string, double>& a, const pair<string, double>& b)
            {
                return a.second > b.second;
            });
        // Выбрать двух ближайших соседей
        vector<string> neighbors;
        for (int i = 0; i < 2 && i < (int)similarities.size(); ++i)
        {
            neighbors.push_back(similarities[i].first);
        }
        if (neighbors.empty()) return {};             // Нет подходящих соседей
        // Множество элементов, уже оценённых текущим пользователем
        set<string> alreadyRated;
        for (const auto& p : currentRatings) alreadyRated.insert(p.first);
        // Собрать элементы, высоко оценённые соседями, но не оценённые текущим
        map<string, int> itemScore;                   // Название -> сумма оценок от соседей
        for (const string& neighbor : neighbors)
        {
            map<string, int> neighborRatings = getUserRatingsMap(neighbor);
            for (const auto& pairRating : neighborRatings)
            {
                const string& title = pairRating.first;
                int score = pairRating.second;
                // Интересуют только оценки 4 и 5
                if (score >= 4 && alreadyRated.find(title) == alreadyRated.end())
                {
                    // Проверить, подходит ли элемент по типу и возрасту
                    for (const ContentItem& item : database)
                    {
                        if (item.title == title && item.type == contentType && isAgeAppropriate(item))
                        {
                            itemScore[title] += score;  
                            break;
                        }
                    }
                }
            }
        }
        // Преобразовать map в вектор для сортировки
        vector<pair<string, int>> sortedItems(itemScore.begin(), itemScore.end());
        std::sort(sortedItems.begin(), sortedItems.end(),
            [](const pair<string, int>& a, const pair<string, int>& b)
            {
                return a.second > b.second;
            });
        // Взять первые 5 названий
        vector<string> results;
        for (size_t i = 0; i < sortedItems.size() && results.size() < 5; ++i)
        {
            results.push_back(sortedItems[i].first);
        }
        return results;
    }

    // Алгоритм 3: Популярность (учитывает только возраст)
    vector<string> getPopularityRecommendations(const string& contentType)
    {
        // Собрать все элементы нужного типа, подходящие по возрасту
        vector<const ContentItem*> candidates;
        for (size_t i = 0; i < database.size(); ++i)
        {
            if (database[i].type == contentType && isAgeAppropriate(database[i]))
            {
                candidates.push_back(&database[i]);    // Добавить указатель
            }
        }
        // Отсортировать по убыванию популярности
        std::sort(candidates.begin(), candidates.end(),
            [](const ContentItem* a, const ContentItem* b)
            {
                return a->popularity > b->popularity;
            });
        // Взять первые 5 названий
        vector<string> results;
        for (size_t i = 0; i < candidates.size() && results.size() < 5; ++i)
        {
            results.push_back(candidates[i]->title);
        }
        return results;
    }

    // Алгоритм 4: Гибридный (взвешенная сумма трёх предыдущих)
    vector<string> getHybridRecommendations(const string& contentType)
    {
        // Получить рекомендации от каждого из трёх алгоритмов
        vector<string> ruleRecs = getRuleBasedRecommendations(contentType);
        vector<string> collabRecs = getCollaborativeRecommendations(contentType);
        vector<string> popRecs = getPopularityRecommendations(contentType);
        // Веса алгоритмов
        const double RULE_WEIGHT = 0.5;
        const double COLLAB_WEIGHT = 0.3;
        const double POP_WEIGHT = 0.2;
        map<string, double> weightMap;                // Название -> итоговый вес
        // Внутренняя функция для добавления весов с учётом позиции в списке
        auto addWeights = [&weightMap](const vector<string>& recs, double algoWeight, double baseScore = 5.0)
            {
                double score = baseScore;                 // Первая позиция получает baseScore
                for (const string& title : recs)
                {
                    weightMap[title] += score * algoWeight; // Прибавить взвешенный вклад
                    score -= 1.0;                         // Следующая позиция на 1 меньше
                    if (score < 1.0) break;               // Не опускаться ниже 1
                }
            };
        // Добавить вклады от каждого алгоритма
        addWeights(ruleRecs, RULE_WEIGHT, 5.0);
        addWeights(collabRecs, COLLAB_WEIGHT, 5.0);
        addWeights(popRecs, POP_WEIGHT, 5.0);
        // Преобразовать map в вектор для сортировки
        vector<pair<string, double>> sorted(weightMap.begin(), weightMap.end());
        std::sort(sorted.begin(), sorted.end(),
            [](const pair<string, double>& a, const pair<string, double>& b)
            {
                return a.second > b.second;
            });
        // Взять первые 5 названий
        vector<string> results;
        for (size_t i = 0; i < sorted.size() && results.size() < 5; ++i)
        {
            results.push_back(sorted[i].first);
        }
        return results;
    }

    // Вывод всех рекомендаций для всех типов контента с метриками
    void printAllRecommendations()
    {
        cout << "\n==================== РЕКОМЕНДАЦИИ ====================\n";
        cout << "Пользователь: " << currentUserName << "\n";
        cout << "Возрастная группа: " << userGroup;
        if (userGroup == "child") cout << " (0-12 лет)\n";
        else if (userGroup == "teen") cout << " (13-17 лет)\n";
        else if (userGroup == "young") cout << " (18-25 лет)\n";
        else if (userGroup == "adult") cout << " (26-35 лет)\n";
        else if (userGroup == "middle") cout << " (36-45 лет)\n";
        else if (userGroup == "senior") cout << " (46-60 лет)\n";
        else cout << " (60+ лет)\n";
        // Список типов контента и их русских названий
        vector<string> types = { "film", "series", "book", "music" };
        vector<string> typeNames = { "Фильмы", "Сериалы", "Книги", "Музыка" };
        // Цикл по четырём типам контента
        for (size_t t = 0; t < types.size(); ++t)
        {
            cout << "\n======== " << typeNames[t] << " ========\n";
            // Лямбда-функция для вывода одного алгоритма
            auto printRecs = [this](const string& algoName, const vector<string>& recs)
                {
                    cout << "--- " << algoName << " ---\n";
                    if (recs.empty())
                    {
                        cout << "  (нет рекомендаций)\n";
                    }
                    else
                    {
                        for (const string& s : recs)
                        {
                            cout << "  - " << s << "\n";
                        }
                    }
                    cout << "  [Точность: " << precision(recs) * 100 << "%]\n";
                    cout << "  [Средняя популярность: " << avgPopularity(recs) << "]\n";
                };
            // Вызвать лямбду для каждого алгоритма
            printRecs("Алгоритм 1 (Правила)", getRuleBasedRecommendations(types[t]));
            printRecs("Алгоритм 2 (Коллаборативная)", getCollaborativeRecommendations(types[t]));
            printRecs("Алгоритм 3 (Популярность)", getPopularityRecommendations(types[t]));
            printRecs("Алгоритм 4 (Гибридный)", getHybridRecommendations(types[t]));
        }
    }
};

// Функция отображения меню жанров
void showGenreMenu()
{
    cout << "\nДоступные жанры (выберите номера через пробел, 0 - конец):\n";
    cout << "1. комедия   2. драма     3. фантастика   4. ужасы\n";
    cout << "5. детектив  6. рок       7. поп          8. хип-хоп\n";
    cout << "9. классика 10. спорт    11. военный     12. мистика\n";
    cout << "13. фэнтези 14. антиутопия 15. сатира    16. мультфильм\n";
    cout << "17. исторический 18. народная  19. джаз   20. эстрада\n";
    cout << "Ваш выбор: ";
}

// Главная функция
int main()
{
    // Настройка кодировки для корректного отображения русского текста в консоли Windows
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    setlocale(LC_ALL, "Russian");

    Recommender recommender;                          // Создать объект рекомендательной системы
    // Инициализация: загрузить базу контента и оценки из файлов
    if (!recommender.init("database.txt", "ratings.txt"))
    {
        cout << "Ошибка: не удалось загрузить database.txt\n";
        return 1;                                    
    }

    string userName;
    cout << "Введите ваше имя (для идентификации): ";
    cin >> userName;
    recommender.setCurrentUser(userName);             // Установить имя текущего пользователя

    // Добавление нового контента (возможность многократного добавления)
    int addChoice;
    cout << "Хотите добавить новый контент? (1 - Да, 0 - Нет): ";
    cin >> addChoice;
    while (addChoice == 1)                            // Пока пользователь хочет добавлять
    {
        recommender.addContentItem();                 // Вызвать метод добавления
        cout << "Добавить ещё? (1 - Да, 0 - Нет): ";
        cin >> addChoice;
    }

    int age = 0;
    cout << "\nДобро пожаловать в рекомендательную систему, " << userName << "!\n";
    cout << "Введите ваш возраст: ";
    cin >> age;
    // Проверка корректности возраста (от 0 до 120)
    while (age < 0 || age > 120)
    {
        cout << "Некорректный возраст, повторите: ";
        cin >> age;
    }
    recommender.setUserAge(age);                      // Установить возраст

    // Список всех жанров (используется для проверки корректности ввода)
    vector<string> allGenres = {
        "комедия", "драма", "фантастика", "ужасы", "детектив",
        "рок", "поп", "хип-хоп", "классика", "спорт", "военный",
        "мистика", "фэнтези", "антиутопия", "сатира", "мультфильм",
        "исторический", "народная", "джаз", "эстрада"
    };
    vector<string> chosenGenres;                      // Выбранные пользователем жанры
    showGenreMenu();                                  // Показать меню жанров
    int choice;
    while (true)
    {
        cin >> choice;
        if (choice == 0) break;                       // 0 - закончить выбор
        if (choice >= 1 && choice <= (int)allGenres.size())
        {
            string genre = allGenres[choice - 1];
            // Проверить, не выбран ли уже этот жанр
            if (std::find(chosenGenres.begin(), chosenGenres.end(), genre) == chosenGenres.end())
            {
                chosenGenres.push_back(genre);
                cout << "Добавлен жанр: " << genre << "\n";
            }
            else
            {
                cout << "Уже выбран.\n";
            }
        }
        else
        {
            cout << "Неверный номер.\n";
        }
        cout << "Следующий (0 - закончить): ";
    }
    if (chosenGenres.empty())                         // Если жанры не выбраны, добавить комедию
    {
        chosenGenres.push_back("комедия");
        cout << "Жанры не выбраны, добавлена комедия по умолчанию.\n";
    }
    recommender.setPreferredGenres(chosenGenres);     // Передать предпочтения в систему

    // Оценивание контента (внутри функции есть свой цикл повторения)
    int rateChoice;
    cout << "\nХотите оценить какой-либо контент (1-5)? (1 - Да, 0 - Нет): ";
    cin >> rateChoice;
    if (rateChoice == 1) recommender.rateContent();

    // Вывести итоговые рекомендации
    recommender.printAllRecommendations();

    cout << "\nНажмите Enter для выхода...";
    cin.ignore();                                  
    cin.get();                                        // Ожидание нажатия клавиши Enter
    return 0;                                         // Завершение работы программы
}
