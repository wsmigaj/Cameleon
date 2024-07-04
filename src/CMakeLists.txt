# Set up a target for the Cameleon application.

configure_file(Version.h.in Version.h)
message(PROJECT_VERSION ${PROJECT_VERSION})
file(GLOB CAMELEON_HEADERS CONFIGURE_DEPENDS *.h ../3pty/glob/*.h)
file(GLOB CAMELEON_SOURCES CONFIGURE_DEPENDS *.cpp ../3pty/glob/*.cpp)
file(GLOB CAMELEON_RESOURCES CONFIGURE_DEPENDS *.qrc *.rc)
file(GLOB CAMELEON_UIS CONFIGURE_DEPENDS *.ui)

list(REMOVE_ITEM CAMELEON_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)
list(APPEND CAMELEON_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/Version.h.in)

add_library(CameleonCore OBJECT ${CAMELEON_SOURCES} ${CAMELEON_HEADERS} ${CAMELEON_RESOURCES} ${CAMELEON_UIS})
target_link_libraries(CameleonCore PRIVATE Qt6::Core Qt6::Concurrent Qt6::Widgets)
target_include_directories(CameleonCore PRIVATE ../3pty ${CMAKE_CURRENT_BINARY_DIR})
target_precompile_headers(CameleonCore PRIVATE <QtWidgets>)

qt_add_executable(Cameleon main.cpp)
target_link_libraries(Cameleon PRIVATE CameleonCore)
target_link_libraries(Cameleon PRIVATE Qt6::Core Qt6::Widgets)
set_target_properties(Cameleon PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
    VS_DEBUGGER_ENVIRONMENT "PATH=%PATH%;$<TARGET_FILE_DIR:Qt6::Widgets>"
)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Cameleon)
