cmake_minimum_required(VERSION 3.19)
project(Cameleon)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.2 REQUIRED COMPONENTS Core Concurrent Widgets)
# qt_standard_project_setup()
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

file(GLOB CAMELEON_HEADERS CONFIGURE_DEPENDS *.h 3pty/glob/*.h)
file(GLOB CAMELEON_SOURCES CONFIGURE_DEPENDS *.cpp 3pty/glob/*.cpp)
file(GLOB CAMELEON_RESOURCES CONFIGURE_DEPENDS *.qrc)

qt_add_executable(Cameleon ${CAMELEON_SOURCES} ${CAMELEON_HEADERS} ${CAMELEON_RESOURCES})
target_link_libraries(Cameleon PRIVATE Qt6::Core Qt6::Concurrent Qt6::Widgets)
target_include_directories(Cameleon PRIVATE 3pty)
target_precompile_headers(Cameleon PRIVATE <QtWidgets>)
set_target_properties(Cameleon PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
    VS_DEBUGGER_ENVIRONMENT "PATH=%PATH%;$<TARGET_FILE_DIR:Qt6::Widgets>"
)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Cameleon)
