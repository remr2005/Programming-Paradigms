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

# Поиск Eigen3
find_package(Eigen3 QUIET)
if(NOT Eigen3_FOUND)
    # Если Eigen не найден через find_package, попробуем стандартные пути
    find_path(EIGEN3_INCLUDE_DIR 
        NAMES Eigen/Dense
        PATHS
        /usr/include/eigen3
        /usr/local/include/eigen3
        /opt/homebrew/include/eigen3
    )
    if(EIGEN3_INCLUDE_DIR)
        message(STATUS "Eigen3 found at: ${EIGEN3_INCLUDE_DIR}")
    else()
        message(WARNING "Eigen3 not found. Please install it: sudo apt-get install libeigen3-dev")
    endif()
endif()

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
#add_executable(procedural2_main1 2_task/procedur/main1.cpp)
#add_executable(procedural2_main2 2_task/procedur/main2.cpp)

# ООП парадигма
#add_executable(oop2_main1 2_task/oop/main1.cpp)
#add_executable(oop2_main2 2_task/oop/main2.cpp)

# Императивная парадигма
#add_executable(imperative2_main1 2_task/imperativ/main1.cpp)
#add_executable(imperative2_main2 2_task/imperativ/main2.cpp)

add_executable(NN_main task2_experimental/oop/main1.cpp)

# Линковка для первого таска
target_link_libraries(procedural_main1 PRIVATE mongocxx bsoncxx)
target_link_libraries(procedural_main2 PRIVATE mongocxx bsoncxx)
target_link_libraries(oop_main1 PRIVATE mongocxx bsoncxx)
target_link_libraries(oop_main2 PRIVATE mongocxx bsoncxx)
target_link_libraries(imperative_main1 PRIVATE mongocxx bsoncxx)
target_link_libraries(imperative_main2 PRIVATE mongocxx bsoncxx)

# Линковка для второго таска
#target_link_libraries(procedural2_main1 PRIVATE mongocxx bsoncxx)
#target_link_libraries(procedural2_main2 PRIVATE mongocxx bsoncxx)
#target_link_libraries(oop2_main1 PRIVATE mongocxx bsoncxx)
#target_link_libraries(oop2_main2 PRIVATE mongocxx bsoncxx)
#target_link_libraries(imperative2_main1 PRIVATE mongocxx bsoncxx)
#target_link_libraries(imperative2_main2 PRIVATE mongocxx bsoncxx)


# Подключение Eigen для NN_main
if(Eigen3_FOUND)
    target_include_directories(NN_main PRIVATE ${EIGEN3_INCLUDE_DIR})
elseif(EIGEN3_INCLUDE_DIR)
    target_include_directories(NN_main PRIVATE ${EIGEN3_INCLUDE_DIR})
endif()
target_link_libraries(NN_main PRIVATE)