#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <iostream>
#include <iomanip>

int main() {
    // Инициализация драйвера
    mongocxx::instance instance{};
    mongocxx::client client{mongocxx::uri{"mongodb://localhost:27017"}};

    auto db = client["university"];
    auto collection = db["students"];

    // Создаем фильтр: "Возраст" < 19
    bsoncxx::builder::basic::document filter_builder;
    filter_builder.append(bsoncxx::builder::basic::kvp("Возраст", bsoncxx::builder::basic::make_document(
        bsoncxx::builder::basic::kvp("$lt", 19)
    )));

    auto cursor = collection.find(filter_builder.view());

    std::cout << "Студенты:" << std::endl;

    std::size_t index = 0;
    std::size_t count = 0;
    double totalAverage = 0.0;

    for (auto&& doc : cursor) {
        ++index;
        ++count;

        // Извлекаем поля
        std::string firstName = doc["Имя"] ? std::string(doc["Имя"].get_string().value) : "";
        std::string lastName  = doc["Фамилия"] ? std::string(doc["Фамилия"].get_string().value) : "";
        std::string group     = doc["Группа"] ? std::string(doc["Группа"].get_string().value) : "";
        
        // Возраст
        int32_t age = doc["Возраст"] ? doc["Возраст"].get_int32().value : 0;
        
        // Средний балл
        double avg = doc["Средний_балл"] ? doc["Средний_балл"].get_double().value : 0.0;
        totalAverage += avg;

        // Красивый вывод в одну строку на студента
        std::cout << index << ") "
                  << lastName << " " << firstName
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
    } else {
        std::cout << "По заданному фильтру студентов не найдено." << std::endl;
    }
}
