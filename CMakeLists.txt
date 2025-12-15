cmake_minimum_required(VERSION 3.16)

project(AutoWord LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network Sql)
find_package(Qt6 COMPONENTS TextToSpeech)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

qt_add_executable(AutoWord WIN32 MANUAL_FINALIZATION
    src/main.cpp
    src/ui/MainWindow.cpp
    src/ui/MainWindow.h
    src/db/DatabaseManager.cpp
    src/db/DatabaseManager.h
    src/ui/ThemeManager.cpp
    src/ui/ThemeManager.h
    src/core/DictionaryParser.cpp
    src/core/DictionaryParser.h
    src/core/WordModel.cpp
    src/core/WordModel.h
    src/ui/PreviewView.cpp
    src/ui/PreviewView.h
    src/core/FsrsScheduler.cpp
    src/core/FsrsScheduler.h
    src/network/WebDavClient.cpp
    src/network/WebDavClient.h
    src/core/TtsEngine.cpp
    src/core/TtsEngine.h
    src/ui/StudyView.cpp
    src/ui/StudyView.h
    src/ui/TestView.cpp
    src/ui/TestView.h
    src/ui/SettingsDialog.cpp
    src/ui/SettingsDialog.h
    src/ui/DashboardView.cpp
    src/ui/DashboardView.h
    src/ui/LearningChart.cpp
    src/ui/LearningChart.h
)

target_link_libraries(AutoWord PRIVATE Qt6::Widgets Qt6::Network Qt6::Sql)

target_include_directories(AutoWord PRIVATE src)

if(Qt6TextToSpeech_FOUND)
    target_link_libraries(AutoWord PRIVATE Qt6::TextToSpeech)
    target_compile_definitions(AutoWord PRIVATE HAVE_QT_TTS)
endif()

if(WIN32)
    set_target_properties(AutoWord PROPERTIES WIN32_EXECUTABLE ON)
    set_target_properties(AutoWord PROPERTIES OUTPUT_NAME "AutoWord_v1.8.3")
endif()

qt_finalize_executable(AutoWord)
