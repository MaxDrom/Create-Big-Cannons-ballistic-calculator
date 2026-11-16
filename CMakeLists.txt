cmake_minimum_required(VERSION 3.16)
project(ballistics_py LANGUAGES CXX)

# Включаем автоматическую генерацию compile_commands.json для Neovim LSP
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Устанавливаем стандарт C++23, как у вас в команде clang++
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Ищем Python (интерпретатор и файлы разработки)
find_package(Python COMPONENTS Interpreter Development REQUIRED)

# Ищем системный pybind11 (пакет Arch: python-pybind11 или pybind11)
find_package(pybind11 REQUIRED)

# Создаем python-модуль (заменяет add_library + настройку флагов -shared -fPIC)
pybind11_add_module(ballistics_py ${CMAKE_CURRENT_SOURCE_DIR}/src/py.cpp)
target_include_directories(ballistics_py PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_options(ballistics_py PRIVATE -O3 -march=native)

# Связываем модуль с Python (автоматически пропишет пути к Python.h)
target_link_libraries(ballistics_py PRIVATE pybind11::module Python::Python)

add_custom_command(TARGET ballistics_py POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/src/main.py ${CMAKE_CURRENT_BINARY_DIR} )

# Стендаонный C++ пример (без Python)
add_executable(ballistics_example ${CMAKE_CURRENT_SOURCE_DIR}/src/example.cpp)
target_include_directories(ballistics_example PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_compile_options(ballistics_example PRIVATE -O3 -march=native)

# Тесты квадратичной баллистики (V = 8, drag = 0.001, k = 6)
enable_testing()
add_executable(ballistics_test ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_quadratic.cpp)
target_include_directories(ballistics_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_compile_options(ballistics_test PRIVATE -O3 -march=native)
add_test(NAME ballistics_test COMMAND ballistics_test)
