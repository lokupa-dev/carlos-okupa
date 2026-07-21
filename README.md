# Carlos Okupa

🎵 **Music Source Separation for macOS & Windows**

Carlos Okupa is a native cross-platform desktop application that separates mixed music into individual instrumental and vocal components using state-of-the-art AI models.

## Features

- 🎼 **High-Quality Separation**: Vocals, Drums, Bass, and Other stems
- 💻 **Cross-Platform**: Native support for macOS and Windows
- 🔒 **Local Processing**: All computation happens on your machine (no cloud, no uploads)
- 🎚️ **Real-Time Mixer**: Sync-accurate playback with mute, solo, and volume controls
- 📤 **Flexible Export**: Export individual stems or custom mixes
- ⚡ **Reproducible Runtime**: Embedded Python environment (no dependencies)
- 📋 **Full Attribution**: Complete SBOM, model provenance, and third-party notices

## Technology Stack

- **Frontend**: Qt 6 (C++, cross-platform)
- **Audio Engine**: WASAPI (Windows) / AVAudioEngine (macOS)
- **Separation**: PyTorch + Demucs v4
- **Build**: CMake 3.24+

## Status

⚠️ **LOCAL DEVELOPMENT BUILD**

This is a work-in-progress implementation. Model weights are currently sourced from datasets with non-commercial restrictions. See `PROVENANCE.json` for details.

## Building

### Prerequisites

**macOS:**
```bash
# Xcode Command Line Tools
xcode-select --install

# Qt 6 (via Homebrew)
brew install qt@6 cmake
```

**Windows:**
- Visual Studio 2022 (C++ workload)
- Qt 6.5+ (download from qt.io)
- CMake 3.24+

### Quick Start

```bash
# Clone
git clone https://github.com/lokupa-dev/carlos-okupa.git
cd carlos-okupa

# Build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release

# Run
# macOS:
./bin/CarlosOkupa.app/Contents/MacOS/CarlosOkupa

# Windows:
./bin/CarlosOkupa.exe
```

## Documentation

- [Architecture](docs/ARCHITECTURE.md) — Design overview
- [Build Instructions](docs/BUILD.md) — Detailed build guide
- [Testing Guide](docs/TESTING.md) — QA procedures
- [Deployment](docs/DEPLOYMENT.md) — Packaging & distribution

## License

- **Code**: MIT License
- **Models**: See [PROVENANCE.json](PROVENANCE.json) — ⚠️ Restricted for non-commercial use
- **Third-Party**: See [THIRD_PARTY_NOTICES.txt](THIRD_PARTY_NOTICES.txt)

## Author

Developed by [lokupa-dev](https://github.com/lokupa-dev)
