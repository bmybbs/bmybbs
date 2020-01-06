cmake_minimum_required(VERSION 3.10)

project(BMYBBS)

find_package(LibXml2 REQUIRED)
#find_package(MySQL REQUIRED)

include_directories(include ythtlib libythtbbs ${LIBXML2_INCLUDE_DIR})

link_directories(ythtlib libythtbbs)

set(CMAKE_C_FLAGS "-Wall -g -pipe")

add_definitions(-D_GNU_SOURCE -DCONF_HAVE_OPENPTY -DLINUX)

add_subdirectory(ythtlib)
add_subdirectory(libythtbbs)
add_subdirectory(src)
add_subdirectory(nju09)
add_subdirectory(local_utl)
