cmake_minimum_required(VERSION 3.22)

project(QuadTable VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(QUADTABLE_FETCH_JUCE "Fetch JUCE from GitHub when JUCE is not available locally" OFF)
set(JUCE_SOURCE_DIR "" CACHE PATH "Optional path to a local JUCE checkout")

include(CTest)

add_library(QuadTableCore STATIC
    Source/Core/WavetableCore.cpp
)
target_include_directories(QuadTableCore PUBLIC Source/Core)
target_compile_features(QuadTableCore PUBLIC cxx_std_17)

add_executable(QuadTableCoreSmokeTest
    Tests/WavetableCoreSmokeTest.cpp
)
target_link_libraries(QuadTableCoreSmokeTest PRIVATE QuadTableCore)

if(BUILD_TESTING)
    add_test(NAME QuadTableCoreSmokeTest COMMAND QuadTableCoreSmokeTest)
endif()

set(QUADTABLE_HAS_JUCE OFF)

if(JUCE_SOURCE_DIR)
    add_subdirectory(${JUCE_SOURCE_DIR} JUCE)
    set(QUADTABLE_HAS_JUCE ON)
else()
    find_package(JUCE CONFIG QUIET)

    if(JUCE_FOUND)
        set(QUADTABLE_HAS_JUCE ON)
    elseif(QUADTABLE_FETCH_JUCE)
        include(FetchContent)
        FetchContent_Declare(
            JUCE
            GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
            GIT_TAG 8.0.12
            GIT_SHALLOW TRUE
        )
        FetchContent_MakeAvailable(JUCE)
        set(QUADTABLE_HAS_JUCE ON)
    endif()
endif()

if(QUADTABLE_HAS_JUCE)
    juce_add_plugin(QuadTable
        COMPANY_NAME "Open Synth Lab"
        BUNDLE_ID "com.opensynthlab.quadtable"
        PLUGIN_MANUFACTURER_CODE Oslb
        PLUGIN_CODE QTbl
        FORMATS VST3 Standalone
        PRODUCT_NAME "QuadTable"
        IS_SYNTH TRUE
        NEEDS_MIDI_INPUT TRUE
        NEEDS_MIDI_OUTPUT FALSE
        IS_MIDI_EFFECT FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS FALSE
        COPY_PLUGIN_AFTER_BUILD FALSE
    )

    juce_generate_juce_header(QuadTable)

    target_sources(QuadTable
        PRIVATE
            Source/PluginEditor.cpp
            Source/PluginProcessor.cpp
            Source/Synth/WavetableOscillator.cpp
            Source/Synth/WavetableSynthVoice.cpp
    )

    target_compile_definitions(QuadTable
        PUBLIC
            JUCE_DISPLAY_SPLASH_SCREEN=0
            JUCE_REPORT_APP_USAGE=0
            JUCE_VST3_CAN_REPLACE_VST2=0
    )

    target_link_libraries(QuadTable
        PRIVATE
            QuadTableCore
            juce::juce_audio_utils
            juce::juce_dsp
        PUBLIC
            juce::juce_recommended_config_flags
            juce::juce_recommended_lto_flags
            juce::juce_recommended_warning_flags
    )
else()
    message(STATUS "JUCE was not found. Configuring the network-free QuadTableCore smoke-test build only.")
    message(STATUS "Pass -DJUCE_SOURCE_DIR=/path/to/JUCE or -DQUADTABLE_FETCH_JUCE=ON to build the VST3/Standalone plugin target.")
endif()
