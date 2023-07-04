cmake_minimum_required(VERSION 3.0)

project("fsr2-for-unity" VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:preprocessor /std:c++20 -D_CRT_SECURE_NO_WARNINGS=1")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
# set output directory
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_SOURCE_DIR}/bin)

# set output postfix for debug
set(CMAKE_DEBUG_POSTFIX d)

add_library("fsr2-unity-plugin" SHARED "")

set(UNITY_PLUGINAPI_INCLUDE_DIR "Set the include dir for Unity plugin API" CACHE PATH "")
set(FFX_FSR2_API_INCLUDE_DIR "Set the include dir for FidelityFX FSR2 API" CACHE PATH "")
set(FFX_FSR2_API_LIB_DIR "Set the FSR 2 library dir for linking the FidelityFX FSR2 API" CACHE PATH "")
set(FSR2_UNITY_PLUGIN_DST_DIR "" CACHE PATH "")
set(FSR2_BACKEND dx11 CACHE STRING "Choose FSR2 backend, must be one of: [dx11]")

target_include_directories("fsr2-unity-plugin" PRIVATE 
$CACHE{UNITY_PLUGINAPI_INCLUDE_DIR}
$CACHE{FFX_FSR2_API_INCLUDE_DIR}
)

target_link_directories("fsr2-unity-plugin" PRIVATE
$CACHE{FFX_FSR2_API_LIB_DIR}
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
else()
set(FFX_FSR2_API_LIB "ffx_fsr2_api_x64")
set(FFX_FSR2_API_DX11_LIB "ffx_fsr2_api_dx11_x64")
endif()

target_link_libraries("fsr2-unity-plugin" PRIVATE
dxguid
debug "ffx_fsr2_api_x64d"
debug "ffx_fsr2_api_$CACHE{FSR2_BACKEND}_x64d"
optimized "ffx_fsr2_api_x64"
optimized "ffx_fsr2_api_$CACHE{FSR2_BACKEND}_x64"
)

set(FSR2_BACKEND_DEF)
if(FSR2_BACKEND STREQUAL "dx11")
set(FSR2_BACKEND_DEF "FSR2_BACKEND_DX11")
else()
message(FATAL_ERROR "Unsupported FSR2 backend, must be one of: [dx11]")
endif()

target_compile_definitions("fsr2-unity-plugin" PRIVATE
${FSR2_BACKEND_DEF}
)


add_subdirectory(src)

if(NOT FSR2_UNITY_PLUGIN_DST_DIR STREQUAL "")
add_custom_command(TARGET "fsr2-unity-plugin" POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:fsr2-unity-plugin> ${FSR2_UNITY_PLUGIN_DST_DIR}
)
endif()
