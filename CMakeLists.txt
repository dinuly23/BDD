set (CMAKE_CXX_FLAGS "--std=gnu++11 ${CMAKE_CXX_FLAGS}")
cmake_minimum_required(VERSION 2.8)
# работаем с библиотекой BuDDy, установленной куда-то в стандартные пути
find_library(BuDDy bdd)

add_executable(main main.cpp)

# подключаем библиотеку BuDDy
target_link_libraries(main ${BuDDy})
