add_executable(champsimtrace_to_sbbt champsimtrace_to_sbbt.cpp)
target_link_libraries(champsimtrace_to_sbbt
  PRIVATE champsim_trace_reader mbp_sbbt_writer)
set_target_properties(champsimtrace_to_sbbt PROPERTIES
  CXX_STANDARD 17
  CXX_EXTENSIONS OFF
  INTERPROCEDURAL_OPTIMIZATION TRUE)
target_compile_options(champsimtrace_to_sbbt PRIVATE "-Wall" "-O3")
