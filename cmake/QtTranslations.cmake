function(setup_qt_system_translations TARGET_NAME)
    # Root of Qt
    get_target_property(_qt_core_location Qt6::Core LOCATION)
    get_filename_component(_qt_bin_dir "${_qt_core_location}" DIRECTORY)
    get_filename_component(_qt_root_dir "${_qt_bin_dir}/.." ABSOLUTE)

    # Find the folder with translations
    find_path(QT_SYSTEM_TR_DIR
        NAMES qtbase_ru.qm
        HINTS
            "${_qt_root_dir}/translations/Qt6"
            "${_qt_root_dir}/translations"
            "${_qt_root_dir}/share/qt6/translations"
        NO_DEFAULT_PATH
    )

    if(NOT QT_SYSTEM_TR_DIR)
        message(WARNING "Qt system translations not found! Standard UI elements will be in English.")
        return()
    endif()

    message(STATUS "Qt system translations found: ${QT_SYSTEM_TR_DIR}")

    # Preparing a folder in the Build Directory
    set(SYS_TR_DEST_DIR "${CMAKE_CURRENT_BINARY_DIR}/translations_system")
    file(MAKE_DIRECTORY "${SYS_TR_DEST_DIR}")

    set(LANGS ru)
    set(TR_FILES_TO_PACK "")

    foreach(LANG ${LANGS})
        set(_filename "qtbase_${LANG}.qm")
        set(_src "${QT_SYSTEM_TR_DIR}/${_filename}")
        set(_dst "${SYS_TR_DEST_DIR}/${_filename}")

        if(EXISTS "${_src}")
            configure_file("${_src}" "${_dst}" COPYONLY)
            set_source_files_properties("${_dst}" PROPERTIES QT_RESOURCE_ALIAS "${_filename}")
            list(APPEND TR_FILES_TO_PACK "${_dst}")
        endif()
    endforeach()

    # Adding resources to the target
    if(TR_FILES_TO_PACK)        
		qt_add_resources(${TARGET_NAME} "qt_system_translations"
            PREFIX "/qt/translations"
            BASE "${SYS_TR_DEST_DIR}"
            FILES ${TR_FILES_TO_PACK}
        )
    endif()
endfunction()