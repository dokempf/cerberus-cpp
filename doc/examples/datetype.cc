#include<cerberus-cpp/validator.hh>
#include<yaml-cpp/yaml.h>

#include<iostream>

// START_DATE
struct SimpleDate {
  int year;

  bool operator==(const SimpleDate& other) const
  {
    return year == other.year;
  }

  bool operator<(const SimpleDate& other) const
  {
    return year < other.year;
  }
};
// END_DATE

// START_YML
namespace YAML {
  template<>
  struct convert<SimpleDate>
  {
    static Node encode(const SimpleDate& rhs)
    {
      return Node(rhs.year);
    }

    static bool decode(const Node& node, SimpleDate& rhs)
    {
      if(!node.IsScalar())
        return false;

      rhs.year = node.as<int>();;
      return true;
    }
  };
}
// END_YML

int main()
{
  YAML::Node schema = YAML::Load(
    "when:           \n"
    "  type: date    \n"
    "  min: 1984     \n"
  );

  YAML::Node document;
  document["when"] = "2001";

  // START_REG
  Cerberus::Validator validator;
  validator.registerType<SimpleDate>("date");
  // END_REG
  if (!validator.validate(document, schema)) 
    std::cerr << validator << std::endl;
  return 0;
}
