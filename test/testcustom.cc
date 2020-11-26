#include<cerberus-cpp/validator.hh>
#include<yaml-cpp/yaml.h>

#include"check.hh"

// For now this is only a different class - nothing fancy
class CustomValidator
  : public cerberus::Validator
{};


int main(int argc, char** argv)
{
  auto input = YAML::LoadFile(argv[1]);
  CustomValidator validator;
  return check(validator, input);
}