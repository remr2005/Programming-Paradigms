#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/collection.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/types/bson_value/value.hpp>
#include <iostream>

class MongoDBHandler {
private:
    mongocxx::instance instance;   // один на всё приложение
    mongocxx::client client;
    mongocxx::database db;
    mongocxx::collection collection;

public:
    // Конструктор
    MongoDBHandler(const std::string& uri,
                   const std::string& db_name,
                   const std::string& coll_name)
        : client{mongocxx::uri{uri}},
          db{client[db_name]},
          collection{db[coll_name]} {}

    // Метод для построения фильтра
    bsoncxx::builder::basic::document build_filter(
        const std::string& field,
        const std::string& op,
        const bsoncxx::types::bson_value::value& value
    ) {
        bsoncxx::builder::basic::document filter;

        if (op == "$regex") {
            // regex кладём напрямую
            filter.append(bsoncxx::builder::basic::kvp(field, value));
        } else {
            // обычные операторы
            filter.append(bsoncxx::builder::basic::kvp(
                field,
                bsoncxx::builder::basic::make_document(
                    bsoncxx::builder::basic::kvp(op, value)
                )
            ));
        }

        return filter;
    }

    // Метод для вывода документов
    void print_collection(const bsoncxx::builder::basic::document& filter) {
        auto cursor = collection.find(filter.view());
        for (auto&& doc : cursor) {
            std::cout << bsoncxx::to_json(doc) << std::endl;
        }
    }
};

int main() {
    try {
        MongoDBHandler handler("mongodb://localhost:27017", "university", "students");

        std::cout << "Фамилия на А:" << std::endl;

        auto filter = handler.build_filter(
            "Фамилия",
            "$regex",
            bsoncxx::types::bson_value::value{bsoncxx::types::b_regex{"^А"}}
        );

        handler.print_collection(filter);

    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
    }
    return 0;
}
