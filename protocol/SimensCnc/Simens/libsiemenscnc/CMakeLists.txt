#set version
cmake_minimum_required ( VERSION 3.10 )

#project
project ( siemenscnc )

set ( EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin )
set ( LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib )


#set include 
include_directories ( ./inc )

#set source file
aux_source_directory ( ./src DIR_CNC_SRC )

#set CC AR LD AS
set ( CMAKE_C_COMPILER $ENV{CC} )
set ( CMAKE_C_AR $ENV{AR} )


set ( CMAKE_C_FLAGS "-g -pthread" )
set ( CMAKE_CXX_FLAGS "-g -std=c++11 -pthread" )

#macro
#add_definitions (
#	-DCMAKE_CXX_COMPILER=$(which arm-linux-gnueabihf-g++) 
#	-DCMAKE_C_COMPILER=$(which arm-linux-gnueabihf-gcc)
#)

#generate target
add_library ( siemenscnc ${DIR_CNC_SRC} )

#add custom 
add_custom_target ( clean-all COMMAND rm -rf ${CMAKE_BINARY_DIR}/* )


