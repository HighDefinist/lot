cmake_minimum_required (VERSION 3.1)
project("mz_lot")

if(MSVC)
   string(REGEX REPLACE " /W[0-4]" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /permissive- /w14242 /w14254 /w14263 /w14265 /w14287 /we4289 /w14296 /w14311 /w14545 /w14546 /w14547 /w14549 /w14555 /w14619 /w14640 /w14826 /w14905 /w14906 /w14928")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wshadow -Wnon-virtual-dtor -Wpedantic -Wold-style-cast -Wcast-align -Woverloaded-virtual -Wconversion -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wnull-dereference -Wuseless-cast -Wdouble-promotion -Wformat=2")
endif()

# Setup as header-only library for parent cmake project
add_library(${PROJECT_NAME} INTERFACE)
set (CMAKE_CXX_STANDARD 11)
target_include_directories(${PROJECT_NAME} INTERFACE ${PROJECT_SOURCE_DIR}/include)

# Only include tests and example, if there is no parent cmake project
get_directory_property(hasParent PARENT_DIRECTORY)
if(NOT hasParent)
  # Include examples and tests
  include_directories(${PROJECT_SOURCE_DIR}/include)
  include_directories(${PROJECT_SOURCE_DIR}/ext)

  add_subdirectory(example)
  add_subdirectory(test)
endif()
 
 
