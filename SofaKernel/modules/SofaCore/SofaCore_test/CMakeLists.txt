cmake_minimum_required(VERSION 3.1)

project(SofaCore_test)

set(SOURCE_FILES
    loader/MeshLoader_test.cpp
    objectmodel/AspectPool_test.cpp
    objectmodel/Data_test.cpp
    objectmodel/BaseLink_test.cpp
    objectmodel/BaseObjectDescription_test.cpp
    objectmodel/DataFileName_test.cpp
    objectmodel/DataCallback_test.cpp
    DataEngine_test.cpp
    TrackedData_test.cpp
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} SofaGTestMain SofaCore)

add_test(NAME SofaCore_test COMMAND SofaCore_test)
