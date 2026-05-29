#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include <cmath>
#include <set>
#include <random>
#include <chrono>
#include <windows.h>

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

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
    string title;         // Название элемента контента (фильма, сериала, книги или музыки)
    string type;          // Тип контента: "film", "series", "book", "music"
    string genre;         // Жанр элемента (из списка 20 жанров)
    int ageRating;        // Возрастной рейтинг: 0,6,12,16,18
    string targetGroup;   // Целевая возрастная группа: child, teen, young, adult, middle, senior, elderly
    int popularity;       // Популярность от 1 до 10 (чем выше, тем популярнее)
};

// Структура для хранения оценки пользователя
struct UserRating
{
    string userName;      // Имя пользователя, который поставил оценку
    string itemTitle;     // Название оцениваемого элемента контента
    int score;            // Оценка от 1 до 5 (1 – очень плохо, 5 – отлично)
};

class Recommender
{
private:
    vector<ContentItem> database;     // База всех элементов контента (загружается из файла)
    vector<UserRating> allRatings;    // Список всех оценок всех пользователей
    string databaseFilename;          // Имя файла, откуда загружается база контента
    string ratingsFilename;           // Имя файла, куда сохраняются и откуда загружаются оценки
    string currentUserName;           // Имя текущего пользователя сессии
    int userAge;                      // Возраст текущего пользователя 
    vector<string> preferredGenres;   // Список жанров, которые выбрал пользователь
    string userGroup;                 // Группа текущего пользователя (child, teen, young, adult, middle, senior, elderly)
    int maxAllowedRating;             // Максимальный возрастной рейтинг, доступный пользователю (0,6,12,16,18)

    // Загружает базу контента из текстового файла. Формат: название|тип|жанр|возраст|группа|популярность
    bool loadDatabase(const string& filename)
    {
        databaseFilename = filename;
        ifstream file(filename);
        if (!file.is_open())
        {
            return false;
        }
        database.clear();
        string line;
        while (getline(file, line))
        {
            if (line.empty())
            {
                continue;
            }
            std::stringstream ss(line);
            string title, type, genre, targetGroup, ageRatingStr, popularityStr;
            getline(ss, title, '|');
            getline(ss, type, '|');
            getline(ss, genre, '|');
            getline(ss, ageRatingStr, '|');
            getline(ss, targetGroup, '|');
            getline(ss, popularityStr, '|');
            int ageRating = std::stoi(ageRatingStr);
            int popularity = std::stoi(popularityStr);
            database.push_back({ title, type, genre, ageRating, targetGroup, popularity });
        }
        file.close();
        return true;
    }

    // Загружает все оценки из файла. Формат: имя_пользователя|название_элемента|оценка
    bool loadRatings(const string& filename)
    {
        ratingsFilename = filename;
        ifstream file(filename);
        if (!file.is_open())
        {
            return true;
        }
        allRatings.clear();
        string line;
        while (getline(file, line))
        {
            if (line.empty())
            {
                continue;
            }
            std::stringstream ss(line);
            string userName, itemTitle, scoreStr;
            getline(ss, userName, '|');
            getline(ss, itemTitle, '|');
            getline(ss, scoreStr, '|');
            int score = std::stoi(scoreStr);
            allRatings.push_back({ userName, itemTitle, score });
        }
        file.close();
        return true;
    }

    // Добавляет одну оценку в конец файла
    bool appendRating(const string& userName, const string& itemTitle, int score)
    {
        ofstream file(ratingsFilename, std::ios::app);
        if (!file.is_open())
        {
            return false;
        }
        file << userName << "|" << itemTitle << "|" << score << "\n";
        file.close();
        return true;
    }

    // Определяет группу пользователя и максимальный рейтинг по возрасту
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

    // Обновляет группу и максимальный рейтинг при изменении возраста
    void updateUserGroupAndRating()
    {
        userGroup = determineUserGroup(userAge, maxAllowedRating);
    }

    // Проверяет, подходит ли элемент по возрасту
    bool isAgeAppropriate(const ContentItem& item)
    {
        return (item.ageRating <= maxAllowedRating);
    }

    // Проверяет, совпадает ли целевая группа элемента с группой пользователя
    bool isTargetGroupMatch(const ContentItem& item)
    {
        return (item.targetGroup == userGroup);
    }

    // Проверяет, есть ли жанр элемента в списке предпочтений пользователя
    bool isGenreMatch(const ContentItem& item)
    {
        return (std::find(preferredGenres.begin(), preferredGenres.end(), item.genre) != preferredGenres.end());
    }

    // Возвращает ассоциативный массив оценок пользователя (название -> оценка)
    map<string, int> getUserRatingsMap(const string& userName)
    {
        map<string, int> ratings;
        for (const UserRating& r : allRatings)
        {
            if (r.userName == userName)
            {
                ratings[r.itemTitle] = r.score;
            }
        }
        return ratings;
    }

    // Возвращает множество релевантных элементов пользователя (оценка >= 4)
    set<string> getRelevantItems(const string& userName)
    {
        set<string> relevant;
        for (const UserRating& r : allRatings)
        {
            if (r.userName == userName && r.score >= 4)
            {
                relevant.insert(r.itemTitle);
            }
        }
        return relevant;
    }

    // Вычисляет косинусное сходство между двумя пользователями на основе их оценок
    double cosineSimilarity(const map<string, int>& ratingsA, const map<string, int>& ratingsB)
    {
        double dot = 0.0, normA = 0.0, normB = 0.0;
        for (const auto& pairA : ratingsA)
        {
            string item = pairA.first;
            int scoreA = pairA.second;
            auto it = ratingsB.find(item);
            if (it != ratingsB.end())
            {
                dot += scoreA * it->second;
            }
            normA += scoreA * scoreA;
        }
        for (const auto& pairB : ratingsB)
        {
            normB += pairB.second * pairB.second;
        }
        if (normA == 0 || normB == 0)
        {
            return 0.0;
        }
        return dot / (sqrt(normA) * sqrt(normB));
    }

    // Получает список всех пользователей, кроме текущего
    vector<string> getAllOtherUsers()
    {
        set<string> users;
        for (const UserRating& r : allRatings)
        {
            if (r.userName != currentUserName)
            {
                users.insert(r.userName);
            }
        }
        return vector<string>(users.begin(), users.end());
    }

    // Precision@5: доля релевантных элементов среди первых 5 рекомендаций
    double precisionAt5(const vector<string>& recommendations, const set<string>& relevant)
    {
        if (recommendations.empty())
        {
            return 0.0;
        }
        int hits = 0;
        for (size_t i = 0; i < recommendations.size() && i < 5; ++i)
        {
            if (relevant.find(recommendations[i]) != relevant.end())
            {
                hits++;
            }
        }
        return (double)hits / (std::min)(5, (int)recommendations.size());
    }

    // Recall@5: доля релевантных элементов, попавших в топ-5, от всех релевантных
    double recallAt5(const vector<string>& recommendations, const set<string>& relevant)
    {
        if (relevant.empty())
        {
            return 0.0;
        }
        int hits = 0;
        for (size_t i = 0; i < recommendations.size() && i < 5; ++i)
        {
            if (relevant.find(recommendations[i]) != relevant.end())
            {
                hits++;
            }
        }
        return (double)hits / relevant.size();
    }

    // HitRate@5: 1 если есть хотя бы одна релевантная рекомендация в топ-5, иначе 0
    double hitRateAt5(const vector<string>& recommendations, const set<string>& relevant)
    {
        for (size_t i = 0; i < recommendations.size() && i < 5; ++i)
        {
            if (relevant.find(recommendations[i]) != relevant.end())
            {
                return 1.0;
            }
        }
        return 0.0;
    }

    // NDCG@5: нормализованный дисконтированный кумулятивный выигрыш (учитывает позицию)
    double ndcgAt5(const vector<string>& recommendations, const set<string>& relevant)
    {
        if (recommendations.empty())
        {
            return 0.0;
        }
        double dcg = 0.0;
        for (size_t i = 0; i < recommendations.size() && i < 5; ++i)
        {
            double gain = (relevant.find(recommendations[i]) != relevant.end()) ? 1.0 : 0.0;
            if (i == 0)
            {
                dcg += gain;
            }
            else
            {
                dcg += gain / log2(i + 1);
            }
        }
        double idcg = 0.0;
        for (size_t i = 0; i < 5 && i < relevant.size(); ++i)
        {
            if (i == 0)
            {
                idcg += 1.0;
            }
            else
            {
                idcg += 1.0 / log2(i + 1);
            }
        }
        if (idcg == 0.0)
        {
            return 0.0;
        }
        return dcg / idcg;
    }

    // Вычисляет среднюю популярность рекомендованных элементов
    double avgPopularity(const vector<string>& recommendations)
    {
        if (recommendations.empty())
        {
            return 0.0;
        }
        double sum = 0.0;
        int count = 0;
        for (const string& title : recommendations)
        {
            for (const ContentItem& item : database)
            {
                if (item.title == title)
                {
                    sum += item.popularity;
                    count++;
                    break;
                }
            }
        }
        return sum / count;
    }

public:
    // Инициализация: загружает базу контента и оценки из файлов
    bool init(const string& dbFile, const string& ratingsFile)
    {
        if (!loadDatabase(dbFile))
        {
            return false;
        }
        if (!loadRatings(ratingsFile))
        {
            return false;
        }
        return true;
    }

    void setCurrentUser(const string& name)
    {
        currentUserName = name;
    }

    void setUserAge(int age)
    {
        userAge = age;
        updateUserGroupAndRating();
    }

    void setPreferredGenres(const vector<string>& genres)
    {
        preferredGenres = genres;
    }

    // Добавляет новый контент (с проверкой дубликата и вводом всех параметров)
    bool addContentItem()
    {
        ContentItem newItem;
        cout << "\n--- Добавление нового контента ---\n";
        cout << "Название: ";
        cin.ignore();
        getline(cin, newItem.title);
        if (newItem.title.empty())
        {
            cout << "Название не может быть пустым.\n";
            return false;
        }
        for (const ContentItem& item : database)
        {
            if (item.title == newItem.title)
            {
                cout << "Ошибка: элемент \"" << newItem.title << "\" уже есть в базе.\n";
                return false;
            }
        }
        cout << "Тип контента:\n";
        cout << "1 - Фильм\n2 - Сериал\n3 - Книга\n4 - Музыка\n";
        cout << "Ваш выбор (1-4): ";
        int typeCode;
        cin >> typeCode;
        if (typeCode == 1)
        {
            newItem.type = "film";
        }
        else if (typeCode == 2)
        {
            newItem.type = "series";
        }
        else if (typeCode == 3)
        {
            newItem.type = "book";
        }
        else if (typeCode == 4)
        {
            newItem.type = "music";
        }
        else
        {
            cout << "Неверный тип.\n";
            return false;
        }

        vector<string> genresList = {
            "комедия", "драма", "фантастика", "ужасы", "детектив",
            "рок", "поп", "хип-хоп", "классика", "спорт", "военный",
            "мистика", "фэнтези", "антиутопия", "сатира", "мультфильм",
            "исторический", "народная", "джаз", "эстрада"
        };
        cout << "Жанр:\n";
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
        newItem.genre = genresList[genreIdx - 1];

        cout << "Возрастной рейтинг (0, 6, 12, 16, 18): ";
        int rating;
        cin >> rating;
        if (rating != 0 && rating != 6 && rating != 12 && rating != 16 && rating != 18)
        {
            cout << "Неверный рейтинг.\n";
            return false;
        }
        newItem.ageRating = rating;

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

        cout << "Популярность (1 - очень низкая, 10 - очень высокая): ";
        int pop;
        cin >> pop;
        if (pop < 1 || pop > 10)
        {
            cout << "Популярность от 1 до 10.\n";
            return false;
        }
        newItem.popularity = pop;

        database.push_back(newItem);
        ofstream file(databaseFilename, std::ios::app);
        if (!file.is_open())
        {
            return false;
        }
        file << newItem.title << "|" << newItem.type << "|" << newItem.genre << "|"
            << newItem.ageRating << "|" << newItem.targetGroup << "|" << newItem.popularity << "\n";
        file.close();
        cout << "Контент успешно добавлен!\n";
        return true;
    }

    // Предлагает пользователю оценить случайные элементы (можно пропустить, введя 0)
    void autoRateRandomItems()
    {
        set<string> alreadyRated;
        for (const UserRating& r : allRatings)
        {
            if (r.userName == currentUserName)
            {
                alreadyRated.insert(r.itemTitle);
            }
        }

        vector<const ContentItem*> candidates;
        for (const ContentItem& item : database)
        {
            if (alreadyRated.find(item.title) == alreadyRated.end())
            {
                candidates.push_back(&item);
            }
        }
        if (candidates.empty())
        {
            cout << "Вы уже оценили все элементы. Спасибо!\n";
            return;
        }

        unsigned seed = static_cast<unsigned>(std::chrono::system_clock::now().time_since_epoch().count());
        std::shuffle(candidates.begin(), candidates.end(), std::default_random_engine(seed));

        int needed = 5;
        size_t index = 0;
        cout << "\n--- Оцените несколько случайных элементов (от 1 до 5) ---\n";
        cout << "Если вы не знакомы с элементом, введите 0 - он будет пропущен.\n";
        cout << "Это поможет системе лучше понять ваши предпочтения.\n";

        while (needed > 0 && index < candidates.size())
        {
            const ContentItem* item = candidates[index];
            cout << "Элемент: " << item->title << " (" << item->type << ")\n";
            cout << "Осталось получить оценок: " << needed << "\n";
            cout << "Ваша оценка (1-5) или 0 (пропустить): ";
            int score;
            cin >> score;
            if (score == 0)
            {
                cout << "Пропускаем.\n";
                index++;
                continue;
            }
            if (score < 1 || score > 5)
            {
                cout << "Неверная оценка. Введите число от 1 до 5 или 0 для пропуска.\n";
                continue;
            }
            allRatings.push_back({ currentUserName, item->title, score });
            appendRating(currentUserName, item->title, score);
            cout << "Спасибо! Оценка сохранена.\n";
            needed--;
            index++;
        }

        if (needed > 0)
        {
            cout << "Недостаточно новых элементов. Получено " << (5 - needed) << " оценок.\n";
        }
        else
        {
            cout << "Оценивание завершено. Теперь рекомендации будут точнее.\n";
        }
    }

    // Ручная оценка контента по поиску части названия
    void rateContent()
    {
        while (true)
        {
            cout << "\n--- Оценка контента (от 1 до 5) ---\n";
            cout << "Введите часть названия для поиска: ";
            cin.ignore();
            string query;
            getline(cin, query);
            vector<int> matches;
            for (size_t i = 0; i < database.size(); ++i)
            {
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
            int cont;
            cout << "Оценить ещё? (1 - Да, 0 - Нет): ";
            cin >> cont;
            if (cont != 1)
            {
                break;
            }
        }
    }

    // Алгоритм 1: Правила (возраст, целевая группа, жанры)
    vector<string> getRuleBasedRecommendations(const string& contentType)
    {
        vector<string> results;
        for (size_t i = 0; i < database.size(); ++i)
        {
            const ContentItem& item = database[i];
            if (item.type == contentType && isAgeAppropriate(item) &&
                isTargetGroupMatch(item) && isGenreMatch(item))
            {
                results.push_back(item.title);
                if (results.size() >= 5)
                {
                    break;
                }
            }
        }
        return results;
    }

    // Алгоритм 2: Коллаборативная фильтрация (косинусное сходство)
    vector<string> getCollaborativeRecommendations(const string& contentType)
    {
        map<string, int> currentRatings = getUserRatingsMap(currentUserName);
        if (currentRatings.empty())
        {
            cout << "  [Совет: поставьте несколько оценок, чтобы заработала коллаборативная фильтрация]\n";
            return {};
        }
        vector<string> otherUsers = getAllOtherUsers();
        if (otherUsers.empty())
        {
            return {};
        }
        vector<pair<string, double>> similarities;
        for (const string& other : otherUsers)
        {
            map<string, int> otherRatings = getUserRatingsMap(other);
            double sim = cosineSimilarity(currentRatings, otherRatings);
            if (sim > 0.0)
            {
                similarities.push_back({ other, sim });
            }
        }
        std::sort(similarities.begin(), similarities.end(),
            [](const pair<string, double>& a, const pair<string, double>& b)
            {
                return a.second > b.second;
            });
        vector<string> neighbors;
        for (int i = 0; i < 2 && i < (int)similarities.size(); ++i)
        {
            neighbors.push_back(similarities[i].first);
        }
        if (neighbors.empty())
        {
            return {};
        }
        set<string> alreadyRated;
        for (const auto& p : currentRatings)
        {
            alreadyRated.insert(p.first);
        }
        map<string, int> itemScore;
        for (const string& neighbor : neighbors)
        {
            map<string, int> neighborRatings = getUserRatingsMap(neighbor);
            for (const auto& pairRating : neighborRatings)
            {
                const string& title = pairRating.first;
                int score = pairRating.second;
                if (score >= 4 && alreadyRated.find(title) == alreadyRated.end())
                {
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
        vector<pair<string, int>> sortedItems(itemScore.begin(), itemScore.end());
        std::sort(sortedItems.begin(), sortedItems.end(),
            [](const pair<string, int>& a, const pair<string, int>& b)
            {
                return a.second > b.second;
            });
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
        vector<const ContentItem*> candidates;
        for (size_t i = 0; i < database.size(); ++i)
        {
            if (database[i].type == contentType && isAgeAppropriate(database[i]))
            {
                candidates.push_back(&database[i]);
            }
        }
        std::sort(candidates.begin(), candidates.end(),
            [](const ContentItem* a, const ContentItem* b)
            {
                return a->popularity > b->popularity;
            });
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
        vector<string> ruleRecs = getRuleBasedRecommendations(contentType);
        vector<string> collabRecs = getCollaborativeRecommendations(contentType);
        vector<string> popRecs = getPopularityRecommendations(contentType);
        const double RULE_WEIGHT = 0.5;
        const double COLLAB_WEIGHT = 0.3;
        const double POP_WEIGHT = 0.2;
        map<string, double> weightMap;
        auto addWeights = [&weightMap](const vector<string>& recs, double algoWeight, double baseScore = 5.0)
            {
                double score = baseScore;
                for (const string& title : recs)
                {
                    weightMap[title] += score * algoWeight;
                    score -= 1.0;
                    if (score < 1.0)
                    {
                        break;
                    }
                }
            };
        addWeights(ruleRecs, RULE_WEIGHT, 5.0);
        addWeights(collabRecs, COLLAB_WEIGHT, 5.0);
        addWeights(popRecs, POP_WEIGHT, 5.0);
        vector<pair<string, double>> sorted(weightMap.begin(), weightMap.end());
        std::sort(sorted.begin(), sorted.end(),
            [](const pair<string, double>& a, const pair<string, double>& b)
            {
                return a.second > b.second;
            });
        vector<string> results;
        for (size_t i = 0; i < sorted.size() && results.size() < 5; ++i)
        {
            results.push_back(sorted[i].first);
        }
        return results;
    }

    // Выводит все рекомендации для всех типов контента с метриками
    void printAllRecommendations()
    {
        cout << "\n==================== РЕКОМЕНДАЦИИ ====================\n";
        cout << "Пользователь: " << currentUserName << "\n";
        cout << "Возрастная группа: " << userGroup;
        if (userGroup == "child")
        {
            cout << " (0-12 лет)\n";
        }
        else if (userGroup == "teen")
        {
            cout << " (13-17 лет)\n";
        }
        else if (userGroup == "young")
        {
            cout << " (18-25 лет)\n";
        }
        else if (userGroup == "adult")
        {
            cout << " (26-35 лет)\n";
        }
        else if (userGroup == "middle")
        {
            cout << " (36-45 лет)\n";
        }
        else if (userGroup == "senior")
        {
            cout << " (46-60 лет)\n";
        }
        else
        {
            cout << " (60+ лет)\n";
        }

        set<string> relevantItems = getRelevantItems(currentUserName);

        vector<string> types = { "film", "series", "book", "music" };
        vector<string> typeNames = { "Фильмы", "Сериалы", "Книги", "Музыка" };

        for (size_t t = 0; t < types.size(); ++t)
        {
            cout << "\n======== " << typeNames[t] << " ========\n";
            vector<string> ruleRecs = getRuleBasedRecommendations(types[t]);
            vector<string> collabRecs = getCollaborativeRecommendations(types[t]);
            vector<string> popRecs = getPopularityRecommendations(types[t]);
            vector<string> hybridRecs = getHybridRecommendations(types[t]);

            auto printRecs = [&](const string& algoName, const vector<string>& recs)
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
                    double p5 = precisionAt5(recs, relevantItems);
                    double r5 = recallAt5(recs, relevantItems);
                    double hr5 = hitRateAt5(recs, relevantItems);
                    double ndcg5 = ndcgAt5(recs, relevantItems);
                    double avgPop = avgPopularity(recs);
                    cout << "  [Precision@5: " << p5 * 100 << "%";
                    cout << "  Recall@5: " << r5 * 100 << "%";
                    cout << "  HitRate@5: " << hr5 * 100 << "%";
                    cout << "  NDCG@5: " << ndcg5 * 100 << "%";
                    cout << "  Средняя популярность: " << avgPop << "]\n";
                };

            printRecs("Алгоритм 1 (Правила)", ruleRecs);
            printRecs("Алгоритм 2 (Коллаборативная)", collabRecs);
            printRecs("Алгоритм 3 (Популярность)", popRecs);
            printRecs("Алгоритм 4 (Гибридный)", hybridRecs);
        }
    }
};

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

int main()
{
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    setlocale(LC_ALL, "Russian");

    Recommender recommender;
    if (!recommender.init("database.txt", "ratings.txt"))
    {
        cout << "Ошибка: не удалось загрузить database.txt\n";
        return 1;
    }

    string userName;
    cout << "Введите ваше имя (для идентификации): ";
    cin >> userName;
    recommender.setCurrentUser(userName);

    int addChoice;
    cout << "Хотите добавить новый контент? (1 - Да, 0 - Нет): ";
    cin >> addChoice;
    while (addChoice == 1)
    {
        recommender.addContentItem();
        cout << "Добавить ещё? (1 - Да, 0 - Нет): ";
        cin >> addChoice;
    }

    int age = 0;
    cout << "\nДобро пожаловать в рекомендательную систему, " << userName << "!\n";
    cout << "Введите ваш возраст: ";
    cin >> age;
    while (age < 0 || age > 120)
    {
        cout << "Некорректный возраст, повторите: ";
        cin >> age;
    }
    recommender.setUserAge(age);

    vector<string> allGenres = {
        "комедия", "драма", "фантастика", "ужасы", "детектив",
        "рок", "поп", "хип-хоп", "классика", "спорт", "военный",
        "мистика", "фэнтези", "антиутопия", "сатира", "мультфильм",
        "исторический", "народная", "джаз", "эстрада"
    };
    vector<string> chosenGenres;
    showGenreMenu();
    int choice;
    while (true)
    {
        cin >> choice;
        if (choice == 0)
        {
            break;
        }
        if (choice >= 1 && choice <= (int)allGenres.size())
        {
            string genre = allGenres[choice - 1];
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
    if (chosenGenres.empty())
    {
        chosenGenres.push_back("комедия");
        cout << "Жанры не выбраны, добавлена комедия по умолчанию.\n";
    }
    recommender.setPreferredGenres(chosenGenres);

    int autoRateChoice;
    cout << "\nХотите оценить несколько случайных элементов (это улучшит рекомендации)? (1 - Да, 0 - Нет): ";
    cin >> autoRateChoice;
    if (autoRateChoice == 1)
    {
        recommender.autoRateRandomItems();
    }

    int rateChoice;
    cout << "\nХотите оценить какой-либо контент самостоятельно (1-5)? (1 - Да, 0 - Нет): ";
    cin >> rateChoice;
    if (rateChoice == 1)
    {
        recommender.rateContent();
    }

    recommender.printAllRecommendations();

    cout << "\nНажмите Enter для выхода.";
    cin.ignore();
    cin.get();
    return 0;
}
