# tonus core: platform-free C++17. Builds as an ESP-IDF component (firmware) or a plain static
# library (host tests, simulator).
cmake_minimum_required(VERSION 3.16)

set(TONUS_CORE_SRCS
    app/boot.cpp
    art/indexed_bitmap.cpp
    art/sprites.cpp
    art/wordmark.cpp
    audio/chiptune.cpp
    i18n/i18n.cpp
)
set(TONUS_STRINGS_SRC_DIR "${CMAKE_CURRENT_LIST_DIR}/../assets/strings")
set(TONUS_STRINGS_GEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")

# Generate the string table at configure time; editing a JSON file re-runs configure.
function(tonus_generate_strings python)
    file(GLOB string_files "${TONUS_STRINGS_SRC_DIR}/*.json")
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
        ${string_files} "${CMAKE_CURRENT_LIST_DIR}/../tools/i18n/gen_strings.py")
    execute_process(
        COMMAND "${python}" "${CMAKE_CURRENT_LIST_DIR}/../tools/i18n/gen_strings.py"
                --src "${TONUS_STRINGS_SRC_DIR}" --out "${TONUS_STRINGS_GEN_DIR}"
        RESULT_VARIABLE rc)
    if(NOT rc EQUAL 0)
        message(FATAL_ERROR "String table generation failed (see errors above)")
    endif()
endfunction()

if(ESP_PLATFORM)
    if(NOT CMAKE_BUILD_EARLY_EXPANSION)
        tonus_generate_strings("${PYTHON}")
    endif()
    idf_component_register(
        SRCS ${TONUS_CORE_SRCS} "${TONUS_STRINGS_GEN_DIR}/tonus_strings_gen.cpp"
        INCLUDE_DIRS "." "${TONUS_STRINGS_GEN_DIR}")
    return()
endif()

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(tonus_core CXX)
    option(TONUS_BUILD_TESTS "Build the core unit tests" ON)
else()
    option(TONUS_BUILD_TESTS "Build the core unit tests" OFF)
endif()

find_package(Python3 REQUIRED COMPONENTS Interpreter)
tonus_generate_strings("${Python3_EXECUTABLE}")

add_library(tonus_core STATIC ${TONUS_CORE_SRCS} "${TONUS_STRINGS_GEN_DIR}/tonus_strings_gen.cpp")
target_include_directories(tonus_core PUBLIC "${CMAKE_CURRENT_LIST_DIR}" "${TONUS_STRINGS_GEN_DIR}")
target_compile_features(tonus_core PUBLIC cxx_std_17)
target_compile_options(tonus_core PRIVATE -Wall -Wextra)

if(TONUS_BUILD_TESTS)
    include(FetchContent)
    FetchContent_Declare(Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.8.1
        GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(Catch2)

    add_executable(tonus_core_tests
        tests/test_boot.cpp
        tests/test_wordmark.cpp
        tests/test_chiptune.cpp
        tests/test_i18n.cpp)
    target_link_libraries(tonus_core_tests PRIVATE tonus_core Catch2::Catch2WithMain)
    target_compile_definitions(tonus_core_tests PRIVATE
        TONUS_STRINGS_DIR="${TONUS_STRINGS_SRC_DIR}")

    include(CTest)
    include(Catch)
    catch_discover_tests(tonus_core_tests)
endif()
