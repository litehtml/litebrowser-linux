cmake_minimum_required(VERSION 3.11)

project(litebrowser CXX)

find_package(PkgConfig REQUIRED)

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "No build type selected, default to Release")
    set(CMAKE_BUILD_TYPE "Release")
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-O0 -DDEBUG -g")
set(CMAKE_C_FLAGS_DEBUG "-O0 -DDEBUG -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_C_FLAGS_RELEASE "-O3")

set(LITEBROWSER_PATH src)
set(LITEHTML_PATH litehtml)
set(CONTAINER_PATH ${LITEHTML_PATH}/containers/linux)

pkg_check_modules(LB_LIBS REQUIRED fontconfig gdkmm-3.0 gtkmm-3.0 libcurl cairo pango pangocairo)

set(SOURCE  ${LITEBROWSER_PATH}/main.cpp
            ${LITEBROWSER_PATH}/html_widget.cpp
            ${CONTAINER_PATH}/container_linux.cpp
            ${LITEBROWSER_PATH}/browser_wnd.cpp
            ${LITEBROWSER_PATH}/web_history.cpp
    )

set(HEADERS ${LITEBROWSER_PATH}/browser_wnd.h
            ${LITEBROWSER_PATH}/html_widget.h
            ${LITEBROWSER_PATH}/globals.h
            ${CONTAINER_PATH}/container_linux.h
            ${LITEBROWSER_PATH}/web_history.h
    )

add_subdirectory(${LITEHTML_PATH})

add_executable(${PROJECT_NAME} ${SOURCE} ${HEADERS} src/http_loader.h src/http_loader.cpp)
include_directories(${PROJECT_NAME} ${LITEHTML_PATH}/include ${LB_LIBS_INCLUDE_DIRS})
target_link_options(${PROJECT_NAME} PRIVATE ${LB_LIBS_LDFLAGS})
target_link_libraries(${PROJECT_NAME} litehtml ${LB_LIBS_LIBRARIES})

set_target_properties(${PROJECT_NAME} PROPERTIES
        CXX_STANDARD 11
        C_STANDARD 99
        )

