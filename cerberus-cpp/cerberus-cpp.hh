#ifndef CERBERUS_CPP_HH

#include<yaml-cpp/yaml.h>

namespace Cerberus {

  class Validator
  {
    public:
    Validator()
      : schema(YAML::Node())
      , document(YAML::Node())
    {}

    Validator(const YAML::Node& schema)
      : schema(schema)
      , document(YAML::Node())
    {}

    bool validate(const YAML::Node& data)
    {
      return false;
    }

    bool validate(const YAML::Node& data, const YAML::Node& schema)
    {
      return false;
    }

    YAML::Node getDocument()
    {
      return document;
    }

    private:
    const YAML::Node& schema;
    YAML::Node document;
  };

} // namespace Cerberus

#endif