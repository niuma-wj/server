# source files
file(GLOB MAHJONG_SRC_LIST
  ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

assign_folder_group(${MAHJONG_SRC_LIST})

# target
add_library(Mahjong STATIC ${MAHJONG_SRC_LIST})

target_compile_options(Mahjong PRIVATE ${COMPILE_OPTIONS_DEFAULT})
target_link_libraries(Mahjong PRIVATE ${FRAMEWORK_LIBRARY})
target_include_directories(Mahjong
  PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>")

update_cached_list(MAHJONG_LIBRARY Mahjong)