if (WIN32)
    include(cmake/fuck_windows/generate_product_version.cmake)
    generate_product_version(
            QV2RAY_RC
            NAME "Nekoray"
            BUNDLE "Nekoray Project Family"
            #            ICON               "${CMAKE_SOURCE_DIR}/assets/icons/qv2ray.ico"
            #            VERSION_MAJOR      ${CMAKE_PROJECT_VERSION_MAJOR}
            #            VERSION_MINOR      ${CMAKE_PROJECT_VERSION_MINOR}
            #            VERSION_PATCH      ${CPACK_PACKAGE_VERSION_PATCH}
            #            VERSION_REVISION   ${QV2RAY_BUILD_VERSION}
            COMPANY_NAME "Nekoray Workgroup"
            COMPANY_COPYRIGHT "Nekoray Workgroup 2021"
            FILE_DESCRIPTION "Nekoray Main Application"
    )
    add_definitions(-DUNICODE -D_UNICODE -DNOMINMAX)
    set(GUI_TYPE WIN32)
    if (USE_MINGW)
        if (NOT DEFINED MinGW_ROOT)
            set(MinGW_ROOT "C:/msys64/mingw64")
        endif ()
    else ()
        add_compile_options("/utf-8")
        add_compile_options("/std:c++17")
        add_definitions(-D_WIN32_WINNT=0x600 -D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS)
    endif ()

    set(PLATFORM_FUCKING_LIBRARIES wininet wsock32 ws2_32 user32 Rasapi32 Iphlpapi)
endif ()
