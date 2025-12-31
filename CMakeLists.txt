cmake_minimum_required(VERSION 3.10)

# set the project name and version
project(Server VERSION 1.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(UNIX)
  # UNIX/Linux/Darwin
  set(COMPILE_OPTIONS_DEFAULT "-fPIC" "-Wall;-Wextra")
elseif(WIN32)
  if (MSVC)
    set(COMPILE_OPTIONS_DEFAULT "/EHsc")
  endif()
endif()

function(assign_folder_group)
    foreach(_source IN ITEMS ${ARGN})
        if (IS_ABSOLUTE "${_source}")
          file(RELATIVE_PATH _source_rel "${CMAKE_CURRENT_SOURCE_DIR}" "${_source}")
        else()
          set(_source_rel "${_source}")
        endif()
        get_filename_component(_source_path "${_source_rel}" PATH)
        string(REPLACE "/" "\\" _source_path_msvc "${_source_path}")
        if("${_source_path_msvc}" STREQUAL "." OR "${_source_path_msvc}" STREQUAL "")
          get_filename_component(extension "${_source}" EXT)
          if("${extension}" STREQUAL ".h")
            set(_source_path_msvc "include")
          else()
            set(_source_path_msvc "source")
          endif()
        endif()
        source_group("${_source_path_msvc}" FILES "${_source}")
    endforeach()
endfunction(assign_folder_group)

function(assign_source_group)
    foreach(_source IN ITEMS ${ARGN})
      get_filename_component(extension "${_source}" EXT)
      if("${extension}" STREQUAL ".h")
        set(_source_path_msvc "include")
      else()
        set(_source_path_msvc "source")
      endif()
      source_group("${_source_path_msvc}" FILES "${_source}")
    endforeach()
endfunction(assign_source_group)

# 全局修改变量
function(update_cached name value)
  set("${name}" "${value}" CACHE INTERNAL "*** Internal ***" FORCE)
endfunction()

# 往全局列表中添加元素
function(update_cached_list name)
  set(_tmp_list "${${name}}")
  list(APPEND _tmp_list "${ARGN}")
  list(REMOVE_DUPLICATES _tmp_list)
  update_cached(${name} "${_tmp_list}")
endfunction()

#首先清空缓存
# 3rdpart libraries
update_cached(3RDPART_LIBRARIES "")

# mysql libraries
update_cached(MYSQL_LIBRARIES "")

# rabbitmq libraries
update_cached(RABBITMQ_LIBRARIES "")

# Framework library
update_cached(FRAMEWORK_LIBRARY "")

# Mahjong library
update_cached(MAHJONG_LIBRARY "")

# Poker library
update_cached(POKER_LIBRARY "")

# add 3rdpart to this project
add_subdirectory(3rdpart)

find_package(Boost REQUIRED COMPONENTS log log_setup filesystem system)
if(Boost_FOUND)
  message("${Boost_INCLUDE_DIRS}")
  message("${Boost_LIBRARIES}")
  message("${Boost_LIBRARY_DIRS}")
endif()
include_directories(${Boost_INCLUDE_DIRS})

# 查找 openssl 是否安装
find_package(OpenSSL REQUIRED)
if(OpenSSL_FOUND)
  message("found OpenSSL library")
  message("${OPENSSL_INCLUDE_DIR}")
  message("${OPENSSL_LIBRARIES}")
endif()
#include_directories(${OPENSSL_INCLUDE_DIR})

# 查找 mysql 是否安装
find_package(mysql-concpp REQUIRED)
if(mysql-concpp_FOUND)
  message("found mysql-concpp library")
  message("${MYSQL_CONCPP_INCLUDE_DIR}")
endif()
#include_directories(SYSTEM ${MYSQL_CONCPP_INCLUDE_DIR})
if(WIN32)
update_cached_list(MYSQL_LIBRARIES mysql::concpp-jdbc-static)
else()
update_cached_list(MYSQL_LIBRARIES mysql::concpp-jdbc)
endif()

# 查找 rabbitmq-c 是否安装
find_package(rabbitmq-c REQUIRED)
if(rabbitmq-c_FOUND)
  message("found rabbitmq-c library")
endif()
update_cached_list(RABBITMQ_LIBRARIES rabbitmq::rabbitmq-static)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rdpart)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rdpart/msgpack)

# add Framework to this project
add_subdirectory(Framework)

# add Mahjong to this project
add_subdirectory(Mahjong)

# add Poker to this project
add_subdirectory(Poker)

# add Poker to this project
add_subdirectory(Server)