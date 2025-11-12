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

// Глобальные переменные
mongocxx::collection global_collection;
bsoncxx::builder::basic::document global_filter;

struct StudentStats {
    std::string name;
    double average_score;
    int age;
    std::string surname;
};

struct GroupStats {
    int count;
    double total_average;
    double max_score;
    double min_score;
    std::vector<StudentStats> students;
};

void init_mongodb_connection() {
    mongocxx::instance instance{};
    mongocxx::client client{mongocxx::uri{"mongodb://localhost:27017"}};
    auto db = client["university"];
    global_collection = db["students"];
}

void clear_global_filter() {
    global_filter = bsoncxx::builder::basic::document{};
}

void add_filter_condition(const std::string& field, const std::string& op, const bsoncxx::types::bson_value::value& value) {
    global_filter.append(bsoncxx::builder::basic::kvp(
        field,
        bsoncxx::builder::basic::make_document(
            bsoncxx::builder::basic::kvp(op, value)
        )
    ));
}

void add_regex_filter(const std::string& field, const std::string& pattern) {
    global_filter.append(bsoncxx::builder::basic::kvp(
        field,
        bsoncxx::builder::basic::make_document(
            bsoncxx::builder::basic::kvp("$regex", pattern)
        )
    ));
}

void add_range_filter(const std::string& field, double min_val, double max_val) {
    global_filter.append(bsoncxx::builder::basic::kvp(
        field,
        bsoncxx::builder::basic::make_document(
            bsoncxx::builder::basic::kvp("$gte", min_val),
            bsoncxx::builder::basic::kvp("$lte", max_val)
        )
    ));
}

GroupStats get_statistics_by_filter() {
    GroupStats stats;
    stats.count = 0;
    stats.total_average = 0.0;
    stats.max_score = 0.0;
    stats.min_score = 100.0;
    
    auto cursor = global_collection.find(global_filter.view());
    
    for (auto& doc : cursor) {
        stats.count++;
        
        StudentStats student;
        student.name = "";
        student.surname = "";
        student.age = 0;
        student.average_score = 0.0;
        
        if (doc["Имя"]) {
            student.name = doc["Имя"].get_string().value.to_string();
        }
        if (doc["Фамилия"]) {
            student.surname = doc["Фамилия"].get_string().value.to_string();
        }
        if (doc["Возраст"]) {
            student.age = doc["Возраст"].get_int32().value;
        }
        if (doc["Средний_балл"]) {
            if (doc["Средний_балл"].type() == bsoncxx::type::k_double) {
                student.average_score = doc["Средний_балл"].get_double().value;
            } else {
                student.average_score = static_cast<double>(doc["Средний_балл"].get_int32().value);
            }
        }
        
        stats.students.push_back(student);
        stats.total_average += student.average_score;
        
        if (student.average_score > stats.max_score) {
            stats.max_score = student.average_score;
        }
        if (student.average_score < stats.min_score) {
            stats.min_score = student.average_score;
        }
    }
    
    if (stats.count > 0) {
        stats.total_average /= stats.count;
    }
    
    return stats;
}

void print_statistics(const GroupStats& stats, const std::string& description) {
    std::cout << description << std::endl;
    std::cout << "   Количество студентов: " << stats.count << std::endl;
    
    if (stats.count > 0) {
        std::cout << "   Средний балл: " << std::fixed << std::setprecision(2) << stats.total_average << std::endl;
        std::cout << "   Максимальный балл: " << std::fixed << std::setprecision(2) << stats.max_score << std::endl;
        std::cout << "   Минимальный балл: " << std::fixed << std::setprecision(2) << stats.min_score << std::endl;
        std::cout << "   Разброс баллов: " << std::fixed << std::setprecision(2) << (stats.max_score - stats.min_score) << std::endl;
    } else {
        std::cout << "   Студентов не найдено." << std::endl;
    }
    std::cout << std::endl;
}

// Функция анализа по возрастным группам
void analyze_age_groups() {
    std::cout << "=== АНАЛИЗ ПО ВОЗРАСТНЫМ ГРУППАМ ===" << std::endl;
    
    std::vector<std::pair<int, int>> age_groups = {{17, 18}, {19, 20}, {21, 22}, {23, 25}, {26, 30}};
    
    for (auto& group : age_groups) {
        clear_global_filter();
        add_range_filter("Возраст", group.first, group.second);
        
        GroupStats stats = get_statistics_by_filter();
        std::string description = "Возрастная группа " + std::to_string(group.first) + "-" + std::to_string(group.second) + " лет:";
        print_statistics(stats, description);
    }
}

// Функция анализа по диапазонам баллов
void analyze_score_ranges() {
    std::cout << "=== РАСПРЕДЕЛЕНИЕ ПО ДИАПАЗОНАМ БАЛЛОВ ===" << std::endl;
    
    std::vector<std::pair<double, double>> score_ranges = {
        {0, 50}, {50, 60}, {60, 70}, {70, 80}, {80, 90}, {90, 100}
    };
    
    for (auto& range : score_ranges) {
        clear_global_filter();
        add_range_filter("Средний_балл", range.first, range.second);
        
        GroupStats stats = get_statistics_by_filter();
        std::string description = "Диапазон баллов " + std::to_string((int)range.first) + "-" + std::to_string((int)range.second) + ":";
        print_statistics(stats, description);
    }
}

// Функция анализа по фамилиям
void analyze_by_surnames() {
    std::cout << "=== АНАЛИЗ ПО ФАМИЛИЯМ ===" << std::endl;
    
    std::vector<char> letters = {'А', 'Б', 'В', 'Г', 'Д', 'Е', 'Ж', 'З'};
    
    for (char letter : letters) {
        clear_global_filter();
        std::string pattern = "^" + std::string(1, letter);
        add_regex_filter("Фамилия", pattern);
        
        GroupStats stats = get_statistics_by_filter();
        std::string description = "Фамилии на букву '" + std::string(1, letter) + "':";
        print_statistics(stats, description);
    }
}

// Функция поиска топ студентов
void find_top_students(int count, bool best = true) {
    clear_global_filter();
    GroupStats all_stats = get_statistics_by_filter();
    
    if (all_stats.students.empty()) return;
    
    // Сортируем студентов по среднему баллу
    std::sort(all_stats.students.begin(), all_stats.students.end(),
              [best](const StudentStats& a, const StudentStats& b) {
                  return best ? (a.average_score > b.average_score) : (a.average_score < b.average_score);
    
    std::string title = best ? "Топ-" + std::to_string(count) + " лучших студентов:" : 
                                 "Топ-" + std::to_string(count) + " проблемных студентов:";
    std::cout << title << std::endl;
    
    int display_count = std::min(count, (int)all_stats.students.size());
    for (int i = 0; i < display_count; i++) {
        std::cout << "   " << (i+1) << ". " << all_stats.students[i].name << " " 
                  << all_stats.students[i].surname 
                  << " (возраст: " << all_stats.students[i].age 
                  << ", балл: " << std::fixed << std::setprecision(2) 
                  << all_stats.students[i].average_score << ")" << std::endl;
    }
    std::cout << std::endl;
}

// Функция комплексного анализа
void comprehensive_analysis() {
    std::cout << "=== КОМПЛЕКСНЫЙ АНАЛИЗ УСПЕВАЕМОСТИ ===" << std::endl;
    std::cout << std::endl;
    
    // Анализ отличников
    clear_global_filter();
    add_filter_condition("Средний_балл", "$gte", 85.0);
    GroupStats excellent_stats = get_statistics_by_filter();
    print_statistics(excellent_stats, "ОТЛИЧНИКИ (средний балл >= 85):");
    
    // Анализ проблемных студентов
    clear_global_filter();
    add_filter_condition("Средний_балл", "$lt", 60.0);
    GroupStats problem_stats = get_statistics_by_filter();
    print_statistics(problem_stats, "ПРОБЛЕМНЫЕ СТУДЕНТЫ (средний балл < 60):");
    
    // Анализ средних студентов
    clear_global_filter();
    add_range_filter("Средний_балл", 60.0, 85.0);
    GroupStats average_stats = get_statistics_by_filter();
    print_statistics(average_stats, "СРЕДНИЕ СТУДЕНТЫ (балл 60-85):");
    
    // Общая статистика
    clear_global_filter();
    GroupStats overall_stats = get_statistics_by_filter();
    print_statistics(overall_stats, "ОБЩАЯ СТАТИСТИКА:");
}

int main() {
    try {
        // Инициализация подключения
        init_mongodb_connection();
        
        std::cout << "=== СИСТЕМА АНАЛИЗА УСПЕВАЕМОСТИ СТУДЕНТОВ ===" << std::endl;
        std::cout << std::endl;
        
        // Комплексный анализ
        comprehensive_analysis();
        
        // Анализ по возрастным группам
        analyze_age_groups();
        
        // Анализ по диапазонам баллов
        analyze_score_ranges();
        
        // Анализ по фамилиям
        analyze_by_surnames();
        
        // Поиск топ студентов
        find_top_students(5, true);  // Топ-5 лучших
        find_top_students(5, false); // Топ-5 проблемных
        
        std::cout << "=== АНАЛИЗ ЗАВЕРШЕН ===" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
