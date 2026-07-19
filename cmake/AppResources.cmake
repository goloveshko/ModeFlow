# ============================================================================
# ModeFlow – Centralized Application Resources Configuration
# ============================================================================

function(setup_app_resources TARGET_NAME)
    # Main App Fonts
    qt_add_resources(${TARGET_NAME} "${TARGET_NAME}_fonts"
        PREFIX "/fonts"
        BASE "${CMAKE_SOURCE_DIR}/assets/fonts"
        BIG_RESOURCES
        FILES
            "${CMAKE_SOURCE_DIR}/assets/fonts/Font Awesome 7 Free-Solid-900.otf"
    )
    
    # Common Icons
    qt_add_resources(${TARGET_NAME} "${TARGET_NAME}_icons"
        PREFIX "/icons"
        BASE  "${CMAKE_SOURCE_DIR}/assets/icons"
        FILES "${CMAKE_SOURCE_DIR}/assets/icons/check_mark.svg"
    )
    
    # App Logo Icon
    qt_add_resources(${TARGET_NAME} "${TARGET_NAME}_icons-app"
        PREFIX "/icons/app"
        BASE  "${CMAKE_SOURCE_DIR}/assets/icons/app"
        FILES "${CMAKE_SOURCE_DIR}/assets/icons/app/icon.svg"
    )
    
    # Light Theme Icons
    qt_add_resources(${TARGET_NAME} "${TARGET_NAME}_icons-light"
        PREFIX "/icons/light"
        BASE "${CMAKE_SOURCE_DIR}/assets/icons/light"
        FILES 
            "${CMAKE_SOURCE_DIR}/assets/icons/light/icon.svg"
    )
    
    # Dark Theme Icons
    qt_add_resources(${TARGET_NAME} "${TARGET_NAME}_icons-dark"
        PREFIX "/icons/dark"
        BASE "${CMAKE_SOURCE_DIR}/assets/icons/dark"
        FILES
            "${CMAKE_SOURCE_DIR}/assets/icons/dark/icon.svg"
    )
    
    # Theme Stylesheets (Absolute paths fix)
    qt_add_resources(${TARGET_NAME} "${TARGET_NAME}_styles"
        PREFIX "/styles"
        BASE "${CMAKE_SOURCE_DIR}/assets/styles"
        FILES
            "${CMAKE_SOURCE_DIR}/assets/styles/common.qss"
            "${CMAKE_SOURCE_DIR}/assets/styles/dark.qss"
            "${CMAKE_SOURCE_DIR}/assets/styles/light.qss"
            "${CMAKE_SOURCE_DIR}/assets/styles/system.qss"
    )
    
    # Compiled App Translations (Manual Workflow with safe existence check)
    # Allows the project to configure successfully even if the binary .qm file
    # hasn't been generated yet (e.g. on fresh clones inside IDEs).
    if(EXISTS "${CMAKE_SOURCE_DIR}/i18n/${PROJECT_NAME}_ru_RU.qm")
        qt_add_resources(${TARGET_NAME} "${TARGET_NAME}_translations"
            PREFIX "/i18n"
            BASE "${CMAKE_SOURCE_DIR}/i18n"
            FILES "${CMAKE_SOURCE_DIR}/i18n/${PROJECT_NAME}_ru_RU.qm"
        )
    else()
        message(STATUS "Note: ${PROJECT_NAME}_ru_RU.qm not found. Russian translation will be omitted until compiled.")
    endif()
endfunction()