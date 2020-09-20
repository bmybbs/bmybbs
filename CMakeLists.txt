cmake_minimum_required(VERSION 3.10)

project(BMYBBS)

find_package(LibXml2 REQUIRED)
#find_package(MySQL REQUIRED)

include_directories(include libythtbbs libbmy ${LIBXML2_INCLUDE_DIR})

link_directories(ythtlib libythtbbs libbmy)

set(CMAKE_C_FLAGS "-Wall -g -pipe -Wextra -Wno-unused-result -Wno-unused-parameter -Wno-unused-variable -Wno-unused-but-set-variable -Wno-unused-value -std=gnu11")

add_definitions(-D_GNU_SOURCE -DCONF_HAVE_OPENPTY -DLINUX -DSMTP_OPENSSL)

add_subdirectory(libytht)
add_subdirectory(libbmy)
add_subdirectory(libythtbbs)
add_subdirectory(src)
add_subdirectory(smth_sshbbsd)
add_subdirectory(nju09)
add_subdirectory(local_utl)
add_subdirectory(atthttpd)
add_subdirectory(api)
add_subdirectory(tests)

