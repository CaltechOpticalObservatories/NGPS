# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = 'NGPS'
copyright = '2026, Caltech Optical Observatories (COO)'
author = 'Caltech Optical Observatories (COO)'
release = '1.0'
html_theme = "shibuya"
html_theme_options = {
    "globaltoc_expand_depth": 1,
    "toctree_maxdepth": 1,
    "toctree_titles_only": True,
    "toctree_collapse": True,
}
html_static_path = ["_static"]
html_favicon = "_static/favicon.ico"

from pathlib import Path

DOCS_SOURCE = Path(__file__).resolve().parent
REPO_ROOT = DOCS_SOURCE.parents[1]
DOXYGEN_XML_DIR = REPO_ROOT / "docs" / "build" / "doxygen" / "xml"

CPP_INPUT_DIRS = [
    REPO_ROOT / "common",
    REPO_ROOT / "lib",
    REPO_ROOT / "utils",
    REPO_ROOT / "sequencerd",
    REPO_ROOT / "camerad",
    REPO_ROOT / "acamd",
    REPO_ROOT / "calibd",
    REPO_ROOT / "focusd",
    REPO_ROOT / "flexured",
    REPO_ROOT / "messaged",
    REPO_ROOT / "powerd",
    REPO_ROOT / "slicecamd",
    REPO_ROOT / "slitd",
    REPO_ROOT / "tcsd",
    REPO_ROOT / "telemd",
    REPO_ROOT / "thermald",
]

CPP_INPUT = " ".join(str(p) for p in CPP_INPUT_DIRS)

extensions = [
    "myst_parser",
    "sphinxcontrib.mermaid",
    "breathe",
    "exhale",
]

breathe_projects = {
    "NGPS": str(DOXYGEN_XML_DIR),
}

breathe_default_project = "NGPS"

exhale_args = {
    "containmentFolder": "./cpp-api",
    "rootFileName": "library_root.rst",
    "rootFileTitle": "C++ API",
    "doxygenStripFromPath": str(REPO_ROOT),
    "createTreeView": True,
    "exhaleExecutesDoxygen": True,
    "exhaleSilentDoxygen": False,
    "exhaleDoxygenStdin": f"""
PROJECT_NAME           = "NGPS C++ API"

GENERATE_HTML          = NO
GENERATE_XML           = YES
XML_PROGRAMLISTING     = NO

INPUT                  = {CPP_INPUT}
RECURSIVE              = YES

FILE_PATTERNS          = *.h *.hh *.hpp

EXCLUDE                = {REPO_ROOT / "java"} \\
                         {REPO_ROOT / "pygui"} \\
                         {REPO_ROOT / "docs"} \\
                         {REPO_ROOT / "build"} \\
                         {REPO_ROOT / ".venv"} \\
                         {REPO_ROOT / ".git"}

EXCLUDE_PATTERNS       = */java/* \\
                         */pygui/* \\
                         */docs/* \\
                         */build/* \\
                         */.venv/* \\
                         */__pycache__/*

EXTRACT_ALL            = NO
EXTRACT_PRIVATE        = NO
EXTRACT_STATIC         = NO
HIDE_UNDOC_MEMBERS     = YES
HIDE_UNDOC_CLASSES     = YES
HIDE_UNDOC_NAMESPACES  = YES
SHOW_NAMESPACES	       = NO

ENABLE_PREPROCESSING   = YES
MACRO_EXPANSION        = NO
SKIP_FUNCTION_MACROS   = YES

QUIET                  = NO
WARN_IF_UNDOCUMENTED   = NO
WARN_AS_ERROR          = NO
""",
}
