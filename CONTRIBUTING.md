Cerberus-cpp welcomes contributions. Before considering to contribute,
please read the following guidelines:

* If you have a use case that does not work in cerberus-cpp, but it does
  work in the Python package cerberus, please open a bug report and attach
  YAML files with a schema and some data. Ideally, the file follows the syntax
  that cerberus-cpp tests use (see the `test/cases` subdirectory).
* Bear in mind that cerberus-cpp tries to stay compatible with the Python package cerberus.
  Pull requests that increase incompatibilities will not be considered, while
  pull requests that remove these are highly welcome.
* If you are implementing a custom rule and you need to extend the interface of
  `ValidationRuleInterface`, please provide a description of your use case, so
  that we can better discuss the interface design.
* When opening a pull request against the cerberus-cpp repository, please add
  your name to COPYING.md as well.
