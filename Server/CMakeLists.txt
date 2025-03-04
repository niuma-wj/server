
file(GLOB SERVER_SRC_LIST
  ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/*.h
  ${CMAKE_CURRENT_SOURCE_DIR}/*/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/*/*.h)

assign_folder_group(${SERVER_SRC_LIST})

add_executable(Server ${SERVER_SRC_LIST})

target_compile_options(Server PRIVATE ${COMPILE_OPTIONS_DEFAULT})
target_link_libraries(Server PRIVATE ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES} ${3RDPART_LIBRARIES} ${MYSQL_LIBRARIES} ${RABBITMQ_LIBRARIES} ${FRAMEWORK_LIBRARY} ${MAHJONG_LIBRARY} ${POKER_LIBRARY})
if(WIN32)
  target_link_directories(Server PRIVATE "${BOOST_ROOT}/lib")
else()
  target_link_directories(Server PRIVATE "${Boost_INCLUDE_DIRS}/../lib")
endif()