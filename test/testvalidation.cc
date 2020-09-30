#include<cerberus-cpp/cerberus-cpp.hh>
#include<yaml-cpp/yaml.h>

#include<iostream>


bool testcase(const std::string& schema, const std::string& data)
{
  Cerberus::Validator validator(YAML::Load(schema));
  bool result = validator.validate(YAML::Load(data));

  // TODO: Compare against Python cerberus
  if (!result)
  {
    std::cerr << "Failure trying to validate this data:" << std::endl;
    std::cerr << data << std::endl;
    std::cerr << "against this schema:" << std::endl;
    std::cerr << schema << std::endl;
    std::cerr << "The following reasons were given:" << std::endl;
    validator.printErrors(std::cerr);
  }
  return result;
}

int main()
{
  bool success = true;

  success = success && testcase(
    "uuid:             \n"
    "  type: integer   \n"
    "  default: 1042   \n"
    ,
    ""
  );

  success = success && testcase(
    "uuid:             \n"
    "  type: integer   \n"
    "  required: true  \n"
    ,
    "uuid: 1042"
  );

  return success ? 0 : 1;
}