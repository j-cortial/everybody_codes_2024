cmake_minimum_required(VERSION 3.20)

project(everybodycodes_2024 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(base INTERFACE)
target_compile_options(base INTERFACE -Wall -Wextra -Wpedantic -Wnon-virtual-dtor)
if(ENABLE_ADDRESS_SANITIZER)
  target_compile_options(base INTERFACE -fsanitize=address -fno-omit-frame-pointer)
  target_link_options(base INTERFACE -fsanitize=address)
endif()

set(quests quest01)

foreach(quest ${quests})
  add_executable(${quest} ${quest}/main.cpp)
  set_property(TARGET ${quest} PROPERTY OUTPUT_NAME main)
  set_property(TARGET ${quest} PROPERTY RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${quest})
  target_link_libraries(${quest} base)

  file(GLOB inputs RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/${quest}/input?.txt)
  foreach(input ${inputs})
    configure_file(${input} ${input} COPYONLY)
  endforeach()
endforeach()
