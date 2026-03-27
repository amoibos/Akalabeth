#!/usr/bin/env python3
"""
SDCC MAP File Analyzer
Analysiert SDCC .map Dateien und zeigt ROM/RAM Nutzung an
"""

import re
import sys
from collections import defaultdict

sys.stdout.reconfigure(encoding='utf-8')
from typing import Dict, List, Tuple

class SDCCMapParser:
    def __init__(self, filename: str):
        self.filename = filename
        self.areas = {}
        self.symbols = []
        self.functions = []
        
    def parse(self):
        """Parse die MAP Datei"""
        with open(self.filename, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()
        
        # Parse Areas
        self._parse_areas(content)
        
        # Parse Symbols/Functions
        self._parse_functions(content)
        
    def _parse_areas(self, content: str):
        """Extrahiere Area-Informationen"""
        # Pattern für Area-Definitionen
        area_pattern = r'^([._A-Z0-9]+)\s+([0-9A-F]{8})\s+([0-9A-F]{8})\s+=\s+(\d+)\.\s+bytes'
        
        for line in content.split('\n'):
            match = re.match(area_pattern, line.strip())
            if match:
                name = match.group(1)
                addr = int(match.group(2), 16)
                size = int(match.group(4))
                
                self.areas[name] = {
                    'addr': addr,
                    'size': size
                }
    
    def _parse_functions(self, content: str):
        """Extrahiere Funktionen und ihre Adressen"""
        # Pattern für Funktionsdefinitionen
        func_pattern = r'^\s+([0-9A-F]{8})\s+(_[a-zA-Z0-9_]+)\s+(\w+)'
        
        lines = content.split('\n')
        current_area = None
        
        for line in lines:
            # Erkenne Area-Abschnitte
            if 'Area' in line and 'Addr' in line:
                continue
            if line.strip().startswith('_CODE') or line.strip().startswith('_HOME'):
                area_match = re.match(r'^([._A-Z0-9]+)\s+([0-9A-F]{8})', line.strip())
                if area_match:
                    current_area = area_match.group(1)
            
            # Parse Funktionen
            match = re.match(func_pattern, line)
            if match and current_area:
                addr = int(match.group(1), 16)
                name = match.group(2)
                module = match.group(3)
                
                self.functions.append({
                    'name': name,
                    'addr': addr,
                    'module': module,
                    'area': current_area
                })
    
    def calculate_function_sizes(self) -> List[Dict]:
        """Berechne Funktionsgrößen aus Adressdifferenzen"""
        # Sortiere nach Adresse
        sorted_funcs = sorted(self.functions, key=lambda x: x['addr'])
        
        func_sizes = []
        for i, func in enumerate(sorted_funcs):
            if i < len(sorted_funcs) - 1:
                next_func = sorted_funcs[i + 1]
                # Nur wenn beide in der gleichen Area sind
                if func['area'] == next_func['area']:
                    size = next_func['addr'] - func['addr']
                    if size > 0 and size < 100000:  # Sanity check
                        func_sizes.append({
                            'name': func['name'],
                            'module': func['module'],
                            'addr': func['addr'],
                            'size': size
                        })
        
        return sorted(func_sizes, key=lambda x: x['size'], reverse=True)
    
    def get_module_sizes(self) -> Dict[str, int]:
        """Gruppiere Funktionsgrößen nach Modulen"""
        func_sizes = self.calculate_function_sizes()
        module_sizes = defaultdict(int)
        
        for func in func_sizes:
            module_sizes[func['module']] += func['size']
        
        return dict(sorted(module_sizes.items(), key=lambda x: x[1], reverse=True))
    
    def print_summary(self):
        """Drucke Zusammenfassung auf die Konsole"""
        print("=" * 80)
        print("SDCC SPEICHER-ANALYSE")
        print("=" * 80)
        print()
        
        # ROM Zusammenfassung
        print("ROM (CODE-SPEICHER):")
        print("-" * 80)
        
        rom_areas = {k: v for k, v in self.areas.items() 
                     if k in ['_CODE', '_HOME', '_INITIALIZER', '_GSINIT', '_GSFINAL']}
        
        total_rom = 0
        for name, info in sorted(rom_areas.items(), key=lambda x: x[1]['size'], reverse=True):
            size = info['size']
            total_rom += size
            print(f"  {name:20s}: {size:6,d} Bytes  ({size/1024:6.2f} KB)  @0x{info['addr']:04X}")
        
        print(f"  {'─' * 78}")
        print(f"  {'GESAMT ROM':20s}: {total_rom:6,d} Bytes  ({total_rom/1024:6.2f} KB)")
        print()
        
        # RAM Zusammenfassung
        print("RAM (DATEN-SPEICHER):")
        print("-" * 80)
        
        ram_areas = {k: v for k, v in self.areas.items() 
                     if k in ['_DATA', '_INITIALIZED', '_BSS', '_HEAP', '_BSEG']}
        
        total_ram = 0
        for name, info in sorted(ram_areas.items(), key=lambda x: x[1]['size'], reverse=True):
            size = info['size']
            total_ram += size
            status = "" if size > 0 else " (nicht genutzt)"
            print(f"  {name:20s}: {size:6,d} Bytes  ({size/1024:6.2f} KB)  @0x{info['addr']:04X}{status}")
        
        print(f"  {'─' * 78}")
        print(f"  {'GESAMT RAM':20s}: {total_ram:6,d} Bytes  ({total_ram/1024:6.2f} KB)")
        print()
        
    def print_module_sizes(self, top_n: int = 20):
        """Drucke Modul-Größen"""
        print("ROM-NUTZUNG NACH MODULEN:")
        print("-" * 80)
        
        module_sizes = self.get_module_sizes()
        total = sum(module_sizes.values())
        
        for i, (module, size) in enumerate(list(module_sizes.items())[:top_n], 1):
            percentage = (size / total * 100) if total > 0 else 0
            bar_length = int(percentage / 2)  # Max 50 chars
            bar = '█' * bar_length
            print(f"  {i:2d}. {module:20s}: {size:6,d} Bytes  {percentage:5.1f}%  {bar}")
        
        print()
    
    def print_largest_functions(self, top_n: int = 20):
        """Drucke größte Funktionen"""
        print(f"GRÖSSTE FUNKTIONEN (Top {top_n}):")
        print("-" * 80)
        print(f"  {'Nr':<4} {'Funktion':<40} {'Modul':<15} {'Adresse':<10} {'Größe':>10}")
        print("-" * 80)
        
        func_sizes = self.calculate_function_sizes()
        
        for i, func in enumerate(func_sizes[:top_n], 1):
            print(f"  {i:<4} {func['name']:<40} {func['module']:<15} "
                  f"0x{func['addr']:04X}    {func['size']:6,d} B")
        
        print()
    
    def print_all_areas(self):
        """Drucke alle Areas"""
        print("ALLE SPEICHERBEREICHE:")
        print("-" * 80)
        print(f"  {'Bereich':<20} {'Start':<12} {'Größe':>12} {'Größe (KB)':>12}")
        print("-" * 80)
        
        for name, info in sorted(self.areas.items(), key=lambda x: x[1]['size'], reverse=True):
            if info['size'] > 0:
                print(f"  {name:<20} 0x{info['addr']:08X}  {info['size']:10,d}  {info['size']/1024:10.2f}")
        
        print()


def main():
    """Hauptfunktion"""
    if len(sys.argv) < 2:
        print("Usage: python sdcc_map_parser.py <mapfile.map>")
        sys.exit(1)
    
    mapfile = sys.argv[1]
    
    try:
        parser = SDCCMapParser(mapfile)
        parser.parse()
        
        # Ausgabe
        parser.print_summary()
        parser.print_module_sizes(top_n=15)
        parser.print_largest_functions(top_n=15)
        parser.print_all_areas()
        
    except FileNotFoundError:
        print(f"Fehler: Datei '{mapfile}' nicht gefunden!")
        sys.exit(1)
    except Exception as e:
        print(f"Fehler beim Parsen: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)


if __name__ == "__main__":
    main()