cmake_minimum_required(VERSION 2.8.12)
project(createswf)
include(${PROJECT_SOURCE_DIR}/cmake/shared.cmake)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(QT_USE_QTGUI TRUE)

find_package(Qt5Widgets REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Xml REQUIRED)
find_package(Qt5XmlPatterns REQUIRED)
find_package(Qt5LinguistTools REQUIRED)

file(GLOB CREATESWF_TRANSLATIONS ${ROOT_DIR}/src/translations/*.ts)

set(CREATESWF_DIRS
	${ROOT_DIR}/src
	${ROOT_DIR}/src/gui
	${ROOT_DIR}/src/common
	${ROOT_DIR}/src/constants
	${ROOT_DIR}/src/gui
	${ROOT_DIR}/src/parsers
	${ROOT_DIR}/src/ports)

unset(CREATESWF_SOURCES)
unset(CREATESWF_HEADERS)
foreach(DIR ${CREATESWF_DIRS})
file(GLOB CREATESWF_SOURCES_ ${DIR}/*.cpp)
file(GLOB CREATESWF_HEADERS_ ${DIR}/*.h)
list(APPEND CREATESWF_SOURCES ${CREATESWF_SOURCES_})
list(APPEND CREATESWF_HEADERS ${CREATESWF_HEADERS_})
unset(CREATESWF_SOURCES_)
unset(CREATESWF_HEADERS_)
endforeach()

add_definitions(-DHAVE_CONFIG_H)
include_directories(${CREATESWF_DIRS} ${AUTOGEN_TARGETS_FOLDER} ${LIBSSH2_INCLUDE_DIRS} ${AUTOMOC_TARGETS_FOLDER} ${CMAKE_BINARY_DIR} ${QT_QTCORE_INCLUDE_DIR} ${QT_QTXML_INCLUDE_DIR} ${QT_QTGUI_INCLUDE_DIR} ${QT_QTWIDGETS_INCLUDE_DIR})
qt5_add_translation(CREATESWF_QM ${CREATESWF_TRANSLATIONS})
qt5_add_resources(CREATESWF_RESOURCES
	${ROOT_DIR}/src/images/images.qrc
	${ROOT_DIR}/src/templates/templates.qrc)

qt5_wrap_ui(CREATESWF_UI
	${ROOT_DIR}/src/gui/mainwindow.ui
	${ROOT_DIR}/src/gui/aboutdialog.ui
	${ROOT_DIR}/src/gui/preferencesdialog.ui)

add_executable(${CMAKE_PROJECT_NAME} ${CREATESWF_SOURCES} ${CREATESWF_QM} ${CREATESWF_RESOURCES} ${CREATESWF_UI} ${CREATESWF_HEADERS})
#Static Linking is broken, bug logged at: https://bugreports.qt.io/browse/QTBUG-38913
target_link_libraries(${CMAKE_PROJECT_NAME} Qt5::Core Qt5::Widgets Qt5::Gui Qt5::Xml Qt5::Xml Qt5::XmlPatterns Qt5::Network)
