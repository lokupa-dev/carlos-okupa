cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

project(CarlosOkupa
    VERSION 0.1.0
    LANGUAGES CXX
    DESCRIPTION "Music Source Separation for macOS & Windows"
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt6
find_package(Qt6 COMPONENTS
    Core
    Gui
    Widgets
    Multimedia
    Concurrent
    Network
    REQUIRED
)

# Platform-specific setup
if(APPLE)
    find_package(AudioToolbox REQUIRED)
    find_package(CoreAudio REQUIRED)
    find_package(CoreFoundation REQUIRED)
elseif(WIN32)
    # WASAPI is built-in
endif()

# Source files
set(SOURCES
    src/main.cpp
    src/application.cpp
    src/mainwindow.cpp
    src/ui/widgets/fileimport.cpp
    src/ui/widgets/trackselector.cpp
    src/ui/widgets/folderchooser.cpp
    src/ui/widgets/mixer.cpp
    src/ui/widgets/timeline.cpp
    src/ui/widgets/progressdialog.cpp
    src/audio/audioengine.cpp
    src/audio/audioplayer.cpp
    src/audio/mixerengine.cpp
    src/audio/waveformrenderer.cpp
    src/processing/separationengine.cpp
    src/processing/modelprovenance.cpp
    src/processing/exportmanager.cpp
    src/python/pythonruntime.cpp
    src/utils/logger.cpp
    src/utils/filemanager.cpp
    src/utils/hashverifier.cpp
)

# Platform-specific sources
if(APPLE)
    list(APPEND SOURCES src/audio/backends/coreaudio_backend.cpp)
elseif(WIN32)
    list(APPEND SOURCES src/audio/backends/wasapi_backend.cpp)
endif()

# Create executable
add_executable(${PROJECT_NAME}
    ${SOURCES}
    resources/resources.qrc
)

# Link libraries
if(APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt6::Core Qt6::Gui Qt6::Widgets
        Qt6::Multimedia Qt6::Concurrent Qt6::Network
        AudioToolbox CoreAudio CoreFoundation
    )
    
    # macOS app bundle properties
    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_EXECUTABLE_NAME ${PROJECT_NAME}
        MACOSX_BUNDLE_BUNDLE_NAME "Carlos Okupa"
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
        MACOSX_DEPLOYMENT_TARGET 11.0
    )
elseif(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt6::Core Qt6::Gui Qt6::Widgets
        Qt6::Multimedia Qt6::Concurrent Qt6::Network
        Winmm.lib Ole32.lib
    )
    
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE ON
    )
endif()

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/audio
    ${CMAKE_CURRENT_SOURCE_DIR}/src/processing
    ${CMAKE_CURRENT_SOURCE_DIR}/src/python
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils
)

# Output directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Compiler warnings
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Testing
enable_testing()

print_status("================================================")
print_status("Carlos Okupa v${PROJECT_VERSION}")
print_status("Build type: ${CMAKE_BUILD_TYPE}")
print_status("Platform: ${CMAKE_SYSTEM_NAME}")
print_status("================================================")
