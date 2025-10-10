#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/collection.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/types/bson_value/value.hpp>
#include <iostream>
#include <iomanip>

// Глобальный фильтр для процедурной парадигмы
bsoncxx::builder::basic::document filter;

// Функция вывода студентов
void print_average(mongocxx::collection& collection,
                      const bsoncxx::builder::basic::document& filter) {
    auto cursor = collection.find(filter.view());
    
    std::cout << "Студенты:" << std::endl;

    double count = 0;
    double totalAverage = 0.0;
    
    for (auto& doc : cursor) {
        count+=1;
        // Средний балл
        double avg = 0.0;
        if (doc["Средний_балл"]) {
            if (doc["Средний_балл"].type() == bsoncxx::type::k_double) {
                avg = doc["Средний_балл"].get_double().value;
            } else {
                avg = static_cast<double>(doc["Средний_балл"].get_int32().value);
            }
        }    
        totalAverage += avg;
    }
    
    if (count > 0) {
        double groupAverage = totalAverage / count;
        std::cout << "Итого студентов: " << count
                  << ", средний балл по выборке: "
                  << std::fixed << std::setprecision(2) << groupAverage
                  << std::defaultfloat
                  << std::endl;
    } else {
        std::cout << "По заданному фильтру студентов не найдено." << std::endl;
    }
}


void print_max(mongocxx::collection& collection,
                      const bsoncxx::builder::basic::document& filter) {
    auto cursor = collection.find(filter.view());
    
    std::cout << "Студенты:" << std::endl;
    
    double count = 0;
    double maxAverage = 0.0;
    
    for (auto& doc : cursor) {
        count +=1;
        // Средний балл
        double avg = 0.0;
        if (doc["Средний_балл"]) {
            if (doc["Средний_балл"].type() == bsoncxx::type::k_double) {
                avg = doc["Средний_балл"].get_double().value;
            } else {
                avg = static_cast<double>(doc["Средний_балл"].get_int32().value);
            }
        }
        
        // Ищем максимальный балл
        if (avg > maxAverage) {
            maxAverage = avg;
        }
    }
    
    if (count > 0) {        
        // Показываем максимальный балл
        std::cout << "Максимальный средний балл среди найденных студентов: "
                  << std::fixed << std::setprecision(2) << maxAverage
                  << std::defaultfloat
                  << std::endl;
    } else {
        std::cout << "По заданному фильтру студентов не найдено." << std::endl;
    }
}

// Добавляем условие в фильтр
void build_filter(
    const std::string& field,
    const std::string& op,
    const bsoncxx::types::bson_value::value& value
) {
    filter.append(bsoncxx::builder::basic::kvp(
        field,
        bsoncxx::builder::basic::make_document(
            bsoncxx::builder::basic::kvp(op, value)
        )
    ));
}

// Очищаем фильтр
void clear_filter() {
    filter = bsoncxx::builder::basic::document{};
}


int main() {
    // Подключаемся к MongoDB
    mongocxx::instance inst{};
    mongocxx::client client{mongocxx::uri{"mongodb://localhost:27017"}};
    auto db = client["university"];
    auto collection = db["students"];

   std::cout << "Студенты: возраст < 19" << std::endl;

    // Очищаем фильтр перед началом
    clear_filter();
        
    // возраст < 19
    build_filter(
            "Возраст",
            "$lt",
            19
        );

    print_average(collection, filter);
    print_max(collection, filter);
}
