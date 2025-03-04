# source files
file(GLOB ZLIB_SRC_LIST
  ${CMAKE_CURRENT_SOURCE_DIR}/*/*.c
  ${CMAKE_CURRENT_SOURCE_DIR}/*/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/*/*.h)

assign_folder_group(${ZLIB_SRC_LIST})

# target
add_library(zlib STATIC ${ZLIB_SRC_LIST})

target_compile_options(zlib PRIVATE ${COMPILE_OPTIONS_DEFAULT})
target_include_directories(zlib PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>")

update_cached_list(3RDPART_LIBRARIES zlib)