cmake_minimum_required(VERSION 3.20)

set(PROJECT_NAME    lusan)
set(PROJECT_VERSION_MAJOR 1)
set(PROJECT_VERSION_MINOR 0)
set(PROJECT_VERSION_PATCH 0)
set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
set(APP_NAME lusan)

project("${PROJECT_NAME}" 
        VERSION "${PROJECT_VERSION}" 
        DESCRIPTION "The user interface tools for AREG communication framework" 
        HOMEPAGE_URL "https://aregtech.com" 
        LANGUAGES CXX
)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(LUSAN_ROOT          "${CMAKE_CURRENT_LIST_DIR}")
set(LUSAN_CMAKE_CONFIG  "${LUSAN_ROOT}/conf/cmake")
set(LUSAN_BASE          "${LUSAN_ROOT}/sources")
set(LUSAN_THIRDPARTY    "${LUSAN_ROOT}/thirdparty")

set(LUSAN_SRC)
set(LUSAN_HDR)
set(LUSAN_RES)
set(LUSAN_UI)
set(LUSAN_TRANS)

include("${LUSAN_CMAKE_CONFIG}/setup.cmake")

if(NOT ${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    message(FATAL_ERROR "LUSAN: >>> You should use QT version 6 or newer to compile this project")
endif()

include_directories(${LUSAN_BASE})
include_directories(${LUSAN_THIRDPARTY})
include("${LUSAN_BASE}/CMakeLists.txt")

list(APPEND PROJECT_SOURCES "${LUSAN_TRANS}" "${LUSAN_RES}" "${LUSAN_UI}" "${LUSAN_HDR}" "${LUSAN_SRC}")

qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${LUSAN_TRANS})
qt_add_executable(${APP_NAME} MANUAL_FINALIZATION ${PROJECT_SOURCES})
target_compile_definitions(${APP_NAME} PRIVATE ${COMMON_COMPILE_DEF} IMP_LOGGER_DLL)
target_link_libraries(${APP_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Widgets areg::areg areg::areglogger)

# copying log and router init files to 'bin/config'
add_custom_command( TARGET ${APP_NAME} POST_BUILD 
                    COMMAND ${CMAKE_COMMAND} -E copy ${LUSAN_BASE}/lusan/res/lusan.init ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/config/lusan.init
                    VERBATIM)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.aregtech.${APP_NAME})
endif()

set_target_properties(${APP_NAME} PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(${APP_NAME})
endif()
