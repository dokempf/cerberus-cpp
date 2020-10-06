# What is cerberus-cpp

cerberus-cpp is a schema validation for YAML data written in C++.
It reimplements schema validation of the Python tool [cerberus](https://github.com/pyeve/cerberus)
in the C++ language. This allows the same schemas to be used in Python
and C++ projects, as well as projects that mix these languages.
For YAML parsing, cerberus-cpp relies on the well-established [yaml-cpp](https://github.com/jbeder/yaml-cpp)
library.

# Incompatibilities between cerberus and cerberus-cpp

Cerberus-cpp tries to be fully compatible with the Python package cerberus.
In reality, some inconsistencies exist. Some of these are bugs, but others
are features. The latter ones will not go away but are intentional design
decisions that were necessary to implement cerberus in a strongly typed world:

* Several validation rules require the `type` rule to be present as well.
  These are the rules that require equality or comparison to be implemented e.g.:
  * `min` and `max`
  * `allowed`
  Your safest bet is to *always* define the `type` rule.

# Road map

This is the roadmap towards cerberus feature completeness:

* General validation stuff
  * [ ] Schema registration
  * [ ] Validation of schemas against Schema Schema
* Validation Rules
  * [ ] allof
  * [x] allowed
  * [ ] anyof
  * [ ] check_with
  * [ ] contains
  * [ ] dependencies
  * [ ] empty
  * [ ] excludes
  * [ ] forbidden
  * [ ] items
  * [ ] keysrules
  * [x] meta
  * [x] min, max
  * [x] minlength, maxlength
  * [ ] oneof
  * [ ] readonly
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
