#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/collection.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/types/bson_value/value.hpp>
#include <iostream>


bsoncxx::builder::basic::document filter;

void print_collection(mongocxx::collection& collection,
                      const bsoncxx::builder::basic::document& filter) {
    auto cursor = collection.find(filter.view());
    for (auto&& doc : cursor) {
        std::cout << bsoncxx::to_json(doc) << std::endl;
    }
}


void build_filter(
    const std::string& field,
    const std::string& op,
    const bsoncxx::types::bson_value::value& value
) {
    // Очищаем фильтр перед добавлением новых условий
    filter = bsoncxx::builder::basic::document{};
    
    filter.append(bsoncxx::builder::basic::kvp(
        field,
        bsoncxx::builder::basic::make_document(
            bsoncxx::builder::basic::kvp(op, value)
        )
    ));
}


int main() {
    mongocxx::instance inst{};
    mongocxx::client client{mongocxx::uri{"mongodb://localhost:27017"}};
    auto db = client["university"];
    auto collection = db["students"];

    std :: cout << "Возраст меньше 19" << std :: endl;
    // Пример: возраст < 19
    build_filter(
        "Возраст",
        "$lt",
        bsoncxx::types::bson_value::value{bsoncxx::types::b_int64{19}}
    );
    
    print_collection(collection, filter);
}
