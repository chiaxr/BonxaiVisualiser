project(BonxaiVisualiser)

cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 17)

# Configure dependencies
set(PROJECT_LIB_DIR ${PROJECT_SOURCE_DIR}/third_party)
# bonxai
set(BONXAI_LIB_DIR ${PROJECT_LIB_DIR}/Bonxai)
# raylib
add_subdirectory(${PROJECT_LIB_DIR}/raylib)
# raygui
set(RAYGUI_LIB_DIR ${PROJECT_LIB_DIR}/raygui)

add_executable(${PROJECT_NAME}
    main.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}
    ${BONXAI_LIB_DIR}/bonxai_core/include
    ${RAYGUI_LIB_DIR}/src
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    raylib
)