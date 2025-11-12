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
#include <map>
#include <algorithm>

int main() {
    // Подключаемся к MongoDB
    mongocxx::instance instance{};
    mongocxx::client client{mongocxx::uri{"mongodb://localhost:27017"}};

    auto db = client["university"];
    auto collection = db["students"];

    std::cout << "=== РАСШИРЕННЫЙ АНАЛИЗ УСПЕВАЕМОСТИ ===" << std::endl;
    std::cout << std::endl;

    // Анализ по фамилиям
    std::cout << "1. АНАЛИЗ ПО ФАМИЛИЯМ:" << std::endl;
    
    std::vector<char> letters = {'А', 'Б', 'В', 'Г', 'Д'};
    std::map<char, double> letter_stats;
    std::map<char, int> letter_counts;
    
    for (char letter : letters) {
        bsoncxx::builder::basic::document filter_builder;
        std::string regex_pattern = "^" + std::string(1, letter);
        filter_builder.append(bsoncxx::builder::basic::kvp("Фамилия", 
            bsoncxx::builder::basic::make_document(
                bsoncxx::builder::basic::kvp("$regex", regex_pattern)
            )
        ));
        
        auto cursor = collection.find(filter_builder.view());
        double total_avg = 0.0;
        int count = 0;
        
        for (auto& doc : cursor) {
            count++;
            double avg = 0.0;
            if (doc["Средний_балл"]) {
                if (doc["Средний_балл"].type() == bsoncxx::type::k_double) {
                    avg = doc["Средний_балл"].get_double().value;
                } else {
                    avg = static_cast<double>(doc["Средний_балл"].get_int32().value);
                }
            }
            total_avg += avg;
        }
        
        letter_counts[letter] = count;
        if (count > 0) {
            letter_stats[letter] = total_avg / count;
        } else {
            letter_stats[letter] = 0.0;
        }
    }
    
    for (char letter : letters) {
        std::cout << "   Фамилии на '" << letter << "': " << letter_counts[letter] 
                  << " студентов, средний балл: " << std::fixed << std::setprecision(2) 
                  << letter_stats[letter] << std::endl;
    }
    std::cout << std::endl;

    // Детальный анализ по возрастам
    std::cout << "2. ДЕТАЛЬНЫЙ АНАЛИЗ ПО ВОЗРАСТАМ:" << std::endl;
    
    std::vector<std::pair<int, int>> age_groups = {{17, 18}, {19, 20}, {21, 22}, {23, 25}, {26, 30}};
    
    for (auto& group : age_groups) {
        bsoncxx::builder::basic::document filter_builder;
        filter_builder.append(bsoncxx::builder::basic::kvp("Возраст", 
            bsoncxx::builder::basic::make_document(
                bsoncxx::builder::basic::kvp("$gte", group.first),
                bsoncxx::builder::basic::kvp("$lte", group.second)
            )
        ));
        
        auto cursor = collection.find(filter_builder.view());
        int count = 0;
        double total_avg = 0.0;
        double max_avg = 0.0;
        double min_avg = 100.0;
        
        for (auto& doc : cursor) {
            count++;
            double avg = 0.0;
            if (doc["Средний_балл"]) {
                if (doc["Средний_балл"].type() == bsoncxx::type::k_double) {
                    avg = doc["Средний_балл"].get_double().value;
                } else {
                    avg = static_cast<double>(doc["Средний_балл"].get_int32().value);
                }
            }
            total_avg += avg;
            if (avg > max_avg) max_avg = avg;
            if (avg < min_avg) min_avg = avg;
        }
        
        std::cout << "   Возраст " << group.first << "-" << group.second << " лет: " << count 
                  << " студентов" << std::endl;
        if (count > 0) {
            std::cout << "     Средний балл: " << std::fixed << std::setprecision(2) 
                      << (total_avg / count) << std::endl;
            std::cout << "     Максимальный: " << std::fixed << std::setprecision(2) 
                      << max_avg << std::endl;
            std::cout << "     Минимальный: " << std::fixed << std::setprecision(2) 
                      << min_avg << std::endl;
        }
    }
    std::cout << std::endl;

    // Распределение по баллам
    std::cout << "3. РАСПРЕДЕЛЕНИЕ ПО ДИАПАЗОНАМ БАЛЛОВ:" << std::endl;
    
    std::vector<std::pair<double, double>> score_ranges = {
        {0, 50}, {50, 60}, {60, 70}, {70, 80}, {80, 90}, {90, 100}
    };
    
    for (auto& range : score_ranges) {
        bsoncxx::builder::basic::document filter_builder;
        filter_builder.append(bsoncxx::builder::basic::kvp("Средний_балл", 
            bsoncxx::builder::basic::make_document(
                bsoncxx::builder::basic::kvp("$gte", range.first),
                bsoncxx::builder::basic::kvp("$lt", range.second)
            )
        ));
        
        auto cursor = collection.find(filter_builder.view());
        int count = 0;
        
        for (auto& doc : cursor) {
            count++;
        }
        
        std::cout << "   Диапазон " << range.first << "-" << range.second << ": " 
                  << count << " студентов" << std::endl;
    }
    std::cout << std::endl;

    // Экстремальные показатели
    std::cout << "4. ЭКСТРЕМАЛЬНЫЕ ПОКАЗАТЕЛИ:" << std::endl;
    
    // Топ-5 лучших
    std::cout << "   Топ-5 самых успешных студентов:" << std::endl;
    auto all_cursor = collection.find({});
    std::vector<std::pair<std::string, double>> all_students;
    
    for (auto& doc : all_cursor) {
        std::string name = "";
        if (doc["Имя"]) {
            name = doc["Имя"].get_string().value.to_string();
        }
        if (doc["Фамилия"]) {
            name += " " + std::string(doc["Фамилия"].get_string().value.to_string());
        }
        
        double avg = 0.0;
        if (doc["Средний_балл"]) {
            if (doc["Средний_балл"].type() == bsoncxx::type::k_double) {
                avg = doc["Средний_балл"].get_double().value;
            } else {
                avg = static_cast<double>(doc["Средний_балл"].get_int32().value);
            }
        }
        
        all_students.push_back({name, avg});
    }
    
    // Сортируем по баллу
    std::sort(all_students.begin(), all_students.end(), 
              [](const std::pair<std::string, double>& a, const std::pair<std::string, double>& b) {
                  return a.second > b.second;
              });
    
    for (int i = 0; i < std::min(5, (int)all_students.size()); i++) {
        std::cout << "     " << (i+1) << ". " << all_students[i].first 
                  << " - " << std::fixed << std::setprecision(2) << all_students[i].second << std::endl;
    }
    
    // Топ-5 проблемных
    std::cout << "   Топ-5 самых проблемных студентов:" << std::endl;
    for (int i = std::max(0, (int)all_students.size() - 5); i < (int)all_students.size(); i++) {
        std::cout << "     " << (all_students.size() - i) << ". " << all_students[i].first 
                  << " - " << std::fixed << std::setprecision(2) << all_students[i].second << std::endl;
    }
    std::cout << std::endl;

    // Анализ по полу
    std::cout << "5. АНАЛИЗ ПО ПОЛУ:" << std::endl;
    
    // Проверяем поле "Пол"
    auto sample_cursor = collection.find({}).limit(1);
    bool has_gender_field = false;
    for (auto& doc : sample_cursor) {
        if (doc["Пол"]) {
            has_gender_field = true;
            break;
        }
    }
    
    if (has_gender_field) {
        std::vector<std::string> genders = {"М", "Ж", "Мужской", "Женский"};
        
        for (const std::string& gender : genders) {
            bsoncxx::builder::basic::document filter_builder;
            filter_builder.append(bsoncxx::builder::basic::kvp("Пол", gender));
            
            auto cursor = collection.find(filter_builder.view());
            int count = 0;
            double total_avg = 0.0;
            
            for (auto& doc : cursor) {
                count++;
                double avg = 0.0;
                if (doc["Средний_балл"]) {
                    if (doc["Средний_балл"].type() == bsoncxx::type::k_double) {
                        avg = doc["Средний_балл"].get_double().value;
                    } else {
                        avg = static_cast<double>(doc["Средний_балл"].get_int32().value);
                    }
                }
                total_avg += avg;
            }
            
            if (count > 0) {
                std::cout << "   Пол '" << gender << "': " << count << " студентов, средний балл: " 
                          << std::fixed << std::setprecision(2) << (total_avg / count) << std::endl;
            }
        }
    } else {
        std::cout << "   Поле 'Пол' не найдено в базе данных." << std::endl;
    }
    
    std::cout << std::endl;
    std::cout << "=== РАСШИРЕННЫЙ АНАЛИЗ ЗАВЕРШЕН ===" << std::endl;
}
