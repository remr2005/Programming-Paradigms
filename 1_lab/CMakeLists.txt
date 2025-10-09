cmake_minimum_required(VERSION 3.10)
project(paradigm_lab CXX)

set(CMAKE_CXX_STANDARD 17)

# Пути к заголовкам MongoDB
include_directories(
    /usr/local/include/mongocxx/v_noabi
    /usr/local/include/bsoncxx/v_noabi
)

# Пути к библиотекам
link_directories(/usr/local/lib)

# Процедурная парадигма
add_executable(procedural_main1 1_task/procedur/main1.cpp)
add_executable(procedural_main2 1_task/procedur/main2.cpp)
add_executable(procedural_main3 1_task/procedur/main3.cpp)

# ООП парадигма
add_executable(oop_main1 1_task/oop/main1.cpp)
add_executable(oop_main2 1_task/oop/main2.cpp)
add_executable(oop_main3 1_task/oop/main3.cpp)

# Императивная парадигма
add_executable(imperative_main1 1_task/imperativ/main1.cpp)
add_executable(imperative_main2 1_task/imperativ/main2.cpp)
add_executable(imperative_main3 1_task/imperativ/main3.cpp)

# Линковка для процедурной парадигмы
target_link_libraries(procedural_main1 PRIVATE mongocxx bsoncxx)
target_link_libraries(procedural_main2 PRIVATE mongocxx bsoncxx)
target_link_libraries(procedural_main3 PRIVATE mongocxx bsoncxx)

# Линковка для ООП парадигмы
target_link_libraries(oop_main1 PRIVATE mongocxx bsoncxx)
target_link_libraries(oop_main2 PRIVATE mongocxx bsoncxx)
target_link_libraries(oop_main3 PRIVATE mongocxx bsoncxx)

# Линковка для императивной парадигмы
target_link_libraries(imperative_main1 PRIVATE mongocxx bsoncxx)
target_link_libraries(imperative_main2 PRIVATE mongocxx bsoncxx)
target_link_libraries(imperative_main3 PRIVATE mongocxx bsoncxx)
