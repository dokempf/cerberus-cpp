#include<cerberus-cpp/validator.hh>
#include<yaml-cpp/yaml.h>

#include<iostream>

int main()
{
  // START
  YAML::Node schema = YAML::Load(
    "name:                  \n"
    "  type: string         \n"
    "  default: John Doe    \n"
    "  rename: user         \n"
  );

  cerberus::Validator validator(schema);
  if (validator.validate(YAML::Node()))
    std::cout << "The normalized document: " << validator.getDocument() << std::endl;
  else
    std::cerr << validator << std::endl;
  // END

  return 0;
}
