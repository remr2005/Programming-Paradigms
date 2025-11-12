#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/types.hpp>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <algorithm>

int main() {
    mongocxx::instance instance{};
    mongocxx::client client{mongocxx::uri{"mongodb://localhost:27017"}};
    auto db = client["university"];
    auto collection = db["students"];

    std::cout << "=== АНАЛИЗ УСПЕВАЕМОСТИ СТУДЕНТОВ ===" << std::endl;
    std::cout << std::endl;

    // Отличники
    std::cout << "1. СТУДЕНТЫ С ВЫСОКИМ СРЕДНИМ БАЛЛОМ (>= 85):" << std::endl;
    
    bsoncxx::builder::basic::document filter_builder;
    filter_builder.append(bsoncxx::builder::basic::kvp("Средний_балл", bsoncxx::builder::basic::make_document(
        bsoncxx::builder::basic::kvp("$gte", 85.0)
    )));

    auto cursor = collection.find(filter_builder.view());
    
    std::vector<std::string> excellent_students;
    double total_excellent = 0.0;
    double count_excellent = 0;
    double max_excellent = 0.0;
    double min_excellent = 100.0;

    for (auto& doc : cursor) {
        count_excellent++;
        
        std::string name = "";
        if (doc["Имя"]) {
            name = doc["Имя"].get_string().value.to_string();
        }
        if (doc["Фамилия"]) {
            name += " " + std::string(doc["Фамилия"].get_string().value.to_string());
        }
        excellent_students.push_back(name);
        
        double avg = 0.0;
        if (doc["Средний_балл"]) {
            if (doc["Средний_балл"].type() == bsoncxx::type::k_double) {
                avg = doc["Средний_балл"].get_double().value;
            } else {
                avg = static_cast<double>(doc["Средний_балл"].get_int32().value);
            }
        }
        
        total_excellent += avg;
        if (avg > max_excellent) max_excellent = avg;
        if (avg < min_excellent) min_excellent = avg;
    }

    if (count_excellent > 0) {
        double avg_excellent = total_excellent / count_excellent;
        std::cout << "   Количество отличников: " << count_excellent << std::endl;
        std::cout << "   Средний балл отличников: " << std::fixed << std::setprecision(2) << avg_excellent << std::endl;
        std::cout << "   Максимальный балл: " << std::fixed << std::setprecision(2) << max_excellent << std::endl;
        std::cout << "   Минимальный балл: " << std::fixed << std::setprecision(2) << min_excellent << std::endl;
        std::cout << "   Разброс баллов: " << std::fixed << std::setprecision(2) << (max_excellent - min_excellent) << std::endl;
    } else {
        std::cout << "   Отличников не найдено." << std::endl;
    }
    std::cout << std::endl;

    // Возрастные группы
    std::cout << "2. АНАЛИЗ ПО ВОЗРАСТНЫМ ГРУППАМ:" << std::endl;
    
    // Молодые (18-20)
    filter_builder = bsoncxx::builder::basic::document{};
    filter_builder.append(bsoncxx::builder::basic::kvp("Возраст", bsoncxx::builder::basic::make_document(
        bsoncxx::builder::basic::kvp("$gte", 18),
        bsoncxx::builder::basic::kvp("$lte", 20)
    )));
    
    cursor = collection.find(filter_builder.view());
    double young_count = 0;
    double young_total = 0.0;
    
    for (auto& doc : cursor) {
        young_count++;
        double avg = 0.0;
        if (doc["Средний_балл"]) {
            if (doc["Средний_балл"].type() == bsoncxx::type::k_double) {
                avg = doc["Средний_балл"].get_double().value;
            } else {
                avg = static_cast<double>(doc["Средний_балл"].get_int32().value);
            }
        }
        young_total += avg;
    }
    
    // Взрослые (21+)
    filter_builder = bsoncxx::builder::basic::document{};
    filter_builder.append(bsoncxx::builder::basic::kvp("Возраст", bsoncxx::builder::basic::make_document(
        bsoncxx::builder::basic::kvp("$gt", 20)
    )));
    
    cursor = collection.find(filter_builder.view());
    double adult_count = 0;
    double adult_total = 0.0;
    
    for (auto& doc : cursor) {
        adult_count++;
        double avg = 0.0;
        if (doc["Средний_балл"]) {
            if (doc["Средний_балл"].type() == bsoncxx::type::k_double) {
                avg = doc["Средний_балл"].get_double().value;
            } else {
                avg = static_cast<double>(doc["Средний_балл"].get_int32().value);
            }
        }
        adult_total += avg;
    }
    
    std::cout << "   Молодые студенты (18-20 лет): " << young_count << " чел., средний балл: ";
    if (young_count > 0) {
        std::cout << std::fixed << std::setprecision(2) << (young_total / young_count);
    } else {
        std::cout << "нет данных";
    }
    std::cout << std::endl;
    
    std::cout << "   Взрослые студенты (21+ лет): " << adult_count << " чел., средний балл: ";
    if (adult_count > 0) {
        std::cout << std::fixed << std::setprecision(2) << (adult_total / adult_count);
    } else {
        std::cout << "нет данных";
    }
    std::cout << std::endl;
    std::cout << std::endl;

    // Проблемные студенты
    std::cout << "3. СТУДЕНТЫ С ПРОБЛЕМАМИ В УЧЕБЕ (средний балл < 60):" << std::endl;
    
    filter_builder = bsoncxx::builder::basic::document{};
    filter_builder.append(bsoncxx::builder::basic::kvp("Средний_балл", bsoncxx::builder::basic::make_document(
        bsoncxx::builder::basic::kvp("$lt", 60.0)
    )));
    
    cursor = collection.find(filter_builder.view());
    double problem_count = 0;
    double problem_total = 0.0;
    std::vector<std::string> problem_students;
    
    for (auto& doc : cursor) {
        problem_count++;
        
        std::string name = "";
        if (doc["Имя"]) {
            name = doc["Имя"].get_string().value.to_string();
        }
        if (doc["Фамилия"]) {
            name += " " + std::string(doc["Фамилия"].get_string().value.to_string());
        }
        problem_students.push_back(name);
        
        double avg = 0.0;
        if (doc["Средний_балл"]) {
            if (doc["Средний_балл"].type() == bsoncxx::type::k_double) {
                avg = doc["Средний_балл"].get_double().value;
            } else {
                avg = static_cast<double>(doc["Средний_балл"].get_int32().value);
            }
        }
        problem_total += avg;
    }
    
    if (problem_count > 0) {
        double avg_problem = problem_total / problem_count;
        std::cout << "   Количество студентов с проблемами: " << problem_count << std::endl;
        std::cout << "   Средний балл проблемных студентов: " << std::fixed << std::setprecision(2) << avg_problem << std::endl;
        std::cout << "   Процент от общего числа: " << std::fixed << std::setprecision(1) << (problem_count / (count_excellent + problem_count) * 100) << "%" << std::endl;
    } else {
        std::cout << "   Студентов с проблемами не найдено." << std::endl;
    }
    std::cout << std::endl;

    // Общая статистика
    std::cout << "4. ОБЩАЯ СТАТИСТИКА:" << std::endl;
    
    cursor = collection.find({});
    double total_students = 0;
    double total_average = 0.0;
    double global_max = 0.0;
    double global_min = 100.0;
    
    for (auto& doc : cursor) {
        total_students++;
        double avg = 0.0;
        if (doc["Средний_балл"]) {
            if (doc["Средний_балл"].type() == bsoncxx::type::k_double) {
                avg = doc["Средний_балл"].get_double().value;
            } else {
                avg = static_cast<double>(doc["Средний_балл"].get_int32().value);
            }
        }
        total_average += avg;
        if (avg > global_max) global_max = avg;
        if (avg < global_min) global_min = avg;
    }
    
    if (total_students > 0) {
        double overall_avg = total_average / total_students;
        std::cout << "   Общее количество студентов: " << total_students << std::endl;
        std::cout << "   Общий средний балл: " << std::fixed << std::setprecision(2) << overall_avg << std::endl;
        std::cout << "   Максимальный балл в университете: " << std::fixed << std::setprecision(2) << global_max << std::endl;
        std::cout << "   Минимальный балл в университете: " << std::fixed << std::setprecision(2) << global_min << std::endl;
        std::cout << "   Общий разброс баллов: " << std::fixed << std::setprecision(2) << (global_max - global_min) << std::endl;
    }
    
    std::cout << std::endl;
    std::cout << "=== АНАЛИЗ ЗАВЕРШЕН ===" << std::endl;
}
