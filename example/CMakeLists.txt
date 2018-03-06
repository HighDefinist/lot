cmake_minimum_required (VERSION 3.1)

set (EXAMPLES_C11 
)

foreach (example ${EXAMPLES_C11})
  add_executable(${example} ${example}/${example}.cpp)
  target_compile_features(${example} INTERFACE cxx_std_11)
endforeach()
