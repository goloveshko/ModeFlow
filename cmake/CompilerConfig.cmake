macro(configure_compiler)
    if(MSVC)
        add_compile_options(/utf-8)
        add_compile_options(/MP)
        add_compile_definitions(
            _COROUTINE_ABI=1
            _HAS_STDPP_COROUTINE_AND_NOT_COROUTINE_TS=0
            _ALLOW_COROUTINE_ABI_MISMATCH
        )
        if(MODEFLOW_STATIC_BUILD)
            set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
        else()
            set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
        endif()
    endif()
endmacro()