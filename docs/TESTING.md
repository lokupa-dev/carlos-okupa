# Testing Carlos Okupa

## Unit Tests

To be implemented with Qt Test framework.

## Integration Tests

1. **Audio Import**
   - Load MP3, WAV, FLAC, OGG files
   - Verify metadata (duration, sample rate, channels)

2. **Separation**
   - Test Demucs v4 model loading
   - Test stem extraction (Vocals, Drums, Bass, Other)
   - Verify output WAV files
   - Check for silent files (should contain signal if input has signal)

3. **Playback**
   - Test real-time playback of separated stems
   - Test mute/solo functionality
   - Test volume control
   - Test seek/timeline scrubbing

4. **Export**
   - Test individual stem export
   - Test mix export with current mixer state
   - Verify WAV format and quality

## Functional Tests

Manual testing with real audio files:

1. Load a song with clear vocals, drums, bass
2. Select all stems
3. Run separation
4. Verify each stem contains expected audio
5. Test mixer controls
6. Export mix and verify
