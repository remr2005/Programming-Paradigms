cmake_minimum_required(VERSION 3.10)
project(paradigm_lab CXX)

set(CMAKE_CXX_STANDARD 17)

# Пути к заголовкам
include_directories(
    /usr/local/include/mongocxx/v_noabi
    /usr/local/include/bsoncxx/v_noabi
    /usr/include/eigen3
)

# Пути к библиотекам
link_directories(/usr/local/lib)

# Первый таск
# Процедурная парадигма
add_executable(procedural_main1 1_task/procedur/main1.cpp)
add_executable(procedural_main2 1_task/procedur/main2.cpp)

# ООП парадигма
add_executable(oop_main1 1_task/oop/main1.cpp)
add_executable(oop_main2 1_task/oop/main2.cpp)

# Императивная парадигма
add_executable(imperative_main1 1_task/imperativ/main1.cpp)
add_executable(imperative_main2 1_task/imperativ/main2.cpp)

# Второй таск
# Процедурная парадигма
add_executable(procedural2_main1 2_task/procedur/main1.cpp)
add_executable(procedural2_main2 2_task/procedur/main2.cpp)

# ООП парадигма
add_executable(oop2_main1 2_task/oop/main1.cpp)
add_executable(oop2_main2 2_task/oop/main2.cpp)

# Императивная парадигма
add_executable(imperative2_main1 2_task/imperativ/main1.cpp)
add_executable(imperative2_main2 2_task/imperativ/main2.cpp)

# Линковка
target_link_libraries(procedural_main1 PRIVATE mongocxx bsoncxx)
target_link_libraries(procedural_main2 PRIVATE mongocxx bsoncxx)
target_link_libraries(oop_main1 PRIVATE mongocxx bsoncxx)
target_link_libraries(oop_main2 PRIVATE mongocxx bsoncxx)
target_link_libraries(imperative_main1 PRIVATE mongocxx bsoncxx)
target_link_libraries(imperative_main2 PRIVATE mongocxx bsoncxx)