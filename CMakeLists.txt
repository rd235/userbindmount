cmake_minimum_required(VERSION 2.6)
project("userbindmount")

add_definitions(-D_GNU_SOURCE)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

find_library(TESTCAP cap)
if(NOT TESTCAP)
	message(FATAL_ERROR "libcap not found")
endif()

add_library(libuserbindmount SHARED libuserbindmount.c)
target_link_libraries(libuserbindmount cap)

set_target_properties(libuserbindmount PROPERTIES
		OUTPUT_NAME "userbindmount"
		VERSION "0.0.0"
    SOVERSION "0")

install(TARGETS libuserbindmount LIBRARY DESTINATION lib)
install(FILES userbindmount.h DESTINATION include)
install(FILES libuserbindmount.3
    DESTINATION ${CMAKE_INSTALL_MANDIR}/man3)

add_executable(userbindmount userbindmount.c)
target_link_libraries(userbindmount libuserbindmount cap)

install(TARGETS userbindmount RUNTIME DESTINATION bin)
install(FILES userbindmount.1
    DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)

