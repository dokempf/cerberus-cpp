#include<cerberus-cpp/validator.hh>
#include<yaml-cpp/yaml.h>

#include"check.hh"

int main(int argc, char** argv)
{
  auto input = YAML::LoadFile(argv[1]);
  cerberus::Validator validator;
  return check(validator, input);
}
