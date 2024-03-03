# Set up a target for the Cameleon application.

file(GLOB CAMELEON_HEADERS CONFIGURE_DEPENDS *.h ../3pty/glob/*.h)
file(GLOB CAMELEON_SOURCES CONFIGURE_DEPENDS *.cpp ../3pty/glob/*.cpp)
file(GLOB CAMELEON_RESOURCES CONFIGURE_DEPENDS *.qrc *.rc)
file(GLOB CAMELEON_UIS CONFIGURE_DEPENDS *.ui)

qt_add_executable(Cameleon ${CAMELEON_SOURCES} ${CAMELEON_HEADERS} ${CAMELEON_RESOURCES} ${CAMELEON_UIS})
target_link_libraries(Cameleon PRIVATE Qt6::Core Qt6::Concurrent Qt6::Widgets)
target_include_directories(Cameleon PRIVATE ../3pty)
target_precompile_headers(Cameleon PRIVATE <QtWidgets>)
set_target_properties(Cameleon PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
    VS_DEBUGGER_ENVIRONMENT "PATH=%PATH%;$<TARGET_FILE_DIR:Qt6::Widgets>"
)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Cameleon)
