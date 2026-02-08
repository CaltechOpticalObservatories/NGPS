cmake_minimum_required(VERSION 3.16)
project(ngps_db_gui LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Widgets QUIET)
if (Qt6_FOUND)
  set(QT_LIBS Qt6::Widgets)
else()
  find_package(Qt5 COMPONENTS Widgets REQUIRED)
  set(QT_LIBS Qt5::Widgets)
endif()

set(MYSQL_DIR "/usr/local/mysql/connector")
find_path(MYSQL_API "mysqlx/xdevapi.h"
          PATHS ${MYSQL_DIR} /usr/local/opt/mysql-connector-c++ /opt/homebrew/opt/mysql-connector-c++)
if (NOT MYSQL_API)
  message(FATAL_ERROR "mysqlx/xdevapi.h not found. Install MySQL Connector/C++")
endif()

find_library(MYSQL_LIB NAMES mysqlcppconnx mysqlcppconn8 mysqlcppconn
             PATHS ${MYSQL_DIR}
                   /usr/local/opt/mysql-connector-c++/lib
                   /opt/homebrew/opt/mysql-connector-c++/lib
                   /usr/local/opt/mysql-connector-c++
                   /opt/homebrew/opt/mysql-connector-c++)
if (NOT MYSQL_LIB)
  message(FATAL_ERROR "MySQL Connector/C++ library not found")
endif()

add_executable(ngps_db_gui
  main.cpp
)

target_include_directories(ngps_db_gui PRIVATE ${MYSQL_API})
target_link_libraries(ngps_db_gui PRIVATE ${QT_LIBS} ${MYSQL_LIB})
