cmake_minimum_required(VERSION 3.24)
project(CyberSentinelX VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(CSX_BUILD_TESTS       "Build unit and integration tests" ON)
option(CSX_ENABLE_DEV_MODE   "Enable developer diagnostics"     ON)
option(CSX_STATIC_LINK       "Prefer static linking"            ON)
option(CSX_BUILD_UI          "Build Qt-based UI modules"         ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(CompilerWarnings)
include(Dependencies)
include(ModuleHelpers)

add_compile_definitions(
    CYBER_SENTINEL_X_VERSION="${PROJECT_VERSION}"
)

if(CSX_ENABLE_DEV_MODE)
    add_compile_definitions(CSX_DEV_MODE=1)
endif()

# Phase 1 — core foundation (active)
csx_add_library(csx_core SRC_DIR src/core)
csx_link_core_deps(csx_core)

# Future modules (scaffolded — sources added per phase)
csx_add_library(csx_utils        SRC_DIR src/utils           DEPS csx_core)
csx_add_library(csx_camera        SRC_DIR src/vision/camera   DEPS csx_core csx_utils)
csx_link_camera_deps(csx_camera)
csx_add_library(csx_motion        SRC_DIR src/vision/motion   DEPS csx_core csx_camera)
csx_link_camera_deps(csx_motion)
csx_add_library(csx_tracking      SRC_DIR src/tracking        DEPS csx_core csx_motion csx_utils)
csx_add_library(csx_database      SRC_DIR src/database        DEPS csx_core csx_utils)
csx_link_database_deps(csx_database)
csx_add_library(csx_recognition   SRC_DIR src/recognition     DEPS csx_core csx_database csx_utils)
csx_link_recognition_deps(csx_recognition)
csx_add_library(csx_behavior      SRC_DIR src/behavior        DEPS csx_core csx_tracking csx_utils)
csx_add_library(csx_threat        SRC_DIR src/threat          DEPS csx_core csx_behavior csx_recognition csx_tracking csx_utils)
csx_add_library(csx_recording     SRC_DIR src/recording       DEPS csx_core csx_camera csx_database csx_utils)
csx_link_recording_deps(csx_recording)
csx_add_library(csx_hud           SRC_DIR src/ui/hud          DEPS csx_core csx_tracking csx_threat)
csx_link_camera_deps(csx_hud)
csx_add_library(csx_radar         SRC_DIR src/radar           DEPS csx_core csx_tracking csx_threat)
csx_add_library(csx_voice         SRC_DIR src/voice           DEPS csx_core)
csx_add_library(csx_alerts        SRC_DIR src/alerts          DEPS csx_core csx_threat csx_voice csx_database)

# Main application executable
set(CSX_APP_SOURCES src/app/main.cpp src/app/Application.cpp src/app/ServiceBootstrap.cpp)

if(CSX_BUILD_UI)
    list(APPEND CSX_APP_SOURCES src/app/UiBootstrap.cpp)
endif()

add_executable(CyberSentinelX ${CSX_APP_SOURCES})
target_link_libraries(CyberSentinelX PRIVATE
    csx_core csx_utils csx_camera csx_motion csx_tracking csx_database
    csx_recognition csx_behavior csx_threat csx_recording csx_hud
    csx_radar csx_voice csx_alerts
)

if(CSX_BUILD_UI)
    set(CMAKE_AUTOMOC ON)

    find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick QuickControls2)

    set(CSX_UI_SOURCES
        src/ui/bridge/DashboardBridge.cpp
        src/ui/bridge/EnrollmentController.cpp
        src/ui/providers/FrameImageProvider.cpp
        src/ui/providers/RadarImageProvider.cpp
        src/ui/widgets/LiveImageItem.cpp
    )
    add_library(csx_ui STATIC ${CSX_UI_SOURCES})
    target_include_directories(csx_ui PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/app
        ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/bridge
        ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/providers
        ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/widgets
    )
    target_link_libraries(csx_ui PUBLIC
        csx_core csx_camera csx_threat csx_tracking csx_radar csx_hud csx_database csx_recognition
        Qt6::Core Qt6::Gui Qt6::Quick Qt6::QuickControls2
    )
    set_target_properties(csx_ui PROPERTIES AUTOMOC ON)

    target_compile_definitions(CyberSentinelX PRIVATE CSX_BUILD_UI=1)
    target_link_libraries(CyberSentinelX PRIVATE csx_ui Qt6::Quick Qt6::QuickControls2)

    add_custom_command(TARGET CyberSentinelX POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:CyberSentinelX>/qml"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/assets/qml"
            "$<TARGET_FILE_DIR:CyberSentinelX>/qml"
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:CyberSentinelX>/assets/faces/authorized"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/assets/faces"
            "$<TARGET_FILE_DIR:CyberSentinelX>/assets/faces"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/assets/models"
            "$<TARGET_FILE_DIR:CyberSentinelX>/assets/models"
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:CyberSentinelX>/config"
        COMMAND ${CMAKE_COMMAND} -E copy
            "${CMAKE_SOURCE_DIR}/config/default.json"
            "$<TARGET_FILE_DIR:CyberSentinelX>/config/default.json"
        COMMENT "Deploying QML, models, and config"
    )
endif()

if(CSX_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

message(STATUS "Cyber Sentinel X ${PROJECT_VERSION} — Phase 10: recording")
