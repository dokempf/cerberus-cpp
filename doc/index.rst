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

* A C++11-compliant C++ compiler
* CMake >= 3.11
* The `yaml-cpp <https://github.com/jbeder/yaml-cpp>`_ library
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

.. code-block:: c++

   YAML::Node schema = YAML::Load(
     "answer:          \n"
     "  type: integer  \n"
     "  default: 42    \n"
     "question:        \n"
     "  type: string   \n"
   );

   YAML::Node document;
   document["question"] = "What is 6x9?";

   Validator validator(schema);
   if (validator.validate(document))
   {
     YAML::Node doc = validator.getDocument();
     std::cout << doc["question"].as<std::string>() << " " << doc["answer"].as<int>() << std::endl;
   }
   else
     std::cerr << validator << std::endl;

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

.. _normalization:

Normalization:
--------------

.. _advanced:

Advanced Usage
==============

.. _custom_rule:

Custom Validation Rules
-----------------------

.. _custom_type:

Custom Types
------------

.. _schema_registration:

Schema Registration
-------------------


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

.. doxygenclass:: Cerberus::Validator
   :members:

.. _rule_api:

ValidationRuleInterface API
---------------------------

.. doxygenclass:: Cerberus::Validator::ValidationRuleInterface
   :members:

.. _contributing:

Contributing
============

Cerberus-cpp welcomes contributions. Before considering to contribute,
please read the following guidelines:

* If you have a use case that does not work in cerberus-cpp, but it does
  work in the Python package cerberus, please open a bug report and attach
  YAML files with a schema and some data. Ideally, the file follows the syntax
  that cerberus-cpp tests use (see the :code:`test/cases` subdirectory).

* Bear in mind that cerberus-cpp tries to stay compatible with the Python package cerberus.
  Pull requests that increase incompatibilities will not be considered, while
  pull requests that remove these are highly welcome.

* If you are implementing a custom rule and you need to extend the :ref:`rule_api`,
  please provide a description of your use case, so
  that we can better discuss the interface design.

* When opening a pull request against the cerberus-cpp repository, please add
  your name to :code:`COPYING.md` as well.
