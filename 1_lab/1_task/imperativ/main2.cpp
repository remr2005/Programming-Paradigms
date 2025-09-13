#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <iostream>

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

    auto cursor = collection.find(filter_builder.view());

    for (auto&& doc : cursor) {
        std::cout << bsoncxx::to_json(doc) << std::endl;
    }

    return 0;
}
