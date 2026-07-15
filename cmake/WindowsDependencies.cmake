function(link_windows_system_libs TARGET_NAME)
    if(WIN32) 
        # Low-level libraries required by our own source code (Mica, Themes, MiniDumps, Task Scheduler)
        # must be linked unconditionally for both static and shared builds.
        target_link_libraries(${TARGET_NAME} PRIVATE
            dwmapi
            uxtheme
            version
            dbghelp
            taskschd
            comsupp
        )

        # Low-level transit libraries required strictly by static Qt/Vcpkg runtimes.
        # In shared builds, Qt DLLs already link these, so we only need them for static builds.
        if(MODEFLOW_STATIC_BUILD)
            target_link_libraries(${TARGET_NAME} PRIVATE
                netapi32
                userenv
                setupapi
                imm32
                winmm
                ws2_32
            )
        endif()
    endif()
endfunction()