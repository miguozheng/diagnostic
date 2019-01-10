#cmake_minimum_required(VERSION 2.6)
#project(cmake_test)
#SET(SRC_LIST
#	main.c
#	slist.c
#	uds_main.c
#	uds_network_layer.c
#	uds_session_layer.c)
#INCLUDE_DIRECTORIES(/usr/include/glib)
#INCLUDE_DIRECTORIES(/usr/include)
#INCLUDE_DIRECTORIES(/usr/local/include/glib-2.0)
#INCLUDE_DIRECTORIES(/usr/local/lib/glib-2.0/include)
#INCLUDE_DIRECTORIES(/usr/local/include)
#ADD_EXECUTABLE(Net ${SRC_LIST})
#TARGET_LINK_LIBRARIES(Net pthread glib-2.0)
cmake_minimum_required(VERSION 2.6)
#add gdb mode
SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")
#
project(cmake_test)
SET(SRC_LIST
	main.c
	slist.c
	uds_main.c
	uds_network_layer.c
	uds_session_layer.c
	uds_application_layer.c
	uds_interface_internal.c
	uds_interface_ext.c
	uds_eeprom.c)
INCLUDE(FindPkgConfig)#启用pkgconfig
PKG_CHECK_MODULES(GLIB glib-2.0 REQUIRED)#查找glib
INCLUDE_DIRECTORIES(/usr/include)
INCLUDE_DIRECTORIES(${GLIB_INCLUDE_DIRS})#包含h文件路径
ADD_EXECUTABLE(Net ${SRC_LIST})
TARGET_LINK_LIBRARIES(Net pthread ${GLIB_LDFLAGS})#链接glib库