#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/collection.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/types/bson_value/value.hpp>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <memory>

class Student {
protected:
    std::string name;
    std::string surname;
    int age;
    double average_score;
    std::string gender;
    std::string performance_level;

public:
    Student(const std::string& n, const std::string& s, int a, double score, const std::string& g = "")
        : name(n), surname(s), age(a), average_score(score), gender(g) {
        performance_level = determine_performance_level(score);
    }

    virtual ~Student() = default;

    std::string get_name() const { return name; }
    std::string get_surname() const { return surname; }
    int get_age() const { return age; }
    double get_average_score() const { return average_score; }
    std::string get_gender() const { return gender; }
    std::string get_performance_level() const { return performance_level; }

    virtual std::string get_full_info() const {
        return name + " " + surname + " (возраст: " + std::to_string(age) + 
               ", балл: " + std::to_string(average_score) + ", уровень: " + performance_level + ")";
    }

    virtual bool is_excellent() const {
        return average_score >= 85.0;
    }

    virtual bool is_problematic() const {
        return average_score < 60.0;
    }

protected:
    std::string determine_performance_level(double score) const {
        if (score >= 90) return "Отлично";
        else if (score >= 80) return "Хорошо";
        else if (score >= 70) return "Удовлетворительно";
        else if (score >= 60) return "Зачет";
        else return "Неудовлетворительно";
    }
};

class ExcellentStudent : public Student {
public:
    ExcellentStudent(const std::string& n, const std::string& s, int a, double score, const std::string& g = "")
        : Student(n, s, a, score, g) {}

    std::string get_full_info() const override {
        return "[ОТЛИЧНИК] " + Student::get_full_info();
    }

    bool is_excellent() const override {
        return true;
    }
};

class ProblematicStudent : public Student {
public:
    ProblematicStudent(const std::string& n, const std::string& s, int a, double score, const std::string& g = "")
        : Student(n, s, a, score, g) {}

    std::string get_full_info() const override {
        return "[ПРОБЛЕМНЫЙ] " + Student::get_full_info();
    }

    bool is_problematic() const override {
        return true;
    }
};

class MongoDBHandler {
private:
    mongocxx::instance instance;
    mongocxx::client client;
    mongocxx::database db;
    mongocxx::collection collection;
    bsoncxx::builder::basic::document filter_;

public:
    MongoDBHandler(const std::string& uri, const std::string& db_name, const std::string& coll_name)
        : client{mongocxx::uri{uri}}, db{client[db_name]}, collection{db[coll_name]} {}

    void clear_filter() {
        filter_ = bsoncxx::builder::basic::document{};
    }

    void add_filter_condition(const std::string& field, const std::string& op, const bsoncxx::types::bson_value::value& value) {
        filter_.append(bsoncxx::builder::basic::kvp(
            field,
            bsoncxx::builder::basic::make_document(
                bsoncxx::builder::basic::kvp(op, value)
            )
        ));
    }

    void add_regex_filter(const std::string& field, const std::string& pattern) {
        filter_.append(bsoncxx::builder::basic::kvp(
            field,
            bsoncxx::builder::basic::make_document(
                bsoncxx::builder::basic::kvp("$regex", pattern)
            )
        ));
    }

    void add_range_filter(const std::string& field, double min_val, double max_val) {
        filter_.append(bsoncxx::builder::basic::kvp(
            field,
            bsoncxx::builder::basic::make_document(
                bsoncxx::builder::basic::kvp("$gte", min_val),
                bsoncxx::builder::basic::kvp("$lte", max_val)
            )
        ));
    }

    mongocxx::collection get_collection() const {
        return collection;
    }

    bsoncxx::builder::basic::document get_filter() const {
        return filter_;
    }
};

// Класс для анализа статистики
class StatisticsAnalyzer {
private:
    std::vector<std::unique_ptr<Student>> students;
    MongoDBHandler& db_handler;

public:
    StatisticsAnalyzer(MongoDBHandler& handler) : db_handler(handler) {}

    // Загрузка студентов из базы данных
    void load_students() {
        students.clear();
        auto cursor = db_handler.get_collection().find(db_handler.get_filter());

        for (auto& doc : cursor) {
            std::string name = "";
            std::string surname = "";
            int age = 0;
            double score = 0.0;
            std::string gender = "";

            if (doc["Имя"]) {
                name = doc["Имя"].get_string().value.to_string();
            }
            if (doc["Фамилия"]) {
                surname = doc["Фамилия"].get_string().value.to_string();
            }
            if (doc["Возраст"]) {
                age = doc["Возраст"].get_int32().value;
            }
            if (doc["Пол"]) {
                gender = doc["Пол"].get_string().value.to_string();
            }
            if (doc["Средний_балл"]) {
                if (doc["Средний_балл"].type() == bsoncxx::type::k_double) {
                    score = doc["Средний_балл"].get_double().value;
                } else {
                    score = static_cast<double>(doc["Средний_балл"].get_int32().value);
                }
            }

            // Создаем соответствующий тип студента
            if (score >= 85.0) {
                students.push_back(std::make_unique<ExcellentStudent>(name, surname, age, score, gender));
            } else if (score < 60.0) {
                students.push_back(std::make_unique<ProblematicStudent>(name, surname, age, score, gender));
            } else {
                students.push_back(std::make_unique<Student>(name, surname, age, score, gender));
            }
        }
    }

    // Получение базовой статистики
    struct BasicStats {
        int count;
        double total_average;
        double max_score;
        double min_score;
        int excellent_count;
        int problematic_count;
    };

    BasicStats get_basic_statistics() const {
        BasicStats stats = {0, 0.0, 0.0, 100.0, 0, 0};

        for (const auto& student : students) {
            stats.count++;
            stats.total_average += student->get_average_score();
            
            if (student->get_average_score() > stats.max_score) {
                stats.max_score = student->get_average_score();
            }
            if (student->get_average_score() < stats.min_score) {
                stats.min_score = student->get_average_score();
            }
            
            if (student->is_excellent()) stats.excellent_count++;
            if (student->is_problematic()) stats.problematic_count++;
        }

        if (stats.count > 0) {
            stats.total_average /= stats.count;
        }

        return stats;
    }

    // Вывод статистики
    void print_statistics(const std::string& description) const {
        BasicStats stats = get_basic_statistics();
        
        std::cout << description << std::endl;
        std::cout << "   Количество студентов: " << stats.count << std::endl;
        
        if (stats.count > 0) {
            std::cout << "   Средний балл: " << std::fixed << std::setprecision(2) << stats.total_average << std::endl;
            std::cout << "   Максимальный балл: " << std::fixed << std::setprecision(2) << stats.max_score << std::endl;
            std::cout << "   Минимальный балл: " << std::fixed << std::setprecision(2) << stats.min_score << std::endl;
            std::cout << "   Отличников: " << stats.excellent_count << std::endl;
            std::cout << "   Проблемных: " << stats.problematic_count << std::endl;
        } else {
            std::cout << "   Студентов не найдено." << std::endl;
        }
        std::cout << std::endl;
    }

    // Получение списка студентов
    const std::vector<std::unique_ptr<Student>>& get_students() const {
        return students;
    }
};

// Класс для комплексного анализа
class ComprehensiveAnalyzer {
private:
    MongoDBHandler& db_handler;

public:
    ComprehensiveAnalyzer(MongoDBHandler& handler) : db_handler(handler) {}

    // Анализ по возрастным группам
    void analyze_age_groups() {
        std::cout << "=== АНАЛИЗ ПО ВОЗРАСТНЫМ ГРУППАМ ===" << std::endl;
        
        std::vector<std::pair<int, int>> age_groups = {{17, 18}, {19, 20}, {21, 22}, {23, 25}, {26, 30}};
        
        for (auto& group : age_groups) {
            db_handler.clear_filter();
            db_handler.add_range_filter("Возраст", group.first, group.second);
            
            StatisticsAnalyzer analyzer(db_handler);
            analyzer.load_students();
            analyzer.print_statistics("Возрастная группа " + std::to_string(group.first) + "-" + std::to_string(group.second) + " лет:");
        }
    }

    // Анализ по диапазонам баллов
    void analyze_score_ranges() {
        std::cout << "=== РАСПРЕДЕЛЕНИЕ ПО ДИАПАЗОНАМ БАЛЛОВ ===" << std::endl;
        
        std::vector<std::pair<double, double>> score_ranges = {
            {0, 50}, {50, 60}, {60, 70}, {70, 80}, {80, 90}, {90, 100}
        };
        
        for (auto& range : score_ranges) {
            db_handler.clear_filter();
            db_handler.add_range_filter("Средний_балл", range.first, range.second);
            
            StatisticsAnalyzer analyzer(db_handler);
            analyzer.load_students();
            analyzer.print_statistics("Диапазон баллов " + std::to_string((int)range.first) + "-" + std::to_string((int)range.second) + ":");
        }
    }

    // Анализ по фамилиям
    void analyze_by_surnames() {
        std::cout << "=== АНАЛИЗ ПО ФАМИЛИЯМ ===" << std::endl;
        
        std::vector<char> letters = {'А', 'Б', 'В', 'Г', 'Д', 'Е', 'Ж', 'З'};
        
        for (char letter : letters) {
            db_handler.clear_filter();
            std::string pattern = "^" + std::string(1, letter);
            db_handler.add_regex_filter("Фамилия", pattern);
            
            StatisticsAnalyzer analyzer(db_handler);
            analyzer.load_students();
            analyzer.print_statistics("Фамилии на букву '" + std::string(1, letter) + "':");
        }
    }

    // Поиск топ студентов
    void find_top_students(int count, bool best = true) {
        db_handler.clear_filter();
        StatisticsAnalyzer analyzer(db_handler);
        analyzer.load_students();
        
        auto students = analyzer.get_students();
        if (students.empty()) return;
        
        // Сортируем студентов по среднему баллу
        std::sort(students.begin(), students.end(),
                  [best](const std::unique_ptr<Student>& a, const std::unique_ptr<Student>& b) {
                      return best ? (a->get_average_score() > b->get_average_score()) : 
                                   (a->get_average_score() < b->get_average_score());
                  });
        
        std::string title = best ? "Топ-" + std::to_string(count) + " лучших студентов:" : 
                                 "Топ-" + std::to_string(count) + " проблемных студентов:";
        std::cout << title << std::endl;
        
        int display_count = std::min(count, (int)students.size());
        for (int i = 0; i < display_count; i++) {
            std::cout << "   " << (i+1) << ". " << students[i]->get_full_info() << std::endl;
        }
        std::cout << std::endl;
    }

    // Комплексный анализ
    void comprehensive_analysis() {
        std::cout << "=== КОМПЛЕКСНЫЙ АНАЛИЗ УСПЕВАЕМОСТИ ===" << std::endl;
        std::cout << std::endl;
        
        // Анализ отличников
        db_handler.clear_filter();
        db_handler.add_filter_condition("Средний_балл", "$gte", 85.0);
        StatisticsAnalyzer excellent_analyzer(db_handler);
        excellent_analyzer.load_students();
        excellent_analyzer.print_statistics("ОТЛИЧНИКИ (средний балл >= 85):");
        
        // Анализ проблемных студентов
        db_handler.clear_filter();
        db_handler.add_filter_condition("Средний_балл", "$lt", 60.0);
        StatisticsAnalyzer problem_analyzer(db_handler);
        problem_analyzer.load_students();
        problem_analyzer.print_statistics("ПРОБЛЕМНЫЕ СТУДЕНТЫ (средний балл < 60):");
        
        // Анализ средних студентов
        db_handler.clear_filter();
        db_handler.add_range_filter("Средний_балл", 60.0, 85.0);
        StatisticsAnalyzer average_analyzer(db_handler);
        average_analyzer.load_students();
        average_analyzer.print_statistics("СРЕДНИЕ СТУДЕНТЫ (балл 60-85):");
        
        // Общая статистика
        db_handler.clear_filter();
        StatisticsAnalyzer overall_analyzer(db_handler);
        overall_analyzer.load_students();
        overall_analyzer.print_statistics("ОБЩАЯ СТАТИСТИКА:");
    }
};

int main() {
    try {
        // Создание обработчика базы данных
        MongoDBHandler db_handler("mongodb://localhost:27017", "university", "students");
        
        // Создание комплексного анализатора
        ComprehensiveAnalyzer analyzer(db_handler);
        
        std::cout << "=== СИСТЕМА АНАЛИЗА УСПЕВАЕМОСТИ СТУДЕНТОВ (ООП) ===" << std::endl;
        std::cout << std::endl;
        
        // Комплексный анализ
        analyzer.comprehensive_analysis();
        
        // Анализ по возрастным группам
        analyzer.analyze_age_groups();
        
        // Анализ по диапазонам баллов
        analyzer.analyze_score_ranges();
        
        // Анализ по фамилиям
        analyzer.analyze_by_surnames();
        
        // Поиск топ студентов
        analyzer.find_top_students(5, true);  // Топ-5 лучших
        analyzer.find_top_students(5, false); // Топ-5 проблемных
        
        std::cout << "=== АНАЛИЗ ЗАВЕРШЕН ===" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
