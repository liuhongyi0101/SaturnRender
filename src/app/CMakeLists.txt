# Function for building single example

set(APP blade)

include_directories(${MAIN_DIR}/external)
include_directories(${MAIN_DIR}/external/glm)
include_directories(${MAIN_DIR}/external/gli)
include_directories(${MAIN_DIR}/external/assimp)
include_directories(${MAIN_DIR}/external/imgui)
include_directories(${MAIN_DIR}/src/vk_base)
include_directories(${MAIN_DIR}/Include)



SET(APP_FOLDER ${CMAKE_CURRENT_SOURCE_DIR})
message(STATUS "Generating project file for APP in ${APP_FOLDER}")

# 添加宏定义
add_definitions(-DBMP_SUPPORTED)
add_definitions(-DGIF_SUPPORTED)
add_definitions(-DPPM_SUPPORTED)
add_definitions(-DTARGA_SUPPORTED)

add_definitions(-D_CRT_SECURE_NO_WARNINGS)
add_definitions(-std=c++11)


# Main
file(GLOB SOURCE "*.cpp")
# imgui example requires additional source files
IF(${APP} STREQUAL "imgui")
	file(GLOB ADD_SOURCE "${MAIN_DIR}/external/imgui/*.cpp")
	SET(SOURCE ${SOURCE} ${ADD_SOURCE})
ENDIF()
# Add shaders
set(SHADER_DIR "${MAIN_DIR}/shader/${APP}")
file(GLOB SHADERS "${SHADER_DIR}/*.vert" "${SHADER_DIR}/*.frag" "${SHADER_DIR}/*.comp" "${SHADER_DIR}/*.geom" "${SHADER_DIR}/*.tesc" "${SHADER_DIR}/*.tese")
file(GLOB BASE_HEADER "*.h")

source_group("Shaders" FILES ${SHADERS})
if(WIN32)
	source_group("Include" FILES ${BASE_HEADER})

	add_executable(${APP} ${SOURCE} ${SHADERS})
	target_link_libraries(${APP} vk_base render ${JPEG_LIBRARY} ${Vulkan_LIBRARY} ${ASSIMP_LIBRARIES})
else(WIN32)
	add_executable(${APP} ${SOURCE} ${SHADERS})
	target_link_libraries(${APP} vk_base render ${JPEG_LIBRARY} ${Vulkan_LIBRARY} ${ASSIMP_LIBRARIES})
endif(WIN32)
