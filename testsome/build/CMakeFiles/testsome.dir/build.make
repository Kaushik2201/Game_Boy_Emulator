# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
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
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/kishora/IE_CODE_PROJECT/testsome

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/kishora/IE_CODE_PROJECT/testsome/build

# Include any dependencies generated for this target.
include CMakeFiles/testsome.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/testsome.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/testsome.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/testsome.dir/flags.make

CMakeFiles/testsome.dir/main.cpp.o: CMakeFiles/testsome.dir/flags.make
CMakeFiles/testsome.dir/main.cpp.o: ../main.cpp
CMakeFiles/testsome.dir/main.cpp.o: CMakeFiles/testsome.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/kishora/IE_CODE_PROJECT/testsome/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/testsome.dir/main.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/testsome.dir/main.cpp.o -MF CMakeFiles/testsome.dir/main.cpp.o.d -o CMakeFiles/testsome.dir/main.cpp.o -c /home/kishora/IE_CODE_PROJECT/testsome/main.cpp

CMakeFiles/testsome.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/testsome.dir/main.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/kishora/IE_CODE_PROJECT/testsome/main.cpp > CMakeFiles/testsome.dir/main.cpp.i

CMakeFiles/testsome.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/testsome.dir/main.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/kishora/IE_CODE_PROJECT/testsome/main.cpp -o CMakeFiles/testsome.dir/main.cpp.s

# Object files for target testsome
testsome_OBJECTS = \
"CMakeFiles/testsome.dir/main.cpp.o"

# External object files for target testsome
testsome_EXTERNAL_OBJECTS =

testsome: CMakeFiles/testsome.dir/main.cpp.o
testsome: CMakeFiles/testsome.dir/build.make
testsome: CMakeFiles/testsome.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/kishora/IE_CODE_PROJECT/testsome/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable testsome"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/testsome.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/testsome.dir/build: testsome
.PHONY : CMakeFiles/testsome.dir/build

CMakeFiles/testsome.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/testsome.dir/cmake_clean.cmake
.PHONY : CMakeFiles/testsome.dir/clean

CMakeFiles/testsome.dir/depend:
	cd /home/kishora/IE_CODE_PROJECT/testsome/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/kishora/IE_CODE_PROJECT/testsome /home/kishora/IE_CODE_PROJECT/testsome /home/kishora/IE_CODE_PROJECT/testsome/build /home/kishora/IE_CODE_PROJECT/testsome/build /home/kishora/IE_CODE_PROJECT/testsome/build/CMakeFiles/testsome.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/testsome.dir/depend

