# source files
file(GLOB FRAMEWORK_SRC_LIST
  ${CMAKE_CURRENT_SOURCE_DIR}/*/*.c
  ${CMAKE_CURRENT_SOURCE_DIR}/*/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/*/*.h)

assign_folder_group(${FRAMEWORK_SRC_LIST})

# target
add_library(Framework STATIC ${FRAMEWORK_SRC_LIST})

#get_property(rabbitmq_interface TARGET "${RABBITMQ_LIBRARIES}" PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
#include_directories(BEFORE ${rabbitmq_interface})
message("${CMAKE_CURRENT_SOURCE_DIR}")

target_compile_options(Framework PRIVATE ${COMPILE_OPTIONS_DEFAULT})
target_link_libraries(Framework PRIVATE ${OPENSSL_LIBRARIES} ${3RDPART_LIBRARIES} ${MYSQL_LIBRARIES} ${RABBITMQ_LIBRARIES})
target_include_directories(Framework PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

update_cached_list(FRAMEWORK_LIBRARY Framework)