#!/usr/bin/env python3
"""
MIDI zu VGM Konverter für TI SN76489 Sound Chip
Konvertiert MIDI-Dateien in VGM-Format für den SN76489 PSG

Abhängigkeiten:
pip install mido

Verwendung:
python midi_to_vgm.py input.mid output.vgm
"""

import mido
import struct
import sys
import math

class SN76489:
    """SN76489 Sound Generator Emulation"""
    
    def __init__(self):
        self.channels = [
            {'frequency': 0, 'volume': 15, 'active': False},  # Kanal 0
            {'frequency': 0, 'volume': 15, 'active': False},  # Kanal 1  
            {'frequency': 0, 'volume': 15, 'active': False},  # Kanal 2
            {'noise_type': 0, 'volume': 15, 'active': False}  # Rauschkanal 3
        ]
        self.master_clock = 3579545  # Standard SN76489 Clock (3.58 MHz)
    
    def midi_note_to_frequency(self, note):
        """Konvertiert MIDI-Note zu SN76489 Frequenz-Register-Wert"""
        if note < 0 or note > 127:
            return 0
        
        # MIDI Note zu Hz: f = 440 * 2^((note-69)/12)
        freq_hz = 440.0 * (2.0 ** ((note - 69) / 12.0))
        
        # SN76489 Frequenz Register: f = Clock / (32 * N)
        # Also: N = Clock / (32 * f)
        n = int(self.master_clock / (32 * freq_hz))
        
        # N muss zwischen 1 und 1023 liegen (10-bit)
        return max(1, min(1023, n))
    
    def midi_velocity_to_volume(self, velocity):
        """Konvertiert MIDI Velocity zu SN76489 Volume (0-15, wobei 15 = stumm)"""
        if velocity == 0:
            return 15  # Stumm
        
        # Lineare Konvertierung: 127 velocity -> 0 volume, 1 velocity -> 14 volume
        return max(0, min(15, 15 - int((velocity / 127.0) * 15)))

class VGMWriter:
    """VGM Datei Writer für SN76489"""
    
    def __init__(self):
        self.commands = []
        self.total_samples = 0
        self.vgm_version = 0x161  # VGM Version 1.61
        self.sn76489_clock = 3579545
        self._first_note_written = False   # Stille am Anfang überspringen
        self._pending_samples = 0          # Gepufferte Samples (Stille am Ende entfernen)
        
    def add_command(self, command, data=None):
        """Fügt einen VGM-Befehl hinzu"""
        if data is not None:
            self.commands.append(struct.pack('BB', command, data))
        else:
            self.commands.append(struct.pack('B', command))
    
    def _flush_pending_wait(self):
        """Schreibt gepufferte Wartezeit tatsächlich in den Stream"""
        samples = self._pending_samples
        self._pending_samples = 0
        if samples == 0:
            return

        self.total_samples += samples

        while samples > 735:
            self.commands.append(struct.pack('<BH', 0x61, 735))
            self.total_samples += 735
            samples -= 735
        if samples > 0:
            self.total_samples += samples
            if samples <= 16:
                self.add_command(0x70 + samples - 1)
            else:
                self.commands.append(struct.pack('<BH', 0x61, samples))

    def add_wait(self, samples):
        """Fügt eine Wartepause hinzu (in 44.1kHz Samples).
        Stille vor der ersten Note wird übersprungen.
        Stille nach der letzten Note wird gepuffert und erst beim
        nächsten Sound-Befehl tatsächlich geschrieben."""
        if samples == 0:
            return
        if not self._first_note_written:
            # Führende Stille verwerfen
            return
        # Stille puffern – wird beim nächsten add_sn76489_write() geleert
        self._pending_samples += samples
    
    def add_sn76489_write(self, data):
        """Schreibt Daten an den SN76489"""
        # Gepufferte Wartezeit vor dem nächsten Sound-Befehl ausgeben
        self._first_note_written = True
        self._flush_pending_wait()
        self.add_command(0x50, data)
    
    def write_vgm_file(self, filename):
        """Schreibt die VGM-Datei"""
        with open(filename, 'wb') as f:
            # VGM Header
            f.write(b'Vgm ')  # VGM Signatur
            f.write(struct.pack('<I', 0))  # EOF Offset (wird später gesetzt)
            f.write(struct.pack('<I', self.vgm_version))  # Version
            f.write(struct.pack('<I', self.sn76489_clock))  # SN76489 Clock
            f.write(struct.pack('<I', 0))  # YM2413 Clock
            f.write(struct.pack('<I', 0))  # GD3 Offset
            f.write(struct.pack('<I', self.total_samples))  # Total Samples
            f.write(struct.pack('<I', 0))  # Loop Offset
            f.write(struct.pack('<I', 0))  # Loop Samples
            f.write(struct.pack('<I', 60))  # Rate (60 Hz)
            f.write(struct.pack('<H', 0))  # SN FB
            f.write(struct.pack('<B', 0))   # SN W
            f.write(struct.pack('<B', 0))   # SF
            
            # Padding bis Offset 0x40 (64)
            f.write(b'\x00' * (0x40 - f.tell()))
            
            # VGM Data
            vgm_data_start = f.tell()
            for command in self.commands:
                f.write(command)
            
            # End of Sound Data
            f.write(struct.pack('B', 0x66))
            
            # EOF Offset im Header setzen
            eof_pos = f.tell()
            f.seek(4)
            f.write(struct.pack('<I', eof_pos - 4))

def convert_midi_to_vgm(midi_file, vgm_file):
    """Konvertiert eine MIDI-Datei zu VGM für SN76489"""
    
    try:
        mid = mido.MidiFile(midi_file)
    except Exception as e:
        print(f"Fehler beim Laden der MIDI-Datei: {e}")
        return False
    
    sn = SN76489()
    vgm = VGMWriter()
    
    channel_instruments = {}  # MIDI-Kanal → Instrument
    drum_channel = None       # Kanal für Schlagzeug

    # Dynamische Voice-Allocation:
    # voice_alloc[sn_channel] = (midi_channel, note) oder None
    NUM_TONE_CHANNELS = 3
    voice_alloc = [None] * NUM_TONE_CHANNELS  # SN-Kanal 0-2

    def find_free_voice():
        for i, v in enumerate(voice_alloc):
            if v is None:
                return i
        return -1

    def find_voice(midi_channel, note):
        for i, v in enumerate(voice_alloc):
            if v is not None and v[0] == midi_channel and v[1] == note:
                return i
        return -1

    def steal_voice():
        """Nimmt den ersten belegten Kanal (simpelstes Voice Stealing)."""
        for i, v in enumerate(voice_alloc):
            if v is not None:
                return i
        return 0

    current_tempo = 500000  # Default: 120 BPM (500000 microseconds per beat)
    
    print(f"Konvertiere {midi_file} zu {vgm_file}...")
    print(f"MIDI Ticks per Beat: {mid.ticks_per_beat}")
    
    # Alle Tracks zu einem einzigen Stream zusammenführen
    messages = []
    for i, track in enumerate(mid.tracks):
        absolute_time = 0
        for msg in track:
            absolute_time += msg.time
            if not msg.is_meta or msg.type in ['set_tempo']:
                messages.append((absolute_time, msg, i))
    
    # Nach Zeit sortieren
    messages.sort(key=lambda x: x[0])
    
    print(f"Gefundene Messages: {len(messages)}")
    
    # Alle MIDI-Events durchgehen
    last_time = 0
    for absolute_time, msg, track_num in messages:
        # Zeit vorwärts bewegen
        delta_time = absolute_time - last_time
        if delta_time > 0:
            # Konvertiere MIDI-Ticks zu VGM-Samples (44.1kHz)
            # Korrekte Tempo-Berechnung: seconds = ticks * (microseconds_per_beat / 1000000) / ticks_per_beat
            seconds = (delta_time * current_tempo) / (mid.ticks_per_beat * 1000000.0)
            samples = int(seconds * 44100)
            if samples > 0:
                vgm.add_wait(samples)
            last_time = absolute_time
        
        # Note On Events
        if msg.type == 'note_on' and msg.velocity > 0:
            midi_channel = msg.channel

            # Spezielle Behandlung für Schlagzeug-Kanal
            if midi_channel == 9 or midi_channel == drum_channel:
                if msg.note >= 35 and msg.note <= 41:
                    noise_type = 0x07
                elif msg.note >= 42 and msg.note <= 54:
                    noise_type = 0x06
                else:
                    noise_type = 0x05
                vgm.add_sn76489_write(0xE0 | noise_type)
                volume = sn.midi_velocity_to_volume(msg.velocity)
                vgm.add_sn76489_write(0xF0 | volume)
                print(f"Drum: Note {msg.note}, Noise Type 0x{noise_type:02X}, Vol {volume}")
                continue

            # Freien SN-Kanal finden (oder stehlen)
            sn_channel = find_free_voice()
            if sn_channel == -1:
                sn_channel = steal_voice()
                # Gestohlenen Kanal zuerst stumm schalten
                vgm.add_sn76489_write(0x90 | (sn_channel << 5) | 0x0F)
                voice_alloc[sn_channel] = None

            voice_alloc[sn_channel] = (midi_channel, msg.note)

            # Instrument-spezifische Lautstärke-Anpassung
            instrument = channel_instruments.get(midi_channel, 0)
            velocity_modifier = 1.0
            if instrument >= 40 and instrument <= 47:
                velocity_modifier = 0.8
            elif instrument >= 56 and instrument <= 63:
                velocity_modifier = 1.2
            elif instrument >= 80 and instrument <= 87:
                velocity_modifier = 1.1
            adjusted_velocity = min(127, int(msg.velocity * velocity_modifier))

            # Frequenz setzen (2 Bytes)
            freq_val = sn.midi_note_to_frequency(msg.note)
            freq_low  = freq_val & 0x0F
            freq_high = (freq_val >> 4) & 0x3F
            vgm.add_sn76489_write(0x80 | (sn_channel << 5) | freq_low)
            vgm.add_sn76489_write(freq_high)

            # Lautstärke setzen
            volume = sn.midi_velocity_to_volume(adjusted_velocity)
            vgm.add_sn76489_write(0x90 | (sn_channel << 5) | volume)

            sn.channels[sn_channel]['active'] = True
            instr_name = channel_instruments.get(midi_channel, "Piano")
            print(f"Note On: CH{midi_channel}→SN{sn_channel}, Note {msg.note}, Vel {msg.velocity}→{adjusted_velocity}, Instr {instr_name}")
        
        # Note Off Events
        elif msg.type == 'note_off' or (msg.type == 'note_on' and msg.velocity == 0):
            midi_channel = msg.channel

            # Schlagzeug Note Off
            if midi_channel == 9 or midi_channel == drum_channel:
                vgm.add_sn76489_write(0xF0 | 0x0F)
                continue

            sn_channel = find_voice(midi_channel, msg.note)
            if sn_channel != -1:
                vgm.add_sn76489_write(0x90 | (sn_channel << 5) | 0x0F)
                voice_alloc[sn_channel] = None
                sn.channels[sn_channel]['active'] = False
                print(f"Note Off: CH{midi_channel}→SN{sn_channel}, Note {msg.note}")
        
        # Program Change Events (Instrument-Wechsel)
        elif msg.type == 'program_change':
            midi_channel = msg.channel
            instrument = msg.program
            channel_instruments[midi_channel] = instrument
            
            # Bekannte Instrument-Namen für Debugging
            instrument_names = {
                0: "Acoustic Grand Piano", 1: "Bright Acoustic Piano", 8: "Celesta",
                24: "Acoustic Guitar", 25: "Electric Guitar", 32: "Acoustic Bass",
                40: "Violin", 56: "Trumpet", 64: "Soprano Sax", 80: "Lead 1 (square)",
                81: "Lead 2 (sawtooth)", 96: "FX 1 (rain)", 120: "Reverse Cymbal"
            }
            
            instr_name = instrument_names.get(instrument, f"Unknown ({instrument})")
            print(f"Program Change: CH{midi_channel} → Instrument {instrument} ({instr_name})")
            
            # Spezielle Behandlung für bestimmte Instrumente
            if midi_channel == 9:  # Standard MIDI Drum Channel
                drum_channel = midi_channel
                print(f"  → Drum Channel erkannt")
            elif instrument >= 80 and instrument <= 87:  # Lead Synthesizer
                print(f"  → Lead Synth erkannt (gut für PSG)")
            elif instrument >= 120:  # Sound Effects
                print(f"  → Sound Effect (möglich für Rauschkanal)")
        
        # Control Change Events (Volume, Pan, etc.)
        elif msg.type == 'control_change':
            if msg.control == 7:  # Channel Volume
                print(f"Volume Change: CH{msg.channel} → {msg.value}")
            elif msg.control == 10:  # Pan
                print(f"Pan Change: CH{msg.channel} → {msg.value} (ignoriert - PSG ist mono)")
        
        # Tempo Change Events
        elif msg.type == 'set_tempo':
            current_tempo = msg.tempo  # Mikrosekunden pro Beat
            tempo_bpm = 60000000 / msg.tempo
            print(f"Tempo Change at {absolute_time}: {tempo_bpm:.1f} BPM ({msg.tempo} µs/beat)")
    
    # VGM-Datei schreiben
    vgm.write_vgm_file(vgm_file)
    
    print(f"Konvertierung abgeschlossen!")
    print(f"Total Samples: {vgm.total_samples}")
    print(f"Spieldauer: {vgm.total_samples / 44100:.2f} Sekunden")
    print(f"Verwendete SN76489 Tonkanäle: {NUM_TONE_CHANNELS}")
    
    return True

def main():
    if len(sys.argv) != 3:
        print("Verwendung: python midi_to_vgm.py <input.mid> <output.vgm>")
        sys.exit(1)
    
    input_file = sys.argv[1]
    output_file = sys.argv[2]
    
    if not convert_midi_to_vgm(input_file, output_file):
        sys.exit(1)
    
    print(f"VGM-Datei erfolgreich erstellt: {output_file}")

if __name__ == "__main__":
    main()