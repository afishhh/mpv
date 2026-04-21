from pathlib import Path
from wcwidth import wcswidth
import re

raw = Path("./test.html")
reference = Path("./spec.html")


def extract_cases(text: bytes):
    RE = re.compile(rb"<dt>(.*?)</dt>\s*<dd>(.*?)</dd>")

    for m in RE.findall(text):
        yield m


for raw, ref in zip(
    extract_cases(raw.read_bytes()), extract_cases(reference.read_bytes()), strict=True
):
    assert raw[0] == ref[0]
    name = raw[0]
    raw = raw[1]
    ref = ref[1]
    print(f"    // {name.decode()}")
    raw_literal = "".join(("\\" + hex(byte)[1:] for byte in raw))
    ref_width = wcswidth(ref.decode())
    print(f'    assert_int_equal(W("{raw_literal}"), {ref_width});')
