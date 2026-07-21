#!/usr/bin/env python3
"""
Demucs Wrapper - IPC interface for music source separation

Reads commands from stdin and outputs JSON progress messages to stdout.
"""

import json
import sys
import os
from pathlib import Path

try:
    import torch
    from demucs.separate import Separator
    from demucs.pretrained import model_name_to_metadata
except ImportError as e:
    print(json.dumps({
        "type": "error",
        "message": f"Failed to import required modules: {str(e)}"
    }))
    sys.exit(1)


class DemucsIPCInterface:
    def __init__(self):
        self.separator = None
        self.model_name = "htdemucs"
        
    def log_progress(self, percent, stage, model, current_chunk, total_chunks):
        """Send progress update to parent process via JSON"""
        msg = {
            "type": "progress",
            "percent": percent,
            "stage": stage,
            "model": model,
            "current_chunk": current_chunk,
            "total_chunks": total_chunks
        }
        print(json.dumps(msg), flush=True)
    
    def log_error(self, message):
        """Send error message to parent process via JSON"""
        msg = {
            "type": "error",
            "message": message
        }
        print(json.dumps(msg), flush=True)
    
    def initialize(self):
        """Initialize Demucs model"""
        try:
            self.log_progress(10, "Initializing model", self.model_name, 0, 1)
            
            # Create separator with appropriate device
            device = "cuda" if torch.cuda.is_available() else "cpu"
            self.separator = Separator(
                model=self.model_name,
                device=device,
                segment=30,  # Process in 30-second chunks
                overlap=0.1,
                shifts=1
            )
            
            self.log_progress(20, "Model loaded", self.model_name, 0, 1)
            return True
        except Exception as e:
            self.log_error(f"Failed to initialize model: {str(e)}")
            return False
    
    def separate(self, input_file, output_folder, stems):
        """Separate audio file into stems"""
        try:
            if not self.separator:
                if not self.initialize():
                    return False
            
            input_path = Path(input_file)
            if not input_path.exists():
                self.log_error(f"Input file not found: {input_file}")
                return False
            
            output_path = Path(output_folder)
            output_path.mkdir(parents=True, exist_ok=True)
            
            self.log_progress(30, "Loading audio file", self.model_name, 0, 1)
            
            # Load and separate
            origin, separated = self.separator.separate_audio_file(input_file)
            
            self.log_progress(80, "Saving stems", self.model_name, 0, 1)
            
            # Save separated stems
            saved_count = 0
            for stem, source in separated.items():
                output_file = output_path / f"{stem}.wav"
                from demucs.api import save_audio
                save_audio(source, str(output_file), 
                          samplerate=self.separator.samplerate)
                saved_count += 1
                self.log_progress(80 + (saved_count * 2), "Saving stems", self.model_name, saved_count, len(separated))
            
            self.log_progress(100, "Separation complete", self.model_name, len(separated), len(separated))
            return True
        
        except Exception as e:
            self.log_error(f"Separation failed: {str(e)}")
            return False


def main():
    """Main entry point - reads arguments from command line"""
    if len(sys.argv) < 3:
        print(json.dumps({
            "type": "error",
            "message": "Usage: demucs_wrapper.py <input_file> <output_folder> [stems...]"
        }))
        sys.exit(1)
    
    input_file = sys.argv[1]
    output_folder = sys.argv[2]
    stems = sys.argv[3:] if len(sys.argv) > 3 else ["vocals", "drums", "bass", "other"]
    
    interface = DemucsIPCInterface()
    
    # Set up signal handlers for graceful shutdown
    import signal
    def signal_handler(sig, frame):
        print(json.dumps({
            "type": "cancelled",
            "message": "Separation cancelled by user"
        }))
        sys.exit(0)
    
    signal.signal(signal.SIGTERM, signal_handler)
    signal.signal(signal.SIGINT, signal_handler)
    
    # Perform separation
    if interface.separate(input_file, output_folder, stems):
        sys.exit(0)
    else:
        sys.exit(1)


if __name__ == "__main__":
    main()
