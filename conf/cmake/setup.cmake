option(CMAKE_AUTOUIC "Handle UIC automatically for Qt targets" ON)
option(CMAKE_AUTOMOC "Handle MOC automatically for Qt targets" ON)
option(CMAKE_AUTORCC "Handle RCC automatically for Qt targets" ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)

include(FetchContent)
find_package(areg CONFIG)
if (NOT areg_FOUND)
    # ##################################################################
    # Areg SDK not found as a package, fetching from GitHub.
    # ##################################################################

    # Location of fetched third-party sources (including Areg SDK).
    if (NOT DEFINED AREG_DEPS_DIR OR "${AREG_DEPS_DIR}" STREQUAL "")
        set(AREG_DEPS_DIR "${CMAKE_BINARY_DIR}/packages")
    endif()
    set(FETCHCONTENT_BASE_DIR "${AREG_DEPS_DIR}")
    
    # The root directory for Areg SDK build outputs.
    set(AREG_BUILD_DIR  "${CMAKE_BINARY_DIR}")
    set(AREG_DEPS_DIR   "${CMAKE_BINARY_DIR}/packages")
    # Build Areg shared library.
    set(AREG_LIB_TYPE         shared)
    set(AREG_LOGGER_LIB_TYPE  shared)
    # Disable building Areg SDK examples, unit tests and build structures.
    option(AREG_TESTS           "Build areg-sdk tests"     OFF)
    option(AREG_EXAMPLES        "Build areg-sdk examples"  OFF)
    option(AREG_SYSTEM_GTEST    "Build GTest"              OFF)
    option(AREG_OUTPUT_LAYOUT   "Areg build structure"     OFF)
    option(AREG_EXTENDED        "Escape building extended" OFF)

    FetchContent_Declare(
        areg
        GIT_REPOSITORY https://github.com/aregtech/areg-sdk.git
        GIT_TAG "master"
    )
    FetchContent_MakeAvailable(areg)

    # Set the root directory of the fetched Areg SDK
    set(AREG_SDK_ROOT         "${areg_SOURCE_DIR}")
    set(AREG_CMAKE_CONFIG_DIR "${AREG_SDK_ROOT}/conf/cmake")
    set(AREG_CMAKE            "${AREG_SDK_ROOT}/areg.cmake")
    message(STATUS ">>> Fetched Areg SDK from GitHub to ${FETCHCONTENT_BASE_DIR}")
    message(STATUS ">>> Location of 'areg.cmake' ${AREG_CMAKE}")

else()
    # Areg SDK package found
    message(STATUS ">>> Found Areg package at '${areg_DIR}',")
    message(STATUS ">>> Libs: '${areg_LIBRARY}', Configs: '${areg_CONFIG}', Package Root: '${areg_ROOT}'")
    message(STATUS ">>> Tools: '${AREG_SDK_TOOLS}', 'areg.cmake': ${AREG_CMAKE}")
endif()

include(${AREG_CMAKE})
set(AREG_RESOURCES "${AREG_FRAMEWORK}/areg/resources")

# ##################################################################
# Qt Advanced Docking System (ADS): dockable Outline/Properties/Toolbar
# that drag/float/tab across the Navigation Window and Design page
# (issue #516). Fetched from GitHub and built as a static library so no
# extra runtime DLL has to be deployed alongside lusan.
# ##################################################################
set(ADS_VERSION       "4.4.1")
set(BUILD_STATIC      ON  CACHE BOOL "Build ADS as a static library" FORCE)
set(BUILD_EXAMPLES    OFF CACHE BOOL "Do not build ADS examples"     FORCE)
set(ADS_BUILD_STATIC  ON  CACHE BOOL "Build ADS as a static library" FORCE)
FetchContent_Declare(
    ads
    GIT_REPOSITORY https://github.com/githubuser0xFFFF/Qt-Advanced-Docking-System.git
    GIT_TAG "4.4.1"
)
FetchContent_MakeAvailable(ads)
set(LUSAN_ADS_TARGET "qtadvanceddocking-qt${QT_VERSION_MAJOR}")
message(STATUS ">>> Fetched Qt-Advanced-Docking-System ${ADS_VERSION}; target '${LUSAN_ADS_TARGET}'")

message(STATUS "-------------------- CMakeLists Status Report Begin --------------------")
message(STATUS "LUSAN: >>> Qt Version = \'${QT_VERSION_MAJOR}\'")
message(STATUS "LUSAN: >>> CMAKE_PREFIX_PATH = ${CMAKE_PREFIX_PATH}")
message(STATUS "-------------------- CMakeLists Status Report End ----------------------")
message(STATUS CMAKE_SOURCE_DIR = ${CMAKE_SOURCE_DIR})
