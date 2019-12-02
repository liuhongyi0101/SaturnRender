# /*
#  * Created on Wed Sep 05 2018
#  *
#  * Copyright (c) 2018 CBIM
#  *
#  * Author: wr
#  */
cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
cmake_policy(VERSION 3.0)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

# Setting the name of project
set(NAME blade)
project(${NAME})

# Open the 64 bit system
set(BUILD_USE_64BITS on)

# set the main directory
set(MAIN_DIR ${PROJECT_SOURCE_DIR})

# 设置编译类型，debug和release
# 终端中使用 -DCMAKE_BUILD_TYPE=Debug指定
if( NOT CMAKE_BUILD_TYPE )
  	set( CMAKE_BUILD_TYPE Debug CACHE STRING
       "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
       FORCE )
	   MESSAGE(STATUS "DEBUG")
	set(BUILD_TYPE "Debug")

else(NOT CMAKE_BUILD_TYPE)
	if(CMAKE_BUILD_TYPE MATCHES "Debug")
		set(BUILD_TYPE "Debug")
	else(CMAKE_BUILD_TYPE MATCHES "Debug")
		set(BUILD_TYPE "Release")
	endif(CMAKE_BUILD_TYPE MATCHES "Debug")
endif()

# 设置应用程序输出路径。
if(WIN32)
	SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
else(WIN32)
# Linux 系统下不会自动生成Debug和Release目录
	SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin/${BUILD_TYPE})
endif(WIN32)


# for vulkan
OPTION(USE_D2D_WSI "Build the project using Direct to Display swapchain" OFF)
OPTION(USE_WAYLAND_WSI "Build the project using Wayland swapchain" OFF)

# 根据系统设置不同输出路径
if(WIN32)
# windows directory of output
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
	set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/libs)
else(WIN32)
# Linux directory of output
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin/${BUILD_TYPE})
	set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/libs/${BUILD_TYPE})
endif(WIN32)

# The directory of link_library
link_directories(${LIBRARY_OUTPUT_PATH})

# Find boost library by the file of FindBoost.cmake

# Use FindVulkan module added with CMAKE 3.7
if (NOT CMAKE_VERSION VERSION_LESS 3.7.0)
	MESSAGE(STATUS "Using module to find Vulkan")
	# find vulkan
	find_package(Vulkan)
endif()

IF(WIN32)
	# find vulkan and assimp library on windows.
	IF (NOT Vulkan_FOUND)
		find_library(Vulkan_LIBRARY NAMES vulkan-1 vulkan PATHS ${CMAKE_SOURCE_DIR}/libs/vulkan)
		IF (Vulkan_LIBRARY)
			set(Vulkan_FOUND ON)
			MESSAGE("Using bundled Vulkan library version")
		ENDIF()
	ENDIF()

	find_library(ASSIMP_LIBRARIES NAMES assimp libassimp.dll.a PATHS ${CMAKE_SOURCE_DIR}/libs/assimp)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_WIN32_KHR")
ELSE(WIN32)
	# find vulkan and assimp library on linux
	IF (NOT Vulkan_FOUND)
		find_library(Vulkan_LIBRARY NAMES vulkan HINTS "$ENV{VULKAN_SDK}/lib" "${CMAKE_SOURCE_DIR}/libs/vulkan" REQUIRED)
		IF (Vulkan_LIBRARY)
			set(Vulkan_FOUND ON)
			MESSAGE("Using bundled Vulkan library version")
		ENDIF()
	ENDIF()
	find_package(ASSIMP REQUIRED)
	find_package(Threads REQUIRED)

	IF(USE_D2D_WSI)
		MESSAGE("Using direct to display extension...")
		add_definitions(-D_DIRECT2DISPLAY)
	ELSEIF(USE_WAYLAND_WSI)
		find_package(Wayland REQUIRED)
		if (NOT WAYLAND_FOUND)
			message(FATAL_ERROR "Wayland development package not found")
		endif ()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_WAYLAND_KHR")
		include_directories(${WAYLAND_INCLUDE_DIR})
	ELSE(USE_D2D_WSI)
		find_package(XCB REQUIRED)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_XCB_KHR")
	ENDIF(USE_D2D_WSI)
		# Todo : android?
	ENDIF(WIN32)

	IF (NOT Vulkan_FOUND)
		message(FATAL_ERROR "Could not find Vulkan library!")
	ELSE()
		message(STATUS ${Vulkan_LIBRARY})
ENDIF()

# Set preprocessor defines
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNOMINMAX -D_USE_MATH_DEFINES")

# C++ defines
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
add_definitions(-std=c++11)

if(UNIX)
	if(CMAKE_BUILD_TYPE MATCHES "Debug")
		add_definitions(-D_DEBUG)
	endif(CMAKE_BUILD_TYPE MATCHES "Debug")
endif(UNIX)


# 添加宏定义
if(RESOURCE_INSTALL_DIR)
	add_definitions(-DVK_EXAMPLE_DATA_DIR=\"${RESOURCE_INSTALL_DIR}/\")
	install(DIRECTORY data/ DESTINATION ${RESOURCE_INSTALL_DIR}/)
else(RESOURCE_INSTALL_DIR)
	add_definitions(-DVK_EXAMPLE_DATA_DIR=\"${CMAKE_SOURCE_DIR}/\")
endif(RESOURCE_INSTALL_DIR)

# log
add_definitions(-DAUTO_INITIALIZE_EASYLOGGINGPP)

# external libraries
# add_subdirectory(external/base)
# add_subdirectory(external/zlib)

# main models
add_subdirectory(src)
