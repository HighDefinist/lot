cmake_minimum_required (VERSION 3.1)
project("mz_lot")

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
 
 
