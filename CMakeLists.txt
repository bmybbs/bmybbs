cmake_minimum_required(VERSION 3.10)
set(CMAKE_EXPORT_COMPILE_COMMANDS on)

project(BMYBBS)

find_package(LibXml2 REQUIRED)
#find_package(MySQL REQUIRED)

include_directories(include ${LIBXML2_INCLUDE_DIR})

link_directories(libytht libbmy libythtbbs)

set(CMAKE_C_FLAGS "-Wall -g -pipe -Wextra -std=gnu11")

add_definitions(-D_GNU_SOURCE -DCONF_HAVE_OPENPTY -DLINUX -DSMTP_OPENSSL)

add_subdirectory(libytht)
add_subdirectory(libbmy)
add_subdirectory(libythtbbs)
add_subdirectory(src)
add_subdirectory(smth_sshbbsd)
add_subdirectory(nju09)
add_subdirectory(local_utl)
add_subdirectory(atthttpd)
if(EXISTS ${CMAKE_SOURCE_DIR}/api-local/CMakeLists.txt)
	add_subdirectory(api-local) # 用于本地开发调试
else(EXISTS ${CMAKE_SOURCE_DIR}/api-local/CMakeLists.txt)
	if(EXISTS ${CMAKE_SOURCE_DIR}/api/CMakeLists.txt)
		add_subdirectory(api) # 用于普通使用，在已拉取子模块的情况下
	endif(EXISTS ${CMAKE_SOURCE_DIR}/api/CMakeLists.txt)
endif(EXISTS ${CMAKE_SOURCE_DIR}/api-local/CMakeLists.txt)
add_subdirectory(tests)

