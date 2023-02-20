#add_subdirectory(lib/googletest)

include_directories(${PT_INCLUDE_DIR})
#include_directories(lib/googletest/googletest/include)

set(TEST_SRC
    main.cpp
    src/test_something.cpp
)

add_executable(pt-tests ${TEST_SRC})
target_link_libraries(pt-tests pt-lib) #gtest)
#install(TARGETS project_tests DESTINATION bin)