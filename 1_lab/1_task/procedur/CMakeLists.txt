cmake_minimum_required(VERSION 3.10)
project(mongo_example CXX)

set(CMAKE_CXX_STANDARD 17)

# Пути к заголовкам
include_directories(
    /usr/local/include/mongocxx/v_noabi
    /usr/local/include/bsoncxx/v_noabi
)

# Пути к библиотекам
link_directories(/usr/local/lib)

add_executable(main1 main1.cpp)
add_executable(main2 main2.cpp)
add_executable(main3 main3.cpp)

# Линкуем
target_link_libraries(main1
    PRIVATE
    mongocxx
    bsoncxx
)

target_link_libraries(main2
    PRIVATE
    mongocxx
    bsoncxx
)

target_link_libraries(main3
    PRIVATE
    mongocxx
    bsoncxx
)
