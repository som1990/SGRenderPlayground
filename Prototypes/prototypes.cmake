include(CMakeParseArguments)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/bgfxToolUtils.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/utils/ConfigureDebugging.cmake)

function(_bgfx_shaderc_parse ARG_OUT)
		cmake_parse_arguments(
			ARG
			"DEPENDS;ANDROID;ASM_JS;IOS;LINUX;OSX;WINDOWS;ORBIS;PREPROCESS;RAW;FRAGMENT;VERTEX;COMPUTE;VERBOSE;DEBUG;DISASM;WERROR"
			"FILE;OUTPUT;VARYINGDEF;BIN2C;PROFILE;O"
			"INCLUDES;DEFINES"
			${ARGN}
		)
		set(CLI "")

		# -f
		if(ARG_FILE)
			list(APPEND CLI "-f" "${ARG_FILE}")
		else()
			message(SEND_ERROR "Call to _bgfx_shaderc_parse() must have an input file path specified.")
		endif()

		# -i
		if(ARG_INCLUDES)
			foreach(INCLUDE ${ARG_INCLUDES})
				list(APPEND CLI "-i")
				list(APPEND CLI "${INCLUDE}")
			endforeach()
		endif()

		# -o
		if(ARG_OUTPUT)
			list(APPEND CLI "-o" "${ARG_OUTPUT}")
		else()
			message(SEND_ERROR "Call to _bgfx_shaderc_parse() must have an output file path specified.")
		endif()

		# --bin2c
		if(ARG_BIN2C)
			list(APPEND CLI "--bin2c" "${ARG_BIN2C}")
		endif()

		# --depends
		if(ARG_DEPENDS)
			list(APPEND CLI "--depends")
		endif()

		# --platform
		set(PLATFORM "")
		set(PLATFORMS "ANDROID;ASM_JS;IOS;LINUX;OSX;WINDOWS;ORBIS")
		foreach(P ${PLATFORMS})
			if(ARG_${P})
				if(PLATFORM)
					message(SEND_ERROR "Call to _bgfx_shaderc_parse() cannot have both flags ${PLATFORM} and ${P}.")
					return()
				endif()
				set(PLATFORM "${P}")
			endif()
		endforeach()
		if(PLATFORM STREQUAL "")
			message(SEND_ERROR "Call to _bgfx_shaderc_parse() must have a platform flag: ${PLATFORMS}")
			return()
		elseif(PLATFORM STREQUAL "ANDROID")
			list(APPEND CLI "--platform" "android")
		elseif(PLATFORM STREQUAL "ASM_JS")
			list(APPEND CLI "--platform" "asm.js")
		elseif(PLATFORM STREQUAL "IOS")
			list(APPEND CLI "--platform" "ios")
		elseif(PLATFORM STREQUAL "OSX")
			list(APPEND CLI "--platform" "osx")
		elseif(PLATFORM STREQUAL "LINUX")
			list(APPEND CLI "--platform" "linux")
		elseif(PLATFORM STREQUAL "WINDOWS")
			list(APPEND CLI "--platform" "windows")
		elseif(PLATFORM STREQUAL "ORBIS")
			list(APPEND CLI "--platform" "orbis")
		endif()

		# --preprocess
		if(ARG_PREPROCESS)
			list(APPEND CLI "--preprocess")
		endif()

		# --define
		if(ARG_DEFINES)
			list(APPEND CLI "--defines")
			set(DEFINES "")
			foreach(DEFINE ${ARG_DEFINES})
				if(NOT "${DEFINES}" STREQUAL "")
					set(DEFINES "${DEFINES}\\\\;${DEFINE}")
				else()
					set(DEFINES "${DEFINE}")
				endif()
			endforeach()
			list(APPEND CLI "${DEFINES}")
		endif()

		# --raw
		if(ARG_RAW)
			list(APPEND CLI "--raw")
		endif()

		# --type
		set(TYPE "")
		set(TYPES "FRAGMENT;VERTEX;COMPUTE")
		foreach(T ${TYPES})
			if(ARG_${T})
				if(TYPE)
					message(SEND_ERROR "Call to _bgfx_shaderc_parse() cannot have both flags ${TYPE} and ${T}.")
					return()
				endif()
				set(TYPE "${T}")
			endif()
		endforeach()
		if("${TYPE}" STREQUAL "")
			message(SEND_ERROR "Call to _bgfx_shaderc_parse() must have a type flag: ${TYPES}")
			return()
		elseif("${TYPE}" STREQUAL "FRAGMENT")
			list(APPEND CLI "--type" "fragment")
		elseif("${TYPE}" STREQUAL "VERTEX")
			list(APPEND CLI "--type" "vertex")
		elseif("${TYPE}" STREQUAL "COMPUTE")
			list(APPEND CLI "--type" "compute")
		endif()

		# --varyingdef
		if(ARG_VARYINGDEF)
			list(APPEND CLI "--varyingdef" "${ARG_VARYINGDEF}")
		endif()

		# --verbose
		if(ARG_VERBOSE)
			list(APPEND CLI "--verbose")
		endif()

		# --debug
		if(ARG_DEBUG)
			list(APPEND CLI "--debug")
		endif()

		# --disasm
		if(ARG_DISASM)
			list(APPEND CLI "--disasm")
		endif()

		# --profile
		if(ARG_PROFILE)
			list(APPEND CLI "--profile" "${ARG_PROFILE}")
		else()
			message(SEND_ERROR "Call to _bgfx_shaderc_parse() must have a shader profile.")
		endif()

		# -O
		if(ARG_O)
			list(APPEND CLI "-O" "${ARG_O}")
		endif()

		# --Werror
		if(ARG_WERROR)
			list(APPEND CLI "--Werror")
		endif()

		set(${ARG_OUT} ${CLI} PARENT_SCOPE)
	endfunction()

function(add_bgfx_shader FILE FOLDER)
    get_filename_component(FILENAME "${FILE}" NAME_WE)
	string(SUBSTRING "${FILENAME}" 0 2 TYPE)
	if("${TYPE}" STREQUAL "fs")
		set(TYPE "FRAGMENT")
	elseif("${TYPE}" STREQUAL "vs")
		set(TYPE "VERTEX")
	elseif("${TYPE}" STREQUAL "cs")
		set(TYPE "COMPUTE")
	else()
		set(TYPE "")
	endif()

	if(NOT "${TYPE}" STREQUAL "")
		set(COMMON FILE ${FILE} ${TYPE} INCLUDES ${BGFX_DIR}/Prototypes)
		set(OUTPUTS "")
		set(OUTPUTS_PRETTY "")

		if(WIN32)
			# dx11
			set(DX11_OUTPUT ${SGRENDER_DIR}/Prototypes/runtime/shaders/dx11/${FILENAME}.bin)
			if(NOT "${TYPE}" STREQUAL "COMPUTE")
				_bgfx_shaderc_parse(
					DX11 ${COMMON} WINDOWS
					PROFILE s_5_0
					O 3
					OUTPUT ${DX11_OUTPUT}
					DEBUG 
					DISASM 
				)
			else()
				_bgfx_shaderc_parse(
					DX11 ${COMMON} WINDOWS
					PROFILE s_5_0
					O 1
					OUTPUT ${DX11_OUTPUT}
					DEBUG
					DISASM
				)
			endif()
			list(APPEND OUTPUTS "DX11")
			set(OUTPUTS_PRETTY "${OUTPUTS_PRETTY}DX11, ")
		endif()

		# essl
		if(NOT "${TYPE}" STREQUAL "COMPUTE")
			set(ESSL_OUTPUT ${SGRENDER_DIR}/Prototypes/runtime/shaders/essl/${FILENAME}.bin)
			_bgfx_shaderc_parse(ESSL ${COMMON} ANDROID PROFILE 100_es OUTPUT ${ESSL_OUTPUT})
			list(APPEND OUTPUTS "ESSL")
			set(OUTPUTS_PRETTY "${OUTPUTS_PRETTY}ESSL, ")
		endif()

		# glsl
		set(GLSL_OUTPUT ${SGRENDER_DIR}/Prototypes/runtime/shaders/glsl/${FILENAME}.bin)
		if(NOT "${TYPE}" STREQUAL "COMPUTE")
			_bgfx_shaderc_parse(GLSL ${COMMON} LINUX PROFILE 140 OUTPUT ${GLSL_OUTPUT})
		else()
			_bgfx_shaderc_parse(GLSL ${COMMON} LINUX PROFILE 430 OUTPUT ${GLSL_OUTPUT})
		endif()
		list(APPEND OUTPUTS "GLSL")
		set(OUTPUTS_PRETTY "${OUTPUTS_PRETTY}GLSL, ")

		# spirv
		if(NOT "${TYPE}" STREQUAL "COMPUTE")
			set(SPIRV_OUTPUT ${SGRENDER_DIR}/Prototypes/runtime/shaders/spirv/${FILENAME}.bin)
			_bgfx_shaderc_parse(SPIRV ${COMMON} LINUX PROFILE spirv OUTPUT ${SPIRV_OUTPUT})
			list(APPEND OUTPUTS "SPIRV")
			set(OUTPUTS_PRETTY "${OUTPUTS_PRETTY}SPIRV")
			set(OUTPUT_FILES "")
			set(COMMANDS "")
		endif()

		foreach(OUT ${OUTPUTS})
			list(APPEND OUTPUT_FILES ${${OUT}_OUTPUT})
			list(APPEND COMMANDS COMMAND "bgfx::shaderc" ${${OUT}})
			get_filename_component(OUT_DIR ${${OUT}_OUTPUT} DIRECTORY)
			file(MAKE_DIRECTORY ${OUT_DIR})
		endforeach()

		file(RELATIVE_PATH PRINT_NAME ${SGRENDER_DIR}/Prototypes ${FILE})
		add_custom_command(
			MAIN_DEPENDENCY ${FILE} OUTPUT ${OUTPUT_FILES} ${COMMANDS}
			COMMENT "Compiling shader ${PRINT_NAME} for ${OUTPUTS_PRETTY}"
		)
	endif()
endfunction()

function(add_prototype ARG_NAME)
    # Parse arguments
    cmake_parse_arguments(ARG "COMMON" "" "DIRECTORIES;SOURCES" ${ARGN})

    # Get all source files
    list(APPEND ARG_DIRECTORIES "${SGRENDER_DIR}/Prototypes/${ARG_NAME}")
    set(SOURCES "")
    set(SHADERS "")
    set(SHADERHEADERS "")

    foreach(DIR ${ARG_DIRECTORIES})
        if(APPLE)
            file(GLOB GLOB_SOURCES ${DIR}/*.mm)
            list(APPEND SOURCES ${GLOB_SOURCES})
        endif()
        file(GLOB GLOB_SOURCES ${DIR}/*.c ${DIR}/*.cpp ${DIR}/*.h ${DIR}/*.hpp ${DIR}/*.sc ${DIR}/*.sh)
        list(APPEND SOURCES ${GLOB_SOURCES})
        file(GLOB GLOB_SHADERS ${DIR}/*.sc)
        list(APPEND SHADERS ${GLOB_SHADERS})
		file(GLOB GLOB_SHADERHEADERS ${DIR}/*.sh)
		list(APPEND SHADERHEADERS ${GLOB_SHADERHEADERS})
    endforeach()
	
	if(ARG_COMMON)
		add_executable(prototype-${ARG_NAME} WIN32 ${SOURCES})

    else()
        if(NOT ANDROID)
            add_executable(prototype-${ARG_NAME} WIN32 ${SOURCES})
        endif()
        #target_link_libraries(prototype-${ARG_NAME} PUBLIC prototype-common)
        configure_debugging(prototype-${ARG_NAME} WORKING_DIR ${SGRENDER_DIR}/Prototypes/runtime)
        if(MSVC)
            set_target_properties(prototype-${ARG_NAME} PROPERTIES LINK_FLAGS "/ENTRY:\"mainCRTStartup\"")
        endif()
        if(BGFX_CUSTOM_TARGETS)
            add_dependencies(prototypes prototype-${ARG_NAME})
        endif()
    endif()
    target_compile_definitions(
        prototype-${ARG_NAME}
        PRIVATE "-D_CRT_SECURE_NO_WARNINGS" #
                "-D__STDC_FORMAT_MACROS" #
                "-DENTRY_CONFIG_IMPLEMENT_MAIN=1"
    )
    
    set(BGDIR ${SGRENDER_DIR}/3rdParty/bgfx.cmake)

    
    target_include_directories(
        prototype-${ARG_NAME} 
        PUBLIC  ${BGDIR}/bgfx/examples/common
                ${BGDIR}/bgfx/3rdparty
                ${BGDIR}/include
                ${BGDIR}/bg/include
                ${BGDIR}/bx/3rdparty
                ${BGDIR}/bx/include
                ${BGDIR}/bx/include/compat/msvc
                ${BGDIR}/bimg/include 
    )

    #link_directories(${SGRENDER_DIR}/.build/3rdParty/bgfx.cmake/cmake/bgfx)
    target_link_libraries(
        prototype-${ARG_NAME}
        PRIVATE bgfx bx bimg example-common
               ${DIRECTX_HEADERS}
    )
    # Configure shaders
    if(NOT ARG_COMMON
        AND NOT IOS
        AND NOT EMSCRIPTEN
        AND NOT ANDROID
    )
        foreach(SHADER ${SHADERS})
            add_bgfx_shader(${SHADER} ${ARG_NAME})
        endforeach()
		
        source_group("Shader Files" FILES ${SHADERS} ${SHADERHEADERS})
    endif()

    set_target_properties(prototype-${ARG_NAME} PROPERTIES FOLDER "SGTestBed/Prototypes")
endfunction()

if(BGFX_CUSTOM_TARGETS)
    add_custom_target(prototypes)
    set_target_properties(prototypes PROPERTIES FOLDER "SGTestBed/Prototypes" )
endif()

add_prototype(
	common 
	COMMON 
	DIRECTORIES
    ${SGRENDER_DIR}/Prototypes/common
)

if(SGTESTBED_BUILD_PROTOTYPES)
    # Add Prototypes
    set(SGTESTBED_PROTOTYPES 
        01-GoochHighlighted
		02-Lights-Basic
    )

    foreach(PROTOTYPE ${SGTESTBED_PROTOTYPES})
        add_prototype(${PROTOTYPE})
    endforeach()

    if(SGTESTBED_INSTALL_EXAMPLES)
        install(DIRECTORY ${SGRENDER_DIR}/Prototypes/runtime/ DESTINATION Prototypes)
        foreach(PROTOTYPE ${SGTESTBED_PROTOTYPES})
            install(TARGETS prototype-${PROTOTYPE} DESTINATION Prototypes)
        endforeach()
    endif()
endif()

    