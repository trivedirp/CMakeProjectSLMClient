# CMakeList.txt : CMake project for CMakeProjectSLMClient, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

set(CMAKE_TOOLCHAIN_FILE "C:\\Users\\User\\Documents\\Repos\\vcpkg\\scripts\\buildsystems\\vcpkg.cmake")

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

set(CMAKE_PREFIX_PATH "C:\\Users\\User\\Documents\\Repos\\vcpkg\\packages\\cppzmq_x64-windows" ;
"C:\\Users\\User\\Documents\\Repos\\vcpkg\\packages\\zeromq_x64-windows")
find_package(cppzmq CONFIG REQUIRED )

project ("CMakeProjectSLMClient")
find_path(CPPZMQ_INCLUDE_DIR NAMES "zmq.hpp" PATH_SUFFIXES "include" )
find_path(CPPZMQ_LIB_DIR NAMES "libzmq-mt-4_3_5.lib" PATH_SUFFIXES "lib" )
set(BLINK_INCLUDE_DIR "C:\\Users\\User\\Documents\\Repos\\CMakeProjectSLMClient\\include\\")
set(BLINK_LIB_DIR "C:\\Users\\User\\Documents\\Repos\\CMakeProjectSLMClient\\lib\\")

# Add source to this project's executable.
add_executable (CMakeProjectSLMClient "CMakeProjectSLMClient.cpp" "CMakeProjectSLMClient.h")

# additional include directories
target_include_directories(CMakeProjectSLMClient PRIVATE ${CPPZMQ_INCLUDE_DIR};${BLINK_INCLUDE_DIR})
target_link_directories(CMakeProjectSLMClient PRIVATE ${CPPZMQ_LIB_DIR};${BLINK_LIB_DIR})
target_link_libraries(CMakeProjectSLMClient PRIVATE libzmq-mt-4_3_5;Blink_SDK;ImageGen)

add_custom_command(TARGET CMakeProjectSLMClient POST_BUILD       
    COMMAND ${CMAKE_COMMAND} -E copy_directory 
        "${PROJECT_SOURCE_DIR}/dll"   
        $<TARGET_FILE_DIR:CMakeProjectSLMClient>)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET CMakeProjectSLMClient PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
