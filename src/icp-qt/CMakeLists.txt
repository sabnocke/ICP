cmake_minimum_required(VERSION 3.16)

project(icp-qt VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        GraphicsScene.cpp
        GraphicsScene.h
        StateItem.cpp
        StateItem.h
        TransitionItem.h
        TransitionItem.cpp
        EditorMode.h
        AutomatModel.cpp
        AutomatModel.h
	
        ${CMAKE_SOURCE_DIR}/fsm/ParserLib.cpp
        ${CMAKE_SOURCE_DIR}/fsm/ParserLib.h
	    ${CMAKE_SOURCE_DIR}/fsm/Interpret.cpp
    	${CMAKE_SOURCE_DIR}/fsm/Interpret.h
        ${CMAKE_SOURCE_DIR}/fsm/Utils.cpp
        ${CMAKE_SOURCE_DIR}/fsm/Utils.h
        ${CMAKE_SOURCE_DIR}/fsm/AutomatLib.h
        ${CMAKE_SOURCE_DIR}/fsm/types/variables.h
)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(icp-qt
        MANUAL_FINALIZATION

        ${PROJECT_SOURCES}
        GraphicsScene.h GraphicsScene.cpp
        StateItem.h StateItem.cpp
        TransitionItem.h TransitionItem.cpp
        EditorMode.h
        AutomatModel.h AutomatModel.cpp
    )
    target_include_directories(icp-qt PRIVATE
      ${CMAKE_SOURCE_DIR}/fsm
      ${CMAKE_SOURCE_DIR}/fsm/external
    )

    target_link_libraries(icp-qt PRIVATE
      Qt${QT_VERSION_MAJOR}::Widgets
      range-v3::range-v3
      re2::re2
      absl::btree
      absl::container_common
      absl::log
      absl::log_initialize
      lua
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET icp-qt APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(icp-qt SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(icp-qt
            ${PROJECT_SOURCES}
        )
        target_include_directories(icp-qt PRIVATE
          ${CMAKE_SOURCE_DIR}/fsm
          ${CMAKE_SOURCE_DIR}/fsm/external
        )

        target_link_libraries(icp-qt PRIVATE
          Qt${QT_VERSION_MAJOR}::Widgets
          range-v3::range-v3
          re2::re2
          absl::btree
          absl::container_common
          absl::log
          absl::log_initialize
          lua
        )
    endif()
endif()

target_link_libraries(icp-qt PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.icp-qt)
endif()
set_target_properties(icp-qt PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS icp-qt
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(icp-qt)
endif()
