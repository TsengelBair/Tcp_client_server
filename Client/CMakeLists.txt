cmake_minimum_required(VERSION 3.14)

project(Client LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Network Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Network Core)

# для использования протобаф
find_package(Protobuf REQUIRED)

# директория с прото файлами (на директорию ниже от текущего проекта)
set(GENERATED_PB_FILES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/..")

# получаем прото файлы (файлы с расширением pb.cc и pb.h)
file(GLOB_RECURSE PROTO_FILES ${GENERATED_PB_FILES_DIR}/*.pb.cc ${GENERATED_PB_FILES_DIR}/*.pb.h)

add_executable(Client
  main.cpp
  client.h
  client.cpp
  ${PROTO_FILES}
)

# указываем протобаф в линковке
target_link_libraries(Client Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Network ${PROTOBUF_LIBRARIES})

include(GNUInstallDirs)
install(TARGETS Client
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
