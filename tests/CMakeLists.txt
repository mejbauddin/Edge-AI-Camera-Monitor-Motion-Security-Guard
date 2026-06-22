cmake_minimum_required(VERSION 3.24)

include(GoogleTest)

function(csx_add_test test_name)
    add_executable(${test_name} ${ARGN})
    target_link_libraries(${test_name} PRIVATE csx_core GTest::gtest_main GTest::gtest)
    csx_set_target_warnings(${test_name})
    gtest_discover_tests(${test_name} DISCOVERY_MODE PRE_TEST)
endfunction()

csx_add_test(csx_core_tests
    unit/core/ring_buffer_test.cpp
    unit/core/event_bus_test.cpp
    unit/core/config_service_test.cpp
    unit/core/thread_pool_test.cpp
    unit/core/stage_metrics_test.cpp
)

csx_add_test(csx_utils_tests
    unit/utils/fps_counter_test.cpp
    unit/utils/path_resolver_test.cpp
    unit/utils/system_metrics_test.cpp
    unit/utils/timer_test.cpp
    unit/utils/cv_helpers_test.cpp
)
target_link_libraries(csx_utils_tests PRIVATE csx_utils)

csx_add_test(csx_camera_tests
    unit/vision/camera/camera_types_test.cpp
    unit/vision/camera/frame_buffer_test.cpp
    unit/vision/camera/synthetic_camera_test.cpp
    unit/vision/camera/camera_engine_test.cpp
)
target_link_libraries(csx_camera_tests PRIVATE csx_camera)

csx_add_test(csx_motion_tests
    unit/vision/motion/morphology_filter_test.cpp
    unit/vision/motion/blob_analyzer_test.cpp
    unit/vision/motion/motion_engine_test.cpp
)
target_link_libraries(csx_motion_tests PRIVATE csx_motion)

csx_add_test(csx_tracking_tests
    unit/tracking/iou_matcher_test.cpp
    unit/tracking/kalman_predictor_test.cpp
    unit/tracking/object_tracker_test.cpp
)
target_link_libraries(csx_tracking_tests PRIVATE csx_tracking)

csx_add_test(csx_database_tests unit/database/database_test.cpp)
target_link_libraries(csx_database_tests PRIVATE csx_database)

csx_add_test(csx_recognition_tests
    unit/recognition/embedding_math_test.cpp
    unit/recognition/face_pipeline_test.cpp
)
target_link_libraries(csx_recognition_tests PRIVATE csx_recognition csx_camera)

csx_add_test(csx_behavior_tests
    unit/behavior/geometry_test.cpp
    unit/behavior/behavior_engine_test.cpp
)
target_link_libraries(csx_behavior_tests PRIVATE csx_behavior)

csx_add_test(csx_threat_tests
    unit/threat/behavior_fusion_test.cpp
    unit/threat/threat_scorer_test.cpp
    unit/threat/threat_history_test.cpp
    unit/threat/threat_engine_test.cpp
)
target_link_libraries(csx_threat_tests PRIVATE csx_threat)

csx_add_test(csx_recording_tests
    unit/recording/rolling_buffer_test.cpp
    unit/recording/snapshot_capture_test.cpp
    unit/recording/storage_manager_test.cpp
    unit/recording/recorder_test.cpp
)
target_link_libraries(csx_recording_tests PRIVATE csx_recording)

csx_add_test(csx_hud_tests
    unit/ui/hud/hud_renderer_test.cpp
)
target_link_libraries(csx_hud_tests PRIVATE csx_hud csx_tracking csx_threat csx_core)
