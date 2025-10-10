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

    // Фильтр: Фамилия начинается с "А"
    bsoncxx::builder::basic::document filter_builder;
    filter_builder.append(
        bsoncxx::builder::basic::kvp("Фамилия", 
            bsoncxx::builder::basic::make_document(
                bsoncxx::builder::basic::kvp("$regex", "^А")
            )
        )
    );


    double count = 0;
    double totalAverage = 0.0;
    double maxAverage = 0.0;

    for (auto& doc : collection.find(filter_builder.view())) {
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


    // -------------------------------------------------------------
    // ВТОРАЯ ЗАДАЧА
    // -------------------------------------------------------------
    count = 0;
    filter_builder = bsoncxx::builder::basic::document{};
    filter_builder.append(bsoncxx::builder::basic::kvp("Средний_балл", bsoncxx::builder::basic::make_document(
        bsoncxx::builder::basic::kvp("$lt", 70.0)
    )));
    filter_builder.append(bsoncxx::builder::basic::kvp("Возраст", bsoncxx::builder::basic::make_document(
        bsoncxx::builder::basic::kvp("$lt", 19)
    )));

    
    for (auto& doc : collection.find(filter_builder.view())) {
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
        if (avg > maxAverage) {
            maxAverage = avg;
        }
    }

    if (count > 0) {
        double groupAverage = totalAverage / count;
        // Вывод максимального среднего балла
        std::cout << "Максимальный средний балл среди найденных студентов: "
                  << std::fixed << std::setprecision(2) << maxAverage
                  << std::defaultfloat
                  << std::endl;
    } else {
        std::cout << "По заданному фильтру студентов не найдено." << std::endl;
    }

}
