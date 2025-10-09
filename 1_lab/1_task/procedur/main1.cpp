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
void print_collection(mongocxx::collection& collection,
                      const bsoncxx::builder::basic::document& filter) {
    auto cursor = collection.find(filter.view());
    
    std::cout << "Студенты:" << std::endl;
    
    int index = 0;
    int count = 0;
    double totalAverage = 0.0;
    double maxAverage = 0.0;
    
    for (auto&& doc : cursor) {
        ++index;
        ++count;
        
        // Получаем данные студента
        std::string firstName = doc["Имя"] ? std::string(doc["Имя"].get_string().value) : "";
        std::string lastName  = doc["Фамилия"] ? std::string(doc["Фамилия"].get_string().value) : "";
        std::string middleName = doc["Отчество"] ? std::string(doc["Отчество"].get_string().value) : "";
        std::string group     = doc["Группа"] ? std::string(doc["Группа"].get_string().value) : "";
        int age = doc["Возраст"] ? doc["Возраст"].get_int32().value : 0;
        double avg = doc["Средний_балл"] ? doc["Средний_балл"].get_double().value : 0.0;
        
        totalAverage += avg;
        
        // Ищем максимальный балл
        if (avg > maxAverage) {
            maxAverage = avg;
        }
        
        // Выводим студента
        std::cout << index << ") "
                  << lastName << " " << firstName << " " << middleName
                  << ", возраст: " << age
                  << ", группа: " << group
                  << ", средний балл: " << std::fixed << std::setprecision(2) << avg
                  << std::defaultfloat
                  << std::endl;
    }
    
    if (count > 0) {
        double groupAverage = totalAverage / static_cast<double>(count);
        std::cout << "Итого студентов: " << count
                  << ", средний балл по выборке: "
                  << std::fixed << std::setprecision(2) << groupAverage
                  << std::defaultfloat
                  << std::endl;
        
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
    
    clear_filter();
    
    // Фильтр: возраст меньше 19
    build_filter(
        "Возраст",
        "$lt",
        19
    );
    
    print_collection(collection, filter);
}
