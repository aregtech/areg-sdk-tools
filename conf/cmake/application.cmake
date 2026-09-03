# ###########################################################################
# Lusan application assembly.
#
# Everything needed to turn the Lusan sources into a running application lives here, so that
# this repository and any project that builds an application from these sources share one
# implementation of it.
#
# Usage:
#   include("<lusan root>/conf/cmake/application.cmake")
#   lusan_setup_dependencies()
#   include("${LUSAN_BASE}/CMakeLists.txt")      # fills LUSAN_SRC, LUSAN_HDR, ...
#   lusan_add_application(myapp SOURCES ... )
# ###########################################################################

# The root of the Lusan sources, found from the location of this file.
get_filename_component(LUSAN_ROOT "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)

# ###########################################################################
# macro ......: lusan_setup_dependencies
# usage ......: lusan_setup_dependencies()
# ###########################################################################
#
# \brief    Resolves Qt, the Areg SDK and the docking library, and sets the variables the
#           Lusan source lists and `lusan_add_application` expect: LUSAN_ROOT, LUSAN_BASE,
#           LUSAN_CMAKE_CONFIG, LUSAN_THIRDPARTY, LUSAN_ADS_TARGET and LUSAN_EDITION.
# \note     Call once, from a directory scope, before any target is created.
#
macro(lusan_setup_dependencies)

    set(CMAKE_AUTOUIC ON)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)

    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)

    set(LUSAN_CMAKE_CONFIG  "${LUSAN_ROOT}/conf/cmake")
    set(LUSAN_BASE          "${LUSAN_ROOT}/sources")
    set(LUSAN_THIRDPARTY    "${LUSAN_ROOT}/thirdparty")

    set(LUSAN_SRC)
    set(LUSAN_HDR)
    set(LUSAN_RES)
    set(LUSAN_UI)
    set(LUSAN_TRANS)

    include("${LUSAN_CMAKE_CONFIG}/edition.cmake")
    include("${LUSAN_CMAKE_CONFIG}/setup.cmake")

    if (NOT ${QT_VERSION_MAJOR} GREATER_EQUAL 6)
        message(FATAL_ERROR "LUSAN: >>> You should use QT version 6 or newer to compile this project")
    endif()

    include_directories(${LUSAN_BASE})
    include_directories(${LUSAN_THIRDPARTY})

    # The Windows application icon is compiled from a resource script.
    if (WIN32)
        enable_language(RC)
    endif()

    get_target_property(_qt_bin_dir Qt6::qmake IMPORTED_LOCATION)
    get_filename_component(_qt_bin_dir "${_qt_bin_dir}" DIRECTORY)

endmacro(lusan_setup_dependencies)

# ###########################################################################
# function ...: lusan_add_application
# usage ......: lusan_add_application(<target> [SOURCES ...] [HEADERS ...] [RESOURCES ...]
#                                              [TRANSLATIONS ...] [INCLUDES ...] [DEFINES ...]
#                                              [LIBRARIES ...] [LICENSE_DIRS ...])
# ###########################################################################
#
# \brief    Creates a Lusan application target from the Lusan sources plus the given additions,
#           links it, deploys the Qt and docking runtime beside it, copies the configuration,
#           schemas and license notices, and adds the Linux desktop integration.
# \param    target          The name of the executable target to create.
# \param    SOURCES         Extra `.cpp` files compiled into the target.
# \param    HEADERS         Extra headers listed with the target.
# \param    RESOURCES       Extra `.qrc` files.
# \param    TRANSLATIONS    Extra `.ts` files.
# \param    INCLUDES        Extra include directories, also scanned for translatable strings.
# \param    DEFINES         Extra private compile definitions.
# \param    LIBRARIES       Extra private link libraries.
# \param    LICENSE_DIRS    Extra directories whose content is copied and installed next to
#                           the executable.
# \note     Call `lusan_setup_dependencies()` and include the Lusan source lists first.
#
function(lusan_add_application target)

    cmake_parse_arguments(LA "" ""
                          "SOURCES;HEADERS;RESOURCES;TRANSLATIONS;INCLUDES;DEFINES;LIBRARIES;LICENSE_DIRS"
                          ${ARGN})

    set(_sources        ${LUSAN_SRC}    ${LA_SOURCES})
    set(_headers        ${LUSAN_HDR}    ${LA_HEADERS})
    set(_resources      ${LUSAN_RES}    ${LA_RESOURCES})
    set(_translations   ${LUSAN_TRANS}  ${LA_TRANSLATIONS})

    # Per-platform application icon source: an .rc embedding the .ico on Windows, and the .icns
    # bundled into Resources on macOS. Linux uses the runtime QIcon plus an installed .desktop
    # entry and hicolor icons (see the install rules below). All are baked from res/logo/lusan.svg
    # by res/logo/make-icons.py.
    set(_app_icon)
    if (WIN32)
        set(_app_icon "${LUSAN_BASE}/lusan/res/logo/lusan.rc")
    elseif (APPLE)
        set(_app_icon "${LUSAN_BASE}/lusan/res/logo/lusan.icns")
        set_source_files_properties("${_app_icon}" PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    endif()

    set(_target_sources ${_translations} ${_resources} ${LUSAN_UI} ${_headers} ${_sources})
    if (_app_icon)
        list(APPEND _target_sources "${_app_icon}")
    endif()

    qt_create_translation(QM_FILES ${LUSAN_ROOT} ${LA_INCLUDES} ${_translations})
    qt_add_executable(${target} MANUAL_FINALIZATION ${_target_sources})

    target_include_directories(${target} PRIVATE ${LUSAN_BASE} ${LUSAN_THIRDPARTY} ${LA_INCLUDES})
    target_compile_definitions(${target} PRIVATE
        ${COMMON_COMPILE_DEF}
        IMP_LOGGER_DLL
        LUSAN_EDITION="${LUSAN_EDITION}"
        ${LA_DEFINES}
    )
    target_link_libraries(${target} PRIVATE
        Qt${QT_VERSION_MAJOR}::Widgets
        areg::areg
        areg::aregextend
        areg::areglogger
        aregsqlite3
        ${LUSAN_ADS_TARGET}
        ${LA_LIBRARIES}
    )

    # copying log and router init files to 'bin/config'
    add_custom_command( TARGET ${target} POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E copy ${LUSAN_BASE}/lusan/res/lusan.init ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/config/lusan.init
                        VERBATIM)

    # Qt and ADS are LGPL and distributed as shared libraries, ship their license texts.
    foreach(_license_dir "${LUSAN_ROOT}/licenses" ${LA_LICENSE_DIRS})
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory "${_license_dir}" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/licenses"
            COMMENT "Copying third-party license notices"
            VERBATIM)
    endforeach()

    # The Apache license and its notice travel with any redistribution of these sources.
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${LUSAN_ROOT}/LICENSE" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/licenses/LICENSE"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${LUSAN_ROOT}/NOTICE"  "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/licenses/NOTICE"
        VERBATIM)

    # The document schemas travel with the Areg SDK. An SDK built before they existed does not carry
    # them, and Lusan must build and run either way, so their absence is a status line and nothing more.
    if (EXISTS "${AREG_SDK_ROOT}/tools/schema")
        message(STATUS ">>> Document schemas found at '${AREG_SDK_ROOT}/tools/schema', copying beside the executable")
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory "${AREG_SDK_ROOT}/tools/schema" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/schema"
            COMMENT "Copying document schemas"
            VERBATIM)
    else()
        message(STATUS ">>> No document schemas at '${AREG_SDK_ROOT}/tools/schema', the built-in format description is used")
    endif()

    # ADS is a shared library (see conf/cmake/setup.cmake); look for it next to the
    # executable at runtime, and copy it there after each build, the same way the
    # Qt runtime libraries are deployed below.
    set_target_properties(${target} PROPERTIES
        BUILD_RPATH   "$ORIGIN;@loader_path"
        INSTALL_RPATH "$ORIGIN;@loader_path"
    )
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:${LUSAN_ADS_TARGET}>" "$<TARGET_FILE_DIR:${target}>"
        COMMENT "Copying ADS shared library next to the executable"
        VERBATIM)
    if (UNIX)
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_SONAME_FILE:${LUSAN_ADS_TARGET}>" "$<TARGET_FILE_DIR:${target}>"
            VERBATIM)
    endif()

    # Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
    # If you are developing for iOS or macOS you should consider setting an
    # explicit, fixed bundle identifier manually though.
    set(_bundle_id_option)
    if(${QT_VERSION} VERSION_LESS 6.1.0)
        set(_bundle_id_option MACOSX_BUNDLE_GUI_IDENTIFIER com.aregtech.${target})
    endif()

    set_target_properties(${target} PROPERTIES
        ${_bundle_id_option}
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        MACOSX_BUNDLE_ICON_FILE lusan.icns
        MACOSX_BUNDLE TRUE
        WIN32_EXECUTABLE TRUE
    )

    if (WIN32)
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND "${_qt_bin_dir}/windeployqt.exe"
                --no-opengl-sw
                --no-compiler-runtime
                --no-translations
                --no-system-dxc-compiler
                --no-quick-import
                --no-ffmpeg
                "$<TARGET_FILE:${target}>"
            COMMENT "Deploying Qt runtime libraries for Win32"
        )
    elseif (APPLE)
        # macdeployqt has its own command line, the .app bundle directory must come first
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND "${_qt_bin_dir}/macdeployqt"
                "$<TARGET_BUNDLE_DIR:${target}>"
                -always-overwrite
            COMMENT "Deploying Qt runtime libraries for macOS"
            VERBATIM
        )
    else()
        message(STATUS "Qt deployment skipped on Linux (handled by system packaging)")
    endif()

    if(QT_VERSION_MAJOR EQUAL 6)
        qt_finalize_executable(${target})
    endif()

    # Linux desktop integration: install the launcher entry and hicolor icons so the app icon
    # shows up in the application menu (the running window's icon comes from QIcon at runtime).
    if (UNIX AND NOT APPLE)
        include(GNUInstallDirs)
        install(TARGETS ${target} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
        install(FILES "$<TARGET_FILE:${LUSAN_ADS_TARGET}>" DESTINATION ${CMAKE_INSTALL_BINDIR})
        install(FILES "$<TARGET_SONAME_FILE:${LUSAN_ADS_TARGET}>" DESTINATION ${CMAKE_INSTALL_BINDIR})
        install(DIRECTORY "${LUSAN_ROOT}/licenses" DESTINATION ${CMAKE_INSTALL_BINDIR})
        foreach(_license_dir ${LA_LICENSE_DIRS})
            install(DIRECTORY "${_license_dir}" DESTINATION ${CMAKE_INSTALL_BINDIR})
        endforeach()
        if (EXISTS "${AREG_SDK_ROOT}/tools/schema")
            install(DIRECTORY "${AREG_SDK_ROOT}/tools/schema" DESTINATION ${CMAKE_INSTALL_BINDIR})
        endif()
        install(FILES "${LUSAN_BASE}/lusan/res/logo/lusan-256.png"
                DESTINATION "${CMAKE_INSTALL_DATADIR}/icons/hicolor/256x256/apps" RENAME lusan.png)
        install(FILES "${LUSAN_BASE}/lusan/res/logo/lusan.svg"
                DESTINATION "${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps" RENAME lusan.svg)
        install(FILES "${LUSAN_BASE}/lusan/res/logo/lusan.desktop"
                DESTINATION "${CMAKE_INSTALL_DATADIR}/applications")
    endif()

endfunction(lusan_add_application)
