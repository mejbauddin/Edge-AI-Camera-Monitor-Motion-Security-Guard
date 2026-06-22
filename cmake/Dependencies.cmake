include(FetchContent)

# spdlog
if(NOT TARGET spdlog::spdlog)
    FetchContent_Declare(spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG        v1.14.1
        GIT_SHALLOW    TRUE
    )
    set(SPDLOG_BUILD_SHARED OFF CACHE BOOL "" FORCE)
    set(SPDLOG_BUILD_EXAMPLE OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(spdlog)
endif()

# nlohmann_json
if(NOT TARGET nlohmann_json::nlohmann_json)
    FetchContent_Declare(nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG        v3.11.3
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(nlohmann_json)
endif()

# GoogleTest (tests only)
if(CSX_BUILD_TESTS AND NOT TARGET GTest::gtest)
    FetchContent_Declare(googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.14.0
        GIT_SHALLOW    TRUE
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

function(csx_link_core_deps target)
    target_link_libraries(${target} PUBLIC spdlog::spdlog nlohmann_json::nlohmann_json)
endfunction()

find_package(OpenCV 4 QUIET COMPONENTS core videoio imgproc)

function(csx_link_camera_deps target)
    if(OpenCV_FOUND)
        target_compile_definitions(${target} PUBLIC CSX_HAS_OPENCV=1)
        target_include_directories(${target} PUBLIC ${OpenCV_INCLUDE_DIRS})
        target_link_libraries(${target} PUBLIC ${OpenCV_LIBS})
        message(STATUS "OpenCV ${OpenCV_VERSION} found — USB/RTSP/IP camera backends enabled")
    else()
        message(STATUS "OpenCV not found — only synthetic camera backend available")
    endif()
endfunction()

# SQLite amalgamation
if(NOT TARGET sqlite3)
    FetchContent_Declare(sqlite3_amalgamation
        URL https://www.sqlite.org/2024/sqlite-amalgamation-3460100.zip
    )
    FetchContent_MakeAvailable(sqlite3_amalgamation)
    add_library(sqlite3 STATIC ${sqlite3_amalgamation_SOURCE_DIR}/sqlite3.c)
    target_include_directories(sqlite3 PUBLIC ${sqlite3_amalgamation_SOURCE_DIR})
    if(MSVC)
        target_compile_options(sqlite3 PRIVATE /W0)
    endif()
endif()

function(csx_link_database_deps target)
    target_link_libraries(${target} PUBLIC sqlite3)
endfunction()

find_package(onnxruntime QUIET)

function(csx_link_recognition_deps target)
    csx_link_camera_deps(${target})
    if(onnxruntime_FOUND)
        target_compile_definitions(${target} PUBLIC CSX_HAS_ONNXRUNTIME=1)
        target_link_libraries(${target} PUBLIC onnxruntime::onnxruntime)
        message(STATUS "ONNX Runtime found — YuNet/ArcFace inference enabled")
    else()
        message(STATUS "ONNX Runtime not found — OpenCV DNN or CPU fallback used for recognition")
    endif()
endfunction()

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(FFMPEG QUIET IMPORTED_TARGET libavcodec libavformat libswscale)
endif()

function(csx_link_recording_deps target)
    if(FFMPEG_FOUND)
        target_compile_definitions(${target} PUBLIC CSX_HAS_FFMPEG=1)
        target_link_libraries(${target} PUBLIC PkgConfig::FFMPEG)
        message(STATUS "FFmpeg found — H264 MP4 recording enabled")
    else()
        message(STATUS "FFmpeg not found — CSX clip fallback used for recording")
    endif()
endfunction()
