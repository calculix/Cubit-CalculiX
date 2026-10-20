#!/usr/bin/env python3
"""Build the offline manual on Windows/Linux without compiling the plugin."""
import argparse
import hashlib
from html.parser import HTMLParser
import importlib.metadata
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tarfile
import urllib.error
import urllib.parse
import urllib.request

DOCS = Path(__file__).resolve().parent
BUILD = DOCS / "_build"
TOOLS = DOCS / ".tools"


def prepare_math(fetch):
    specification = json.loads((DOCS / "assets.json").read_text(encoding="utf-8"))["katex"]
    TOOLS.mkdir(exist_ok=True)
    archive = TOOLS / ("katex-" + specification["version"] + ".tgz")
    if not archive.is_file():
        if not fetch:
            print("Missing cached KaTeX archive. Run once with --fetch-assets, or copy the pinned archive into docs/.tools/.")
            return False
        print("Downloading pinned KaTeX styles, fonts, and license...", flush=True)
        request = urllib.request.Request(specification["url"], headers={"User-Agent": "Cubit-CalculiX-docs"})
        with urllib.request.urlopen(request, timeout=60) as response:
            data = response.read()
        if hashlib.sha256(data).hexdigest() != specification["sha256"]:
            print("KaTeX download checksum mismatch.")
            return False
        archive.write_bytes(data)
    if hashlib.sha256(archive.read_bytes()).hexdigest() != specification["sha256"]:
        print("Cached KaTeX archive checksum mismatch: " + str(archive))
        return False
    destination = TOOLS / "static" / "vendor" / "katex"
    destination.mkdir(parents=True, exist_ok=True)
    # Extract only known asset names. Never use unrestricted tar extraction.
    with tarfile.open(archive, "r:gz") as package:
        for member in package.getmembers():
            if not member.isfile():
                continue
            name = member.name
            if name == "package/dist/katex.min.css":
                target = destination / "katex.min.css"
            elif name == "package/LICENSE":
                target = destination / "LICENSE"
            elif re.fullmatch(r"package/dist/fonts/[A-Za-z0-9_.-]+\.(woff2?|ttf)", name):
                target = destination / "fonts" / Path(name).name
            else:
                continue
            target.parent.mkdir(parents=True, exist_ok=True)
            with package.extractfile(member) as stream:
                target.write_bytes(stream.read())
    if not (destination / "katex.min.css").is_file() or not (destination / "LICENSE").is_file():
        print("KaTeX archive is missing required assets.")
        return False
    return True


class References(HTMLParser):
    def __init__(self):
        super().__init__(convert_charrefs=True)
        self.links = []

    def handle_starttag(self, tag, attrs):
        values = dict(attrs)
        for attribute in ("src", "href", "poster"):
            if values.get(attribute):
                resource = attribute != "href" or tag == "link"
                self.links.append((values[attribute], resource))


def check_html(folder):
    """Check shipped file links and embedded resources; online citations are OK."""
    problems = []
    pages = list(folder.rglob("*.html"))
    if not pages or not (folder / "searchindex.js").is_file():
        print("Missing HTML pages or search index.")
        return False
    options = folder / "_static" / "documentation_options.js"
    if not options.is_file() or "SHOW_SEARCH_SUMMARY: false" not in options.read_text(encoding="utf-8"):
        print("Search context fetching must be disabled for file:// distribution.")
        return False

    def check(source, reference, embedded):
        parsed = urllib.parse.urlsplit(reference)
        if parsed.scheme in ("http", "https") or parsed.netloc:
            if embedded:
                problems.append(str(source.relative_to(folder)) + ": remote resource " + reference)
            return
        if parsed.scheme in ("data", "mailto", "javascript") or not parsed.path:
            return
        target = (source.parent / urllib.parse.unquote(parsed.path)).resolve()
        if not target.is_relative_to(folder.resolve()) or not target.is_file():
            problems.append(str(source.relative_to(folder)) + ": missing/outside file " + reference)

    for page in pages:
        parser = References()
        parser.feed(page.read_text(encoding="utf-8"))
        for reference, embedded in parser.links:
            check(page, reference, embedded)
    for stylesheet in folder.rglob("*.css"):
        for reference in re.findall(r"url\(\s*['\"]?([^)'\"\s]+)", stylesheet.read_text(encoding="utf-8")):
            check(stylesheet, reference, True)
    for problem in sorted(set(problems)):
        print("ERROR: " + problem)
    if problems:
        return False
    print("Offline asset/file checks passed for %d HTML pages (including CSS font resources)." % len(pages))
    return True


def main():
    if os.name == "nt":
        # Report missing portable-Doxygen DLLs as command errors, not modal
        # Windows dialogs that block an unattended documentation build.
        import ctypes
        ctypes.windll.kernel32.SetErrorMode(0x0001 | 0x0002 | 0x8000)
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--doxygen", help="Doxygen executable; otherwise use DOXYGEN, PATH, or docs/.tools/doxygen/doxygen.exe")
    parser.add_argument("--fetch-assets", action="store_true", help="Allow downloading the pinned KaTeX archive when not cached")
    parser.add_argument("--release", default="development", help="Plugin release label displayed in the manual")
    parser.add_argument("--check-only", action="store_true", help="Check the existing generated HTML without building")
    args = parser.parse_args()
    if args.check_only:
        return 0 if check_html(BUILD / "html") else 1
    try:
        for package in ("Sphinx", "myst-parser", "breathe", "sphinxcontrib-katex"):
            importlib.metadata.version(package)
    except importlib.metadata.PackageNotFoundError:
        print("Install docs/requirements.txt using this Python interpreter first.")
        return 1
    doxygen = args.doxygen or os.environ.get("DOXYGEN") or shutil.which("doxygen")
    portable = TOOLS / "doxygen" / "doxygen.exe"
    if not doxygen and portable.is_file():
        doxygen = str(portable)
    if not doxygen:
        print("Doxygen is required. Install Doxygen 1.12.0+ or supply --doxygen /path/to/doxygen.")
        return 1
    try:
        if not prepare_math(args.fetch_assets):
            return 1
        BUILD.mkdir(exist_ok=True)
        version = subprocess.check_output([doxygen, "--version"], text=True).strip()
        print("Generating project API XML with Doxygen " + version, flush=True)
        result = subprocess.run([doxygen, str(DOCS / "Doxyfile")], cwd=DOCS, check=False)
        if result.returncode:
            return result.returncode
        if not (BUILD / "doxygen" / "xml" / "index.xml").is_file():
            print("Doxygen did not produce its XML index.")
            return 1
        result = subprocess.run([
            sys.executable, "-m", "sphinx", "-b", "html", "-E", "-W", "--keep-going",
            "-D", "release=" + args.release, "-D", "version=" + args.release,
            str(DOCS), str(BUILD / "html"),
        ], cwd=DOCS, check=False)
        if result.returncode:
            return result.returncode
        if not check_html(BUILD / "html"):
            return 1
        information = {"release": args.release, "doxygen": version, "python": sys.version.split()[0]}
        information["packages"] = {name: importlib.metadata.version(name) for name in ("Sphinx", "myst-parser", "breathe", "sphinxcontrib-katex")}
        (BUILD / "html" / "build-info.json").write_text(json.dumps(information, indent=2) + "\n", encoding="utf-8")
        print("Open " + str(BUILD / "html" / "index.html"))
        print("Ship the entire docs/_build/html directory, including _static and _downloads.")
        return 0
    except (OSError, ValueError, tarfile.TarError, subprocess.SubprocessError) as error:
        print("Documentation build failed: " + str(error))
        return 1


if __name__ == "__main__":
    sys.exit(main())
