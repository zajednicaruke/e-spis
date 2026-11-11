import re

with open("src/pdf.c", "r") as f:
    content = f.read()

# Zamjena standardnog font objekta s onim koji sadrži /Differences array
old_font_obj = 'fprintf(f, "4 0 obj\\n<</Type/Font/Subtype/Type1/BaseFont/Helvetica/Encoding/WinAnsiEncoding>>\\nendobj\\n");'
new_font_obj = '''fprintf(f, "4 0 obj\\n<</Type/Font/Subtype/Type1/BaseFont/Helvetica/Encoding<</Type/Encoding/BaseEncoding/WinAnsiEncoding/Differences[128/Ccaron/ccaron/Cacute/cacute/Dcroat/dcroat/Scaron/scaron/Zcaron/zcaron]>>>>\\nendobj\\n");'''

if old_font_obj in content:
    content = content.replace(old_font_obj, new_font_obj)
    with open("src/pdf.c", "w") as f:
        f.write(content)
    print("[OK] Uspješno ugrađen /Differences array u src/pdf.c!")
else:
    print("[INFO] Font objekt već modificiran ili ima drugačiji format.")
