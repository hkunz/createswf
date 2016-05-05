cmake_minimum_required(VERSION 2.8.11)
project(createswf)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5Widgets)
find_package(Qt5Core)
find_package(Qt5Xml)

file(GLOB_RECURSE CPP_FILES ${PROJECT_SOURCE_DIR}/src/ "*.cpp")
file(GLOB_RECURSE CPP_HEADERS *.h)
file(GLOB_RECURSE UI_FILES *.ui)
file(GLOB_RECURSE RES_FILES *.qrc)


qt5_add_resources(ResourceSources ${RES_FILES})

qt5_wrap_cpp(CPP_SOURCES ${CPP_FILES})
qt5_wrap_ui(UI_SOURCES ${UI_FILES})

message(STATUS "${PROJECT_SOURCE_DIR} ===== ${CPP_FILES}")

get_target_property(QtCore_location Qt5::Core LOCATION)
get_target_property(QtXml_location Qt5::Xml LOCATION)
get_target_property(QtWidgets_location Qt5::Widgets LOCATION)

set(CMAKE_CXX_FLAGS_COVERAGE "${CMAKE_CXX_FLAGS_RELEASE} -fprofile-arcs -ftest-coverage")
set_target_properties(Qt5::Core PROPERTIES MAP_IMPORTED_CONFIG_COVERAGE "RELEASE")

add_executable(createswf src/main.cpp ${CPP_SOURCES} ${CPP_HEADERS} ${UI_SOURCES} ${ResourceSources})

target_link_libraries(createswf Qt5::Widgets Qt5::Xml Qt5::Core)
