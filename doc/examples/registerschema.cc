#include<cerberus-cpp/validator.hh>
#include<yaml-cpp/yaml.h>

#include<iostream>

int main()
{
  YAML::Node schema = YAML::Load(
    "name:            \n"
    "  type: string   \n"
    "  required: true \n"
    "age:             \n"
    "  type: integer  \n"
    "  min: 0         \n"
  );

  YAML::Node document;
  document["name"] = "Me";

  // START
  cerberus::Validator validator;
  validator.registerSchema("user", schema);
  if (!validator.validate(document, "user"))
    std::cerr << validator << std::endl;
  // END

  return 0;
}
