cmake_minimum_required(VERSION 3.19)
project(Cameleon VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    get_property(IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
    if (NOT IS_MULTI_CONFIG AND NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
    endif()
endif()

# Define options.

option(ENABLE_TESTS "Build tests" ON)

# Set up the Qt dependency.

find_package(Qt6 6.2 REQUIRED COMPONENTS Core Concurrent Widgets Test Tools)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)

# Start of a code snippet copied from the definition of the 
# qt_standard_project_setup() macro in Qt 6.6.

# Use standard install locations, provided by GNUInstallDirs. All
# platforms should have this included so that we know the
# CMAKE_INSTALL_xxxDIR variables will be set.
include(GNUInstallDirs)
if(WIN32)
    # Windows has no RPATH support, so we need all non-plugin DLLs in
    # the same directory as application executables if we want to be
    # able to run them without having to augment the PATH environment
    # variable. Don't discard an existing value in case the project has
    # already set this to somewhere else. Our setting is somewhat
    # opinionated, so make it easy for projects to choose something else.
    if(NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    endif()
elseif(NOT APPLE)
    # Apart from Windows and Apple, most other platforms support RPATH
    # and $ORIGIN. Make executables and non-static libraries use an
    # install RPATH that allows them to find library dependencies if the
    # project installs things to the directories defined by the
    # CMAKE_INSTALL_xxxDIR variables (which is what CMake's defaults
    # are based on).
    file(RELATIVE_PATH __qt_relDir
        ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}
        ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}
    )
    list(APPEND CMAKE_INSTALL_RPATH $ORIGIN $ORIGIN/${__qt_relDir})
    list(REMOVE_DUPLICATES CMAKE_INSTALL_RPATH)
    unset(__qt_reldir)
endif()

# End of the snippet copied from qt_standard_project_setup().

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY QT_TARGETS_FOLDER QtInternalTargets)
set_property(GLOBAL PROPERTY AUTOGEN_TARGETS_FOLDER QtInternalTargets)
set_property(GLOBAL PROPERTY AUTOGEN_SOURCE_GROUP "Generated Files")

# Build the main application.

add_subdirectory(src)

# Build tests, if requested.

if (ENABLE_TESTS)
    enable_testing(true)
    add_subdirectory(test)
endif()

# Set up installation.

install(TARGETS Cameleon
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if (WIN32)
    set(deployScript ${CMAKE_CURRENT_BINARY_DIR}/deployscript-$<CONFIG>.cmake)
    file(GENERATE OUTPUT "${deployScript}" CONTENT [[
    execute_process(
        WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}
        COMMAND "$<TARGET_FILE:Qt6::windeployqt>"
            bin/Cameleon.exe
            --no-translations
        COMMAND_ERROR_IS_FATAL LAST
    )
    ]])
    install(SCRIPT "${deployScript}")
elseif (APPLE)
    set(deployScript ${CMAKE_CURRENT_BINARY_DIR}/deployscript-$<CONFIG>.cmake)
    file(GENERATE OUTPUT "${deployScript}" CONTENT [[
    execute_process(
        WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}
        COMMAND "$<TARGET_FILE_DIR:Qt6::qhelpgenerator>/macdeployqt"
            Cameleon.app
            -dmg -always-overwrite
        COMMAND_ERROR_IS_FATAL LAST
    )
    ]])
    install(SCRIPT "${deployScript}")
endif()

# Set up packaging.

set(CPACK_PACKAGE_NAME Cameleon)
set(CPACK_PACKAGE_VENDOR Cameleon)
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Cameleon")
set(CPACK_PACKAGE_INSTALL_DIRECTORY ${CPACK_PACKAGE_NAME})
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_VERBATIM_VARIABLES YES)

if(WIN32)
    set(CPACK_GENERATOR ZIP)
else()
    set(CPACK_GENERATOR TGZ)
endif()

include(CPack)
