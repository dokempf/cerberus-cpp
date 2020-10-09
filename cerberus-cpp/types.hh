#ifndef CERBERUS_CPP_TYPES_HH
#define CERBERUS_CPP_TYPES_HH

#include<yaml-cpp/yaml.h>
#include<string>

namespace Cerberus {

  /** @brief Abstract base class that represents a type in the validation process
   * 
   * This defines the interface that we expect from a type implementation.
   * As an end-user you will typically *use* this interface from custom rule
   * implementations, where you depend on type-specific behaviour e.g. when
   * comparing elements.
   */
  struct TypeItemBase
  {
    virtual bool is_convertible(const YAML::Node&) const = 0;
    virtual bool equality(const YAML::Node&, const YAML::Node&) const = 0;
    virtual bool less(const YAML::Node&, const YAML::Node&) const = 0;
  };

  /** @brief An implementation of the @c TypeItemBase interface that wraps a C++ type
   * 
   * @tparam T The C++ type that is wrapped
   * 
   * This makes a given C++ type available through the virtual interface of
   * @c TypeItemBase. This item is usually instantiated by the @c registerType
   * method of the @c Validator class.
   */
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

  /** @brief Register all the built-in types from cerberus 
   * 
   * This is called from the constructor of the @c Validator class.
   */
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