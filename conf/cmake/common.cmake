option(CMAKE_AUTOUIC "Handle UIC automatically for Qt targets" ON)
option(CMAKE_AUTOMOC "Handle MOC automatically for Qt targets" ON)
option(CMAKE_AUTORCC "Handle RCC automatically for Qt targets" ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)

message(STATUS "-------------------- CMakeLists Status Report Begin --------------------")
message(STATUS "LUSAN: >>> Qt Version = \'${QT_VERSION_MAJOR}\'")
message(STATUS "LUSAN: >>> CMAKE_PREFIX_PATH = ${CMAKE_PREFIX_PATH}")
message(STATUS "-------------------- CMakeLists Status Report End ----------------------")
message(STATUS CMAKE_SOURCE_DIR = ${CMAKE_SOURCE_DIR})
