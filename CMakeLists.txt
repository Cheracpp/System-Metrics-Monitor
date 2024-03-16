cmake_minimum_required (VERSION 3.0)

# --- Fetch FTXUI --------------------------------------------------------------
include(FetchContent)
set(FETCHCONTENT_UPDATES_DISCONNECTED TRUE)
set(FETCHCONTENT_QUIET FALSE)

FetchContent_Declare(ftxui
        GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
        GIT_TAG b2853c8f147bfa032688773ea05027f80b317a57
        GIT_PROGRESS TRUE
        GIT_SHALLOW FALSE
)

FetchContent_GetProperties(ftxui)
if(NOT ftxui_POPULATED)
    FetchContent_Populate(ftxui)
    add_subdirectory(
            ${ftxui_SOURCE_DIR}
            ${ftxui_BINARY_DIR}
            EXCLUDE_FROM_ALL
    )
endif()

# --- Build --------------------------------------------------------------------

project(System_Metrics_Monitor
        LANGUAGES CXX
        VERSION 1.0.0
)
add_executable(System_Metrics_Monitor src/main.cpp src/monitor/cpumonitor.cpp src/monitor/cpumonitor.h src/monitor/memorymonitor.cpp src/monitor/memorymonitor.h src/monitor/diskmonitor.cpp src/monitor/diskmonitor.h src/ui/terminalui.cpp src/ui/terminalui.h src/monitor/process.cpp src/monitor/process.h src/monitor/processmonitor.cpp src/monitor/processmonitor.h)


target_include_directories(System_Metrics_Monitor PRIVATE src)
target_link_libraries(System_Metrics_Monitor
        PRIVATE ftxui::screen
        PRIVATE ftxui::dom
        PRIVATE ftxui::component
)
set_target_properties(System_Metrics_Monitor PROPERTIES CXX_STANDARD 20)