cmake_minimum_required(VERSION 3.16)
project(TicTacToeInfinite LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()


# wxWidgets
set(wxWidgets_ROOT_DIR "C:/wxWidgets")

find_package(wxWidgets REQUIRED COMPONENTS core base)
include(${wxWidgets_USE_FILE})

find_package(Threads REQUIRED)


# Приложение
add_executable(tictactoe_gui WIN32
    src/main.cpp
    src/gui/MainWindow.hpp
)

target_include_directories(tictactoe_gui PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(tictactoe_gui PRIVATE
    ${wxWidgets_LIBRARIES}
    Threads::Threads
)


# GoogleTest
include(FetchContent)

if(MSVC)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)

FetchContent_MakeAvailable(googletest)

enable_testing()

# Тесты
add_executable(run_game_tests
    tests/gtest_main.cpp
    tests/point_tests.cpp
    tests/dictionary_tests.cpp
    tests/board_tests.cpp
    tests/minimax_tests.cpp
)

target_include_directories(run_game_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(run_game_tests PRIVATE
    GTest::gtest
)

add_test(
    NAME run_all_game_tests
    COMMAND run_game_tests
)