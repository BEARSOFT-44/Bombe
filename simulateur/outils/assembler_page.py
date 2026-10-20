#!/usr/bin/env python3
"""Insere le programme WebAssembly et la description du montage (boutons, buzzer) dans la page.
Usage : assembler_page.py sketch.ino bombe.wasm page.html sortie.html"""
import sys, re, json, base64, hashlib, datetime, os
ino, wasm, modele, sortie = sys.argv[1:5]
code = open(ino, encoding='utf-8', errors='replace').read()
consts = {m.group(1): int(m.group(2)) for m in re.finditer(r'(?:#define\s+(\w+)\s+(\d+))', code)}
consts.update({m.group(1): int(m.group(2)) for m in re.finditer(r'const\s+(?:int|byte|uint8_t)\s+(\w+)\s*=\s*(\d+)\s*;', code)})
def broche(v):
    v = v.strip()
    return int(v) if v.isdigit() else consts.get(v)
boutons = [{"nom": m.group(1), "broche": broche(m.group(2))} for m in re.finditer(r'EasyButton\s+(\w+)\s*\(\s*(\w+)', code)]
boutons = [b for b in boutons if b["broche"] is not None]
buzzer = next((v for k, v in consts.items() if 'buzz' in k.lower()), None)
octets = open(wasm, 'rb').read()
config = {
    "wasm": base64.b64encode(octets).decode(),
    "boutons": boutons,
    "buzzer": buzzer,
    "fichier": os.path.basename(ino),
    "empreinte": hashlib.sha256(code.encode('utf-8')).hexdigest()[:8],
    "date": datetime.datetime.now(datetime.timezone.utc).strftime('%Y-%m-%dT%H:%M:%SZ'),
    "lignes": code.count('\n') + 1,
}
page = open(modele, encoding='utf-8').read()
if '"__CONFIG__"' not in page: sys.exit("Marqueur __CONFIG__ absent de page.html")
open(sortie, 'w', encoding='utf-8').write(page.replace('"__CONFIG__"', json.dumps(config, ensure_ascii=False)))
print(f"Page creee : {sortie} ({len(octets)//1024} Ko de code, boutons {', '.join(b['nom'] + '=' + str(b['broche']) for b in boutons)}, buzzer={buzzer})")
