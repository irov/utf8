cmake_minimum_required(VERSION 3.10)

PROJECT(utf8 LANGUAGES C)

MACRO(ADD_FILTER group_name)
    SOURCE_GROUP(${group_name} FILES ${ARGN})
    SET(SOURCE_FILES ${SOURCE_FILES} ${ARGN})
ENDMACRO()

ADD_FILTER(
src
    include/utf8/utf8.h
    src/utf8.c
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

ADD_LIBRARY(${PROJECT_NAME} STATIC ${SOURCE_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE C)
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER ${PROJECT_NAME})

# Tests
option(BUILD_TESTS "Build test executable" ON)
if(BUILD_TESTS)
    add_executable(${PROJECT_NAME}_test tests/test_utf8.c)
    target_link_libraries(${PROJECT_NAME}_test PRIVATE ${PROJECT_NAME})
    target_include_directories(${PROJECT_NAME}_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    set_target_properties(${PROJECT_NAME}_test PROPERTIES FOLDER ${PROJECT_NAME})
    enable_testing()
    add_test(NAME ${PROJECT_NAME}_test COMMAND $<TARGET_FILE:${PROJECT_NAME}_test>)
endif()