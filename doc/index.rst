Welcome to cerberus-cpp's documentation!
========================================

.. toctree::
   :maxdepth: 2
   :caption: Contents:


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

Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`
