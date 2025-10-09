#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/types.hpp>
#include <iostream>
#include <iomanip>

int main() {
    // Инициализация драйвера
    mongocxx::instance instance{};
    mongocxx::client client{mongocxx::uri{"mongodb://localhost:27017"}};

    auto db = client["university"];
    auto collection = db["students"];

    
    bsoncxx::builder::basic::document filter_builder;
    filter_builder.append(bsoncxx::builder::basic::kvp("Средний_балл", bsoncxx::builder::basic::make_document(
        bsoncxx::builder::basic::kvp("$lt", 70.0)
    )));
    filter_builder.append(bsoncxx::builder::basic::kvp("Возраст", bsoncxx::builder::basic::make_document(
        bsoncxx::builder::basic::kvp("$lt", 19)
    )));


    auto cursor = collection.find(filter_builder.view());

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
        
        // Средний балл
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
        
        // Поиск максимального среднего балла
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
        
        // Вывод максимального среднего балла
        std::cout << "Максимальный средний балл среди найденных студентов: "
                  << std::fixed << std::setprecision(2) << maxAverage
                  << std::defaultfloat
                  << std::endl;
    } else {
        std::cout << "По заданному фильтру студентов не найдено." << std::endl;
    }
}
