target_sources("fsr2-unity-plugin" PRIVATE
${CMAKE_CURRENT_SOURCE_DIR}/fsr2_unity_plugin.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fsr2_gfx_device.cpp
${CMAKE_CURRENT_SOURCE_DIR}/fsr2_gfx_device_$CACHE{FSR2_BACKEND}.cpp
)