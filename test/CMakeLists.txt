# Set up unit tests.

enable_testing(true)

qt_add_executable(TestPatternMatching TestPatternMatching.cpp TestPatternMatching.h)
target_link_libraries(TestPatternMatching PRIVATE CameleonCore Qt::Test)
set_target_properties(TestPatternMatching PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
    VS_DEBUGGER_ENVIRONMENT "PATH=%PATH%;$<TARGET_FILE_DIR:Qt6::Widgets>"
)
add_test(NAME TestPatternMatching COMMAND TestPatternMatching)
