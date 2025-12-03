# Ensure the directory exists
if(NOT IS_DIRECTORY ${SGRENDER_DIR})
	message(SEND_ERROR "Could not load bgfx, directory does not exist. ${SGRENDER_DIR}")
	return()
endif()

if(NOT DEAR_IMGUI_LIBRARIES)
    file(
        GLOB 
        DEAR_IMGUI_SOURCES
        $(SGRENDER_DIR)/3rdparty/dear-imgui/*.cpp 
        $(SGRENDER_DIR)/3rdparty/dear-imgui/*.h 
        $(SGRENDER_DIR)/3rdparty/dear-imgui/*.inl 
    )
    set(DEAR_IMGUI_INCLUDE_DIR ${SGRENDER_DIR}/3rdparty)
endif()