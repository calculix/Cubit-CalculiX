"""Check project-owned C++ for conditional operators without building.

Bundled dependencies under src/Utility and src/Library are excluded.
Strings (including raw strings), character literals and comments are skipped.
"""
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
SOURCE_ROOTS = ("Framework", "CalculiX", "Postprocessor", "Report", "Solvers")
EXTENSIONS = {".c", ".cc", ".cpp", ".cxx", ".h", ".hpp", ".hxx", ".inl", ".tpp"}
TOKENS = re.compile(
    r'R"(?P<delimiter>[^ ()\\\t\r\n]{0,16})\(.*?\)(?P=delimiter)"'
    r'|"(?:\\.|[^"\\])*"'
    r"|'(?:\\.|[^'\\])*'"
    r'|//[^\r\n]*|/\*.*?\*/|(?P<conditional>\?)',
    re.DOTALL,
)


def conditional_lines(source):
    for token in TOKENS.finditer(source):
        if token.group("conditional") is not None:
            yield source.count("\n", 0, token.start()) + 1


def main():
    failures = 0
    files = 0
    for directory in SOURCE_ROOTS:
        for path in sorted((ROOT / "src" / directory).rglob("*")):
            if path.is_file() and path.suffix.lower() in EXTENSIONS:
                files += 1
                source = path.read_text(encoding="utf-8-sig")
                for line in conditional_lines(source):
                    print(f"{path.relative_to(ROOT)}:{line}: conditional operator requires explicit if/else")
                    failures += 1
    if failures:
        print(f"Found {failures} conditional operators in {files} project-owned files.")
        return 1
    print(f"No conditional operators in {files} project-owned C++ files.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
