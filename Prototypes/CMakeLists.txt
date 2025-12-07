# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.20)
project ("SGTestBed")

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0054)
  cmake_policy(SET CMP0054 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()


# Includes
set(DIRECTX_HEADERS )

# Include sub-projects.
include(prototypes.cmake)
