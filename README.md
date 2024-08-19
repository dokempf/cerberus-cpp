[![GitHub Workflow Status](https://img.shields.io/github/actions/workflow/status/dokempf/cerberus-cpp/ci.yml?branch=main)](https://github.com/dokempf/cerberus-cpp/actions/workflows/ci.yml)
[![Documentation Status](https://readthedocs.org/projects/cerberus-cpp/badge/?version=latest)](https://cerberus-cpp.readthedocs.io/en/latest/?badge=latest)
[![codecov](https://codecov.io/gh/dokempf/cerberus-cpp/branch/main/graph/badge.svg?token=OMP0HYTKD6)](https://codecov.io/gh/dokempf/cerberus-cpp)

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

* A C++14-compliant C++ compiler
* CMake >= 3.11
* The [yaml-cpp](https://github.com/jbeder/yaml-cpp) library version >= 0.6, e.g. by installing
  the Debian/Ubuntu package `libyaml-cpp-dev`.

# Documentation

For usage examples, detailed instructions and an overview on the compatibility
with the Python package, check out the
[cerberus-cpp documentation](https://cerberus-cpp.readthedocs.io)
