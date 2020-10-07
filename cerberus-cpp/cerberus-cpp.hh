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

  enum class RulePriority
  {
    NORMALIZATION = 0,
    VALIDATION = 1,
    TYPECHECKING = 2
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
        [](RecursiveValidator& v)
        {
          // Extract type information from the larger schema
          auto type = v.extractType();
          bool found = false;
          for(auto item: v.getSchema())
            if (type->equality(item, v.document))
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
        [](RecursiveValidator& v)
        {
          // Extract type information from the larger schema
          auto type = v.extractType();
          for(auto item: v.getSchema())
            if (type->equality(item, v.document))
              v.raiseError({"Forbidden-Rule violated: " + item.as<std::string>()});
        }
      );

      registerRule(
        YAML::Load(
          "items:\n"
          "  type: list"
        ),
        [](RecursiveValidator& v)
        {
          auto schemait = v.getSchema().begin();
          auto datait = v.document.begin();
          while (schemait != v.getSchema().end())
          {
            RecursiveValidator vnew(v);
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
        [](RecursiveValidator& v)
        {
          RecursiveValidator vnew(v);
          for(auto item: v.document)
          {
            vnew.document = YAML::Clone(item.first);
            vnew.validateItem(v.getSchema(), vnew.document);
          }
        }
      );

      registerRule(
        YAML::Load("meta: {}"),
        [](RecursiveValidator&){}
      );

      registerRule(
        YAML::Load("max: {}"),
        [](RecursiveValidator& v)
        {
          // Extract type information from the larger schema
          auto type = v.extractType();

          if((type->less(v.getSchema(), v.document)) || (type->equality(v.document, v.getSchema())))
            v.raiseError({"Max-Rrule violated!"});
        }
      );

      registerRule(
        YAML::Load("min: {}"),
        [](RecursiveValidator& v)
        {
          // Extract type information from the larger schema
          auto type = v.extractType();

          if(!(type->less(v.getSchema(), v.document)))
            v.raiseError({"Min-Rule violated!"});
        }
      );

      registerRule(
        YAML::Load(
          "maxlength:\n"
          "  type: integer \n"
          "  min: 1"
        ),
        [](RecursiveValidator& v)
        {
          if(!((v.document.IsSequence()) || (v.document.IsMap())))
            v.raiseError({"Maxlength-Rule applied to non-iterable data container!"});
          else
          {
            unsigned int count = 0;
            for(auto item: v.document)
              ++count;
            if(count > v.getSchema().as<int>())
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
        [](RecursiveValidator& v)
        {
          if(!((v.document.IsSequence()) || (v.document.IsMap())))
            v.raiseError({"Minlength-Rule applied to non-iterable data container!"});
          else
          {
            unsigned int count = 0;
            for(auto item: v.document)
              ++count;
            if(count < v.getSchema().as<int>())
              v.raiseError({"Minlength-Rule violated!"});
          }
        }
      );

      registerRule(
        YAML::Load(
          "regex: \n"
          "  type: string"
        ),
        [](RecursiveValidator& v)
        {
          if(!std::regex_match(v.document.as<std::string>(), std::regex(v.getSchema().as<std::string>())))
            v.raiseError({"Regex-Rule violated!"});
        }
      );

      registerRule(
        YAML::Load(
          "type: \n"
          "  type: string"
        ),
        [](RecursiveValidator& v)
        {
          if(v.document.IsNull())
            return;

          auto type = v.getSchema().as<std::string>();
          if (type == "list")
          {
            if(!v.document.IsSequence())
              v.raiseError({"Expecting a list"});
          }
          else if(type == "dict")
          {
            if(!v.document.IsMap())
              v.raiseError({"Expecting a map"});
          }
          else
            if (!v.validator.typesmapping[type]->is_convertible(v.document))
              v.raiseError({"Error in type rule"});
        },
        RulePriority::TYPECHECKING
      );

      registerRule(
        YAML::Load(
          "required:\n"
          "  type: boolean"
        ),
        [](RecursiveValidator& v)
        {
          if((v.getSchema().as<bool>()) && (v.document.IsNull()))
            v.raiseError({"Error: Missing required field!"});
        }
      );

      registerRule(
        YAML::Load(
          "schema:       \n"
          "  type: dict    "
        ),
        [](RecursiveValidator& v)
        {
          // Detect whether this is the schema(list) or schema(dict) rule by investigating
          // either the type information explicitly given or looking at the given data
          SchemaRuleType subrule = SchemaRuleType::UNSUPPORTED;
          auto typenode = v.getSchema(1)["type"];
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
            if(v.document.IsMap())
              subrule = SchemaRuleType::DICT;
            if(v.document.IsSequence())
              subrule = SchemaRuleType::LIST;
          }

          if(subrule == SchemaRuleType::DICT)
          {
            v.validate(v.getSchema());
          }
          if(subrule == SchemaRuleType::LIST)
          {
            RecursiveValidator vnew(v);
            for(auto item: v.document)
            {
             vnew.document = YAML::Clone(item);
             vnew.validateItem(v.getSchema(), vnew.document);
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
        [](RecursiveValidator& v)
        {
          RecursiveValidator vnew(v);
          for(auto item: v.document)
          {
            vnew.document = YAML::Clone(item.second);
            vnew.validateItem(v.getSchema(), vnew.document);
          }
        }
      );


      // Normalization rules
      registerRule(
        YAML::Load("default: {}"),
        [](RecursiveValidator& v)
        {
          if(!v.document.IsDefined())
            v.document = v.getSchema();
        },
        RulePriority::NORMALIZATION
      );

      // Populate the types map
      registerType<long long>("integer");
      registerType<std::string>("string");
      registerType<long double>("float");
      registerType<long double>("number");
      registerType<bool>("boolean");
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
    struct RecursiveValidator
    {
      RecursiveValidator(Validator& validator)
        : validator(validator)
        , schema_stack(std::make_shared<std::vector<YAML::Node>>())
        , errors(std::make_shared<std::vector<ValidationErrorItem>>())
      {}

      RecursiveValidator(const RecursiveValidator& other)
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

          RecursiveValidator vnew(*this);
          vnew.document = document[field];
          vnew.validateItem(rules, vnew.document);
          document[field] = vnew.document;
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
        auto type = getSchema(1)["type"].as<std::string>();
        return validator.typesmapping[type];
      }

      const YAML::Node& getSchema(std::size_t index = 0) const
      {
        return *(schema_stack->rbegin() + index);
      }

      Validator& validator;
      YAML::Node document;
      std::shared_ptr<std::vector<YAML::Node>> schema_stack;
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