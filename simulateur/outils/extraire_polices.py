#!/usr/bin/env python3
"""Extrait de u8g2_fonts.c uniquement les polices utilisees par le sketch.
Usage : extraire_polices.py sketch.ino fichier_sortie.c [u8g2_fonts.c]
Si u8g2_fonts.c n'est pas fourni, il est telecharge depuis GitHub (version figee)."""
import re, sys, urllib.request, os
COMMIT = "d6c8499"
sketch, sortie = sys.argv[1], sys.argv[2]
noms = sorted(set(re.findall(r'\bu8g2_font_\w+', open(sketch, encoding='utf-8', errors='replace').read())))
cache = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'vendor', 'polices_cache.c')
# polices deja extraites lors d'une construction precedente
connues = {}
def lire(texte):
    for m in re.finditer(r'(const uint8_t (u8g2_font_\w+)\[\d+\] U8[GX][28X]?_FONT_SECTION\("\w+"\) = \n(?:  ".*"\n)*?  ".*";\n)', texte):
        connues[m.group(2)] = m.group(1)
if os.path.exists(cache): lire(open(cache, encoding='latin-1').read())
manque = [n for n in noms if n not in connues]
garder = set(connues) | set(noms)
if manque:
    if len(sys.argv) > 3: src = open(sys.argv[3], encoding='latin-1').read()
    else:
        url = f"https://raw.githubusercontent.com/olikraus/u8g2/{COMMIT}/csrc/u8g2_fonts.c"
        print("Telechargement des polices u8g2 :", ", ".join(manque))
        src = urllib.request.urlopen(url).read().decode('latin-1')
    lire(src)
    manque = [n for n in noms if n not in connues]
    if manque: sys.exit("Police(s) introuvable(s) dans u8g2 : " + ", ".join(manque))
    open(cache, 'w', encoding='latin-1').write('#include "u8g2.h"\n' + "".join(connues[k] for k in sorted(garder)))
open(sortie, 'w', encoding='latin-1').write('#include "u8g2.h"\n' + "".join(connues[n] for n in noms))
print("Polices :", ", ".join(noms))
