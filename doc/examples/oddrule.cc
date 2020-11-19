#include<cerberus-cpp/validator.hh>
#include<yaml-cpp/yaml.h>

#include<iostream>

int main()
{
  YAML::Node schema = YAML::Load(
    "value:           \n"
    "  type: integer  \n"
    "  oddity: true   \n"
  );

  YAML::Node document;
  document["value"] = 37;

  // START
  Cerberus::Validator validator;
  validator.registerRule(
    YAML::Load(
      "oddity:            \n"
      "  type: boolean    \n"
      "  dependencies:    \n"
      "    type: integer  \n"
    ),
    [](auto& v) {
      if(!v.getDocument().IsDefined())
        return;

      if(v.getDocument().template as<int>() % 2 != v.getSchema().template as<bool>())
        v.raiseError("oddity-Rule violated!");
    }
  );
  // END
  
  if (!validator.validate(document, schema))
    std::cerr << validator << std::endl;

  return 0;
}
