# ImGui source files
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../external/imgui)

set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

# Find dependencies
find_package(OpenGL REQUIRED)
find_package(GLFW3 REQUIRED)

# Collect source files
file(GLOB_RECURSE SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/glad.c
    ${CMAKE_CURRENT_SOURCE_DIR}/renderer/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sim/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/init/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/benchmark/*.cpp
)

# Define the executable
add_executable(fluidSim
    ${SOURCE_FILES}
    ${IMGUI_SOURCES}
)

# Include directories
target_include_directories(fluidSim PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/renderer
    ${CMAKE_CURRENT_SOURCE_DIR}/sim
    ${CMAKE_CURRENT_SOURCE_DIR}/init
    ${CMAKE_CURRENT_SOURCE_DIR}/benchmark
    ${CMAKE_CURRENT_SOURCE_DIR}/../external/glad/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../external/glm
    ${CMAKE_CURRENT_SOURCE_DIR}/../external # provides KHR/khrplatform.h, included by glad.h
    ${GLFW3_INCLUDE_DIR}
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

# So benchmark.cpp can always write benchmarks/results/ into the source tree,
# regardless of the working directory fluidSim.exe is launched from (it must
# run from build/src for shaders' relative paths to resolve).
target_compile_definitions(fluidSim PRIVATE FLUIDSIM_SOURCE_DIR="${CMAKE_SOURCE_DIR}")

# Link libraries
target_link_libraries(fluidSim
    OpenGL::GL
    ${GLFW3_LIBRARY}
)

# The prebuilt GLFW3 static library was built with mingw-w64 gcc and references
# a few mingw runtime helper symbols (__mingw_vsnprintf, __mingw_vsscanf,
# __mingw_vsprintf) that MSVC's link.exe cannot resolve on its own. When linking
# with MSVC, pull in the mingw-w64 runtime archive that provides them.
if(MSVC)
    find_library(MINGWEX_LIBRARY
        NAMES mingwex libmingwex
        PATHS
            "C:/msys64/ucrt64/lib"
            "C:/msys64/mingw64/lib"
    )
    if(MINGWEX_LIBRARY)
        target_link_libraries(fluidSim ${MINGWEX_LIBRARY})
    else()
        message(WARNING "libmingwex.a not found; linking the mingw-w64-built GLFW3 static lib with MSVC may fail with unresolved __mingw_vs* symbols.")
    endif()

    # libmingwex.a's vfscanf/pformat implementations use GCC's stack-probing
    # helper (___chkstk_ms), provided by libgcc.a from the same mingw-w64 toolchain.
    file(GLOB _mingw_libgcc_candidates
        "C:/msys64/ucrt64/lib/gcc/x86_64-w64-mingw32/*/libgcc.a"
        "C:/msys64/mingw64/lib/gcc/x86_64-w64-mingw32/*/libgcc.a"
    )
    if(_mingw_libgcc_candidates)
        list(GET _mingw_libgcc_candidates 0 MINGW_LIBGCC_LIBRARY)
        target_link_libraries(fluidSim ${MINGW_LIBGCC_LIBRARY})
    else()
        message(WARNING "libgcc.a (mingw-w64) not found; linking may fail with an unresolved ___chkstk_ms symbol.")
    endif()
endif()
