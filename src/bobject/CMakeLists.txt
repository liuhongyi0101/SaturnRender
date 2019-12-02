file(GLOB BASE_SRC "*.cpp" "light/*.cpp" "math/*.cpp" "geometries/*.cpp" "camera/*.cpp")
file(GLOB BASE_HEADERS "../../include/bobject/*.h" "../../include/bobject/*/*.h" )

include_directories(${MAIN_DIR}/external)
include_directories(${MAIN_DIR}/external/glm)
include_directories(${MAIN_DIR}/external/gli)
include_directories(${MAIN_DIR}/external/assimp)
include_directories(${MAIN_DIR}/src/vk_base)
include_directories(${MAIN_DIR}/include)



if(WIN32)
	source_group("Include" FILES ${BASE_HEADERS})
    add_library(bobject STATIC ${BASE_SRC} ${BASE_HEADERS})
    target_link_libraries(bobject vk_base ${Vulkan_LIBRARY} ${ASSIMP_LIBRARIES})
 else(WIN32)
    add_library(bobject STATIC ${BASE_SRC})
    target_link_libraries(bobject vk_base ${Vulkan_LIBRARY} ${ASSIMP_LIBRARIES} ${XCB_LIBRARIES} ${WAYLAND_CLIENT_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
endif(WIN32)