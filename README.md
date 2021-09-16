# SpecificJSON

A tool to generate C++ header and source file with templates for parsing and
writing JSON from expected input description in YAML.

# Requirements

For unit tests, you need https://github.com/onqtam/doctest to compile them.
Install somewhere where `#include <doctest/doctest.h>` works. See test/port
scripts for one option.

# Building the script

The build script needs a piece file list to import. The piece file list is
simply a list of YAML-file names to include, in the desired order.

The resulting script is designed to be usable in export, make edits, then
re-run with clean, import, and build options to produce a new script. If you
are ok with the provided pieces, you probably want just to import your own
parsers and writers.

## Build tests with cmake

You need cmake and compiler for C++17. Assuming a build directory parallel to
main datalackey directory, you can use:

    cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release ../specificjson

The build type affects only the test program. Assuming you want to use release
settings in the project where you use the sources generated using specificjson,
using release for test is the proper choice.

To specify the compiler, set for example:

    CXX=clang++
    CXX=g++

To build and test, assuming Unix Makefiles:

    make -j 2
    make test

Note that if you do not want to build gem but copy the script elsewhere, the
test target has built the specificjson script into the build directory.

## Build gem without tests

If you are ok with not running tests, run in the source code directory:

    rake gem
    rake install

The original intended usage was that the script gets built with changes
that are needed in a project, under the assumption that no single version
will cover the needs of everyone. Using the gem basically makes the base
version easier to install. You might want to put your own sources into a
repository and install this gem to generate your own version.

# Usage

Default values for fields and some other information are printed when you run
the script using the `--document` argument.

The pieces that are built into the script or imported using a list file are
the types that are available for use in format fields. You specify object
fields and their contents using types in the specification passed via `--input`
option. The script generates a header and source file with indicated contents.

Include the source file in your C++ program source file list.

For type "Foo" generated for input, you will have "Foo" as the type that
contains the values. A "Foo_Parser" contains the parser class that takes in
the input via Parse-method. Errors in parsing, including a type the code was
not expecting will throw an exception. Once the parser object Finished-method
returns true, you can Swap the value from parser to actual value object.

For a writer of type "Foo", there will be a template class with indicated
field names. You need to define NAMESPACE_FOO_TYPE macro using the
template "Foo_Template" and types for template paramaters as the value.
The typedef Foo will then be available in NAMESPACE.

The generated Write template function for type "Foo" will check for the
presence of field values as appropriate. It writes the field names and
values to the Sink type, which is assumed to have a method "write" that
takes a pointer and byte count. For example std::ostream works.

For a practical example see https://github.com/ismo-karkkainen/fileio
README.md file and readimage sources.

If you want to provide your own parsers, you can export the existing ones,
add your own, list the suitable ones and build a new script by calling the
existing specificjson with suitable parameters. The way you can obtain a
version that is specific to the project you are working on.

## Input

There are parser classes for some basic types and an array. The "types"
allows you to specify the fields in a JSON object. For each field you specify
format that specifies which parsers to use. Essentially the code generation
produces typedefs that indicate what you expect and the matching C++ types.

## Output

Since the type information is available in the C++ types that you intend to
output, only thing to do is have a Write-function template for each type so
that compiler can find the proper match. The generated type template is for
convenience, in case you do not already have a class with output fields.

The Write function generation requires that you tell how to check whether
optional field should be output, and how to access the field. As the template
has public members named as the fields, the default is to use those directly.
The check is used in conditional expression as "Value.checker" where checker
is the expression and Value refers to the object that is being written.
Likewise "Value.accessor" is used as parameter to Write where accessor is the
given expression. If you use the template, then this is the member name.

Integer types std::(u)intN_t where N is 8, 16, 32, or 64, cover the fundamental
types that are used in typedefs in the cstdint header. Hence int most likely
works but long might not. I am making the assumption that if you do not care
about the sizes, you use int and otherwise explicitly pick a type that has the
needed number of bits.

## Limitations

Input is expected to be a JSON object.

If you have an array, you have to provide the typedefs for the classes
yourself. A workable short-cut might be to specify an object with array value
with desired elements and use parser and value classes for that array instead
of the object.

Scalars can be parsed by calling the appropriate parser class method directly.
Although you can directly use the same standard functions that specificjson
uses internally.

# License

Copyright © 2020, 2021 Ismo Kärkkäinen

Licensed under Universal Permissive License. See LICENSE.txt.
