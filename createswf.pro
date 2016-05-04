TEMPLATE = app
TARGET = createswf
DEPENDPATH += . \
              src \
              src/common \
              src/constants \
              src/gui \
              src/images \
              src/parsers \
              src/ports \
              src/templates \
              src/translations
INCLUDEPATH += . src src/common src/constants src/gui src/parsers src/ports

# Input
HEADERS += config.h \
           src/CoreApplication.h \
           src/common/Compiler.h \
           src/common/Logger.h \
           src/common/MemoryAllocator.h \
           src/common/Version.h \
           src/constants/CompileMode.h \
           src/constants/Content.h \
           src/constants/FileType.h \
           src/gui/AboutDialog.h \
           src/gui/MainWindow.h \
           src/gui/PreferencesDialog.h \
           src/parsers/AbstractAssetsParser.h \
           src/parsers/CommandLineParser.h \
           src/parsers/DefinitionParser.h \
           src/parsers/DirectoryParser.h \
           src/ports/Darwin.h \
           src/ports/ISystem.h \
           src/ports/System.h \
           src/ports/Unix.h \
           src/ports/Windows.h
FORMS += src/gui/aboutdialog.ui \
         src/gui/mainwindow.ui \
         src/gui/preferencesdialog.ui
SOURCES += src/CoreApplication.cpp \
           src/main.cpp \
           src/common/Compiler.cpp \
           src/common/Logger.cpp \
           src/common/MemoryAllocator.cpp \
           src/gui/AboutDialog.cpp \
           src/gui/MainWindow.cpp \
           src/gui/PreferencesDialog.cpp \
           src/parsers/AbstractAssetsParser.cpp \
           src/parsers/CommandLineParser.cpp \
           src/parsers/DefinitionParser.cpp \
           src/parsers/DirectoryParser.cpp \
           src/ports/Darwin.cpp \
           src/ports/Unix.cpp \
           src/ports/Windows.cpp
RESOURCES += src/images/images.qrc src/templates/templates.qrc
TRANSLATIONS += src/translations/app_de.ts src/translations/app_en.ts

win32:RC_FILE = src/images/app.rc
unix:ICON = src/images/icon.icns

MOC_DIR = .moc
UI_DIR = .uic
RCC_DIR = .rcc
OBJECTS_DIR = .obj
QT += xml
