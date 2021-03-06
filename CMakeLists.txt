cmake_minimum_required(VERSION 3.12)
project(userbindmount
		VERSION 0.0.1
		DESCRIPTION "bind mount in user namespaces"
		HOMEPAGE_URL "https://github.com/rd235/userbindmount"
		LANGUAGES C)

include(GNUInstallDirs)
add_definitions(-D_GNU_SOURCE)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_FORTIFY_SOURCE=2 -O2 -pedantic -Wall -Wextra")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

find_library(TESTCAP cap)
if(NOT TESTCAP)
	message(FATAL_ERROR "libcap not found")
endif()

add_library(libuserbindmount SHARED libuserbindmount.c)
target_link_libraries(libuserbindmount cap)
set_target_properties(libuserbindmount PROPERTIES
		OUTPUT_NAME "userbindmount"
		VERSION ${PROJECT_VERSION}
		SOVERSION ${PROJECT_VERSION_MAJOR})

add_library(libuserbindmount-static STATIC libuserbindmount.c)
set_target_properties(libuserbindmount-static
		PROPERTIES OUTPUT_NAME userbindmount)

configure_file(userbindmount.pc.in userbindmount.pc @ONLY)

install(TARGETS libuserbindmount
		LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(TARGETS libuserbindmount-static
		ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES userbindmount.h
		DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/userbindmount.pc
		DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

add_executable(userbindmount userbindmount.c)
target_link_libraries(userbindmount libuserbindmount cap)

install(TARGETS userbindmount
		RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

add_subdirectory(man)

add_custom_target(uninstall
  "${CMAKE_COMMAND}" -P "${PROJECT_SOURCE_DIR}/Uninstall.cmake")
