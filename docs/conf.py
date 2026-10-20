"""Offline manual configuration; generated independently of the plugin."""
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parent
REPOSITORY = ROOT.parent

project = "Cubit-CalculiX"
author = "Cubit-CalculiX contributors"
copyright = "Cubit-CalculiX contributors"
release = "development"
language = "en"
extensions = ["myst_parser", "breathe", "sphinxcontrib.katex"]
source_suffix = {".rst": "restructuredtext", ".md": "markdown"}
exclude_patterns = ["_build", ".venv", ".tools", "images/README.md"]
root_doc = "index"
nitpicky = False  # SDK/Qt declarations are outside this API extraction.
myst_enable_extensions = ["colon_fence", "dollarmath"]
myst_heading_anchors = 3
highlight_language = "text"

breathe_projects = {"Cubit-CalculiX": str(ROOT / "_build/doxygen/xml")}
breathe_default_project = "Cubit-CalculiX"
breathe_default_members = ()

# Alabaster is bundled with Sphinx; use system fonts and local assets only.
html_theme = "alabaster"
html_theme_options = {"description": "User and developer documentation", "fixed_sidebar": True}
html_sidebars = {"**": ["about.html", "navigation.html", "relations.html", "searchbox.html"]}
html_static_path = ["_static", ".tools/static"]
html_css_files = ["manual.css"]
html_title = "Cubit-CalculiX documentation"
html_show_sourcelink = False
html_show_sphinx = False
html_copy_source = False
# Search uses its local JS index. Context snippets use fetch() on HTML pages,
# which browsers can reject for file://; keep title/link results fully local.
html_show_search_summary = False
html_last_updated_fmt = None
html_baseurl = ""
katex_css_path = "vendor/katex/katex.min.css"
# JS and auto-render are included in the pinned sphinxcontrib-katex wheel.
katex_options = '{"throwOnError": true}'


def localize_existing_links(app, docname, source):
    """Keep existing Markdown guides usable as both repo files and offline pages.

    Source-file links outside docs become Sphinx downloads. Directory links
    point to the appropriate manual section instead of outside the shipped tree.
    """
    if docname not in {"architecture", "source-map", "adding-a-feature"}:
        return

    def replace(match):
        label, target = match.groups()
        if ":" in target or target.startswith("#"):
            return match.group(0)
        path = (ROOT / target).resolve()
        if not path.is_relative_to(REPOSITORY) or path.is_relative_to(ROOT):
            return match.group(0)
        if path.is_file():
            return "{download}`" + label + " <" + target + ">`"
        if path.is_dir():
            page = "tutorials/index" if target.startswith(("../examples", "../journal_files")) else "developer/api"
            return "[" + label + "](" + page + ")"
        return match.group(0)

    source[0] = re.sub(r"(?<!!)\[([^\]]+)\]\(([^)]+)\)", replace, source[0])


def setup(app):
    # Preserve the existing diagrams as readable source without a remote renderer.
    from pygments.lexers.special import TextLexer
    app.add_lexer("mermaid", TextLexer)
    app.connect("source-read", localize_existing_links)
