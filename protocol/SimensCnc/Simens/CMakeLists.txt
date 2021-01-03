#cmake version
cmake_minimum_required ( VERSION 3.10 )

project ( siemenscnctest )

#reset output path
set ( EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin )
set ( LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib )

set ( CMAKE_SHARED_LIBRARY_LINK_C_FLAGS )

set (CMAKE_PREFIX_PATH $ENV{XDC_PROTOCOL_DIR})

message (STATUS ${CMAKE_PREFIX_PATH})
#set include file
include_directories (${CMAKE_PREFIX_PATH}/SimensCnc/Simens/libsiemenscnc/inc)
include_directories (./libsiemenscnc/828d)

#set source file
aux_source_directory ( . DIR_USR_SOURCE )
aux_source_directory ( ./libsiemenscnc/828d DIR_USR_SOURCE )

#set CC AR LD AS
set ( CMAKE_C_COMPILER $ENV{CC} )
set ( CMAKE_C_AR $ENV{AR} )

#CFLAGS
set ( CMAKE_C_FLAGS "-g -O3 -Wall")
#set ( CMAKE_C_FLAGS "-lcrypt")

#load extern library
add_library ( siemenscnc STATIC IMPORTED )
set_property (TARGET siemenscnc PROPERTY IMPORTED_LOCATION ${CMAKE_PREFIX_PATH}/SimensCnc/Simens/libsiemenscnc/build/lib/libsiemenscnc.a)

#generate target
add_executable ( ${CMAKE_PROJECT_NAME} ${DIR_USR_SOURCE} )

target_link_libraries ( ${CMAKE_PROJECT_NAME} PRIVATE siemenscnc)

#add custom
add_custom_target ( clean-all COMMAND rm -rf {CMAKE_BINARY_DIR}/* )
