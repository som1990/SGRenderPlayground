
cmake_minimum_required(VERSION 3.20)
project(SGTestBed)
cmake_policy(SET CMP0054 NEW)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


option(SGTESTBED_BUILD_PROTOTYPES "Build SG Test Bed Prototypes" ON)
option(SGTESTBED_INSTALL_PROTOTYPES "Install SG Test Bed Prototypes" ON)

if(NOT SGRENDER_DIR)
    set(SGRENDER_DIR "${CMAKE_CURRENT_SOURCE_DIR}" CACHE STRING "Location of SG Render Playground")
elseif(NOT IS_ABSOLUTE "${SGRENDER_DIR}")
    get_filename_component(SGRENDER_DIR "${SGRENDER_DIR}" REALPATH BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
endif()

set(CMAKE_INSTALL_PREFIX "./install")
set(BGFX_BUILD_TOOLS ON)
set(BGFX_CUSTOM_TARGETS ON)
set(BGFX_INSTALL ON)
set(BGFX_INSTALL_EXAMPLES ON)
set(BGFX_BUILD_EXAMPLE_COMMON ON)

include(cmake/3rdparty/dear-imgui.cmake)
# Adding folder Structure
add_subdirectory(Prototypes)
add_subdirectory(3rdParty/bgfx.cmake)





