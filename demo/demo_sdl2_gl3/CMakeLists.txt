
cmake_minimum_required (VERSION 3.21)

if (${CMAKE_GENERATOR} MATCHES "Visual Studio*")
    set(VSToolset "v143" CACHE STRING "Visual Studio: Platform Toolset")
    set_property(CACHE VSToolset PROPERTY STRINGS "v142" "v143" "v144" "clangcl")
    set(CMAKE_GENERATOR_TOOLSET "${VSToolset}")
endif()

set(Proj Demo_SDL2_GL3)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(${Proj})

set(RenderBackend "gl3" CACHE STRING "" FORCE)
set(InputBackend "sdl2" CACHE STRING "" FORCE)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

add_subdirectory("../../" "${CMAKE_CURRENT_BINARY_DIR}/Obj/DmGui")
add_executable(${Proj} "main.c")
target_link_libraries(${Proj} LINK_PUBLIC DmGui SDL2::SDL2)

if (WIN32)
    add_custom_command(TARGET ${Proj} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_RUNTIME_DLLS:${Proj}>" "$<TARGET_FILE_DIR:${Proj}>"
    )
endif()

add_custom_command(TARGET ${Proj} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different "${CMAKE_SOURCE_DIR}/../data" "$<TARGET_FILE_DIR:${Proj}>"
)

set_target_properties ( ${Proj} PROPERTIES WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
set_property (DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${Proj} )
set_property (TARGET ${Proj} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
set_property (TARGET ${Proj} PROPERTY VS_DPI_AWARE "PerMonitor")
