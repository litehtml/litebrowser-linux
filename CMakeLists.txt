cmake_minimum_required(VERSION 2.8)

include(FindPkgConfig)

project(litebrowser)

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "No build type selected, default to Release")
    set(CMAKE_BUILD_TYPE "Release")
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-std=c++11 -O0 -DDEBUG -g")
set(CMAKE_C_FLAGS_DEBUG "-std=c++11 -O0 -DDEBUG -g")
set(CMAKE_CXX_FLAGS_RELEASE "-std=c++11 -O3")
set(CMAKE_C_FLAGS_RELEASE "-std=c++11 -O3")

set(LITEBROWSER_PATH src)
set(LITEHTML_PATH litehtml)
set(CONTAINER_PATH ${LITEHTML_PATH}/containers/linux)

pkg_check_modules(LB_LIBS REQUIRED fontconfig gdkmm-3.0 gtkmm-3.0 libcurl cairo)

set(SOURCE  ${LITEBROWSER_PATH}/main.cpp
            ${LITEBROWSER_PATH}/utils.cpp
            ${LITEBROWSER_PATH}/html_widget.cpp
            ${CONTAINER_PATH}/container_linux.cpp
            ${LITEBROWSER_PATH}/broser_wnd.cpp
    )

include_directories(${LITEHTML_PATH}/include ${LB_LIBS_INCLUDE_DIRS})

add_subdirectory(${LITEHTML_PATH})

find_program(XXD_COMMAND xxd)
add_custom_command(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/src/master.css.inc
                   COMMAND cat ${CMAKE_CURRENT_SOURCE_DIR}/${LITEHTML_PATH}/include/master.css | xxd -i > ${CMAKE_CURRENT_SOURCE_DIR}/src/master.css.inc)
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/src/master.css.inc PROPERTIES GENERATED TRUE)

add_executable(litebrowser ${SOURCE} ${CMAKE_CURRENT_SOURCE_DIR}/src/master.css.inc src/http_loader.h src/http_loader.cpp)

target_link_libraries(litebrowser litehtml ${LB_LIBS_LIBRARIES} PocoFoundation)

