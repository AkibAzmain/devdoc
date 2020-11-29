# Devdoc

An extension for viewing Devhelp documentations on Docview.

## Usage

First enable it in Docview. Then add directory with Devhelp documentations to documentation search paths (e.g `/usr/share/devhelp/books`). If the directory contains any valid documentations, it'll appear in the sidebar.

## Compiling

Just execute `make` . If you've installed Docview in custom directory, set proper compiler arguments in `CXXFLAGS` and `LDFLAGS` variables.

### Dependencies

This extension depends on libxml++, libdocview and the C++ standard library.
