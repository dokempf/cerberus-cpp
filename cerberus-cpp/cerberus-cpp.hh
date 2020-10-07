#ifndef CERBERUS_CPP_HH

#include<yaml-cpp/yaml.h>

#include<exception>
#include<functional>
#include<map>
#include<memory>
#include<regex>
#include<sstream>
#include<string>

#include<iostream>


namespace Cerberus {

  struct ValidationErrorItem
  {
    std::string message;
  };

  class CerberusError
    : public std::exception
  {};

  class SchemaError
    : public CerberusError
  {
    public:
    template<typename V>
    SchemaError(const V& v)
    {
      std::stringstream sstream;
      v.printErrors(sstream);
      message = sstream.str().c_str();
    }

    virtual const char* what() const noexcept override
    {
      return message;
    }

    private:
    const char* message;
  };

  enum class SchemaRuleType
  {
    UNSUPPORTED = 0,
    DICT = 1,
    LIST = 2
  };

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
      registerRule(
        YAML::Load(
          "allowed:\n"
          "  type: list\n"
        ),
        [](ValidationState& v, const YAML::Node& schema, const YAML::Node& data)
        {
          // Extract type information from the larger schema
          auto type = v.extractType();
          bool found = false;
          for(auto item: schema)
            if (type->equality(item, data))
              found = true;
          
          if(!found)
            v.raiseError({"Value disallowed by Allowed-Rule!"});
        }
      );

      registerRule(
        YAML::Load(
          "forbidden:\n"
          "  type: list"
        ),
        [](ValidationState& v, const YAML::Node& schema, const YAML::Node& data)
        {
          // Extract type information from the larger schema
          auto type = v.extractType();
          for(auto item: schema)
            if (type->equality(item, data))
              v.raiseError({"Forbidden-Rule violated: " + item.as<std::string>()});
        }
      );

      registerRule(
        YAML::Load(
          "items:\n"
          "  type: list"
        ),
        [](ValidationState& v, const YAML::Node& schema, const YAML::Node& data)
        {
          auto schemait = schema.begin();
          auto datait = data.begin();
          while (schemait != schema.end())
          {
            ValidationState vnew(v);
            vnew.document = YAML::Clone(*(datait++));
            vnew.validateItem(*(schemait++), vnew.document);
          }
        }
      );

      registerRule(
        YAML::Load(
          "keysrules:\n"
          "  type: dict"
        ),
        [](ValidationState& v, const YAML::Node& schema, const YAML::Node& data)
        {
          ValidationState vnew(v);
          for(auto item: data)
          {
            vnew.document = YAML::Clone(item.first);
            vnew.validateItem(schema, vnew.document);
          }
        }
      );

      registerRule(
        YAML::Load("meta: {}"),
        [](ValidationState&, const YAML::Node&, const YAML::Node&){}
      );

      registerRule(
        YAML::Load("max: {}"),
        [](ValidationState& v, const YAML::Node& schema, const YAML::Node& data)
        {
          // Extract type information from the larger schema
          auto type = v.extractType();

          if((type->less(schema, data)) || (type->equality(data, schema)))
            v.raiseError({"Max-Rrule violated!"});
        }
      );

      registerRule(
        YAML::Load("min: {}"),
        [](ValidationState& v, const YAML::Node& schema, const YAML::Node& data)
        {
          // Extract type information from the larger schema
          auto type = v.extractType();

          if(!(type->less(schema, data)))
            v.raiseError({"Min-Rule violated!"});
        }
      );

      registerRule(
        YAML::Load(
          "maxlength:\n"
          "  type: integer \n"
          "  min: 1"
        ),
        [](ValidationState& v, const YAML::Node& schema, const YAML::Node& data)
        {
          if(!((data.IsSequence()) || (data.IsMap())))
            v.raiseError({"Maxlength-Rule applied to non-iterable data container!"});
          else
          {
            unsigned int count = 0;
            for(auto item: data)
              ++count;
            if(count > schema.as<int>())
              v.raiseError({"Maxlength-Rule violated!"});
          }
        }
      );

      registerRule(
        YAML::Load(
          "minlength:\n"
          "  type: integer \n"
          "  min: 0"
        ),
        [](ValidationState& v, const YAML::Node& schema, const YAML::Node& data)
        {
          if(!((data.IsSequence()) || (data.IsMap())))
            v.raiseError({"Minlength-Rule applied to non-iterable data container!"});
          else
          {
            unsigned int count = 0;
            for(auto item: data)
              ++count;
            if(count < schema.as<int>())
              v.raiseError({"Minlength-Rule violated!"});
          }
        }
      );

      registerRule(
        YAML::Load(
          "regex: \n"
          "  type: string"
        ),
        [](ValidationState& v, const YAML::Node& schema, const YAML::Node& data)
        {
          if(!std::regex_match(data.as<std::string>(), std::regex(schema.as<std::string>())))
            v.raiseError({"Regex-Rule violated!"});
        }
      );

      registerRule(
        YAML::Load(
          "type: \n"
          "  type: string"
        ),
        [](ValidationState& v, const YAML::Node& schema, const YAML::Node& data)
        {
          if(data.IsNull())
            return;

          auto type = schema.as<std::string>();
          if (type == "list")
          {
            if(!data.IsSequence())
              v.raiseError({"Expecting a list"});
          }
          else if(type == "dict")
          {
            if(!data.IsMap())
              v.raiseError({"Expecting a map"});
          }
          else
            if (!v.validator.typesmapping[type]->is_convertible(data))
              v.raiseError({"Error in type rule"});
        }
      );

      registerRule(
        YAML::Load(
          "required:\n"
          "  type: boolean"
        ),
        [](ValidationState& v, const YAML::Node& schema, const YAML::Node& data)
        {
          if((schema.as<bool>()) && (data.IsNull()))
            v.raiseError({"Error: Missing required field!"});
        }
      );

      registerRule(
        YAML::Load(
          "schema:       \n"
          "  type: dict    "
        ),
        [](ValidationState& v, const YAML::Node& schema, const YAML::Node& data)
        {
          // Detect whether this is the schema(list) or schema(dict) rule by investigating
          // either the type information explicitly given or looking at the given data
          SchemaRuleType subrule = SchemaRuleType::UNSUPPORTED;
          auto typenode = v.schema_stack->back()["type"];
          if(typenode)
          {
            auto type = typenode.as<std::string>();
            if(type == "dict")
              subrule = SchemaRuleType::DICT;
            if(type == "list")
              subrule = SchemaRuleType::LIST;
          }
          else
          {
            if(data.IsMap())
              subrule = SchemaRuleType::DICT;
            if(data.IsSequence())
              subrule = SchemaRuleType::LIST;
          }

          if(subrule == SchemaRuleType::DICT)
          {
            ValidationState vnew(v);
            vnew.document = data;
            vnew.validate(schema);
          }
          if(subrule == SchemaRuleType::LIST)
          {
            ValidationState vnew(v);
            for(auto item: data)
            {
             vnew.document = YAML::Clone(item);
             vnew.validateItem(schema, vnew.document);
            }
          }
          if(subrule == SchemaRuleType::UNSUPPORTED)
            v.raiseError({"Schema-Rule is only available for type=dict|list"});
        }
      ); 

      registerRule(
        YAML::Load(
          "valuesrules:\n"
          "  type: dict"
        ),
        [](ValidationState& v, const YAML::Node& schema, const YAML::Node& data)
        {
          ValidationState vnew(v);
          for(auto item: data)
          {
            vnew.document = YAML::Clone(item.second);
            vnew.validateItem(schema, vnew.document);
          }
        }
      );


      // Normalization rules
      registerNormalizationRule(
        YAML::Load("default: {}"),
        [](ValidationState&, const YAML::Node& schema, YAML::Node& data)
        {
          if(data.IsNull())
            data = schema;
        }
      );

      // Populate the types map
      registerType<long long>("integer");
      registerType<std::string>("string");
      registerType<long double>("float");
      registerType<bool>("boolean");
    }

    template<typename T>
    void registerType(const std::string& name)
    {
      typesmapping[name] = std::make_shared<TypeItem<T>>();
    }

    template<typename Rule>
    void registerRule(const YAML::Node& schema, Rule&& rule)
    {
      schema_schema[schema.begin()->first] = schema.begin()->second;
      rulemapping[schema.begin()->first.as<std::string>()] = std::forward<Rule>(rule);
    }

    template<typename Rule>
    void registerNormalizationRule(const YAML::Node& schema, Rule&& rule)
    {
      schema_schema[schema.begin()->first] = schema.begin()->second;
      normalizationmapping[schema.begin()->first.as<std::string>()] = std::forward<Rule>(rule);
    }

    bool validate(const YAML::Node& data)
    {
      return validate(data, schema_);
    }

    bool validate(const YAML::Node& data, const YAML::Node& schema)
    {
      // Validate the schema first
      state.errors->clear();
      state.document = YAML::Clone(schema);
      state.validate(schema_schema);
      if(!state.errors->empty())
        throw SchemaError(state);

      state.errors->clear();
      state.document = YAML::Clone(data);
      state.validate(schema);
      return state.errors->empty();
    }

    YAML::Node getDocument() const
    {
      return state.document;
    }

    template<typename Stream>
    void printErrors(Stream& stream) const
    {
      return state.printErrors(stream);
    }

    private:
    struct ValidationState
    {
      ValidationState(Validator& validator)
        : validator(validator)
        , schema_stack(std::make_shared<std::vector<YAML::Node>>())
        , errors(std::make_shared<std::vector<ValidationErrorItem>>())
      {}

      ValidationState(const ValidationState& other)
        : validator(other.validator)
        , document(YAML::Clone(other.document))
        , schema_stack(other.schema_stack)
        , errors(other.errors)
      {}

      void raiseError(ValidationErrorItem error)
      {
        errors->push_back(error);
      }

      void validateItem(const YAML::Node& schema, YAML::Node& data)
      {
        schema_stack->push_back(schema);

        // Apply normalization rules
        for(auto ruleval : schema)
        {
          auto rule = ruleval.first.as<std::string>();
          if(validator.normalizationmapping.find(rule) != validator.normalizationmapping.end())
            validator.normalizationmapping[rule](*this, ruleval.second, data);
        }

        // Apply validation rules
        for(auto ruleval : schema)
        {
          auto rule = ruleval.first.as<std::string>();
          if(validator.rulemapping.find(rule) != validator.rulemapping.end())
            validator.rulemapping[rule](*this, ruleval.second, data);
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

          YAML::Node subdata;
          if (auto d = document[field])
            subdata = d;

          validateItem(rules, subdata);
          document[field] = subdata;
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

      const std::shared_ptr<TypeItemBase>& extractType()
      {
        auto type = schema_stack->back()["type"].as<std::string>();
        return validator.typesmapping[type];
      }

      Validator& validator;
      YAML::Node document;
      std::shared_ptr<std::vector<YAML::Node>> schema_stack;
      std::shared_ptr<std::vector<ValidationErrorItem>> errors;
    };

    YAML::Node schema_;
    ValidationState state;

    // The registered rules: The arguments these functions take are in this order
    // * The schema entry
    // * The data
    // * The normalized return data
    std::map<std::string, std::function<void(ValidationState&, const YAML::Node&, const YAML::Node&)>> rulemapping;
    std::map<std::string, std::function<void(ValidationState&, const YAML::Node&, YAML::Node&)>> normalizationmapping;
    std::map<std::string, std::shared_ptr<TypeItemBase>> typesmapping;

    // The schema that is used to validate user provided schemas.
    // This is update with snippets as rules are registered
    YAML::Node schema_schema;
  };

} // namespace Cerberus

#endif