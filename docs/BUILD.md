# Building Carlos Okupa

## Prerequisites

### macOS

```bash
# Install Xcode Command Line Tools
xcode-select --install

# Install Qt6 via Homebrew
brew install qt@6 cmake

# Verify installation
qmake --version
cmake --version
```

### Windows

1. Install **Visual Studio 2022** with C++ workload
2. Download and install **Qt 6.5+** from https://www.qt.io/download
   - Select MSVC 2022 compiler
3. Download and install **CMake 3.24+** from https://cmake.org/download

## Build Steps

### macOS

```bash
cd carlos-okupa
mkdir build && cd build

# Configure
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build . --config Release

# Run
./bin/CarlosOkupa.app/Contents/MacOS/CarlosOkupa
```

### Windows

```cmd
cd carlos-okupa
mkdir build && cd build

REM Configure
cmake .. -DCMAKE_BUILD_TYPE=Release

REM Build
cmake --build . --config Release

REM Run
.\bin\CarlosOkupa.exe
```

## Troubleshooting

### Qt Not Found

Set `Qt6_DIR` environment variable:

**macOS:**
```bash
export Qt6_DIR=$(brew --prefix qt@6)/lib/cmake/Qt6
cmake .. -DCMAKE_BUILD_TYPE=Release
```

**Windows:**
```cmd
set Qt6_DIR=C:\Qt\6.5.0\msvc2022_64\lib\cmake\Qt6
cmake .. -DCMAKE_BUILD_TYPE=Release
```

### CMake Version Error

Upgrade CMake:

```bash
brew upgrade cmake  # macOS
choco upgrade cmake # Windows (with Chocolatey)
```
