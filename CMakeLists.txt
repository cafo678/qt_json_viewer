cmake_minimum_required(VERSION 3.16)

project(SimpleFileViewer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)
find_package(Qt6 REQUIRED COMPONENTS Concurrent)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(appSimpleFileViewer
    main.cpp
)

qt_add_qml_module(appSimpleFileViewer
    URI SimpleFileViewer
    QML_FILES
        Main.qml
        SOURCES filereader.h filereader.cpp
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appSimpleFileViewer PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appSimpleFileViewer
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appSimpleFileViewer
    PRIVATE Qt6::Quick
    PRIVATE Qt6::Concurrent
)

include(GNUInstallDirs)
install(TARGETS appSimpleFileViewer
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
