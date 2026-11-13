cmake_minimum_required(VERSION 3.19)
project(Battery-Chker LANGUAGES CXX RC)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets LinguistTools)

qt_standard_project_setup()

qt_add_executable(Battery-Chker
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    resouces.qrc
    icon.rc
)

qt_add_translations(
    TARGETS Battery-Chker
    TS_FILES Battery-Chker_zh_CN.ts
    TS_FILES Battery-Chker_en.ts
)

target_link_libraries(Battery-Chker
    PRIVATE
        Qt::Core
        Qt::Widgets
        setupapi
)

include(CheckIPOSupported)
check_ipo_supported(RESULT BC_IPO_SUPPORTED LANGUAGES CXX)

target_compile_options(Battery-Chker PRIVATE
    $<$<NOT:$<CONFIG:Debug>>:-Os>
    $<$<NOT:$<CONFIG:Debug>>:-ffunction-sections>
    $<$<NOT:$<CONFIG:Debug>>:-fdata-sections>
)

target_link_options(Battery-Chker PRIVATE
    $<$<NOT:$<CONFIG:Debug>>:-Wl,--gc-sections>
    $<$<NOT:$<CONFIG:Debug>>:-Wl,--strip-all>
)

if(BC_IPO_SUPPORTED)
    target_compile_options(Battery-Chker PRIVATE $<$<NOT:$<CONFIG:Debug>>:-flto>)
    target_link_options(Battery-Chker PRIVATE $<$<NOT:$<CONFIG:Debug>>:-flto>)
endif()

install(TARGETS Battery-Chker
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET Battery-Chker
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
