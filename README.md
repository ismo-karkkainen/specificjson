# SpecificJSON

A tool to generate C++ header and source file with templates for parsing and
writing JSON.

# Requirements

For unit tests, you need https://github.com/onqtam/doctest to compile the
sources. Even if you do not need to run the tests, you still need it for
definitions on the test macros that are located around the source code.

You need edicta for getting the specification in YAML format if you want to
have the specification in human-readable format. As the script reads JSON,
it is not strictly required, merely intended to be used that way. See
https://github.com/ismo-karkkainen/edicta 

# Building the script

The build script needs a recipe and script file to combine the information
with. The recipe is simply a list of YAML-file names to include, in the
desired order. Script is by default script.rb. Default output is specificjson.

Replacing the recipe file is probably the only thing you want to do, when
you want to add your own parsing classes or writing methods.


# License

Copyright (C) 2020 Ismo Kärkkäinen

Licensed under Universal Permissive License. See License.txt.
