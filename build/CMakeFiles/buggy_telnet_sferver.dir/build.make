# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

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
CMAKE_SOURCE_DIR = /root/proj/ecomm/libs/buggy_telnet

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /root/proj/ecomm/libs/buggy_telnet/build

# Include any dependencies generated for this target.
include CMakeFiles/buggy_telnet_sferver.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/buggy_telnet_sferver.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/buggy_telnet_sferver.dir/flags.make

CMakeFiles/buggy_telnet_sferver.dir/server.cpp.o: CMakeFiles/buggy_telnet_sferver.dir/flags.make
CMakeFiles/buggy_telnet_sferver.dir/server.cpp.o: ../server.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/root/proj/ecomm/libs/buggy_telnet/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/buggy_telnet_sferver.dir/server.cpp.o"
	/bin/clang++-11  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/buggy_telnet_sferver.dir/server.cpp.o -c /root/proj/ecomm/libs/buggy_telnet/server.cpp

CMakeFiles/buggy_telnet_sferver.dir/server.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/buggy_telnet_sferver.dir/server.cpp.i"
	/bin/clang++-11 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /root/proj/ecomm/libs/buggy_telnet/server.cpp > CMakeFiles/buggy_telnet_sferver.dir/server.cpp.i

CMakeFiles/buggy_telnet_sferver.dir/server.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/buggy_telnet_sferver.dir/server.cpp.s"
	/bin/clang++-11 $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /root/proj/ecomm/libs/buggy_telnet/server.cpp -o CMakeFiles/buggy_telnet_sferver.dir/server.cpp.s

# Object files for target buggy_telnet_sferver
buggy_telnet_sferver_OBJECTS = \
"CMakeFiles/buggy_telnet_sferver.dir/server.cpp.o"

# External object files for target buggy_telnet_sferver
buggy_telnet_sferver_EXTERNAL_OBJECTS =

buggy_telnet_sferver: CMakeFiles/buggy_telnet_sferver.dir/server.cpp.o
buggy_telnet_sferver: CMakeFiles/buggy_telnet_sferver.dir/build.make
buggy_telnet_sferver: CMakeFiles/buggy_telnet_sferver.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/root/proj/ecomm/libs/buggy_telnet/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable buggy_telnet_sferver"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/buggy_telnet_sferver.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/buggy_telnet_sferver.dir/build: buggy_telnet_sferver

.PHONY : CMakeFiles/buggy_telnet_sferver.dir/build

CMakeFiles/buggy_telnet_sferver.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/buggy_telnet_sferver.dir/cmake_clean.cmake
.PHONY : CMakeFiles/buggy_telnet_sferver.dir/clean

CMakeFiles/buggy_telnet_sferver.dir/depend:
	cd /root/proj/ecomm/libs/buggy_telnet/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /root/proj/ecomm/libs/buggy_telnet /root/proj/ecomm/libs/buggy_telnet /root/proj/ecomm/libs/buggy_telnet/build /root/proj/ecomm/libs/buggy_telnet/build /root/proj/ecomm/libs/buggy_telnet/build/CMakeFiles/buggy_telnet_sferver.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/buggy_telnet_sferver.dir/depend

