# Set up unit tests.

function(add_cameleon_test)
    set(OPTION_PREFIX ARG)
    set(OPTIONS_WITH_ZERO_ARGS)
    set(OPTIONS_WITH_ONE_ARG NAME)
    set(OPTIONS_WITH_MULTIPLE_ARGS SOURCES)
    cmake_parse_arguments(
        PARSE_ARGV 0 ${OPTION_PREFIX}
        "${OPTIONS_WITH_ZERO_ARGS}" "${OPTIONS_WITH_ONE_ARG}" "${OPTIONS_WITH_MULTIPLE_ARGS}"
    )
    
    qt_add_executable(${ARG_NAME} ${ARG_SOURCES})
    target_link_libraries(${ARG_NAME} PRIVATE CameleonCore Qt::Test Qt::Widgets)
    target_include_directories(${ARG_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src ${CMAKE_CURRENT_BINARY_DIR})
    set_target_properties(${ARG_NAME} PROPERTIES
        WIN32_EXECUTABLE ON
        MACOSX_BUNDLE ON
        VS_DEBUGGER_ENVIRONMENT "PATH=%PATH%;$<TARGET_FILE_DIR:Qt6::Widgets>"
    )
    add_test(NAME ${ARG_NAME} COMMAND ${ARG_NAME})
    if (WIN32)
        set_property(TEST ${ARG_NAME} APPEND PROPERTY ENVIRONMENT "PATH=%PATH%\;$<TARGET_FILE_DIR:Qt6::Widgets>")
    else()
        set_property(TEST ${ARG_NAME} APPEND PROPERTY ENVIRONMENT "PATH=$ENV{PATH}:$<TARGET_FILE_DIR:Qt6::Widgets>")
    endif()
endfunction()

set(TEST_DATA_DIR "${CMAKE_CURRENT_BINARY_DIR}/data")
configure_file(data/black/checkerboard.png "${TEST_DATA_DIR}/black/checkerboard.png" COPYONLY)
configure_file(data/blue/checkerboard.png "${TEST_DATA_DIR}/blue/checkerboard.png" COPYONLY)
configure_file(data/green/checkerboard.png "${TEST_DATA_DIR}/green/checkerboard.png" COPYONLY)
configure_file(data/magenta/checkerboard.png "${TEST_DATA_DIR}/magenta/checkerboard.png" COPYONLY)
configure_file(data/red/checkerboard.png "${TEST_DATA_DIR}/red/checkerboard.png" COPYONLY)
configure_file(data/black/inverted_checkerboard.png "${TEST_DATA_DIR}/black/inverted_checkerboard.png" COPYONLY)
configure_file(data/blue/inverted_checkerboard.png "${TEST_DATA_DIR}/blue/inverted_checkerboard.png" COPYONLY)
configure_file(data/green/inverted_checkerboard.png "${TEST_DATA_DIR}/green/inverted_checkerboard.png" COPYONLY)
configure_file(data/magenta/inverted_checkerboard.png "${TEST_DATA_DIR}/magenta/inverted_checkerboard.png" COPYONLY)
configure_file(data/red/inverted_checkerboard.png "${TEST_DATA_DIR}/red/inverted_checkerboard.png" COPYONLY)
configure_file(data/colours.cml.in "${TEST_DATA_DIR}/colours.cml")
configure_file(data/red-checkerboard.cml.in "${TEST_DATA_DIR}/red-checkerboard.cml")
configure_file(data/green-checkerboards.cml.in "${TEST_DATA_DIR}/green-checkerboards.cml")
configure_file(data/no-matches.cml.in "${TEST_DATA_DIR}/no-matches.cml")
configure_file(TestDataDir.h.in TestDataDir.h)

add_cameleon_test(NAME TestPatternMatching SOURCES TestPatternMatching.cpp TestPatternMatching.h)
add_cameleon_test(NAME TestFindInstances SOURCES TestFindInstances.cpp TestFindInstances.h)
add_cameleon_test(NAME TestNewDocument SOURCES TestNewDocument.cpp TestNewDocument.h TestUtils.h TestDataDir.h.in)
add_cameleon_test(NAME TestOpenDocument SOURCES TestOpenDocument.cpp TestOpenDocument.h TestUtils.h TestDataDir.h.in)
add_cameleon_test(NAME TestMiscAlbumMenuItems SOURCES TestMiscAlbumMenuItems.cpp TestMiscAlbumMenuItems.h TestUtils.h TestDataDir.h.in)
add_cameleon_test(NAME TestViewMenu SOURCES TestViewMenu.cpp TestViewMenu.h TestUtils.h TestDataDir.h.in)
add_cameleon_test(NAME TestNavigationMenu SOURCES TestNavigationMenu.cpp TestNavigationMenu.h TestUtils.h TestDataDir.h.in)
add_cameleon_test(NAME TestBookmarksMenu SOURCES TestBookmarksMenu.cpp TestBookmarksMenu.h TestUtils.h TestDataDir.h.in)
