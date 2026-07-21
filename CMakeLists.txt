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

# Find libsndfile for audio file I/O
find_package(PkgConfig REQUIRED)
pkg_check_modules(SNDFILE sndfile REQUIRED)

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
    src/application.h
    src/mainwindow.cpp
    src/mainwindow.h
    src/ui/widgets/fileimport.cpp
    src/ui/widgets/fileimport.h
    src/ui/widgets/trackselector.cpp
    src/ui/widgets/trackselector.h
    src/ui/widgets/folderchooser.cpp
    src/ui/widgets/folderchooser.h
    src/ui/widgets/mixer.cpp
    src/ui/widgets/mixer.h
    src/ui/widgets/timeline.cpp
    src/ui/widgets/timeline.h
    src/ui/widgets/progressdialog.cpp
    src/ui/widgets/progressdialog.h
    src/audio/audioengine.cpp
    src/audio/audioengine.h
    src/audio/audiotrack.cpp
    src/audio/audiotrack.h
    src/audio/audioplayer.cpp
    src/audio/audioplayer.h
    src/audio/mixerengine.cpp
    src/audio/mixerengine.h
    src/audio/realtimemixer.cpp
    src/audio/realtimemixer.h
    src/audio/mixercontroller.cpp
    src/audio/mixercontroller.h
    src/audio/waveformrenderer.cpp
    src/audio/waveformrenderer.h
    src/audio/backends/audiobackend.h
    src/processing/separationengine.cpp
    src/processing/separationengine.h
    src/processing/modelprovenance.cpp
    src/processing/modelprovenance.h
    src/processing/exportmanager.cpp
    src/processing/exportmanager.h
    src/python/pythonprocess.cpp
    src/python/pythonprocess.h
    src/python/pythonruntime.cpp
    src/python/pythonruntime.h
    src/python/demucswrapper.cpp
    src/python/demucswrapper.h
    src/utils/logger.cpp
    src/utils/logger.h
    src/utils/filemanager.cpp
    src/utils/filemanager.h
    src/utils/hashverifier.cpp
    src/utils/hashverifier.h
)

# Platform-specific sources
if(APPLE)
    list(APPEND SOURCES
        src/audio/backends/coreaudio_backend.cpp
        src/audio/backends/coreaudio_backend.h
    )
elseif(WIN32)
    list(APPEND SOURCES
        src/audio/backends/wasapi_backend.cpp
        src/audio/backends/wasapi_backend.h
    )
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
        ${SNDFILE_LIBRARIES}
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
        ${SNDFILE_LIBRARIES}
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
    ${SNDFILE_INCLUDE_DIRS}
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

message(STATUS "================================================")
message(STATUS "Carlos Okupa v${PROJECT_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "================================================")
