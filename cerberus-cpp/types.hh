#ifndef CERBERUS_CPP_TYPES_HH
#define CERBERUS_CPP_TYPES_HH

#include<yaml-cpp/yaml.h>

namespace Cerberus {

  struct TypeItemBase
  {
    virtual bool is_convertible(const YAML::Node&) const = 0;
    virtual bool equality(const YAML::Node&, const YAML::Node&) const = 0;
    virtual bool less(const YAML::Node&, const YAML::Node&) const = 0;
  };

  template<typename T>
  struct TypeItem
    : TypeItemBase
  {
    virtual bool is_convertible(const YAML::Node& node) const override
    {
      T val;
      return YAML::convert<T>::decode(node, val);
    }

    virtual bool equality(const YAML::Node& op1, const YAML::Node& op2) const override
    {
      T cop1, cop2;
      YAML::convert<T>::decode(op1, cop1);
      YAML::convert<T>::decode(op2, cop2);
      return cop1 == cop2;
    }

    virtual bool less(const YAML::Node& op1, const YAML::Node& op2) const override
    {
      T cop1, cop2;
      YAML::convert<T>::decode(op1, cop1);
      YAML::convert<T>::decode(op2, cop2);
      return cop1 < cop2;
    }
  };

  template<typename Validator>
  void registerBuiltinTypes(Validator& validator)
  {
    validator.template registerType<long long>("integer");
    validator.template registerType<std::string>("string");
    validator.template registerType<long double>("float");
    validator.template registerType<long double>("number");
    validator.template registerType<bool>("boolean");
  }

} // namespace Cerberus 

#endif