cmake_minimum_required(VERSION 3.5)

project(nekoray VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network Svg LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network Svg LinguistTools)

# Windows
include("cmake/fuck_windows/fuck.cmake")

# My dependencies
include("cmake/print.cmake")
set(MY_DEPS_DIR "${CMAKE_SOURCE_DIR}/libs/deps/built")
list(APPEND CMAKE_PREFIX_PATH ${MY_DEPS_DIR})
#set(ENV{PKG_CONFIG_PATH} "${MY_DEPS_DIR}/share/pkgconfig")
#find_package(PkgConfig REQUIRED)

# NKR
include("cmake/nkr.cmake")

if (NKR_NO_EXTERNAL)
    add_compile_definitions(NKR_NO_EXTERNAL)
else ()
    if (NKR_NO_GRPC)
        add_compile_definitions(NKR_NO_GRPC)
    else ()
        # My proto
        include("cmake/myproto.cmake")
        list(APPEND NKR_EXTERNAL_TARGETS myproto)
    endif ()

    # yaml-cpp (static)
    find_package(yaml-cpp CONFIG REQUIRED) # only Release is built
    list(APPEND NKR_EXTERNAL_TARGETS yaml-cpp)

    # zxing-cpp
    find_package(ZXing CONFIG REQUIRED)
    list(APPEND NKR_EXTERNAL_TARGETS ZXing::ZXing)
endif ()

# debug print
if (DBG_CMAKE)
    print_all_variables()
    print_target_properties(myproto)
    print_target_properties(yaml-cpp)
    print_target_properties(ZXing::ZXing)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CMAKE_COMMAND} -E time")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK "${CMAKE_COMMAND} -E time")
endif ()

# Sources
set(PROJECT_SOURCES
        ${PLATFORM_FUCKING_SOURCES}

        main/main.cpp
        main/NekoRay.cpp

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
        db/TrafficLooper.cpp
        db/ProfileFilter.cpp

        fmt/Bean2CoreObj.cpp
        fmt/Bean2External.cpp
        fmt/Bean2Link.cpp
        fmt/Link2Bean.cpp
        fmt/ConfigBuilder.cpp

        sub/RawUpdater.cpp

        sys/ExternalProcess.cpp

        ui/CheckUpdate.cpp
        ui/ThemeManager.cpp

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
        ui/edit/edit_trojan.h
        ui/edit/edit_trojan.cpp
        ui/edit/edit_trojan.ui

        ui/edit/edit_naive.h
        ui/edit/edit_naive.cpp
        ui/edit/edit_naive.ui

        ui/edit/edit_custom.h
        ui/edit/edit_custom.cpp
        ui/edit/edit_custom.ui

        ui/dialog_basic_settings.cpp
        ui/dialog_basic_settings.h
        ui/dialog_basic_settings.ui

        ui/dialog_manage_groups.cpp
        ui/dialog_manage_groups.h
        ui/dialog_manage_groups.ui

        ui/dialog_manage_routes.cpp
        ui/dialog_manage_routes.h
        ui/dialog_manage_routes.ui

        ui/widget/ProxyItem.cpp
        ui/widget/ProxyItem.h
        ui/widget/ProxyItem.ui
        ui/widget/GroupItem.cpp
        ui/widget/GroupItem.h
        ui/widget/GroupItem.ui

        res/neko.qrc
        res/theme/feiyangqingyun/qss.qrc
        ${QV2RAY_RC}
        )

# Translations
set(TS_FILES
        translations/zh_CN.ts
        )
qt_create_translation(QM_FILES ${PROJECT_SOURCES} ${TS_FILES} OPTIONS -locations none)
configure_file(translations/translations.qrc ${CMAKE_BINARY_DIR} COPYONLY)
set(PROJECT_SOURCES ${PROJECT_SOURCES} ${TS_FILES} ${QM_FILES} ${CMAKE_BINARY_DIR}/translations.qrc)

# Qt exe
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

# Target

set_property(TARGET nekoray PROPERTY AUTOUIC ON)
set_property(TARGET nekoray PROPERTY AUTOMOC ON)
set_property(TARGET nekoray PROPERTY AUTORCC ON)

# Target Link

target_link_libraries(nekoray PRIVATE
        Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network Qt${QT_VERSION_MAJOR}::Svg
        ${NKR_EXTERNAL_TARGETS}
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
