#cmake version
cmake_minimum_required ( VERSION 3.10 )

project ( xDC CXX C)

#reset output path
set ( EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin )
set ( LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin )

set ( CMAKE_SHARED_LIBRARY_LINK_C_FLAGS )

#set include file
include_directories (./include)
include_directories (./feature/easyloggingpp/src)
include_directories (./common/typeany/inc)
include_directories (./thirdparty/restclient/include)
include_directories (./protocol/SimensCnc/Simens/libsiemenscnc/inc)
include_directories (./protocol/SimensCnc/Simens/libsiemenscnc/828d)
include_directories (./product/inc)
#set source file
aux_source_directory ( . DIR_USR_SOURCE )
aux_source_directory ( ./common/typeany/src DIR_USR_SOURCE )
aux_source_directory ( ./protocol/SimensCnc/Simens/libsiemenscnc/src DIR_USR_SOURCE )
aux_source_directory ( ./protocol/SimensCnc/Simens/libsiemenscnc/828d DIR_USR_SOURCE )
aux_source_directory ( ./product/src DIR_USR_SOURCE )
aux_source_directory ( ./feature/easyloggingpp/src DIR_USR_SOURCE )

#set CC AR LD AS
set ( CMAKE_C_COMPILER $ENV{CC} )
set ( CMAKE_C_AR $ENV{AR} )
set ( CMAKE_C_LD $ENV{LD} )

#CFLAGS
set ( CMAKE_C_FLAGS "-g -std=c++11" )
set ( CMAKE_CXX_FLAGS "-g -std=c++11" )

add_definitions (
	-DELPP_FEATURE_ALL
	#-DELPP_FEATURE_CRASH_LOG
	#-DELPP_NO_DEFAULT_LOG_FILE
	-DELPP_THREAD_SAFE
	-DELPP_OS_UNIX
	#-DELPP_SYSLOG
)

#set debug
if(CMAKE_BUILD_TYPE STREQUAL Debug)
	add_definitions (
		-DPRINT_DEBUG_INFO
	)
endif()

#load extern library
#add_library ( easyloggingpp STATIC IMPORTED )
#set_property (TARGET easyloggingpp PROPERTY IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/feature/easyloggingpp/build/libeasyloggingpp.a)

add_library ( restclient-cpp STATIC IMPORTED )
set_property (TARGET restclient-cpp PROPERTY IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/thirdparty/restclient/lib/librestclient-cpp.a)

add_library ( siemenscnc STATIC IMPORTED )
set_property (TARGET siemenscnc PROPERTY IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/protocol/SimensCnc/Simens/libsiemenscnc/build/lib/libsiemenscnc.a)

MESSAGE( STATUS "There are source files : ${DIR_USR_SOURCE}.")
#generate target
add_executable ( ${CMAKE_PROJECT_NAME} ${DIR_USR_SOURCE} )

target_link_libraries ( ${CMAKE_PROJECT_NAME} PRIVATE  restclient-cpp siemenscnc pthread dl curl )
#add custom
add_custom_target ( clean-all COMMAND rm -rf {CMAKE_BINARY_DIR}/* )
