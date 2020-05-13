# SpecificJSON

A tool to generate C++ header and source file with templates for parsing and
writing JSON.

# Requirements

For unit tests, you need https://github.com/onqtam/doctest to compile them.
Install somewhere where `#include <doctest/doctest.h>` works.

# Building the script

The build script needs a piece file list to import. The piece file list is
simply a list of YAML-file names to include, in the desired order.

The resulting script is designed to be usable in export, make edits, then
re-run with clean, import, and build options to produce a new script. If you
are ok with the provided pieces, you probably want just to import your own
parsers and writers.

## Build without cmake

If you are ok with not running tests, run:

    ./build --import build.yaml --build specificjson

Resulting specificjson is the script you can then install or copy where needed.

# Usage

Default values for fields and some other information are printed when you run
the script using the `--document` argument.

The pieces that are built in to the script or imported usinga list file are the
types that are available for use in format fields. You specify object fields
and their contents using types in the specification passed via `--input`
option. The script generates a header and source file with indicated contents.

Include the source file in your C++ program source file list.

For type "Foo" generated for input, you will have "Foo" as the type that
contains the values. A "Foo_Parser" contains the parser class that takes in
the input via Parse-method. Errors in parsing, including a type the code was
not expecting will throw an exception. Once the parser object Finished-method
returns true, you can Swap the value from parser to actual value object.

For a writer of type "Foo", there will be a template class with indicated
field names. You need to define NAMESPACE_TYPENAME_TYPE macro using the
template "Foo_Template" and types as value for typedef. The generated Write
template function for type "Foo" will then check for presence of field values
as appropriate and it writes the field names and values to the Sink type,
which is assumed to have a method "write" that takes a pointer and byte count.
For example std::ostream works.

For a practical example see https://github.com/ismo-karkkainen/imageio
README.md file and sources.

# License

Copyright (C) 2020 Ismo Kärkkäinen

Licensed under Universal Permissive License. See License.txt.
