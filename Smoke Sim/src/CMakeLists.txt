set(TARGET_NAME ObjViewer)

set(${TARGET_NAME}_SOURCES
    main.cpp
)


add_executable(${TARGET_NAME} ${${TARGET_NAME}_SOURCES} "transform.h" "voxel.hpp" "voxel.cpp" "scene.hpp" "scene.cpp"   "flood_fill.cpp" "worley.cpp" "worley.hpp" "helpers.hpp" "helpers.cpp")
target_link_libraries(${TARGET_NAME} PUBLIC tga_vulkan tga_utils ${CMAKE_THREAD_LIBS_INIT})
add_dependencies(${TARGET_NAME} tga_shaders)
if(WIN32)
    set_property(TARGET ${TARGET_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${EXAMPLES_WORKING_DIR}")
endif(WIN32)

