# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.11

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/fchuanlin/datamodule

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/fchuanlin/datamodule/Build

# Include any dependencies generated for this target.
include CMakeFiles/xDC.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/xDC.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/xDC.dir/flags.make

CMakeFiles/xDC.dir/main.cpp.o: CMakeFiles/xDC.dir/flags.make
CMakeFiles/xDC.dir/main.cpp.o: ../main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/fchuanlin/datamodule/Build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/xDC.dir/main.cpp.o"
	/usr/lib64/ccache/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/xDC.dir/main.cpp.o -c /home/fchuanlin/datamodule/main.cpp

CMakeFiles/xDC.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/xDC.dir/main.cpp.i"
	/usr/lib64/ccache/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/fchuanlin/datamodule/main.cpp > CMakeFiles/xDC.dir/main.cpp.i

CMakeFiles/xDC.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/xDC.dir/main.cpp.s"
	/usr/lib64/ccache/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/fchuanlin/datamodule/main.cpp -o CMakeFiles/xDC.dir/main.cpp.s

CMakeFiles/xDC.dir/common/typeany/src/typeany.cpp.o: CMakeFiles/xDC.dir/flags.make
CMakeFiles/xDC.dir/common/typeany/src/typeany.cpp.o: ../common/typeany/src/typeany.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/fchuanlin/datamodule/Build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/xDC.dir/common/typeany/src/typeany.cpp.o"
	/usr/lib64/ccache/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/xDC.dir/common/typeany/src/typeany.cpp.o -c /home/fchuanlin/datamodule/common/typeany/src/typeany.cpp

CMakeFiles/xDC.dir/common/typeany/src/typeany.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/xDC.dir/common/typeany/src/typeany.cpp.i"
	/usr/lib64/ccache/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/fchuanlin/datamodule/common/typeany/src/typeany.cpp > CMakeFiles/xDC.dir/common/typeany/src/typeany.cpp.i

CMakeFiles/xDC.dir/common/typeany/src/typeany.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/xDC.dir/common/typeany/src/typeany.cpp.s"
	/usr/lib64/ccache/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/fchuanlin/datamodule/common/typeany/src/typeany.cpp -o CMakeFiles/xDC.dir/common/typeany/src/typeany.cpp.s

CMakeFiles/xDC.dir/protocol/SimensCnc/Simens/libsiemenscnc/src/siemenscnc.c.o: CMakeFiles/xDC.dir/flags.make
CMakeFiles/xDC.dir/protocol/SimensCnc/Simens/libsiemenscnc/src/siemenscnc.c.o: ../protocol/SimensCnc/Simens/libsiemenscnc/src/siemenscnc.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/fchuanlin/datamodule/Build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building C object CMakeFiles/xDC.dir/protocol/SimensCnc/Simens/libsiemenscnc/src/siemenscnc.c.o"
	gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/xDC.dir/protocol/SimensCnc/Simens/libsiemenscnc/src/siemenscnc.c.o   -c /home/fchuanlin/datamodule/protocol/SimensCnc/Simens/libsiemenscnc/src/siemenscnc.c

CMakeFiles/xDC.dir/protocol/SimensCnc/Simens/libsiemenscnc/src/siemenscnc.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/xDC.dir/protocol/SimensCnc/Simens/libsiemenscnc/src/siemenscnc.c.i"
	gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/fchuanlin/datamodule/protocol/SimensCnc/Simens/libsiemenscnc/src/siemenscnc.c > CMakeFiles/xDC.dir/protocol/SimensCnc/Simens/libsiemenscnc/src/siemenscnc.c.i

CMakeFiles/xDC.dir/protocol/SimensCnc/Simens/libsiemenscnc/src/siemenscnc.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/xDC.dir/protocol/SimensCnc/Simens/libsiemenscnc/src/siemenscnc.c.s"
	gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/fchuanlin/datamodule/protocol/SimensCnc/Simens/libsiemenscnc/src/siemenscnc.c -o CMakeFiles/xDC.dir/protocol/SimensCnc/Simens/libsiemenscnc/src/siemenscnc.c.s

CMakeFiles/xDC.dir/protocol/SimensCnc/Simens/libsiemenscnc/src/siemenscnc_data.c.o: CMakeFiles/xDC.dir/flags.make
CMakeFiles/xDC.dir/protocol/SimensCnc/Simens/libsiemenscnc/src/siemenscnc_data.c.o: ../protocol/SimensCnc/Simens/libsiemenscnc/src/siemenscnc_data.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/fchuanlin/datamodule/Build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building C object CMakeFiles/xDC.dir/protocol/SimensCnc/Simens/libsiemenscnc/src/siemenscnc_data.c.o"
	gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/xDC.dir/protocol/SimensCnc/Simens/libsiemenscnc/src/siemenscnc_data.c.o   -c /home/fchuanlin/datamodule/protocol/SimensCnc/Simens/libsiemenscnc/src/siemenscnc_data.c

CMakeFiles/xDC.dir/protocol/SimensCnc/Simens/libsiemenscnc/src/siemenscnc_data.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/xDC.dir/protocol/SimensCnc/Simens/libsiemenscnc/src/siemenscnc_data.c.i"
	gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/fchuanlin/datamodule/protocol/SimensCnc/Simens/libsiemenscnc/src/siemenscnc_data.c > CMakeFiles/xDC.dir/protocol/SimensCnc/Simens/libsiemenscnc/src/siemenscnc_data.c.i

CMakeFiles/xDC.dir/protocol/SimensCnc/Simens/libsiemenscnc/src/siemenscnc_data.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/xDC.dir/protocol/SimensCnc/Simens/libsiemenscnc/src/siemenscnc_data.c.s"
	gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/fchuanlin/datamodule/protocol/SimensCnc/Simens/libsiemenscnc/src/siemenscnc_data.c -o CMakeFiles/xDC.dir/protocol/SimensCnc/Simens/libsiemenscnc/src/siemenscnc_data.c.s

CMakeFiles/xDC.dir/protocol/SimensCnc/Simens/libsiemenscnc/src/siemenscnc_private.c.o: CMakeFiles/xDC.dir/flags.make
CMakeFiles/xDC.dir/protocol/SimensCnc/Simens/libsiemenscnc/src/siemenscnc_private.c.o: ../protocol/SimensCnc/Simens/libsiemenscnc/src/siemenscnc_private.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/fchuanlin/datamodule/Build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building C object CMakeFiles/xDC.dir/protocol/SimensCnc/Simens/libsiemenscnc/src/siemenscnc_private.c.o"
	gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/xDC.dir/protocol/SimensCnc/Simens/libsiemenscnc/src/siemenscnc_private.c.o   -c /home/fchuanlin/datamodule/protocol/SimensCnc/Simens/libsiemenscnc/src/siemenscnc_private.c

CMakeFiles/xDC.dir/protocol/SimensCnc/Simens/libsiemenscnc/src/siemenscnc_private.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/xDC.dir/protocol/SimensCnc/Simens/libsiemenscnc/src/siemenscnc_private.c.i"
	gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/fchuanlin/datamodule/protocol/SimensCnc/Simens/libsiemenscnc/src/siemenscnc_private.c > CMakeFiles/xDC.dir/protocol/SimensCnc/Simens/libsiemenscnc/src/siemenscnc_private.c.i

CMakeFiles/xDC.dir/protocol/SimensCnc/Simens/libsiemenscnc/src/siemenscnc_private.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/xDC.dir/protocol/SimensCnc/Simens/libsiemenscnc/src/siemenscnc_private.c.s"
	gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/fchuanlin/datamodule/protocol/SimensCnc/Simens/libsiemenscnc/src/siemenscnc_private.c -o CMakeFiles/xDC.dir/protocol/SimensCnc/Simens/libsiemenscnc/src/siemenscnc_private.c.s

CMakeFiles/xDC.dir/protocol/SimensCnc/Simens/libsiemenscnc/src/siemenscnc_tcp_private.c.o: CMakeFiles/xDC.dir/flags.make
CMakeFiles/xDC.dir/protocol/SimensCnc/Simens/libsiemenscnc/src/siemenscnc_tcp_private.c.o: ../protocol/SimensCnc/Simens/libsiemenscnc/src/siemenscnc_tcp_private.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/fchuanlin/datamodule/Build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building C object CMakeFiles/xDC.dir/protocol/SimensCnc/Simens/libsiemenscnc/src/siemenscnc_tcp_private.c.o"
	gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/xDC.dir/protocol/SimensCnc/Simens/libsiemenscnc/src/siemenscnc_tcp_private.c.o   -c /home/fchuanlin/datamodule/protocol/SimensCnc/Simens/libsiemenscnc/src/siemenscnc_tcp_private.c

CMakeFiles/xDC.dir/protocol/SimensCnc/Simens/libsiemenscnc/src/siemenscnc_tcp_private.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/xDC.dir/protocol/SimensCnc/Simens/libsiemenscnc/src/siemenscnc_tcp_private.c.i"
	gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/fchuanlin/datamodule/protocol/SimensCnc/Simens/libsiemenscnc/src/siemenscnc_tcp_private.c > CMakeFiles/xDC.dir/protocol/SimensCnc/Simens/libsiemenscnc/src/siemenscnc_tcp_private.c.i

CMakeFiles/xDC.dir/protocol/SimensCnc/Simens/libsiemenscnc/src/siemenscnc_tcp_private.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/xDC.dir/protocol/SimensCnc/Simens/libsiemenscnc/src/siemenscnc_tcp_private.c.s"
	gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/fchuanlin/datamodule/protocol/SimensCnc/Simens/libsiemenscnc/src/siemenscnc_tcp_private.c -o CMakeFiles/xDC.dir/protocol/SimensCnc/Simens/libsiemenscnc/src/siemenscnc_tcp_private.c.s

CMakeFiles/xDC.dir/protocol/SimensCnc/Simens/libsiemenscnc/828d/siemens_828d_new.c.o: CMakeFiles/xDC.dir/flags.make
CMakeFiles/xDC.dir/protocol/SimensCnc/Simens/libsiemenscnc/828d/siemens_828d_new.c.o: ../protocol/SimensCnc/Simens/libsiemenscnc/828d/siemens_828d_new.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/fchuanlin/datamodule/Build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Building C object CMakeFiles/xDC.dir/protocol/SimensCnc/Simens/libsiemenscnc/828d/siemens_828d_new.c.o"
	gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/xDC.dir/protocol/SimensCnc/Simens/libsiemenscnc/828d/siemens_828d_new.c.o   -c /home/fchuanlin/datamodule/protocol/SimensCnc/Simens/libsiemenscnc/828d/siemens_828d_new.c

CMakeFiles/xDC.dir/protocol/SimensCnc/Simens/libsiemenscnc/828d/siemens_828d_new.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/xDC.dir/protocol/SimensCnc/Simens/libsiemenscnc/828d/siemens_828d_new.c.i"
	gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/fchuanlin/datamodule/protocol/SimensCnc/Simens/libsiemenscnc/828d/siemens_828d_new.c > CMakeFiles/xDC.dir/protocol/SimensCnc/Simens/libsiemenscnc/828d/siemens_828d_new.c.i

CMakeFiles/xDC.dir/protocol/SimensCnc/Simens/libsiemenscnc/828d/siemens_828d_new.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/xDC.dir/protocol/SimensCnc/Simens/libsiemenscnc/828d/siemens_828d_new.c.s"
	gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/fchuanlin/datamodule/protocol/SimensCnc/Simens/libsiemenscnc/828d/siemens_828d_new.c -o CMakeFiles/xDC.dir/protocol/SimensCnc/Simens/libsiemenscnc/828d/siemens_828d_new.c.s

CMakeFiles/xDC.dir/product/src/datacollecter.cpp.o: CMakeFiles/xDC.dir/flags.make
CMakeFiles/xDC.dir/product/src/datacollecter.cpp.o: ../product/src/datacollecter.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/fchuanlin/datamodule/Build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_8) "Building CXX object CMakeFiles/xDC.dir/product/src/datacollecter.cpp.o"
	/usr/lib64/ccache/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/xDC.dir/product/src/datacollecter.cpp.o -c /home/fchuanlin/datamodule/product/src/datacollecter.cpp

CMakeFiles/xDC.dir/product/src/datacollecter.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/xDC.dir/product/src/datacollecter.cpp.i"
	/usr/lib64/ccache/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/fchuanlin/datamodule/product/src/datacollecter.cpp > CMakeFiles/xDC.dir/product/src/datacollecter.cpp.i

CMakeFiles/xDC.dir/product/src/datacollecter.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/xDC.dir/product/src/datacollecter.cpp.s"
	/usr/lib64/ccache/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/fchuanlin/datamodule/product/src/datacollecter.cpp -o CMakeFiles/xDC.dir/product/src/datacollecter.cpp.s

CMakeFiles/xDC.dir/feature/easyloggingpp/src/easylogging++.cc.o: CMakeFiles/xDC.dir/flags.make
CMakeFiles/xDC.dir/feature/easyloggingpp/src/easylogging++.cc.o: ../feature/easyloggingpp/src/easylogging++.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/fchuanlin/datamodule/Build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_9) "Building CXX object CMakeFiles/xDC.dir/feature/easyloggingpp/src/easylogging++.cc.o"
	/usr/lib64/ccache/g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/xDC.dir/feature/easyloggingpp/src/easylogging++.cc.o -c /home/fchuanlin/datamodule/feature/easyloggingpp/src/easylogging++.cc

CMakeFiles/xDC.dir/feature/easyloggingpp/src/easylogging++.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/xDC.dir/feature/easyloggingpp/src/easylogging++.cc.i"
	/usr/lib64/ccache/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/fchuanlin/datamodule/feature/easyloggingpp/src/easylogging++.cc > CMakeFiles/xDC.dir/feature/easyloggingpp/src/easylogging++.cc.i

CMakeFiles/xDC.dir/feature/easyloggingpp/src/easylogging++.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/xDC.dir/feature/easyloggingpp/src/easylogging++.cc.s"
	/usr/lib64/ccache/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/fchuanlin/datamodule/feature/easyloggingpp/src/easylogging++.cc -o CMakeFiles/xDC.dir/feature/easyloggingpp/src/easylogging++.cc.s

# Object files for target xDC
xDC_OBJECTS = \
"CMakeFiles/xDC.dir/main.cpp.o" \
"CMakeFiles/xDC.dir/common/typeany/src/typeany.cpp.o" \
"CMakeFiles/xDC.dir/protocol/SimensCnc/Simens/libsiemenscnc/src/siemenscnc.c.o" \
"CMakeFiles/xDC.dir/protocol/SimensCnc/Simens/libsiemenscnc/src/siemenscnc_data.c.o" \
"CMakeFiles/xDC.dir/protocol/SimensCnc/Simens/libsiemenscnc/src/siemenscnc_private.c.o" \
"CMakeFiles/xDC.dir/protocol/SimensCnc/Simens/libsiemenscnc/src/siemenscnc_tcp_private.c.o" \
"CMakeFiles/xDC.dir/protocol/SimensCnc/Simens/libsiemenscnc/828d/siemens_828d_new.c.o" \
"CMakeFiles/xDC.dir/product/src/datacollecter.cpp.o" \
"CMakeFiles/xDC.dir/feature/easyloggingpp/src/easylogging++.cc.o"

# External object files for target xDC
xDC_EXTERNAL_OBJECTS =

bin/xDC: CMakeFiles/xDC.dir/main.cpp.o
bin/xDC: CMakeFiles/xDC.dir/common/typeany/src/typeany.cpp.o
bin/xDC: CMakeFiles/xDC.dir/protocol/SimensCnc/Simens/libsiemenscnc/src/siemenscnc.c.o
bin/xDC: CMakeFiles/xDC.dir/protocol/SimensCnc/Simens/libsiemenscnc/src/siemenscnc_data.c.o
bin/xDC: CMakeFiles/xDC.dir/protocol/SimensCnc/Simens/libsiemenscnc/src/siemenscnc_private.c.o
bin/xDC: CMakeFiles/xDC.dir/protocol/SimensCnc/Simens/libsiemenscnc/src/siemenscnc_tcp_private.c.o
bin/xDC: CMakeFiles/xDC.dir/protocol/SimensCnc/Simens/libsiemenscnc/828d/siemens_828d_new.c.o
bin/xDC: CMakeFiles/xDC.dir/product/src/datacollecter.cpp.o
bin/xDC: CMakeFiles/xDC.dir/feature/easyloggingpp/src/easylogging++.cc.o
bin/xDC: CMakeFiles/xDC.dir/build.make
bin/xDC: ../thirdparty/restclient/lib/librestclient-cpp.a
bin/xDC: ../protocol/SimensCnc/Simens/libsiemenscnc/build/lib/libsiemenscnc.a
bin/xDC: CMakeFiles/xDC.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/fchuanlin/datamodule/Build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_10) "Linking CXX executable bin/xDC"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/xDC.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/xDC.dir/build: bin/xDC

.PHONY : CMakeFiles/xDC.dir/build

CMakeFiles/xDC.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/xDC.dir/cmake_clean.cmake
.PHONY : CMakeFiles/xDC.dir/clean

CMakeFiles/xDC.dir/depend:
	cd /home/fchuanlin/datamodule/Build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/fchuanlin/datamodule /home/fchuanlin/datamodule /home/fchuanlin/datamodule/Build /home/fchuanlin/datamodule/Build /home/fchuanlin/datamodule/Build/CMakeFiles/xDC.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/xDC.dir/depend

