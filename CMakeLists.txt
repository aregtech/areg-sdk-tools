cmake_minimum_required(VERSION 3.21.1)

set(PROJECT_NAME    lusan)
set(PROJECT_VERSION_MAJOR 1)
set(PROJECT_VERSION_MINOR 0)
set(PROJECT_VERSION_PATCH 0)
set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
set(APP_NAME lusan)

project("${PROJECT_NAME}"
        VERSION "${PROJECT_VERSION}"
        DESCRIPTION "The user interface tools for Areg communication framework"
        HOMEPAGE_URL "https://areg.tech"
        LANGUAGES CXX
)

# The application assembly, shared with any project that builds a Lusan application from these
# sources. Included unconditionally: CMake functions are global, so a consumer gets them by
# adding this directory.
include("${CMAKE_CURRENT_LIST_DIR}/conf/cmake/application.cmake")

# Fetched or added as a subdirectory, this file defines the functions above and builds nothing.
# The consumer creates its own application target instead.
if (NOT PROJECT_IS_TOP_LEVEL)
    return()
endif()

lusan_setup_dependencies()

include("${LUSAN_BASE}/CMakeLists.txt")

lusan_add_application(${APP_NAME})

message(STATUS "-------------------- CMakeLists Status Report Begin --------------------")
message(STATUS "LUSAN: >>> Edition = \'${LUSAN_EDITION}\', version = \'${PROJECT_VERSION}\'")
message(STATUS "LUSAN: >>> Sources = \'${LUSAN_BASE}\'")
message(STATUS "-------------------- CMakeLists Status Report End ----------------------")

# Opt-in unit tests (additive; the default application build is unaffected).
option(LUSAN_BUILD_TESTS "Build Lusan unit tests" OFF)
if(LUSAN_BUILD_TESTS)
    include("${LUSAN_ROOT}/tests/CMakeLists.txt")
endif()
