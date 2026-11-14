cmake_minimum_required(VERSION 3.20)

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

pkg_check_modules(LB_ADW libadwaita-1)

set(SOURCE  ${LITEBROWSER_PATH}/main.cpp
			${LITEBROWSER_PATH}/browser_wnd.cpp
	)

set(HEADERS ${LITEBROWSER_PATH}/browser_wnd.h
			${LITEBROWSER_PATH}/html_dumper.h
		)

if (FOR_TESTING)
	set(HEADERS ${HEADERS} litehtml-tests/fonts.h)
	set(SOURCE ${SOURCE} litehtml-tests/fonts.cpp)
	pkg_check_modules(FONTCONFIG REQUIRED fontconfig)
	add_compile_options(-DFOR_TESTING)
endif ()

set(LITEHTML_BUILD_GTKMM4 ON CACHE BOOL "build litehtml_gtkmm4 library (requires gtkmm-4.0)" FORCE)
set(LITEHTML_BUILD_RENDER2PNG ON CACHE BOOL "build litehtml_render2png library (requires gtk4, gdk-pixbuf-2.0, fontconfig)")
add_subdirectory(${LITEHTML_PATH})

# additional warnings
add_compile_options(-Wall -Wextra -Wpedantic)
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	add_compile_options(-Wno-c2y-extensions)
endif()

add_executable(${PROJECT_NAME} ${SOURCE} ${HEADERS})

target_include_directories(${PROJECT_NAME} PRIVATE ${LB_ADW_INCLUDE_DIRS} ${FONTCONFIG_INCLUDE_DIRS} litehtml-tests)
target_link_options(${PROJECT_NAME} PRIVATE ${FONTCONFIG_LDFLAGS} ${LB_ADW_LDFLAGS})
target_link_libraries(${PROJECT_NAME} litehtml_gtkmm4 ${LB_ADW_LIBRARIES} ${FONTCONFIG_LIBRARIES})

if (LB_ADW_FOUND)
	target_compile_definitions(${PROJECT_NAME} PUBLIC LIBADWAITA_AVAILABLE=1)
endif ()

set_target_properties(${PROJECT_NAME} PROPERTIES
		CXX_STANDARD 17
		C_STANDARD 99
		)

