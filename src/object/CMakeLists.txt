file(GLOB BASE_SRC "*.cpp" "*/*.cpp")
file(GLOB BASE_HEADERS "../../include/object/*.h" "../../include/object/*/*.h")

include_directories(${MAIN_DIR}/external)
include_directories(${MAIN_DIR}/external/glm)
include_directories(${MAIN_DIR}/external/gli)
include_directories(${MAIN_DIR}/external/assimp)
include_directories(${MAIN_DIR}/src/vk_base)
include_directories(${MAIN_DIR}/include)



if(WIN32)
	source_group("Include" FILES ${BASE_HEADERS})
    add_library(object STATIC ${BASE_SRC} ${BASE_HEADERS})
    target_link_libraries(object vk_base ${Vulkan_LIBRARY} ${ASSIMP_LIBRARIES})
 else(WIN32)
    add_library(object STATIC ${BASE_SRC})
    target_link_libraries(object vk_base ${Vulkan_LIBRARY} ${ASSIMP_LIBRARIES} ${XCB_LIBRARIES} ${WAYLAND_CLIENT_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
endif(WIN32)