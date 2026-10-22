# Documentation: authoring and building

The offline manual contains separate user and developer sections. Its build is
independent of CMake and never compiles or runs the plugin or solver.

## Build requirements

- Python 3.10 or newer; use an independent virtual environment, not Cubit's Python.
- Doxygen 1.12.0 or newer, on `PATH` or passed with `--doxygen`.
- Python dependencies pinned in `requirements.txt`.

Windows (from the repository root):

```powershell
python -m venv docs/.venv
docs/.venv/Scripts/python.exe -m pip install -r docs/requirements.txt
docs/.venv/Scripts/python.exe docs/build.py --fetch-assets
```

Linux:

```sh
python3 -m venv docs/.venv
docs/.venv/bin/python -m pip install -r docs/requirements.txt
docs/.venv/bin/python docs/build.py --fetch-assets
```

`--fetch-assets` permits downloading the pinned KaTeX CSS/fonts archive. Its
SHA-256 and URL are recorded in `assets.json`. Once cached in `docs/.tools/`,
omit that flag to build without network access. An offline build machine can
receive that exact archive and install the Python requirements from a prepared
wheel cache. Doxygen is installed separately; there is no automatic system install.
To select a portable executable, use `--doxygen "path/to/doxygen"`.
Keep the DLLs from the Doxygen distribution beside its executable; on Windows,
the portable 1.12.0 archive includes the required `libclang.dll`.

For a release, add `--release 2026.10` (using the actual release being packaged).
Open `docs/_build/html/index.html` directly in a browser. Ship **all contents**
of `docs/_build/html/` as the release's `documentation/` folder. It contains local
styles, equation scripts/fonts, the search index, and journal/source downloads.
End users need none of the build tools or a web server. External manual citations
remain online links; they are not required for navigation or rendering.
Search uses the local index and shows linked results. Fetched context snippets
are disabled because browsers can reject page fetches under `file://`.

## Stage and ship the manual

Build the manual first, then explicitly stage it beside the plugin resources.
The CMake target `stage_documentation` copies existing HTML; it does not run
Python, Doxygen, a plugin compilation, or the solver. It is outside `ALL`, so
normal plugin builds have no documentation-generation dependency. Reconfigure
an existing build directory once to make the new target available.

Windows, from the repository root:

```powershell
docs/.venv/Scripts/python.exe docs/build.py --release 2026.10
cmake --build build --config Release --target stage_documentation
```

Linux:

```sh
docs/.venv/bin/python docs/build.py --release 2026.10
cmake --build build --target stage_documentation
```

Use the actual release label when packaging. The default destination is
`build/Release/documentation/` on Windows and `build/documentation/` on Linux,
following the current plugin resource layout. `CCX_DOCUMENTATION_SOURCE` and
`CCX_DOCUMENTATION_DESTINATION` are CMake cache paths for overriding the source
HTML tree and the destination. The destination names the documentation folder
itself, not its parent. For example, configure with
`-DCCX_DOCUMENTATION_DESTINATION=/release/plugin/documentation` for a separate
release directory. Use a clean release destination to avoid retaining obsolete
pages from an older manual; staging copies files without deleting unrelated files.

The script can also be run without configuring or building the plugin; supply
absolute paths (quote the complete `-D...` argument if it contains spaces):

```text
cmake -DCCX_DOCUMENTATION_SOURCE=<absolute-html-directory> -DCCX_DOCUMENTATION_DESTINATION=<absolute-plugin-directory>/documentation -P src/cmake/StageDocumentation.cmake
```

Ship the resulting layout together:

```text
plugin directory/
  calculix_plugin.dll          # Windows; libcalculix_plugin.so on Linux
  ...other plugin libraries and resources...
  documentation/
    index.html
    _static/
    _downloads/
    ...all other generated pages and assets...
```

After rebuilding the plugin with the Help action, use
**Help > Cubit-CalculiX documentation**. The action finds the plugin directory
through the component registry and opens its local `documentation/index.html`
using the desktop's configured HTML application. Missing or unreadable manuals
report the expected path; failure to submit the open request is also reported.
An accepted desktop request does not prove the browser finished rendering.
Menu initialization does not duplicate the action; cleanup removes only entries
owned by this plugin's documentation menu integration.

## Validate generated output

The build checks local page/download links and remote embedded resource URLs.
Run `python docs/build.py --check-only` to repeat those checks on existing output.
Before shipping, also open the pages via `file://` in supported browsers, disable
network access, and check search, equations, navigation, and downloads. The static
check does not execute JavaScript and does not replace that browser verification.
Doxygen warnings are saved to `docs/_build/doxygen-warnings.log`.

## Authoring conventions

- Write user workflows in `user/`, syntax/defaults in `commands/`, runnable-example
  explanations in `tutorials/`, and implementation/math in `developer/`.
- Both Markdown (MyST) and reStructuredText are supported.
- Reference maintained `.jou` files with Sphinx downloads or literal includes;
  do not maintain a second copy of a complete journal in a page.
- Put actual GUI screenshots in `images/`; record version and context in captions.
- Keep online CalculiX references version-specific. Describe the plugin's exported
  keywords and assumptions, rather than implying every solver feature is supported.
- Document APIs with Doxygen comments in project code. Add cross-references in
  `developer/api.rst`; existing declaration extraction is only initial coverage.
- Keep runtime assets local. Sphinx uses its bundled Alabaster theme with system
  fonts and the pinned KaTeX extension; KaTeX's CSS/fonts and license are packaged.
- Existing Mermaid fences currently display as source diagrams. Use local SVGs
  for rendered diagrams until a separate offline Mermaid build is introduced.

## Existing developer guides

Start here when changing Cubit-CalculiX:

1. [Architecture overview](architecture.md): runtime components and data flow.
2. [Source-code map](source-map.md): where to find each responsibility.
3. [Adding a feature](adding-a-feature.md): a walkthrough of the existing `loadssurfacetraction` implementation and the integration points for a similar new load.
