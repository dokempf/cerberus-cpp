#include<cerberus-cpp/cerberus-cpp.hh>
#include<yaml-cpp/yaml.h>

#include<iostream>


int main(int argc, char** argv)
{
  auto input = YAML::LoadFile(argv[1]);
  Cerberus::Validator validator(input["schema"]);
  auto result validator.validate(input["data"]) ? 0 : 1;

  if(!result)
  {
    std::cerr << "Failure trying to validate this data:" << std::endl;
    std::cerr << data << std::endl;
    std::cerr << "against this schema:" << std::endl;
    std::cerr << schema << std::endl;
    std::cerr << "The following reasons were given:" << std::endl;
    validator.printErrors(std::cerr);    
  }
  return result ? 0 : 1;
}
