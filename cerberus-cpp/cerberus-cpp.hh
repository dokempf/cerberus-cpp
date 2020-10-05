#ifndef CERBERUS_CPP_HH

#include<yaml-cpp/yaml.h>

#include<functional>
#include<map>
#include<memory>
#include<string>

#include<iostream>


namespace Cerberus {

  struct ValidationErrorItem
  {
    std::string message;
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
        YAML::Load("meta: {}"),
        [](ValidationState&, const YAML::Node&, const YAML::Node&){}
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
            if (!v.applyType(type, data))
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
          ValidationState vnew(v);
          vnew.document = data;
          v.normalize(schema);
          v.validate(schema);
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
      typesmapping[name] = [](const YAML::Node& node)
      {
        T val;
        return YAML::convert<T>::decode(node, val);
      };
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
      state.errors->clear();
      state.document = data;
      state.normalize(schema);
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
        , errors(std::make_shared<std::vector<ValidationErrorItem>>())
      {}

      void applyRule(const std::string& name, const YAML::Node& schema, const YAML::Node& data)
      {
        validator.rulemapping[name](*this, schema, data);
      }

      void applyNormalization(const std::string& name, const YAML::Node& schema, YAML::Node& data)
      {
        validator.normalizationmapping[name](*this, schema, data);
      }

      bool applyType(const std::string& name, const YAML::Node& data)
      {
        return validator.typesmapping[name](data);
      }

      void raiseError(ValidationErrorItem error)
      {
        errors->push_back(error);
      }

      void normalize(const YAML::Node& schema)
      {
        // Perform normalization
        for(auto fieldrules : schema)
        {
          YAML::Node subdata;
          if (auto d = document[fieldrules.first])
            subdata = d;

          for(auto ruleval : fieldrules.second)
          {
            try {
              applyNormalization(ruleval.first.as<std::string>(), ruleval.second, subdata);
            }
            catch(std::bad_function_call)
            {}
          }

          if(!subdata.IsNull())
            document[fieldrules.first] = subdata;
        }
      }

      bool validate(const YAML::Node& schema)
      {
        // Perform validation
        for(auto fieldrules : schema)
        {
          auto field = fieldrules.first.as<std::string>();
          auto rules = fieldrules.second;
          
          const auto& subdata = document[field];

          for(auto ruleval : rules)
          {
            try {
              applyRule(ruleval.first.as<std::string>(), ruleval.second, subdata);
            }
            catch(std::bad_function_call)
            {
              if(!validator.normalizationmapping[ruleval.first.as<std::string>()])
              {
                raiseError({"Unknown rule " + ruleval.first.as<std::string>()});
              }
            }
          }
        }

        return errors->empty();
      }

      // TODO: This should be replaced with a more general error handling concept
      template<typename Stream>
      void printErrors(Stream& stream) const
      {
        for(auto error: *errors)
          stream << error.message << std::endl;
      }

      Validator& validator;
      YAML::Node document;
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
    std::map<std::string, std::function<bool(const YAML::Node&)>> typesmapping;

    // The schema that is used to validate user provided schemas.
    // This is update with snippets as rules are registered
    YAML::Node schema_schema;
  };

} // namespace Cerberus

#endif