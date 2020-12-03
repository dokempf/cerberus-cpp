The cerberus-cpp documentation
==============================

.. toctree::
   :maxdepth: 2
   :caption: Contents:

.. _started:

Getting Started
===============

.. _install:

Installation
------------

Cerberus-cpp is header-only, so it should be fairly easy to get up and running.
It requires the following software to be available:

* A C++14-compliant C++ compiler
* CMake >= 3.11
* The `yaml-cpp <https://github.com/jbeder/yaml-cpp>`_ library, version >= 0.6
* git

The easiest way to get yaml-cpp is:

* On Debian, Ubuntu: :code:`sudo apt install libyaml-cpp-dev`
* On MacOS: :code:`brew install yaml-cpp`

With these prerequisites met, cerberus-cpp is installed just as any other
CMake project is, e.g.

.. code-block:: bash

   git clone https://github.com/dokempf/cerberus-cpp.git
   cd cerberus-cpp
   mkdir build
   cd build
   cmake ..
   make
   make install

.. _example:

Usage example
-------------

This is the most basic usage example that validates a given document against
a schema.

.. literalinclude:: examples/first.cc
   :language: c++

As you can see, both the schema and the document are defined using the :code:`YAML::Node`
data structure. In order to work with cerberus-cpp, it is important to be familiar with
the basic usage of yaml-cpp as e.g. described in `the yaml-cpp documentation <https://github.com/jbeder/yaml-cpp/wiki/Tutorial>`_.
In the above examples, we are loading the schema from inline YAML with :code:`YAML::Load`,
while we programmatically construct the document. Often, your document will of course come
from user input e.g. by loading it from disk with :code:`YAML::LoadFile`.

.. _basic:

Basic Usage
===========

.. _validation:

Validation
----------

The most important component provided by cerberus-cpp is the :code:`cerberus::Validator` class.
An instance of this validator is given a schema and a document. The document is then validated
against this schema using the :code:`validate` method - returning a boolean value indicating success.
If the validation process fails, the errors can be written to a stream using the validator's
:code:`printErrors` method, or more conveniently by passing the validator itself into the stream:

.. code-block:: c++

   cerberus::Validator validator;
   if(!validator.validate(document, schema))
     std::cerr << validator << std::endl;

The schema and the document are both provided as instances of :code:`YAML::Node`. Using the same
data structure for schemas and documents is considered a feature of cerberus-cpp. A tutorial
on how to construct these documents from YAML files, from inline strings or programmatically
can be found in `the yaml-cpp documentation <https://github.com/jbeder/yaml-cpp/wiki/Tutorial>`_.
Both schemas and documents are always expected to be mappings.


The validator class has the following configurable validation policies:

* whether or not unknown fields in the document (fields that do not appear in the schema) should
  make the validation process fail (can be toggled using the :code:`setAllowUnknown(value)` method).
  The default is :code:`false`.
* whether or not all fields are considered to be required fields (can be toggled using the :code:`setRequireAll(value)` method).
  The default is :code:`false`.


In the following, we will provide a summary of the available validation rules in cerberus-cpp.
As cerberus-cpp aims for compatibility with Python package cerberus, you can read more on the
semantics of these rules in the `Cerberus Validation Rule Documentation <https://docs.python-cerberus.org/en/stable/validation-rules.html>`_.
For inconsistencies between the Python package and cerberus-cpp see :ref:`compatibility`.
This is the list of implemented validation rules in order of relevance for most applications:

* :code:`type` specifies the expected type for this field. Possible values are :code:`integer`, :code:`float`, :code:`string`, :code:`boolean`, :code:`list`, :code:`dict` or any identifier of a custom type (see :ref:`custom_rule`).
* :code:`required` forces the existence of the field in the document.
* :code:`schema` specifies a schema for a submapping or a sublist
* :code:`allowed` specifies the list of allowed values for the field.
* :code:`forbidden` in contrast specifies a list of forbidden values for the field
* :code:`min` and :code:`max` specify a minimum and maximum for the value and require :code:`type` to be set to something that allows comparison
* :code:`regex` matches the field's value against the given regular expression
* :code:`keysrules`, :code:`valuesrules` allow rules only for keys resp. values of submapping.
* :code:`minlength` and :code:`maxlength` constrain the allowed length of list or mapping
* :code:`items` specifies a list of schemas that the entries of a list need to fulfill
* :code:`contains` forces the existence of a value within a list
* :code:`dependencies` forces the existence of another field, if the given field is present
* :code:`excludes` forces the absence of another field, if the given field is present
* :code:`nullable` specifies whether the field accepts a null value
* :code:`allow_unknown` changes the validators policy regarding unknown values e.g. for a submapping
* :code:`require_all` changes the validators policy regarding requiring all fields e.g. for a submapping


.. _normalization:

Normalization
--------------

Cerberus-cpp can not only perform validation, but also modify the document according to
normalization rules. Cerberus-cpp does not do this in-place. Instead you need to use the validator's :code:`getDocument()` method to
access the normalized document.

.. literalinclude:: examples/normalization.cc
   :language: c++
   :start-after: START
   :end-before: END

The normalized output document of above example would be :code:`user: John Doe`.

Additionally, the validator class has a configurable policy whether or not unknown fields
should be purged from the normalized document (can be toggled using the :code:`setPurgeUnknown(value)` method).
The default is :code:`false`.

This is a list of normalization rules available in cerberus-cpp:

* :code:`default` provides the field's default value
* :code:`rename` renames a given field in the normalized document
* :code:`purge_unknown` changes the validators policy regarding purging unknown fields e.g. for a submapping

.. _advanced:

Advanced Usage
==============

This section is only relevant to users who seek to enhance the capabilities of
cerberus-cpp by e.g. providing custom rules and types. All customizations described
in this documentation operate on instances of :code:`cerberus::Validator`. You may
also apply these in the constructor of a derived class.

.. _custom_rule:

Custom Validation Rules
-----------------------

Custom validation rules can be registered on instances of :code:`cerberus::Validator`.
This is an example that registers a custom rule :code:`oddity` that only accepts odd
integer values:

.. literalinclude:: examples/oddrule.cc
   :language: c++
   :start-after: START
   :end-before: END

The first argument here defines a schema that is used to validate the rule in the
user-provided schemas (a meta-schema so to say). This on one hand defines the name
of the rule (here: :code:`oddity`) and on the other hand rules out misuse (like e.g.
providing :code:`oddity: 42`, where only :code:`bool` arguments are allowed). You can
use all available schema rules, though typically only the name is required. Here, we
additionally enforce the argument to be of the :code:`integer` type by adding a
:code:`dependencies` rule.

The second argument is expected to be a templated callable (here: a generic lambda)
that implements the rule. The only argument is typically a reference to an instance of the :ref:`rule_api`,
although the type is accepted as a template parameter to integrate well with custom
derived validator classes. In our example, only the most relevant methods of the
:ref:`rule_api` are used:

* :code:`getDocument()` gives the :code:`YAML::Node` that describes the document snippet that is currently validated.
* :code:`getSchema()` provides the :code:`YAML::Node` that describes the schema snippet for this validation.
* :code:`raiseError()` reports a validation error

Some rules require to be applied before or after certain other rules in order to
implement the correct semantics. Cerberus-cpp gives control over this by providing
a number of hooks, when rules execute. The hook at which a custom rule executes can
be controlled by passing a third argument. The enumeration :code:`cerberus::RulePriority`
lists the possible values:

.. doxygenenum:: cerberus::RulePriority

.. _custom_type:

Custom Types
------------

By default, cerberus-cpp supports integers, floating point types, strings, boolean values,
as well a sequences and mappings. You can however provide custom types as well. We illustrate
this by implementing a simple date type that only stores a year. While this of course could be
achieved with an integer as well, we use this to illustrate how a custom class is validated.

.. literalinclude:: examples/datetype.cc
   :language: c++
   :start-after: START_DATE
   :end-before: END_DATE

This simple implementation at the same time documents the minimum requirement on the interface of eligible C++ types:
:code:`operator==` and :code:`operator<` need to be defined.
On top of that :code:`yaml-cpp` s (de)serialization needs to be implemented for this type according to their
`Guide <https://github.com/jbeder/yaml-cpp/wiki/Tutorial#converting-tofrom-native-data-types>`_ :

.. literalinclude:: examples/datetype.cc
   :language: c++
   :start-after: START_YML
   :end-before: END_YML

Registration of the new type with a given validator is then as simple as this:

.. literalinclude:: examples/datetype.cc
   :language: c++
   :start-after: START_REG
   :end-before: END_REG

Now, this type can be referenced with a rule :code:`type: date` and validation will fail if the
YAML deserialization of the input fails.

.. _schema_registration:

Schema Registration
-------------------

If you intend to reuse schemas a lot, you can also have a validator instance store them by
using the :code:`registerSchema` method. Later on, the schema can either be retrieved by
using the special signature of :code:`validate` that specifies the schema with a string or
by specifying the string as the value for a :code:`schema` validation rule.

.. literalinclude:: examples/registerschema.cc
   :language: c++
   :start-after: START
   :end-before: END

.. _compatibility:

Compatibility with cerberus
===========================

Cerberus-cpp tries to be compatible with the Python package cerberus.
In reality, some inconsistencies exist. If you have a use case where
cerberus-cpp differs from cerberus that cannot be explained by one of
the following reasons please open an issue attaching YAML files with
schema and data:

* Several validation rules require the :code:`type` rule to be present as well.
  These are the rules that require equality or comparison to be implemented e.g.:

  * :code:`min` and :code:`max`

  * :code:`allowed`
  
  Your safest bet is to *always* define the :code:`type` rule.

* The :code:`allowed` rule does not validate iterables, because that would lead to
  conflicting semantics of the :code:`type` field.

* The :code:`contains` rule has currently no access to the item type
  information. It currently assumes string values, although it could be changed
  to inspect a given :code:`schema` rule for type information - I am not sure yet I
  want to go that route.

* Some of the types built into cerberus are hard to implement in C++ and
  are therefore omitted from the library. If you need these, register a custom type
  and choose the correct C++ data structure yourself. These are:

  * :code:`date` and :code:`datetime`: With C++ lacking standardization of these types and completely missing a parser for such types, it would be unwise to implement these.
  * :code:`binary`: There is no sensible C++ equivalent of a Python bytes object, so it seems wise to skip on this one.
  * :code:`set`: This type seems to be inaccessible when starting from serialized YAML. I am currently not planning to add this.

* The :code:`regex` rule is not guaranteed to accept exactly the same dialect of
  regular expressions as in the Python package. Currently, the C++ implementation
  uses plain :code:`std::regex`. Maybe this can be fixed by picking the correct grammar
  for :code:`std::regex`.

* The following rules are currently considered a *won't fix* for one reason or
  the other:

  * :code:`allof`, :code:`anyof`, :code:`noneof`, :code:`oneof`: These rules are a major headache to
    implement. Yet, the cerberus documentation actively warns users that the need
    for such rule hints at a design flaw. Also, these rules disable normalization.
    Currently, I would rather opt to not doing these rules at all.

  * :code:`readonly`: Just from reading the documentation I do not get both the semantics
    or the use case for this rule. So, I am omitting it until somebody urges me to
    implement it.

  * :code:`check_with`: In the context of cerberus-cpp, I fail to see how this rule differs
    from applying a custom rule, which you should do in that case.

  * :code:`coerce`: Similarly to :code:`check_with`, a a custom coercer is not really different
    from a custom normalization rule. Might add a :code:`coerce` rule later for compatibility
    with Python cerberus later though.

.. _api:

API documentation
=================

Cerberus-cpp has two core APIs:

* The :ref:`validator_api` is the end user interface that is used when validating
  data against schemas.
* The :ref:`rule_api` is the interface used when developing custom rules.
  It gives access to the internal state of the validation process that is
  necessary to implement custom validation logic.

If you do not intend to implement custom rules, there is no need to understand the latter.

.. _validator_api:

Validator API
-------------

.. doxygenclass:: cerberus::Validator
   :members:

.. _rule_api:

ValidationRuleInterface API
---------------------------

.. doxygenclass:: cerberus::Validator::ValidationRuleInterface
   :members:

.. _contributing:

Contributing
============

Cerberus-cpp welcomes contributions. Before considering to contribute,
please read the following guidelines:

* If you have a use case that does not work in cerberus-cpp, but it does
  work in the Python package cerberus, please open a bug report and attach
  YAML files with a schema and some data. Ideally, the file follows the syntax
  that cerberus-cpp tests use (see the :code:`test/testdata.yml` file).

* Bear in mind that cerberus-cpp tries to stay compatible with the Python package cerberus.
  Pull requests that increase incompatibilities will not be considered, while
  pull requests that remove these are highly welcome.

* If you are implementing a custom rule and you need to extend the :ref:`rule_api`,
  please provide a description of your use case, so
  that we can better discuss the interface design.

* When opening a pull request against the cerberus-cpp repository, please add
  your name to :code:`COPYING.md` as well.
