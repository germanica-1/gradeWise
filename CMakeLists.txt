cmake_minimum_required(VERSION 3.19)
project(GwaCalculator VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network)
qt_standard_project_setup()
set(CMAKE_AUTORCC ON) # compiles resources/resources.qrc (the stylesheet) into the app

# Plain C++ logic (no Qt) shared by the app and the tests.
set(CORE_SOURCES
    src/core/GradingScale.cpp
    src/core/Calculator.cpp
    src/core/Validation.cpp
    src/core/SubjectManager.cpp
    src/ai/PromptBuilder.cpp
)

qt_add_executable(gwa_calculator WIN32
    src/main.cpp
    ${CORE_SOURCES}
    src/storage/Storage.cpp
    src/ai/GeminiClient.cpp
    src/gui/MainWindow.cpp
    src/gui/DashboardPage.cpp
    src/gui/SubjectsPage.cpp
    src/gui/FeedbackPage.cpp
    src/gui/SettingsPage.cpp
    resources/resources.qrc
)
target_link_libraries(gwa_calculator PRIVATE Qt6::Widgets Qt6::Network)

# Tests: plain asserts, run with `ctest --test-dir build`.
enable_testing()
add_executable(core_tests tests/core_tests.cpp ${CORE_SOURCES})
add_test(NAME core_tests COMMAND core_tests)
