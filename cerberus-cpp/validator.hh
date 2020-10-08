#ifndef CERBERUS_CPP_VALIDATOR_HH
#define CERBERUS_CPP_VALIDATOR_HH

#include<cerberus-cpp/error.hh>
#include<cerberus-cpp/rules.hh>
#include<cerberus-cpp/stack.hh>
#include<cerberus-cpp/types.hh>

#include<yaml-cpp/yaml.h>

#include<functional>
#include<iostream>
#include<map>
#include<memory>
#include<string>
#include<tuple>

namespace Cerberus {


  class Validator
  {
    public:
    Validator()
      : Validator(YAML::Node())
    {}

    Validator(const YAML::Node& schema)
      : schema_(schema)
      , state(*this)
    {
      registerBuiltinRules(*this);
      registerBuiltinTypes(*this);
    }

    template<typename T>
    void registerType(const std::string& name)
    {
      typesmapping[name] = std::make_shared<TypeItem<T>>();
    }

    template<typename Rule>
    void registerRule(const YAML::Node& schema, Rule&& rule, RulePriority priority = RulePriority::VALIDATION)
    {
      schema_schema[schema.begin()->first] = schema.begin()->second;
      rulemapping[schema.begin()->first.as<std::string>()] = std::make_pair(priority, std::forward<Rule>(rule));
    }

    void setAllowUnknown(bool value)
    {
      state.allow_unknown = value;
    }

    bool validate(const YAML::Node& data)
    {
      return validate(data, schema_);
    }

    bool validate(const YAML::Node& data, const YAML::Node& schema)
    {
      // // Validate the schema first
      // state.errors->clear();
      // state.document = YAML::Clone(schema);
      // state.validate(schema_schema);
      // if(!state.errors->empty())
      //   throw SchemaError(state);

      state.errors.clear();
      YAML::Node copy = YAML::Clone(data);
      state.setDocument(copy);
      state.validate(schema);
      return state.errors.empty();
    }

    const YAML::Node& getDocument()
    {
      return state.getDocument();
    }

    template<typename Stream>
    void printErrors(Stream& stream) const
    {
      return state.printErrors(stream);
    }

    private:
    struct RecursiveValidator
    {
      RecursiveValidator(Validator& validator)
        : validator(validator)
        , allow_unknown(false)
      {}

      // We prohibit copies of the recursive validator. They would only create a mess!
      RecursiveValidator(const RecursiveValidator&) = delete;

      void raiseError(const std::string& error)
      {
        errors.push_back({document_stack.stringPath(), error});
      }

      void validateItem(const YAML::Node& schema)
      {
        schema_stack.push_back(schema);

        // Apply validation rules
        for(const auto priority : { RulePriority::NORMALIZATION,
                                    RulePriority::VALIDATION,
                                    RulePriority::TYPECHECKING })
          for(auto ruleval : schema)
          {
            auto rule = validator.rulemapping.find(ruleval.first.as<std::string>());
            if((rule != validator.rulemapping.end()) && (rule->second.first == priority))
            {
              schema_stack.push_back(ruleval.second);
              rule->second.second(*this);
              schema_stack.pop_back();
            }
          }

        schema_stack.pop_back();
      }

      bool validate(const YAML::Node& schema)
      {
        // Store the schema in validation state to have it accessible in rules
        schema_stack.push_back(schema);

        // Perform validation
        std::vector<std::string> found;
        for(auto fieldrules : schema)
        {
          auto field = fieldrules.first.as<std::string>();
          auto rules = fieldrules.second;
          found.push_back(field);

          document_stack.pushDictItem(field);
          validateItem(rules);
          getDocument(1)[field] = getDocument();
          document_stack.pop();
        }

        if(!allow_unknown)
        {
          for(auto item: getDocument())
            if(std::find(found.begin(), found.end(), item.first.as<std::string>()) == found.end())
                raiseError("Unknown item found in validator that does not accept unknown items: " + item.first.as<std::string>());
        }

        schema_stack.pop_back();

        return errors.empty();
      }

      // TODO: This should be replaced with a more general error handling concept
      template<typename Stream>
      void printErrors(Stream& stream) const
      {
        for(auto error: errors)
        {
          stream << "Error validating data field " << error.path << std::endl;
          stream << "Message: " << error.message << std::endl;
        }
      }

      const std::shared_ptr<TypeItemBase>& extractType(const std::string& name)
      {
        return validator.typesmapping[name];
      }

      const std::shared_ptr<TypeItemBase>& extractType(std::size_t level = 0)
      {
        auto type = getSchema(level)["type"].as<std::string>();
        return validator.typesmapping[type];
      }

      const YAML::Node& getSchema(std::size_t level = 0) const
      {
        return schema_stack.get(level);
      }

      YAML::Node& getDocument(std::size_t level = 0)
      {
        return document_stack.get(level);
      }

      void setDocument(const YAML::Node& document)
      {
        document_stack.reset(document);
      }

      Validator& validator;
      DocumentStack schema_stack;
      DocumentStack document_stack;
      std::vector<ValidationErrorItem> errors;
      bool allow_unknown;
    };

    YAML::Node schema_;
    RecursiveValidator state;

    std::map<std::string, std::pair<RulePriority, std::function<void(RecursiveValidator&)>>> rulemapping;
    std::map<std::string, std::shared_ptr<TypeItemBase>> typesmapping;

    // The schema that is used to validate user provided schemas.
    // This is update with snippets as rules are registered
    YAML::Node schema_schema;
  };

} // namespace Cerberus

#endif