# What is cerberus-cpp

cerberus-cpp is a schema validation tool for YAML data written in C++.
It reimplements a subset of the schema validation of the Python tool [cerberus](https://github.com/pyeve/cerberus)
in the C++ language. This allows the same schemas to be used in Python
and C++ projects, as well as projects that mix these languages.
For YAML parsing, cerberus-cpp relies on the well-established [yaml-cpp](https://github.com/jbeder/yaml-cpp)
library.

# Core Features

* Simple, yet powerful definition of validation schema
* Validation of arbitrarily nested data
* Modern C++11 design
* Schema are implemented in the same language as the data - no DSLs to learn!
* Validation of schemas against a schema that is incrementally built from rules definitions
* Registration of custom validation and normalization rules
* Registration of custom types
* Compatibility with a well-established Python package

# Prerequisites

Cerberus-cpp is header-only, so it should be fairly easy to get up and running.
It requires the following software to be available:

* A C++11-compliant C++ compiler
* CMake >= 3.11
* The [yaml-cpp](https://github.com/jbeder/yaml-cpp) library, e.g. by installing
  the Debian package `libyaml-cpp-dev`.

# Documentation

For usage examples, detailed instructions and an overview on the compatibility
with the Python package, check out the
[cerberus-cpp documentation](https://cerberus-cpp.readthedocs.io)
