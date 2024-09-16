cmake_minimum_required(VERSION 3.16)

set(PROJECT_NAME    lusan)
set(PROJECT_VERSION "1.0.0")
set(PROJECT_VERSION_MAJOR 0)
set(PROJECT_VERSION_MINOR 0)
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
set(LUSAN_RES)
set(LUSAN_UI)
set(LUSAN_TRANS)

include("${LUSAN_CMAKE_CONFIG}/common.cmake")

include_directories(${LUSAN_BASE})
include_directories(${LUSAN_THIRDPARTY})
include("${LUSAN_BASE}/CMakeLists.txt")

list(APPEND PROJECT_SOURCES "${LUSAN_SRC}" "${LUSAN_UI}" "${LUSAN_RES}" "${LUSAN_TRANS}")

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(${APP_NAME} MANUAL_FINALIZATION ${PROJECT_SOURCES})
    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${LUSAN_TRANS})
else()
    add_executable(${APP_NAME} ${PROJECT_SOURCES})
    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${LUSAN_TRANS})
endif()

target_link_libraries(${APP_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.${APP_NAME})
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
