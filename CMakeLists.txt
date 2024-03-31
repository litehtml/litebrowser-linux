cmake_minimum_required(VERSION 3.11)

project(litebrowser CXX)

find_package(PkgConfig REQUIRED)

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "No build type selected, default to Release")
    set(CMAKE_BUILD_TYPE "Release")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -DDEBUG -g")
set(CMAKE_C_FLAGS_DEBUG "-O0 -DDEBUG -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_C_FLAGS_RELEASE "-O3")

set(LITEBROWSER_PATH src)
set(LITEHTML_PATH litehtml)
set(CONTAINER_PATH ${LITEHTML_PATH}/containers/cairo)

pkg_check_modules(LB_LIBS REQUIRED gdkmm-3.0 gtkmm-3.0 libcurl cairo pango pangocairo)

set(SOURCE  ${LITEBROWSER_PATH}/main.cpp
            ${LITEBROWSER_PATH}/html_widget.cpp
            ${LITEBROWSER_PATH}/browser_wnd.cpp
            ${LITEBROWSER_PATH}/web_history.cpp
            ${LITEBROWSER_PATH}/http_request.cpp
            ${LITEBROWSER_PATH}/web_page.cpp
            ${LITEBROWSER_PATH}/http_requests_pool.cpp
            ${CONTAINER_PATH}/container_cairo.cpp
            ${CONTAINER_PATH}/cairo_borders.cpp
            ${CONTAINER_PATH}/container_cairo_pango.cpp
    )

set(HEADERS ${LITEBROWSER_PATH}/browser_wnd.h
            ${LITEBROWSER_PATH}/html_widget.h
            ${LITEBROWSER_PATH}/globals.h
            ${LITEBROWSER_PATH}/web_history.h
            ${LITEBROWSER_PATH}/http_request.h
            ${LITEBROWSER_PATH}/web_page.h
            ${LITEBROWSER_PATH}/html_host.h
            ${LITEBROWSER_PATH}/http_requests_pool.h
            ${LITEBROWSER_PATH}/html_dumper.h
            ${CONTAINER_PATH}/container_cairo.h
            ${CONTAINER_PATH}/cairo_borders.h
            ${CONTAINER_PATH}/container_cairo_pango.h
            ${CONTAINER_PATH}/cairo_images_cache.h
        )

option(LITEHTML_BUILD_TESTING "enable testing for litehtml" OFF)
add_subdirectory(${LITEHTML_PATH})

# additional warnings
add_compile_options(-Wall -Wextra -Wpedantic)

add_executable(${PROJECT_NAME} ${SOURCE} ${HEADERS})
include_directories(${PROJECT_NAME} ${LITEHTML_PATH}/include ${LB_LIBS_INCLUDE_DIRS} ${CONTAINER_PATH})
target_link_options(${PROJECT_NAME} PRIVATE ${LB_LIBS_LDFLAGS})
target_link_libraries(${PROJECT_NAME} litehtml ${LB_LIBS_LIBRARIES})

set_target_properties(${PROJECT_NAME} PROPERTIES
        CXX_STANDARD 17
        C_STANDARD 99
        )

