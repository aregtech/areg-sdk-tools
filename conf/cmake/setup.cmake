option(CMAKE_AUTOUIC "Handle UIC automatically for Qt targets" ON)
option(CMAKE_AUTOMOC "Handle MOC automatically for Qt targets" ON)
option(CMAKE_AUTORCC "Handle RCC automatically for Qt targets" ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)

set(AREG_BINARY         shared)
set(AREG_LOGGER_BINARY  shared)
option(AREG_BUILD_TESTS     "Escape building unit tests"    OFF)
option(AREG_BUILD_EXAMPLES  "Escape building examples"      OFF)
option(AREG_EXTENDED        "Escape building extended"      OFF)
option(AREG_GTEST_PACKAGE   "Escape GTest"                  OFF)
option(AREG_ENABLE_OUTPUTS  "Ignore AREG build structure"   OFF)

find_package(areg CONFIG)
if (NOT areg_FOUND)
    # ##################################################################
    # AREG SDK not found as a package, fetching from GitHub.
    # ##################################################################

    # Specify where to fetch third-party sources (including AREG SDK).
    if (NOT DEFINED AREG_PACKAGES OR "${AREG_PACKAGES}" STREQUAL "")
        set(AREG_PACKAGES "${CMAKE_BINARY_DIR}/packages")
    endif()

    include(FetchContent)
    set(FETCHCONTENT_BASE_DIR "${AREG_PACKAGES}")
    message(STATUS "LUSAN: >>> Fetching AREG SDK from GitHub to ${FETCHCONTENT_BASE_DIR}")
        
    FetchContent_Declare(
        areg-sdk
        GIT_REPOSITORY https://github.com/aregtech/areg-sdk.git
        GIT_TAG "bugfix/530-wrong-log-database-file-name"
        # GIT_TAG "master"
    )
    message(STATUS "LUSAN: >>> AREG SDK sources are fetched, setting up areg-sdk ...")
    FetchContent_MakeAvailable(areg-sdk)

    # Set the root directory of the fetched AREG SDK
    set(AREG_SDK_ROOT "${areg-sdk_SOURCE_DIR}")
    set(AREG_CMAKE_CONFIG_DIR "${AREG_SDK_ROOT}/conf/cmake")

endif()

include(${AREG_SDK_ROOT}/areg.cmake)

message(STATUS "-------------------- CMakeLists Status Report Begin --------------------")
message(STATUS "LUSAN: >>> Qt Version = \'${QT_VERSION_MAJOR}\'")
message(STATUS "LUSAN: >>> CMAKE_PREFIX_PATH = ${CMAKE_PREFIX_PATH}")
message(STATUS "-------------------- CMakeLists Status Report End ----------------------")
message(STATUS CMAKE_SOURCE_DIR = ${CMAKE_SOURCE_DIR})
