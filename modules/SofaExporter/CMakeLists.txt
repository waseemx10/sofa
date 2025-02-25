cmake_minimum_required(VERSION 3.1)
project(SofaExporter VERSION 1.0 LANGUAGES CXX)

find_package(SofaSimulation REQUIRED)
sofa_find_package(ZLIB REQUIRED)
sofa_find_package(SofaPython QUIET)

set(HEADER_FILES
    src/SofaExporter/config.h.in
    src/SofaExporter/initExporter.h
    )

set(SOURCE_FILES
    src/SofaExporter/initExporter.cpp
    )

set(EXTRA_FILES
    README.md
    examples/MeshExporter.scn
    examples/STLExporter.scn
    examples/OBJExporter.scn
    )

list(APPEND HEADER_FILES
    src/SofaExporter/BlenderExporter.h
    src/SofaExporter/BlenderExporter.inl
    src/SofaExporter/MeshExporter.h
    src/SofaExporter/OBJExporter.h
    src/SofaExporter/STLExporter.h
    src/SofaExporter/VTKExporter.h
    src/SofaExporter/WriteState.h
    src/SofaExporter/WriteState.inl
    src/SofaExporter/WriteTopology.h
    src/SofaExporter/WriteTopology.inl
    )

list(APPEND SOURCE_FILES
    src/SofaExporter/BlenderExporter.cpp
    src/SofaExporter/MeshExporter.cpp
    src/SofaExporter/OBJExporter.cpp
    src/SofaExporter/STLExporter.cpp
    src/SofaExporter/VTKExporter.cpp
    src/SofaExporter/WriteState.cpp
    src/SofaExporter/WriteTopology.cpp
    )

if(SofaPython_FOUND)
    list(APPEND HEADER_FILES
        src/SofaExporter/bindings/Binding_OBJExporter.h
        src/SofaExporter/bindings/Binding_STLExporter.h
    )
    list(APPEND SOURCE_FILES
        src/SofaExporter/bindings/Binding_OBJExporter.cpp
        src/SofaExporter/bindings/Binding_STLExporter.cpp
    )
else()
    message(STATUS "SofaPython disabled, will not compile python bindings.")
endif()

add_library(${PROJECT_NAME} SHARED ${HEADER_FILES} ${SOURCE_FILES} ${EXTRA_FILES})
target_link_libraries(${PROJECT_NAME} PUBLIC SofaSimulationTree)
target_link_libraries(${PROJECT_NAME} PUBLIC ZLIB::ZLIB)
if(CMAKE_SYSTEM_NAME STREQUAL Windows)
    sofa_install_libraries(TARGETS ZLIB::ZLIB)
endif()
if(SofaPython_FOUND)
    target_link_libraries(${PROJECT_NAME} PUBLIC SofaPython)
endif()
target_compile_definitions(${PROJECT_NAME} PRIVATE "-DSOFA_BUILD_EXPORTER")
target_include_directories(${PROJECT_NAME} PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>")
target_include_directories(${PROJECT_NAME} PUBLIC "$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>")
target_include_directories(${PROJECT_NAME} PUBLIC "$<INSTALL_INTERFACE:include>")

sofa_generate_package(
    NAME SofaExporter
    VERSION ${PROJECT_VERSION}
    TARGETS ${PROJECT_NAME}
    INCLUDE_SOURCE_DIR "src/SofaExporter"
    INCLUDE_INSTALL_DIR "SofaExporter"
    RELOCATABLE "plugins"
    )

# Tests
# If SOFA_BUILD_TESTS exists and is OFF, then these tests will be auto-disabled
cmake_dependent_option(SOFAEXPORTER_BUILD_TESTS "Compile the automatic tests" ON "SOFA_BUILD_TESTS OR NOT DEFINED SOFA_BUILD_TESTS" OFF)
if(SOFAEXPORTER_BUILD_TESTS)
    enable_testing()
    add_subdirectory(SofaExporter_test)
endif()
