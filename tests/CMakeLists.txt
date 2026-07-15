# ============================================================================
# ModeFlow – Unit Tests Target Configuration
# ============================================================================

# Localized search: Find the Qt6::Test module strictly within the tests subdirectory
find_package(Qt6 REQUIRED COMPONENTS Test)

# Declare the test runner executable using inherited ${TEST_TARGET} variable
qt_add_executable(${TEST_TARGET}
    ModeFlowTests.cpp
)

# Simply link against our inherited Core Object Library variable!
target_link_libraries(${TEST_TARGET}
    PRIVATE
        ${CORE_TARGET}
        Qt6::Test
)

# Add CTest registration
add_test(NAME ${TEST_TARGET} COMMAND ${TEST_TARGET})

# Configure Visual Studio start-up debugging environment for the test runner
if(WIN32 AND CMAKE_PREFIX_PATH)
    set(_modeflow_qt_prefix "${CMAKE_PREFIX_PATH}")
    set(_modeflow_qt_bin_dir "${_modeflow_qt_prefix}/bin")
    set_tests_properties(${TEST_TARGET} PROPERTIES
        ENVIRONMENT
            "PATH=${_modeflow_qt_bin_dir};$ENV{PATH};QT_PLUGIN_PATH=${_modeflow_qt_prefix}/plugins"
    )
endif()