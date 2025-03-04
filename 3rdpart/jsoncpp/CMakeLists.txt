# source files
file(GLOB JSONCPP_SRC_LIST
  ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/json/*.h)

assign_source_group(${JSONCPP_SRC_LIST})

# target
add_library(jsoncpp STATIC ${JSONCPP_SRC_LIST})
#add_library(NiuMa::jsoncpp ALIAS jsoncpp)

target_compile_options(jsoncpp PRIVATE ${COMPILE_OPTIONS_DEFAULT})
target_include_directories(jsoncpp PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>")

update_cached_list(3RDPART_LIBRARIES jsoncpp)