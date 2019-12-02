file(GLOB BASE_SRC  "*.cpp")
file(GLOB BASE_HEADERS "../../include/renderer/*.h" )


include_directories(${MAIN_DIR}/external)
include_directories(${MAIN_DIR}/external/glm)
include_directories(${MAIN_DIR}/external/gli)
include_directories(${MAIN_DIR}/external/assimp)
include_directories(${MAIN_DIR}/src/vk_base)
include_directories(${MAIN_DIR}/include)



if(WIN32)
	source_group("Include" FILES ${BASE_HEADERS})
    add_library(render STATIC ${BASE_SRC} ${BASE_HEADERS})
    target_link_libraries(render vk_base bobject graphics ${Vulkan_LIBRARY} ${ASSIMP_LIBRARIES})
 else(WIN32)
    add_library(render STATIC ${BASE_SRC})
    target_link_libraries(render vk_base bobject graphics ${Vulkan_LIBRARY} ${ASSIMP_LIBRARIES})
endif(WIN32)