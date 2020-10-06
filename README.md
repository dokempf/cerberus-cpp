# What is cerberus-cpp

cerberus-cpp is a schema validation for YAML data written in C++.
It reimplements schema validation of the Python tool [cerberus](https://github.com/pyeve/cerberus)
in the C++ language. This allows the same schemas to be used in Python
and C++ projects, as well as projects that mix these languages.
For YAML parsing, cerberus-cpp relies on the well-established [yaml-cpp](https://github.com/jbeder/yaml-cpp)
library.

# Incompatibilities between cerberus and cerberus-cpp

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
* Some of the scalar types built into cerberus are hard to implement in C++ and
  are therefore omitted from the library. If you need these, register a custom type
  and choose the correct C++ data structure yourself. These are:
  * `data` and `datetime`: With C++ lacking standardization of these types and
    completely missing a parser for such types, it would be unwise to implement
    these.
  * `binary`: There is no sensible C++ equivalent of a Python bytes object, so
    it seems wise to skip on this one.
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

# Road map

This is the roadmap towards cerberus feature completeness:

* General validation stuff
  * [ ] Schema registration
  * [ ] Validation of schemas against Schema Schema
* Validation Rules
  * [x] allof
  * [x] allowed
  * [x] anyof
  * [x] check_with
  * [ ] contains
  * [ ] dependencies
  * [ ] empty
  * [ ] excludes
  * [x] forbidden
  * [x] items
  * [ ] keysrules
  * [x] meta
  * [x] min, max
  * [x] minlength, maxlength
  * [x] noneof
  * [x] oneof
  * [x] readonly
  * [x] regex
  * [ ] require_all
  * [x] required
  * [x] schema (dict)
  * [x] schema (list)
  * [ ] type
    *  [x] integer
    *  [x] float
    *  [x] string
    *  [x] boolean
    *  [ ] number
    *  [ ] binary
    *  [ ] date
    *  [ ] datetime
    *  [x] dict
    *  [ ] list
    *  [ ] set
  * [ ] valuesrules
* Normalization Rules
  * [ ] Renaming
  * [ ] Purging
  * [x] Default Values
  * [ ] Value Coercion
* [ ] Error Handling
* Customization
  * [ ] Custom error handling
  * [ ] Custom validation rules
  * [ ] Custom data types
  * [ ] Custom coercers
  * [ ] Custom default setters
