#include<cerberus-cpp/validator.hh>
#include<yaml-cpp/yaml.h>

#include<iostream>

int main()
{
  YAML::Node schema = YAML::Load(
    "answer:          \n"
    "  type: integer  \n"
    "  default: 42    \n"
    "question:        \n"
    "  type: string   \n"
  );

  YAML::Node document;
  document["question"] = "What is 6x9?";

  Cerberus::Validator validator(schema);
  if (validator.validate(document))
  {
    YAML::Node doc = validator.getDocument();
    std::cout << doc["question"].as<std::string>() << " " << doc["answer"].as<int>() << std::endl;
  }
  else
    std::cerr << validator << std::endl;
  
  return 0;
}
