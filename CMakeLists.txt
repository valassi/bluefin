CMAKE_MINIMUM_REQUIRED(VERSION 2.8.5)

project(BLUEFIN)

# Sanity checks and debug printouts
message(STATUS "")
message(STATUS "====== WELCOME TO BLUEFIN CMakeLists.txt ======")
string(TIMESTAMP date "%Y-%m-%d %H:%M:%S")
message(STATUS "Date is ${date}")
cmake_host_system_information(RESULT host QUERY HOSTNAME)
message(STATUS "Hostname is ${host}")
message(STATUS "OS is ${CMAKE_HOST_SYSTEM_NAME} ${CMAKE_HOST_SYSTEM_VERSION}")
message(STATUS "CMAKE_COMMAND is ${CMAKE_COMMAND}")
message(STATUS "===============================================")

# Print out the initial values of all other relevant variables.
# Note that the C and C++ compiler paths are usually inferred from CC and CXX.
message(STATUS "CMAKE_BINARY_DIR is ${CMAKE_BINARY_DIR}")
message(STATUS "CMAKE_BUILD_TYPE is ${CMAKE_BUILD_TYPE}")
message(STATUS "CMAKE_C_COMPILER is ${CMAKE_C_COMPILER}")
message(STATUS "CMAKE_CXX_COMPILER is ${CMAKE_CXX_COMPILER}")
message(STATUS "CMAKE_CXX_FLAGS is ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_INSTALL_PREFIX is ${CMAKE_INSTALL_PREFIX}")
message(STATUS "CMAKE_PREFIX_PATH [env] is $ENV{CMAKE_PREFIX_PATH}") # env only
message(STATUS "CMAKE_SOURCE_DIR is ${CMAKE_SOURCE_DIR}")
message(STATUS "CMAKE_VERBOSE_MAKEFILE is ${CMAKE_VERBOSE_MAKEFILE}")
message(STATUS "=============================================")
message(STATUS "")

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

find_package(ROOT REQUIRED MathCore)
include_directories(${ROOT_INCLUDE_DIRS})

add_library(BlueFin SHARED
  src/BlueFish1Obs.cpp
  src/BlueFish.cpp
  src/InfoAnalyzer.cpp
  src/InfoMinimizer.cpp
  src/InputParser.cpp
  src/TextReporter.cpp
)
target_include_directories(BlueFin PRIVATE .)

add_executable(bluefin
  ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/BlueFinLogo.dir/utilities/BlueFinLogo.jpg.o
  ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/BlueFinReport.dir/utilities/BlueFinReport.tex.o
  utilities/bluefin.cpp
)
target_include_directories(bluefin PRIVATE .)
target_link_libraries(bluefin BlueFin ${ROOT_LIBRARIES})

add_custom_command(OUTPUT CMakeFiles/BlueFinLogo.dir/utilities/BlueFinLogo.jpg.o
  COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR}/utilities && objcopy -Bi386 --input binary --output elf64-x86-64 BlueFinLogo.jpg ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/BlueFinLogo.dir/utilities/BlueFinLogo.jpg.o
)

add_custom_command(OUTPUT CMakeFiles/BlueFinReport.dir/utilities/BlueFinReport.tex.o
  COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR}/utilities && objcopy -Bi386 --input binary --output elf64-x86-64 BlueFinReport.tex ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/BlueFinReport.dir/utilities/BlueFinReport.tex.o
)
