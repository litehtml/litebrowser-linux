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
set(SUPPORT_PATH ${LITEHTML_PATH}/support)

pkg_check_modules(LB_LIBS REQUIRED gtkmm-4.0 libcurl cairo pango pangocairo)
pkg_check_modules(LB_ADW libadwaita-1)

set(SOURCE  ${LITEBROWSER_PATH}/main.cpp
			${LITEBROWSER_PATH}/browser_wnd.cpp
			${SUPPORT_PATH}/gtkmm4/html_widget.cpp
			${SUPPORT_PATH}/webpage/web_history.cpp
			${SUPPORT_PATH}/webpage/http_request.cpp
			${SUPPORT_PATH}/webpage/web_page.cpp
			${SUPPORT_PATH}/webpage/http_requests_pool.cpp
			${SUPPORT_PATH}/draw_buffer/draw_buffer.cpp
			${CONTAINER_PATH}/container_cairo.cpp
			${CONTAINER_PATH}/cairo_borders.cpp
			${CONTAINER_PATH}/container_cairo_pango.cpp
			${CONTAINER_PATH}/conic_gradient.cpp
	)

set(HEADERS ${LITEBROWSER_PATH}/browser_wnd.h
			${LITEBROWSER_PATH}/html_dumper.h
			${SUPPORT_PATH}/gtkmm4/html_widget.h
			${SUPPORT_PATH}/webpage/web_history.h
			${SUPPORT_PATH}/webpage/http_request.h
			${SUPPORT_PATH}/webpage/web_page.h
			${SUPPORT_PATH}/webpage/html_host.h
			${SUPPORT_PATH}/webpage/http_requests_pool.h
			${SUPPORT_PATH}/draw_buffer/draw_buffer.h
			${CONTAINER_PATH}/container_cairo.h
			${CONTAINER_PATH}/cairo_borders.h
			${CONTAINER_PATH}/container_cairo_pango.h
			${CONTAINER_PATH}/cairo_images_cache.h
			${CONTAINER_PATH}/conic_gradient.h
		)

if (FOR_TESTING)
	set(HEADERS ${HEADERS} litehtml-tests/fonts.h)
	set(SOURCE ${SOURCE} litehtml-tests/fonts.cpp)
	pkg_check_modules(FONTCONFIG REQUIRED fontconfig)
	add_compile_options(-DFOR_TESTING)
endif ()

add_subdirectory(${LITEHTML_PATH})

# additional warnings
add_compile_options(-Wall -Wextra -Wpedantic)

add_executable(${PROJECT_NAME} ${SOURCE} ${HEADERS})

include_directories(${PROJECT_NAME} ${LITEHTML_PATH}/include ${SUPPORT_PATH}/webpage ${SUPPORT_PATH}/draw_buffer ${SUPPORT_PATH}/gtkmm4 ${LB_LIBS_INCLUDE_DIRS} ${LB_ADW_INCLUDE_DIRS} ${CONTAINER_PATH} litehtml-tests)
target_link_options(${PROJECT_NAME} PRIVATE ${LB_LIBS_LDFLAGS} ${FONTCONFIG_LDFLAGS} ${LB_ADW_LDFLAGS})
target_link_libraries(${PROJECT_NAME} litehtml ${LB_LIBS_LIBRARIES} ${LB_ADW_LIBRARIES} ${FONTCONFIG_LIBRARIES})

if (LB_ADW_FOUND)
	target_compile_definitions(${PROJECT_NAME} PUBLIC LIBADWAITA_AVAILABLE=1)
endif ()

set_target_properties(${PROJECT_NAME} PROPERTIES
		CXX_STANDARD 17
		C_STANDARD 99
		)

