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

* The `min` and `max` rules require the presence of the `type` rule.

# Road map

This is the roadmap towards cerberus feature completeness:

* General validation stuff
  * [ ] Schema registration
  * [ ] Validation of schemas against Schema Schema
* Validation Rules
  * [ ] allof
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
  * [ ] minlength, maxlength
  * [ ] oneof
  * [ ] readonly
  * [ ] regex
  * [ ] require_all
  * [x] required
  * [x] schema (dict)
  * [ ] schema (list)
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
