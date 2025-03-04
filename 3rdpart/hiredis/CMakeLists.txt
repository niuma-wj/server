# source files
file(GLOB HIREDIS_SRC_LIST
  ${CMAKE_CURRENT_SOURCE_DIR}/*/*.c
  ${CMAKE_CURRENT_SOURCE_DIR}/*/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/*/*.h)

assign_folder_group(${HIREDIS_SRC_LIST})

# target
add_library(hiredis STATIC ${HIREDIS_SRC_LIST})

include_directories(BEFORE ${OPENSSL_INCLUDE_DIR})

target_compile_options(hiredis PRIVATE ${COMPILE_OPTIONS_DEFAULT})
if(WIN32)
  target_compile_definitions(hiredis PRIVATE "WIN32_LEAN_AND_MEAN")
endif()
target_include_directories(hiredis PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>")

update_cached_list(3RDPART_LIBRARIES hiredis)