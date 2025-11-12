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
#include <cmath>

// Глобальные переменные для процедурной парадигмы
mongocxx::collection global_collection;
bsoncxx::builder::basic::document global_filter;

// Расширенная структура для хранения статистики студента
struct DetailedStudentStats {
    std::string name;
    std::string surname;
    double average_score;
    int age;
    std::string gender;
    std::vector<double> subject_scores;  // Оценки по предметам
    double gpa;  // Средний балл
    std::string performance_level;  // Уровень успеваемости
};

// Структура для детальной групповой статистики
struct DetailedGroupStats {
    int count;
    double total_average;
    double max_score;
    double min_score;
    double standard_deviation;
    std::vector<DetailedStudentStats> students;
    std::map<std::string, int> performance_distribution;
};

// Функция инициализации подключения к MongoDB
void init_mongodb_connection() {
    mongocxx::instance instance{};
    mongocxx::client client{mongocxx::uri{"mongodb://localhost:27017"}};
    auto db = client["university"];
    global_collection = db["students"];
}

// Функция очистки глобального фильтра
void clear_global_filter() {
    global_filter = bsoncxx::builder::basic::document{};
}

// Функция добавления условия в фильтр
void add_filter_condition(const std::string& field, const std::string& op, const bsoncxx::types::bson_value::value& value) {
    global_filter.append(bsoncxx::builder::basic::kvp(
        field,
        bsoncxx::builder::basic::make_document(
            bsoncxx::builder::basic::kvp(op, value)
        )
    ));
}

// Функция добавления условия с регулярным выражением
void add_regex_filter(const std::string& field, const std::string& pattern) {
    global_filter.append(bsoncxx::builder::basic::kvp(
        field,
        bsoncxx::builder::basic::make_document(
            bsoncxx::builder::basic::kvp("$regex", pattern)
        )
    ));
}

// Функция добавления диапазонного условия
void add_range_filter(const std::string& field, double min_val, double max_val) {
    global_filter.append(bsoncxx::builder::basic::kvp(
        field,
        bsoncxx::builder::basic::make_document(
            bsoncxx::builder::basic::kvp("$gte", min_val),
            bsoncxx::builder::basic::kvp("$lte", max_val)
        )
    ));
}

// Функция определения уровня успеваемости
std::string determine_performance_level(double score) {
    if (score >= 90) return "Отлично";
    else if (score >= 80) return "Хорошо";
    else if (score >= 70) return "Удовлетворительно";
    else if (score >= 60) return "Зачет";
    else return "Неудовлетворительно";
}

// Функция получения детальной статистики по фильтру
DetailedGroupStats get_detailed_statistics_by_filter() {
    DetailedGroupStats stats;
    stats.count = 0;
    stats.total_average = 0.0;
    stats.max_score = 0.0;
    stats.min_score = 100.0;
    stats.standard_deviation = 0.0;
    
    auto cursor = global_collection.find(global_filter.view());
    
    for (auto& doc : cursor) {
        stats.count++;
        
        // Получаем данные студента
        DetailedStudentStats student;
        student.name = "";
        student.surname = "";
        student.age = 0;
        student.average_score = 0.0;
        student.gender = "";
        
        if (doc["Имя"]) {
            student.name = doc["Имя"].get_string().value.to_string();
        }
        if (doc["Фамилия"]) {
            student.surname = doc["Фамилия"].get_string().value.to_string();
        }
        if (doc["Возраст"]) {
            student.age = doc["Возраст"].get_int32().value;
        }
        if (doc["Пол"]) {
            student.gender = doc["Пол"].get_string().value.to_string();
        }
        if (doc["Средний_балл"]) {
            if (doc["Средний_балл"].type() == bsoncxx::type::k_double) {
                student.average_score = doc["Средний_балл"].get_double().value;
            } else {
                student.average_score = static_cast<double>(doc["Средний_балл"].get_int32().value);
            }
        }
        
        student.performance_level = determine_performance_level(student.average_score);
        student.gpa = student.average_score;
        
        stats.students.push_back(student);
        stats.total_average += student.average_score;
        
        if (student.average_score > stats.max_score) {
            stats.max_score = student.average_score;
        }
        if (student.average_score < stats.min_score) {
            stats.min_score = student.average_score;
        }
        
        // Распределение по уровням успеваемости
        stats.performance_distribution[student.performance_level]++;
    }
    
    if (stats.count > 0) {
        stats.total_average /= stats.count;
        
        // Вычисляем стандартное отклонение
        double variance = 0.0;
        for (const auto& student : stats.students) {
            variance += pow(student.average_score - stats.total_average, 2);
        }
        stats.standard_deviation = sqrt(variance / stats.count);
    }
    
    return stats;
}

// Функция вывода детальной статистики
void print_detailed_statistics(const DetailedGroupStats& stats, const std::string& description) {
    std::cout << description << std::endl;
    std::cout << "   Количество студентов: " << stats.count << std::endl;
    
    if (stats.count > 0) {
        std::cout << "   Средний балл: " << std::fixed << std::setprecision(2) << stats.total_average << std::endl;
        std::cout << "   Максимальный балл: " << std::fixed << std::setprecision(2) << stats.max_score << std::endl;
        std::cout << "   Минимальный балл: " << std::fixed << std::setprecision(2) << stats.min_score << std::endl;
        std::cout << "   Стандартное отклонение: " << std::fixed << std::setprecision(2) << stats.standard_deviation << std::endl;
        std::cout << "   Разброс баллов: " << std::fixed << std::setprecision(2) << (stats.max_score - stats.min_score) << std::endl;
        
        std::cout << "   Распределение по уровням успеваемости:" << std::endl;
        for (const auto& level : stats.performance_distribution) {
            double percentage = (double)level.second / stats.count * 100;
            std::cout << "     " << level.first << ": " << level.second << " (" 
                      << std::fixed << std::setprecision(1) << percentage << "%)" << std::endl;
        }
    } else {
        std::cout << "   Студентов не найдено." << std::endl;
    }
    std::cout << std::endl;
}

// Функция анализа корреляции между возрастом и успеваемостью
void analyze_age_performance_correlation() {
    std::cout << "=== КОРРЕЛЯЦИЯ ВОЗРАСТ-УСПЕВАЕМОСТЬ ===" << std::endl;
    
    std::vector<std::pair<int, int>> age_groups = {{17, 18}, {19, 20}, {21, 22}, {23, 25}, {26, 30}, {31, 40}};
    
    for (auto& group : age_groups) {
        clear_global_filter();
        add_range_filter("Возраст", group.first, group.second);
        
        DetailedGroupStats stats = get_detailed_statistics_by_filter();
        std::string description = "Возрастная группа " + std::to_string(group.first) + "-" + std::to_string(group.second) + " лет:";
        print_detailed_statistics(stats, description);
    }
}

// Функция анализа по полу
void analyze_by_gender() {
    std::cout << "=== АНАЛИЗ ПО ПОЛУ ===" << std::endl;
    
    std::vector<std::string> genders = {"М", "Ж", "Мужской", "Женский", "м", "ж"};
    
    for (const std::string& gender : genders) {
        clear_global_filter();
        add_filter_condition("Пол", "$eq", gender);
        
        DetailedGroupStats stats = get_detailed_statistics_by_filter();
        if (stats.count > 0) {
            std::string description = "Пол '" + gender + "':";
            print_detailed_statistics(stats, description);
        }
    }
}

// Функция анализа академических рисков
void analyze_academic_risks() {
    std::cout << "=== АНАЛИЗ АКАДЕМИЧЕСКИХ РИСКОВ ===" << std::endl;
    
    // Студенты с низкой успеваемостью
    clear_global_filter();
    add_filter_condition("Средний_балл", "$lt", 60.0);
    DetailedGroupStats low_performance = get_detailed_statistics_by_filter();
    print_detailed_statistics(low_performance, "СТУДЕНТЫ С НИЗКОЙ УСПЕВАЕМОСТЬЮ (< 60):");
    
    // Студенты на грани отчисления
    clear_global_filter();
    add_range_filter("Средний_балл", 50.0, 60.0);
    DetailedGroupStats at_risk = get_detailed_statistics_by_filter();
    print_detailed_statistics(at_risk, "СТУДЕНТЫ НА ГРАНИ ОТЧИСЛЕНИЯ (50-60):");
    
    // Студенты с нестабильной успеваемостью (высокое стандартное отклонение)
    clear_global_filter();
    DetailedGroupStats all_students = get_detailed_statistics_by_filter();
    
    if (all_students.count > 0) {
        std::cout << "СТУДЕНТЫ С НЕСТАБИЛЬНОЙ УСПЕВАЕМОСТЬЮ:" << std::endl;
        std::cout << "   (высокое стандартное отклонение: " << std::fixed << std::setprecision(2) 
                  << all_students.standard_deviation << ")" << std::endl;
        
        // Находим студентов с баллами, сильно отклоняющимися от среднего
        double threshold = all_students.standard_deviation * 1.5;
        int unstable_count = 0;
        
        for (const auto& student : all_students.students) {
            if (abs(student.average_score - all_students.total_average) > threshold) {
                unstable_count++;
                std::cout << "     " << student.name << " " << student.surname 
                          << " (балл: " << std::fixed << std::setprecision(2) 
                          << student.average_score << ")" << std::endl;
            }
        }
        
        if (unstable_count == 0) {
            std::cout << "     Нестабильных студентов не найдено." << std::endl;
        }
        std::cout << std::endl;
    }
}

// Функция прогнозирования успеваемости
void predict_performance_trends() {
    std::cout << "=== ПРОГНОЗИРОВАНИЕ ТРЕНДОВ УСПЕВАЕМОСТИ ===" << std::endl;
    
    // Анализ по возрастным группам для выявления трендов
    std::vector<std::pair<int, int>> age_groups = {{17, 19}, {20, 22}, {23, 25}, {26, 30}};
    std::vector<double> age_averages;
    
    for (auto& group : age_groups) {
        clear_global_filter();
        add_range_filter("Возраст", group.first, group.second);
        
        DetailedGroupStats stats = get_detailed_statistics_by_filter();
        if (stats.count > 0) {
            age_averages.push_back(stats.total_average);
            std::cout << "Возраст " << group.first << "-" << group.second << ": средний балл " 
                      << std::fixed << std::setprecision(2) << stats.total_average << std::endl;
        }
    }
    
    // Анализ тренда
    if (age_averages.size() >= 2) {
        std::cout << std::endl << "АНАЛИЗ ТРЕНДА:" << std::endl;
        
        bool increasing = true;
        bool decreasing = true;
        
        for (size_t i = 1; i < age_averages.size(); i++) {
            if (age_averages[i] <= age_averages[i-1]) increasing = false;
            if (age_averages[i] >= age_averages[i-1]) decreasing = false;
        }
        
        if (increasing) {
            std::cout << "   Тренд: УСПЕВАЕМОСТЬ РАСТЕТ с возрастом" << std::endl;
        } else if (decreasing) {
            std::cout << "   Тренд: УСПЕВАЕМОСТЬ СНИЖАЕТСЯ с возрастом" << std::endl;
        } else {
            std::cout << "   Тренд: УСПЕВАЕМОСТЬ НЕСТАБИЛЬНА по возрастным группам" << std::endl;
        }
    }
    std::cout << std::endl;
}

// Функция рекомендаций по улучшению
void generate_recommendations() {
    std::cout << "=== РЕКОМЕНДАЦИИ ПО УЛУЧШЕНИЮ УСПЕВАЕМОСТИ ===" << std::endl;
    
    clear_global_filter();
    DetailedGroupStats all_stats = get_detailed_statistics_by_filter();
    
    if (all_stats.count == 0) {
        std::cout << "   Данные для анализа отсутствуют." << std::endl;
        return;
    }
    
    std::cout << "   ОБЩИЕ РЕКОМЕНДАЦИИ:" << std::endl;
    
    if (all_stats.total_average < 70) {
        std::cout << "   - Критический уровень успеваемости! Требуется срочное вмешательство." << std::endl;
        std::cout << "   - Рекомендуется введение дополнительных занятий." << std::endl;
    } else if (all_stats.total_average < 80) {
        std::cout << "   - Успеваемость ниже среднего. Необходимы меры поддержки." << std::endl;
        std::cout << "   - Рекомендуется индивидуальная работа с отстающими студентами." << std::endl;
    } else {
        std::cout << "   - Успеваемость на хорошем уровне. Поддерживать текущие показатели." << std::endl;
    }
    
    if (all_stats.standard_deviation > 15) {
        std::cout << "   - Высокий разброс в успеваемости. Необходима дифференцированная работа." << std::endl;
    }
    
    // Анализ проблемных групп
    int problem_count = 0;
    for (const auto& student : all_stats.students) {
        if (student.average_score < 60) {
            problem_count++;
        }
    }
    
    double problem_percentage = (double)problem_count / all_stats.count * 100;
    std::cout << "   - Процент проблемных студентов: " << std::fixed << std::setprecision(1) 
              << problem_percentage << "%" << std::endl;
    
    if (problem_percentage > 20) {
        std::cout << "   - ВНИМАНИЕ: Высокий процент проблемных студентов!" << std::endl;
    }
    
    std::cout << std::endl;
}

int main() {
    try {
        // Инициализация подключения
        init_mongodb_connection();
        
        std::cout << "=== РАСШИРЕННАЯ СИСТЕМА АНАЛИЗА УСПЕВАЕМОСТИ ===" << std::endl;
        std::cout << std::endl;
        
        // Анализ корреляции возраст-успеваемость
        analyze_age_performance_correlation();
        
        // Анализ по полу
        analyze_by_gender();
        
        // Анализ академических рисков
        analyze_academic_risks();
        
        // Прогнозирование трендов
        predict_performance_trends();
        
        // Генерация рекомендаций
        generate_recommendations();
        
        std::cout << "=== РАСШИРЕННЫЙ АНАЛИЗ ЗАВЕРШЕН ===" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
