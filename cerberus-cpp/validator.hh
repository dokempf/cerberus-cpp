#ifndef CERBERUS_CPP_VALIDATOR_HH
#define CERBERUS_CPP_VALIDATOR_HH

#include<cerberus-cpp/error.hh>
#include<cerberus-cpp/rules.hh>
#include<cerberus-cpp/stack.hh>
#include<cerberus-cpp/types.hh>

#include<yaml-cpp/yaml.h>

#include<functional>
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

      state.errors->clear();
      YAML::Node copy = YAML::Clone(data);
      state.setDocument(copy);
      state.validate(schema);
      return state.errors->empty();
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
        , schema_stack(std::make_shared<DocumentStack>())
        , document_stack(std::make_shared<DocumentStack>())
        , errors(std::make_shared<std::vector<ValidationErrorItem>>())
      {}

      RecursiveValidator(const RecursiveValidator& other)
        : validator(other.validator)
        , document(YAML::Clone(other.document))
        , schema_stack(other.schema_stack)
        , document_stack(other.document_stack)
        , errors(other.errors)
      {}

      void raiseError(ValidationErrorItem error)
      {
        errors->push_back(error);
      }

      void validateItem(const YAML::Node& schema)
      {
        schema_stack->push_back(schema);

        // Apply validation rules
        for(const auto priority : { RulePriority::NORMALIZATION,
                                    RulePriority::VALIDATION,
                                    RulePriority::TYPECHECKING })
          for(auto ruleval : schema)
          {
            auto rule = validator.rulemapping.find(ruleval.first.as<std::string>());
            if((rule != validator.rulemapping.end()) && (rule->second.first == priority))
            {
              schema_stack->push_back(ruleval.second);
              rule->second.second(*this);
              schema_stack->pop_back();
            }
          }

        schema_stack->pop_back();
      }

      bool validate(const YAML::Node& schema)
      {
        // Store the schema in validation state to have it accessible in rules
        schema_stack->push_back(schema);

        // Perform validation
        for(auto fieldrules : schema)
        {
          auto field = fieldrules.first.as<std::string>();
          auto rules = fieldrules.second;

          document_stack->push_back(getDocument()[field]);
          validateItem(rules);
          getDocument(1)[field] = getDocument();
          document_stack->pop_back();
        }

        schema_stack->pop_back();

        return errors->empty();
      }

      // TODO: This should be replaced with a more general error handling concept
      template<typename Stream>
      void printErrors(Stream& stream) const
      {
        for(auto error: *errors)
          stream << error.message << std::endl;
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
        return *(schema_stack->rbegin() + level);
      }

      YAML::Node& getDocument(std::size_t level = 0)
      {
        return *(document_stack->rbegin() + level);
      }

      void setDocument(YAML::Node& document)
      {
        document_stack->clear();
        document_stack->push_back(document);
      }

      Validator& validator;
      std::shared_ptr<DocumentStack> schema_stack;
      std::shared_ptr<DocumentStack> document_stack;
      YAML::Node document;
      std::shared_ptr<std::vector<ValidationErrorItem>> errors;
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