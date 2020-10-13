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

# Basic usage example

Here is a very basic usage of cerberus-cpp:

```
#include<cerberus-cpp/validator.hh>

int main()
{
  YAML::Node schema = YAML::Load(    
    "user:            \n"
    "  type: string   \n"
    "  forbidden:     \n"
    "    - admin      \n"
    "    - root       \n"
  );
  YAML::Node document = YAML::Load(
    "user: dkempf"
  );

  Validator validator(schema);

  if(validator.validate(document))
    doSomething(validator.getDocument())
  else
    std::cerr << validator << std::endl;
}
```

For more information, check the documentation.

# (In)Compatibility between cerberus and cerberus-cpp

Cerberus-cpp tries to be compatible with the Python package cerberus.
In reality, some inconsistencies exist. If you have a use case where
cerberus-cpp differs from cerberus that cannot be explained by one of
the following reasons please open an issue attaching YAML files with
schema and data:

* Several validation rules require the `type` rule to be present as well.
  These are the rules that require equality or comparison to be implemented e.g.:
  * `min` and `max`
  * `allowed`
  
  Your safest bet is to *always* define the `type` rule.
* The `allowed` rule does not validate iterables, because that would lead to
  conflicting semantics of the `type` field.
* The `contains` rule has currently no access to the item type
  information. It currently assumes string values, although it could be changed
  to inspect a given `schema` rule for type information - I am not sure yet I
  want to go that route.
* Some of the types built into cerberus are hard to implement in C++ and
  are therefore omitted from the library. If you need these, register a custom type
  and choose the correct C++ data structure yourself. These are:
  * `data` and `datetime`: With C++ lacking standardization of these types and
    completely missing a parser for such types, it would be unwise to implement
    these.
  * `binary`: There is no sensible C++ equivalent of a Python bytes object, so
    it seems wise to skip on this one.
  * `set`: This type seems to be inaccessible when starting from serialized YAML.
    I am currently not planning to add this.
* The `regex` rule is not guaranteed to accept exactly the same dialect of
  regular expressions as in the Python package. Currently, the C++ implementation
  uses plain `std::regex`. Maybe this can be fixed by picking the correct grammar
  for `std::regex`.
* The following rules are currently considered a *won't fix* for one reason or
  the other:
  * `allof`, `anyof`, `noneof`, `oneof`: These rules are a major headache to
    implement. Yet, the cerberus documentation actively warns users that the need
    for such rule hints at a design flaw. Also, these rules disable normalization.
    Currently, I would rather opt to not doing these rules at all.
  * `readonly`: Just from reading the documentation I do not get both the semantics
    or the use case for this rule. So, I am omitting it until somebody urges me to
    implement it.
  * `check_with`: In the context of cerberus-cpp, I fail to see how this rule differs
    from applying a custom rule, which you should do in that case.
  * `coerce`: Similarly to `check_with`, a a custom coercer is not really different
    from a custom normalization rule. Might add a `coerce` rule later for compatibility
    with Python cerberus later though.
