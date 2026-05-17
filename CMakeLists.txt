cmake_minimum_required(VERSION 3.22)

project(QuadTable VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(JUCE_SOURCE_DIR "" CACHE PATH "Optional path to a local JUCE checkout")

if(JUCE_SOURCE_DIR)
    add_subdirectory(${JUCE_SOURCE_DIR} JUCE)
else()
    include(FetchContent)
    FetchContent_Declare(
        JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.12
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(JUCE)
endif()

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
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
