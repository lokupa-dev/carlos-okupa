# Building Carlos Okupa (Complete)

## Complete Build Guide

### Prerequisites

**macOS:**
```bash
# Install dependencies
brew install qt@6 cmake libsndfile python@3.11

# Install Python packages
pip3 install torch demucs librosa scipy
```

**Windows:**
1. Visual Studio 2022 (C++ workload)
2. Qt 6.5+ (https://www.qt.io/download)
3. CMake 3.24+ (https://cmake.org/download)
4. Python 3.11+ (https://www.python.org/downloads/)
5. libsndfile (vcpkg or binary)

```cmd
# Install Python packages
pip install torch demucs librosa scipy
```

### Build Steps

#### macOS

```bash
cd carlos-okupa
mkdir build && cd build

# Configure
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build . --config Release -j$(sysctl -n hw.ncpu)

# Run
./bin/CarlosOkupa.app/Contents/MacOS/CarlosOkupa
```

#### Windows (Visual Studio)

```cmd
cd carlos-okupa
mkdir build && cd build

REM Configure
cmake .. -DCMAKE_BUILD_TYPE=Release -G "Visual Studio 17 2022"

REM Build
cmake --build . --config Release --parallel %NUMBER_OF_PROCESSORS%

REM Run
.\bin\CarlosOkupa.exe
```

### Dependency Installation

#### libsndfile on Different Platforms

**macOS (Homebrew):**
```bash
brew install libsndfile
```

**Windows (vcpkg):**
```bash
# Clone vcpkg if not present
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat

# Install libsndfile
.\vcpkg install libsndfile:x64-windows

# Then in CMake:
cmake .. -DCMAKE_TOOLCHAIN_FILE=path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
```

### Testing the Build

```bash
# After successful build, test audio initialization
./bin/CarlosOkupa  # macOS
.\bin\CarlosOkupa.exe  # Windows
```

You should see:
```
===========================================
  Carlos Okupa v0.1.0 - Starting
===========================================
Step 1/4: Initializing Python runtime...
✓ Python runtime ready
Step 2/4: Initializing audio engine...
✓ Audio engine ready: 44100 Hz, 2 channels
  Device: System Output (CoreAudio/WASAPI)
Step 3/4: Initializing separation engine...
✓ Separation engine ready
Step 4/4: Creating user interface...
✓ User interface ready
===========================================
  Initialization Complete!
===========================================
```

### Usage Workflow

1. **Load Audio File**
   - Drag & drop audio file or click Browse
   - Supported: MP3, WAV, FLAC, OGG

2. **Select Stems**
   - Choose which stems to extract: Vocals, Drums, Bass, Other
   - Select all or individual stems

3. **Choose Output Folder**
   - Where separated stems will be saved

4. **Start Separation**
   - Click "Start Separation" button
   - Progress dialog shows real-time progress
   - Processing time: ~1-3 minutes depending on song length

5. **Playback & Mixing**
   - Once complete, stems load automatically
   - Use mixer sliders for volume control
   - Mute/Solo individual stems
   - Play/Pause/Stop buttons for playback

6. **Export**
   - Export Mix: Export current mixer state as WAV
   - Export Stems: Save individual stems

### Troubleshooting

**"Failed to initialize Python runtime"**
- Ensure Python 3.11+ is installed
- Check `python3 --version` or `python --version`
- Install Demucs: `pip install demucs`

**"Failed to initialize audio engine"**
- macOS: Check AudioToolbox framework
- Windows: Ensure WASAPI is available (usually built-in)
- Try different audio output device

**"Separation failed"**
- Check sufficient disk space
- Verify input audio file is valid
- Monitor Python error output in logs

**Qt not found**
- Set environment: `export Qt6_DIR=$(brew --prefix qt@6)/lib/cmake/Qt6`
- Windows: Add Qt to PATH manually

### Performance Notes

- **GPU Acceleration**: PyTorch will auto-detect CUDA/Metal
- **Memory**: Peak ~2-3 GB during separation
- **CPU**: All cores will be utilized
- **Disk**: Needs ~2x input file size for temporary storage

### Next Steps for Development

- [ ] GPU support verification
- [ ] Model weight download automation
- [ ] Progress bar improvements
- [ ] Batch processing
- [ ] Preset mixer states
- [ ] Audio visualization
- [ ] Settings dialog
- [ ] Installers (DMG, NSIS)
