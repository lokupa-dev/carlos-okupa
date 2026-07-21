# Carlos Okupa — Architecture Overview

## Design Principles

1. **Cross-Platform Native**: Qt6 C++ on both macOS and Windows
2. **Local Processing**: No cloud, no uploads, full privacy
3. **Sample-Accurate Mixing**: Synchronized playback via single audio engine
4. **Embedded Runtime**: Self-contained Python (no Homebrew/external deps)
5. **Model Provenance**: Full SHA-256 verification, licensing, attribution
6. **Fail-Closed**: Unknown or unlicensed models are rejected

## Layered Architecture

```
┌────────────────────────────────────────────────────────────┐
│          User Interface (Qt6 QML/Widgets)       │
│  ┌────────────────────────────────────────────────────┐ │
│  │  File Import │ Track Selector │ Mixer    │        │ │
│  │  Timeline    │ Progress       │ Export   │        │ │
│  └────────────────────────────────────────────────────┘ │
└────────────────────────┬────────────────────────────────┘
                         │ Signals/Slots
                         ▼
┌────────────────────────────────────────────────────────────┐
│     Application Layer (Controllers)             │
│  ┌────────────────────────────────────────────────────┐ │
│  │ MainWindow │ SeparationEngine │ Mixer Ctrl        │ │
│  └────────────────────────────────────────────────────┘ │
└────────────────────────┬────────────────────────────────┘
                         │ C++ API
                         ▼
┌────────────────────────────────────────────────────────────┐
│      Processing Layer (Core Logic)              │
│  ┌────────────────────────┬────────────────────────┐   │
│  │  Python IPC      │  Audio Mixing Engine │   │
│  │  (Demucs v4)     │  (WASAPI/AVAudio)    │   │
│  │  Export Manager  │  Waveform Renderer   │   │
│  └────────────────────────┴────────────────────────┘   │
└────────────────────────┬────────────────────────────────┘
                    ┌────┴────┐
                    ▼         ▼
        ┌──────────────────────┐ ┌──────────────────────┐
        │ Embedded    │        │ │ Audio Device │
        │ Python 3.11 │        │ │ (Speaker)    │
        │ + PyTorch   │        │ └──────────────────────┘
        └──────────────────────┘
```

## Module Breakdown

### UI Layer (`src/ui/`)

**FileImport Widget**
- Drag-and-drop zone for audio files
- File info display (name, duration, sample rate, channels)
- Format validation (MP3, WAV, FLAC, OGG)

**TrackSelector Widget**
- "Select All" / "Deselect All" buttons
- Individual checkboxes: Vocals, Drums, Bass, Other
- Disabled tracks with explanations if model unavailable

**FolderChooser Widget**
- File browser for output directory
- Displays selected path
- Creates directory if needed (with permission check)

**Mixer Widget**
- Track list with vertical sliders
- Mute, Solo buttons per track
- Volume display (%dB)
- Optional waveform meters

**Timeline Widget**
- Waveform visualization (optional, complex)
- Playback position scrubber
- Time display (current / total)
- Play, Pause, Stop buttons
- Keyboard shortcut: Spacebar for play/pause

**ProgressDialog**
- Modal dialog during separation
- Model name, stage, progress %
- ETA, elapsed time
- Cancel button (safe shutdown)
- Error display (user-friendly)

### Audio Layer (`src/audio/`)

**AudioEngine (Base Class)**
- Abstract interface for audio device management
- Sample rate, channel config, buffer size
- Initialization/shutdown lifecycle

**Backends**
- `CoreAudioBackend` (macOS): AVAudioEngine, AVAudioPlayerNode
- `WasapiBackend` (Windows): WASAPI exclusive/shared mode

**AudioPlayer**
- Manages playback state (play, pause, stop)
- Seek with smooth crossfade (no clicks)
- Real-time mute/solo/volume control

**MixerEngine**
- Loads multiple WAV files (separation outputs)
- Routes to single audio clock
- Applies gain/mute/solo in real-time
- NO stream restart on parameter changes

**WaveformRenderer**
- Renders waveform for timeline display
- Peak detection for visual feedback

### Processing Layer (`src/processing/`)

**SeparationEngine**
- Spawns Python subprocess (demucs_wrapper.py)
- IPC via stdin/stdout (JSON messages)
- Progress callback handling
- Cancellation: sends SIGTERM, waits gracefully
- Captures stderr for logging (not user display)

**ModelProvenance**
- Loads PROVENANCE.json
- SHA-256 verification of model weights
- Checks commercial/redistribution flags
- **Fail-closed**: Rejects unknown models

**ExportManager**
- Offline rendering (no real-time playback)
- Applies current mixer state (volume, mute, solo)
- Detects clipping, logs warning/mitigation
- Exports WAV PCM (16/24/32-bit)
- Exports mix with applied effects

### Python Layer (`src/python/`)

**PythonRuntime**
- Locates embedded Python.framework (macOS) or pythonXX.dll (Windows)
- Sets PYTHONHOME, PYTHONPATH
- Sets PYTHONNOUSERSITE=1 (no user site-packages)
- Initializes interpreter
- Manages subprocess lifecycle

**demucs_wrapper.py**
- Loads Demucs v4 model
- Reads JSON commands from stdin
- Processes audio file
- Writes progress to stdout (JSON)
- Handles errors, logs to stderr
- Exits cleanly on SIGTERM

### Utilities (`src/utils/`)

**Logger**
- File + console logging (DEBUG/INFO/WARNING/ERROR)
- Timestamps, thread IDs
- Rotation policy (max 10 MB per file)

**FileManager**
- Safe file operations (mkdir, rm, move)
- Permission checks
- Atomic operations (no partial files)

**HashVerifier**
- SHA-256 for model weights
- SHA-256 for Python runtime
- Incremental verification (progress callback)

## Data Flow

### Separation Workflow

```
User drops file
      ▼
FileImport validates
      ▼
Display metadata
      ▼
User selects stems & folder
      ▼
SeparationEngine spawns Python
      ▼
demucs_wrapper loads model, checks PROVENANCE
      ▼
╔═══════════════════════════════════════════════════╗
║ For each chunk of audio:            ║
║  1. Run model                       ║
║  2. Send progress (JSON) to C++     ║
║  3. Write stem to temp file         ║
╚═══════════════════════════════════════════════════╝
      ▼
Python writes all stems to output folder
      ▼
C++ loads stems into MixerEngine
      ▼
UI enables playback controls
```

### Playback Workflow

```
User presses Play
      ▼
AudioPlayer → AudioEngine
      ▼
AudioEngine opens audio device
      ▼
MixerEngine feeds samples from all tracks
      ▼
╔═══════════════════════════════════════════════════╗
║ Real-time audio callback:           ║
║  FOR each track:                    ║
║    - Read next samples              ║
║    - Apply gain (ramp if changed)   ║
║    - Apply mute (silent)            ║
║    - Apply solo logic               ║
║  - Sum all tracks                   ║
║  - Send to device                   ║
╚═══════════════════════════════════════════════════╝
      ▼
User adjusts volume (no restart!)
      ▼
Gain ramp applied smoothly
      ▼
No click, no glitch, playback continues
```

### Export Workflow

```
User clicks "Export Mix"
      ▼
ExportManager reads current state (volumes, mute, solo)
      ▼
ExportManager opens all stems
      ▼
╔═══════════════════════════════════════════════════╗
║ Offline rendering (no playback):    ║
║  FOR each sample position:          ║
║    - Mix according to current state ║
║    - Detect peak                    ║
║  - Check if clipping needed         ║
╚═══════════════════════════════════════════════════╝
      ▼
Warn user if clipping
      ▼
Export to output folder as WAV
      ▼
Open folder in explorer/Finder
```

## Threading Model

- **Main Thread (Qt Event Loop)**
  - UI updates
  - Signal/slot connections

- **Audio Thread (Real-time, Audio Engine)**
  - High priority, fixed buffer callback
  - MixerEngine runs here
  - NO allocations, NO I/O

- **Worker Thread (Qt Concurrent)**
  - Separation (Python subprocess)
  - Export rendering
  - File I/O
  - Progress reporting back to main thread

- **Python Subprocess**
  - Independent process (not thread-safe)
  - IPC via stdin/stdout
  - Can be killed safely

## Error Handling

### User-Facing Errors

- "Invalid audio file: Expected MP3/WAV, got XYZ"
- "Model not found: Vocals model not available"
- "Output folder permission denied"
- "Separation failed: Insufficient memory"

### Logged Details (Not Shown)

- Python stack traces
- FFmpeg stderr
- WASAPI/AVAudio error codes
- Internal state dumps

### Recovery

- Separation interrupted: Delete partial stems, return to file selector
- Export failed: Preserve original stems, report issue
- Audio device lost: Pause, allow user to reconnect

## Model Management

Each model requires:

```json
{
  "id": "demucs-htdemucs-v4",
  "path": "resources/models/htdemucs",
  "hash_sha256": "abc123...",
  "weights_license": "AMBIGUOUS - MUSDB18 HQ",
  "commercial_authorized": false,
  "redistribution_authorized": false,
  "status": "DEVELOPMENT_ONLY"
}
```

- Models not in PROVENANCE.json → **REJECTED** (fail-closed)
- Hash mismatch → **REJECTED**
- Commercial use flagged but trying to build commercial release → **ERROR** (fail-closed)

## Security & Privacy

1. **No uploads**: Audio never leaves the machine
2. **No tracking**: No telemetry, no analytics
3. **Model verification**: SHA-256, provenance check
4. **Safe cancellation**: SIGTERM, wait 5s, SIGKILL if needed
5. **No temp files in bundle**: Use Application Support / Caches / TMP
6. **Readonly bundle**: Application.app is not modified at runtime

## Performance Targets

- Separation: ~1.5x real-time (matches Demucs baseline)
- Playback: <10ms latency (audio callback buffer)
- Seek: <50ms to position
- UI: 60 FPS (Qt default)
- Memory: <2 GB peak (Python + PyTorch overhead)

## Next Steps

1. Implement UI wireframes (Qt Designer)
2. Implement audio backends (AVAudioEngine, WASAPI)
3. Implement Python IPC layer
4. Integrate Demucs wrapper
5. Implement mixer real-time mixing
6. Test separation + playback
7. Build installers (DMG, NSIS)
