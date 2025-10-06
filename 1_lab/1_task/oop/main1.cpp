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

    // Метод для построения фильтра (обновляет поле класса)
    void build_filter(
        const std::string& field,
        const std::string& op,
        const bsoncxx::types::bson_value::value& value
    ) {
        filter_ = bsoncxx::builder::basic::document{};
        filter_.append(bsoncxx::builder::basic::kvp(
            field,
            bsoncxx::builder::basic::make_document(
                bsoncxx::builder::basic::kvp(op, value)
            )
        ));
    }

    // Метод для вывода документов (использует фильтр класса)
    void print_collection() {
        auto cursor = collection.find(filter_.view());
        for (auto&& doc : cursor) {
            std::cout << bsoncxx::to_json(doc) << std::endl;
        }
    }
};

int main() {
    try {
        MongoDBHandler handler("mongodb://localhost:27017", "university", "students");

        std::cout << "Возраст меньше 19" << std::endl;

        handler.build_filter(
            "Возраст",
            "$lt",
            bsoncxx::types::bson_value::value{bsoncxx::types::b_int64{19}}
        );

        handler.print_collection();

    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
    }

}
