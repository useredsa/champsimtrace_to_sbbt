add_library(champsim_trace_reader SHARED champsim/tracereader.cpp)
target_include_directories(champsim_trace_reader PUBLIC ../include)
set_target_properties(champsim_trace_reader PROPERTIES
  INTERPROCEDURAL_OPTIMIZATION TRUE)
target_compile_features(champsim_trace_reader PUBLIC cxx_std_17)
target_compile_options(champsim_trace_reader PRIVATE "-Wall" "-O3")
