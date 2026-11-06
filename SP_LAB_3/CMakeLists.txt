cmake_minimum_required(VERSION 3.10)

set(FLEX_EXECUTABLE
    "C:/Tools/win_flex.exe"
    CACHE FILEPATH "Flex executable"
)

set(BISON_EXECUTABLE
    "C:/Tools/win_bison.exe"
    CACHE FILEPATH "Bison executable"
)

find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

BISON_TARGET(Parser
    ${CMAKE_CURRENT_SOURCE_DIR}/parser.y
    ${CMAKE_CURRENT_BINARY_DIR}/parser.tab.cpp
    DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/parser.tab.h
)

FLEX_TARGET(Lexer
    ${CMAKE_CURRENT_SOURCE_DIR}/lexer.l
    ${CMAKE_CURRENT_BINARY_DIR}/lexer.cpp
)

ADD_FLEX_BISON_DEPENDENCY(Lexer Parser)

add_executable(SP_LAB_3
    SP_LAB_3.cpp
    SP_LAB_3.h
    ast.h
    ${BISON_Parser_OUTPUTS}
    ${FLEX_Lexer_OUTPUTS}
)

target_include_directories(SP_LAB_3 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

set_target_properties(SP_LAB_3 PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
