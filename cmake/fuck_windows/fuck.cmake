if (WIN32)
    set(PLATFORM_FUCKING_SOURCES 3rdparty/WinCommander.cpp)
    set(PLATFORM_FUCKING_LIBRARIES wininet wsock32 ws2_32 user32 rasapi32 iphlpapi)

    include(cmake/fuck_windows/generate_product_version.cmake)
    generate_product_version(
            QV2RAY_RC
            NAME "Nekoray"
            BUNDLE "Nekoray Project Family"
            ICON "${CMAKE_SOURCE_DIR}/res/nekoray.ico"
            COMPANY_NAME "Nekoray Workgroup"
            COMPANY_COPYRIGHT "Nekoray Workgroup"
            FILE_DESCRIPTION "Nekoray Main Application"
    )
    add_definitions(-DUNICODE -D_UNICODE -DNOMINMAX)
    set(GUI_TYPE WIN32)
    if (MINGW)
        if (NOT DEFINED MinGW_ROOT)
            set(MinGW_ROOT "C:/msys64/mingw64")
        endif ()
    else ()
        add_compile_options("/utf-8")
        add_compile_options("/std:c++17")
        add_definitions(-D_WIN32_WINNT=0x600 -D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS)
        list(APPEND PLATFORM_FUCKING_SOURCES sys/windows/MiniDump.cpp)
    endif ()
endif ()
