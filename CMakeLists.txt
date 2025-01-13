cmake_minimum_required(VERSION 3.0)

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
if (NOT ftxui_POPULATED)
    FetchContent_Populate(ftxui)
    add_subdirectory(
            ${ftxui_SOURCE_DIR}
            ${ftxui_BINARY_DIR}
            EXCLUDE_FROM_ALL
    )
endif ()

# --- Build --------------------------------------------------------------------

project(System_Metrics_Monitor
        LANGUAGES CXX
        VERSION 1.0.0
)
add_executable(System_Metrics_Monitor src/main.cpp src/monitor/cpu_monitor.cpp src/monitor/cpu_monitor.h src/monitor/memory_monitor.cpp src/monitor/memory_monitor.h src/monitor/disk_monitor.cpp src/monitor/disk_monitor.h src/ui/main_component.cpp src/ui/main_component.h src/monitor/process_worker.cpp src/monitor/process_worker.h src/monitor/process_monitor.cpp src/monitor/process_monitor.h
        src/monitor/process.h
        src/ui/process_info_component.cpp
        src/ui/process_info_component.h
        src/ui/cpu_component.h
        src/ui/cpu_component.cpp
      src/ui/memory_component.h
      src/ui/memory_component.cpp)


target_include_directories(System_Metrics_Monitor PRIVATE src)
target_link_libraries(System_Metrics_Monitor
        PRIVATE ftxui::screen
        PRIVATE ftxui::dom
        PRIVATE ftxui::component
)
set_target_properties(System_Metrics_Monitor PROPERTIES CXX_STANDARD 20)
