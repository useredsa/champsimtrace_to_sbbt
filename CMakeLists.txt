cmake_minimum_required(VERSION 3.10)

project(ChampSimTraceToSbbt
  VERSION 1.0.0
  DESCRIPTION "A translator from Champsim's trace format to SBBT 1.0"
  LANGUAGES CXX
)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
  set(CMAKE_CXX_EXTENSIONS OFF)
endif()

add_subdirectory(src)
add_subdirectory(app)
