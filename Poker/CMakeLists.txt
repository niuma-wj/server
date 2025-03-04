# source files
file(GLOB POKER_SRC_LIST
  ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

assign_folder_group(${POKER_SRC_LIST})

# target
add_library(Poker STATIC ${POKER_SRC_LIST})

target_compile_options(Poker PRIVATE ${COMPILE_OPTIONS_DEFAULT})
target_link_libraries(Poker PRIVATE ${FRAMEWORK_LIBRARY})
target_include_directories(Poker
  PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>")

update_cached_list(POKER_LIBRARY Poker)