cmake_minimum_required(VERSION 3.5)

project(nekoray VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network Svg)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network Svg)

# Windows
include("cmake/fuck_windows/fuck.cmake")

# My proto
include("cmake/myproto.cmake")

# My dependencies
include("cmake/print.cmake")
#list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/libs/deps/built")

# yaml-cpp (static)
#find_package(yaml-cpp REQUIRED) # not working with CMAKE_PREFIX_PATH ??
find_package(PkgConfig REQUIRED)
pkg_check_modules(YAML_CPP REQUIRED yaml-cpp)
message(STATUS "[yaml-cpp] ${YAML_CPP_LINK_LIBRARIES}")

set(PROJECT_SOURCES
        main.cpp
        NekoRay.cpp

        3rdparty/qrcodegen.cpp

        qv2ray/ui/LogHighlighter.cpp
        qv2ray/ui/QvAutoCompleteTextEdit.cpp
        qv2ray/utils/HTTPRequestHelper.cpp
        qv2ray/components/proxy/QvProxyConfigurator.cpp
        qv2ray/ui/widgets/common/QJsonModel.cpp

        qv2ray/ui/widgets/editors/w_JsonEditor.cpp
        qv2ray/ui/widgets/editors/w_JsonEditor.hpp
        qv2ray/ui/widgets/editors/w_JsonEditor.ui

        rpc/gRPC.cpp

        db/Database.cpp
        db/traffic/TrafficLooper.cpp
        db/filter/ProfileFilter.cpp

        fmt/Bean2CoreObj.cpp
        fmt/Bean2Link.cpp
        fmt/Link2Bean.cpp
        fmt/ConfigBuilder.cpp

        sub/RawUpdater.cpp

        ui/mainwindow.cpp
        ui/mainwindow.h
        ui/mainwindow.ui

        ui/edit/dialog_edit_profile.h
        ui/edit/dialog_edit_profile.cpp
        ui/edit/dialog_edit_profile.ui
        ui/edit/dialog_edit_group.h
        ui/edit/dialog_edit_group.cpp
        ui/edit/dialog_edit_group.ui

        ui/edit/edit_chain.h
        ui/edit/edit_chain.cpp
        ui/edit/edit_chain.ui
        ui/edit/edit_socks.h
        ui/edit/edit_socks.cpp
        ui/edit/edit_socks.ui
        ui/edit/edit_shadowsocks.h
        ui/edit/edit_shadowsocks.cpp
        ui/edit/edit_shadowsocks.ui
        ui/edit/edit_vmess.h
        ui/edit/edit_vmess.cpp
        ui/edit/edit_vmess.ui

        ui/dialog_basic_settings.cpp
        ui/dialog_basic_settings.h
        ui/dialog_basic_settings.ui

        ui/dialog_manage_groups.cpp
        ui/dialog_manage_groups.h
        ui/dialog_manage_groups.ui

        ui/dialog_manage_routes.cpp
        ui/dialog_manage_routes.h
        ui/dialog_manage_routes.ui

        res/neko.qrc
        )

if (${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(nekoray
            MANUAL_FINALIZATION
            ${PROJECT_SOURCES}
            )
    # Define target properties for Android with Qt 6 as:
    #    set_property(TARGET nekoray APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
    #                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
    # For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else ()
    if (ANDROID)
        add_library(nekoray SHARED
                ${PROJECT_SOURCES}
                )
        # Define properties for Android with Qt 5 after find_package() calls as:
        #    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else ()
        add_executable(nekoray
                ${PROJECT_SOURCES}
                )
    endif ()
endif ()

target_include_directories(nekoray PRIVATE
        ${YAML_CPP_INCLUDE_DIRS}
        )

if (MINGW)
    unset(YAML_CPP_LINK_LIBRARIES)
    unset(PLATFORM_FUCKING_LIBRARIES)
else ()
    set(MY_PROTO_LIBRARIES myproto)
endif ()

target_link_libraries(nekoray PRIVATE
        ${MY_PROTO_LIBRARIES}
        Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network Qt${QT_VERSION_MAJOR}::Svg
        ${YAML_CPP_LINK_LIBRARIES}
        ${PLATFORM_FUCKING_LIBRARIES}
        )

set_target_properties(nekoray PROPERTIES
        MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        MACOSX_BUNDLE TRUE
        WIN32_EXECUTABLE TRUE
        )

if (QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(nekoray)
endif ()
