cmake_minimum_required(VERSION 3.31.5)

project("gr-engine-editor" VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

if(NOT DEFINED GR_ENGINE_CORE)
    message(FATAL_ERROR "GR_ENGINE_CORE NÃO foi definida!")
endif()

if(NOT GR_ENGINE_CORE)
    message(FATAL_ERROR "A variável GR_ENGINE_CORE está definida, mas está vazia.")
endif()

message(STATUS "GR_ENGINE_CORE configurada com sucesso: ${GR_ENGINE_CORE}")

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cpp")

add_library(${CMAKE_PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/${CMAKE_PROJECT_NAME}>
    ${GR_ENGINE_CORE}/include
    ${GR_ENGINE_CORE}/imgui
)




