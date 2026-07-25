# -*- coding: ascii -*-
#############################################################################
#  This file is part of the Lusan project, an official component of the Areg SDK.
#
#  (c) 2023-2026 Aregtech (Artak Avetyan).
#  file        lusan/res/logo/make-icons.py
#  brief       Rasterizes lusan.svg into the multi-resolution application
#              icon assets (PNG set, Windows .ico, macOS .icns).
#
#  The SVG uses SVG filter effects (glow, drop-shadow) that Qt's own SVG
#  renderer does not reproduce, so the master art is rasterized with headless
#  Chrome (a full SVG-filter renderer) at each target size, then packed into
#  the .ico / .icns container formats with a tiny pure-Python writer -- no
#  ImageMagick, Pillow, or other third-party dependency is required.
#
#  Usage:  python make-icons.py [path-to-chrome]
#          (run from this directory; regenerate after editing lusan.svg)
#############################################################################

import os
import struct
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
SVG_PATH = os.path.join(HERE, "lusan.svg")

# Every pixel size we ever rasterize; individual targets pick a subset below.
ALL_SIZES = [16, 24, 32, 48, 64, 128, 256, 512, 1024]

# Runtime QIcon (embedded in the Qt resource, used by QApplication::setWindowIcon).
PNG_SIZES = [16, 24, 32, 48, 64, 128, 256]

# Windows .exe icon (Explorer / taskbar / title bar).
ICO_SIZES = [16, 24, 32, 48, 64, 128, 256]

# macOS .icns bundle icon: (pixel size -> OSType). Only PNG-accepting types are used.
ICNS_TYPES = [
    (16, b"icp4"),
    (32, b"icp5"),
    (128, b"ic07"),
    (256, b"ic08"),
    (512, b"ic09"),
    (1024, b"ic10"),
]

CHROME_CANDIDATES = [
    r"C:\Program Files\Google\Chrome\Application\chrome.exe",
    r"C:\Program Files (x86)\Google\Chrome\Application\chrome.exe",
    r"C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe",
    r"C:\Program Files\Microsoft\Edge\Application\msedge.exe",
    "google-chrome",
    "chromium",
    "chromium-browser",
]


def find_chrome():
    if len(sys.argv) > 1:
        return sys.argv[1]
    for cand in CHROME_CANDIDATES:
        if os.path.sep in cand:
            if os.path.isfile(cand):
                return cand
        else:
            return cand
    raise SystemExit("No Chrome/Chromium/Edge found; pass its path as an argument.")


def render_png(chrome, svg_text, size, workdir):
    """Rasterize the SVG at size x size on a transparent background via headless Chrome."""
    html = (
        "<!doctype html><html><head><meta charset=\"utf-8\"><style>"
        "html,body{margin:0;padding:0;background:transparent;}"
        "svg{display:block;width:%dpx;height:%dpx;}"
        "</style></head><body>%s</body></html>" % (size, size, svg_text)
    )
    html_path = os.path.join(workdir, "s%d.html" % size)
    png_path = os.path.join(workdir, "s%d.png" % size)
    with open(html_path, "w", encoding="ascii") as f:
        f.write(html)

    profile = os.path.join(workdir, "profile%d" % size)
    cmd = [
        chrome,
        "--headless=new",
        "--disable-gpu",
        "--hide-scrollbars",
        "--force-device-scale-factor=1",
        "--default-background-color=00000000",
        "--user-data-dir=" + profile,
        "--window-size=%d,%d" % (size, size),
        "--screenshot=" + png_path,
        html_path,
    ]
    subprocess.run(cmd, check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

    with open(png_path, "rb") as f:
        data = f.read()
    if data[:8] != b"\x89PNG\r\n\x1a\n":
        raise SystemExit("Chrome did not produce a PNG for size %d" % size)
    w, h = struct.unpack(">II", data[16:24])
    if (w, h) != (size, size):
        raise SystemExit("Rendered size %dx%d != requested %d" % (w, h, size))
    return data


def build_ico(pngs):
    """Pack {size: png_bytes} into a Vista-style PNG-compressed .ico."""
    entries = sorted(ICO_SIZES)
    count = len(entries)
    header = struct.pack("<HHH", 0, 1, count)
    dir_blob = b""
    data_blob = b""
    offset = 6 + 16 * count
    for size in entries:
        png = pngs[size]
        b = size if size < 256 else 0
        dir_blob += struct.pack("<BBBBHHII", b, b, 0, 0, 1, 32, len(png), offset)
        data_blob += png
        offset += len(png)
    return header + dir_blob + data_blob


def build_icns(pngs):
    """Pack the ICNS_TYPES entries into an .icns container of PNG elements."""
    body = b""
    for size, ostype in ICNS_TYPES:
        png = pngs[size]
        body += ostype + struct.pack(">I", 8 + len(png)) + png
    return b"icns" + struct.pack(">I", 8 + len(body)) + body


def main():
    chrome = find_chrome()
    with open(SVG_PATH, "r", encoding="ascii") as f:
        svg_text = f.read()

    pngs = {}
    workdir = tempfile.mkdtemp(prefix="lusan-icons-")
    for size in ALL_SIZES:
        print("rendering %dx%d ..." % (size, size))
        pngs[size] = render_png(chrome, svg_text, size, workdir)

    for size in PNG_SIZES:
        out = os.path.join(HERE, "app-logo-%d.png" % size)
        with open(out, "wb") as f:
            f.write(pngs[size])
        print("wrote", os.path.basename(out))

    # A single 256 PNG for Linux hicolor installs.
    with open(os.path.join(HERE, "lusan-256.png"), "wb") as f:
        f.write(pngs[256])
    print("wrote lusan-256.png")

    with open(os.path.join(HERE, "lusan.ico"), "wb") as f:
        f.write(build_ico(pngs))
    print("wrote lusan.ico")

    with open(os.path.join(HERE, "lusan.icns"), "wb") as f:
        f.write(build_icns(pngs))
    print("wrote lusan.icns")

    print("done")


if __name__ == "__main__":
    main()
