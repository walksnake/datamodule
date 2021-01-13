#set version
cmake_minimum_required ( VERSION 3.10 )

#project
project ( easylog )

set ( EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin )
set ( LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib )


#set include 
include_directories ( ./inc )

#set source file
aux_source_directory ( ./src DIR_LOG_SRC )

#set CC AR LD AS
set ( CMAKE_C_COMPILER $ENV{CC} )
set ( CMAKE_C_AR $ENV{AR} )


set ( CMAKE_C_FLAGS "-g -lpthread" )
set ( CMAKE_CXX_FLAGS "-g -std=c++11 -lpthread" )

#for log
add_definitions (
	#-DELPP_NO_DEFAULT_LOG_FILE
		-DELPP_THREAD_SAFE
)


#generate target
add_library ( easylog ${DIR_LOG_SRC} )

#add custom 
add_custom_target ( clean-all COMMAND rm -rf ${CMAKE_BINARY_DIR}/* )


