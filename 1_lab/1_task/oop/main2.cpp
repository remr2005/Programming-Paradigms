#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/collection.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/types/bson_value/value.hpp>
#include <iostream>
#include <iomanip>

class MongoDBHandler {
private:
    mongocxx::instance instance;
    mongocxx::client client;
    mongocxx::database db;
    mongocxx::collection collection;
    bsoncxx::builder::basic::document filter_;

public:
    // Конструктор
    MongoDBHandler(const std::string& uri,
                   const std::string& db_name,
                   const std::string& coll_name)
        : client{mongocxx::uri{uri}},
          db{client[db_name]},
          collection{db[coll_name]} {}

    // Метод для построения фильтра (дополняет фильтр новыми условиями)
    void build_filter(
        const std::string& field,
        const std::string& op,
        const bsoncxx::types::bson_value::value& value
    ) {
        // Дополняем фильтр новыми условиями (не очищаем!)
        filter_.append(bsoncxx::builder::basic::kvp(
            field,
            bsoncxx::builder::basic::make_document(
                bsoncxx::builder::basic::kvp(op, value)
            )
        ));
    }
    
    // Метод для очистки фильтра (если нужно начать заново)
    void clear_filter() {
        filter_ = bsoncxx::builder::basic::document{};
    }

    // Метод для вывода документов (использует фильтр класса)
    void print_collection() {
        auto cursor = collection.find(filter_.view());

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
};

int main() {
    try {
        MongoDBHandler handler("mongodb://localhost:27017", "university", "students");

        std::cout << "Студенты: фамилия на 'А'" << std::endl;

        // Очищаем фильтр перед началом
        handler.clear_filter();
        
        // фамилия начинается с "А"
        handler.build_filter(
            "Фамилия",
            "$regex",
            "^А"
        );

        handler.print_collection();

    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
    }
}
