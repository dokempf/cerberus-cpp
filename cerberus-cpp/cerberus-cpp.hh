#ifndef CERBERUS_CPP_HH

#include<yaml-cpp/yaml.h>

#include<exception>
#include<functional>
#include<map>
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
        "meta",
        [](ValidationState&, const YAML::Node&, const YAML::Node&){}
      );

      registerRule(
        "type",
        [](ValidationState& v, const YAML::Node& schema, const YAML::Node& data)
        {
          if(data.IsNull())
            return;

          auto type = schema.as<std::string>();
          if (type == "list")
          {}
          else if(type == "dict")
          {}
          else
            if (!v.applyType(type, data))
              v.raiseError({"Error in type rule"});
        }
      );

      registerRule(
        "required",
        [](ValidationState& v, const YAML::Node& schema, const YAML::Node& data)
        {
          if((schema.as<bool>()) && (data.IsNull()))
            v.raiseError({"Error: Missing required field!"});
        }
      );

      // Normalization rules
      registerNormalizationRule(
        "default",
        [](ValidationState&, const YAML::Node& schema, const YAML::Node& data, YAML::Node& vdata)
        {
          if(data.IsNull())
            vdata = schema;
        }
      );

      // Populate the types map
      registerType<int>("integer");
      registerType<std::string>("string");
      registerType<long double>("float");
      registerType<bool>("boolean");
    }

    template<typename T>
    void registerType(const std::string& name)
    {
      typesmapping[name] = [](const YAML::Node& node)
      {
        try
        {
          node.as<T>();
          return true;
        }
        catch (...)
        {
          return false;
        }
      };
    }

    template<typename Rule>
    void registerRule(const std::string& name, Rule&& rule)
    {
      rulemapping[name] = std::forward<Rule>(rule);
    }

    template<typename Rule>
    void registerNormalizationRule(const std::string& name, Rule&& rule)
    {
      normalizationmapping[name] = std::forward<Rule>(rule);
    }

    bool validate(const YAML::Node& data)
    {
      return validate(data, schema_);
    }

    bool validate(const YAML::Node& data, const YAML::Node& schema)
    {
      state.errors.clear();
      state.document = data;
      YAML::Node normalized = state.normalize(data, schema);
      state.validate(data, schema);
      return state.errors.empty();
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
      {}

      void applyRule(const std::string& name, const YAML::Node& schema, const YAML::Node& data)
      {
        validator.rulemapping[name](*this, schema, data);
      }

      void applyNormalization(const std::string& name, const YAML::Node& schema, const YAML::Node& data, YAML::Node& vdata)
      {
        validator.normalizationmapping[name](*this, schema, data, vdata);
      }

      bool applyType(const std::string& name, const YAML::Node& data)
      {
        return validator.typesmapping[name](data);
      }

      void raiseError(ValidationErrorItem error)
      {
        errors.push_back(error);
      }

      YAML::Node normalize(const YAML::Node& data, const YAML::Node& schema)
      {
        // Perform normalization
        for(auto fieldrules : schema)
        {
          YAML::Node subdata;
          if (auto d = data[fieldrules.first])
            subdata = d;

          YAML::Node normalized_subdata(subdata);

          for(auto ruleval : fieldrules.second)
          {
            try {
              applyNormalization(ruleval.first.as<std::string>(), ruleval.second, subdata, normalized_subdata);
            }
            catch(std::bad_function_call)
            {}
          }
          document[fieldrules.first] = normalized_subdata;
        }

        return document;
      }

      bool validate(const YAML::Node& data, const YAML::Node& schema)
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
                std::cerr << "Unknown rule " << ruleval.first.as<std::string>() << std::endl;
                std::cerr << "This will be replaced with validation in the future";
              }
            }
          }
        }

        return errors.empty();
      }

      // TODO: This should be replaced with a more general error handling concept
      template<typename Stream>
      void printErrors(Stream& stream) const
      {
        for(auto error: errors)
          stream << error.message << std::endl;
      }

      Validator& validator;
      YAML::Node document;
      std::vector<ValidationErrorItem> errors;
    };

    YAML::Node schema_;
    ValidationState state;

    // The registered rules: The arguments these functions take are in this order
    // * The schema entry
    // * The data
    // * The normalized return data
    std::map<std::string, std::function<void(ValidationState&, const YAML::Node&, const YAML::Node&)>> rulemapping;
    std::map<std::string, std::function<void(ValidationState&, const YAML::Node&, const YAML::Node&, YAML::Node&)>> normalizationmapping;
    std::map<std::string, std::function<bool(const YAML::Node&)>> typesmapping;
  };

} // namespace Cerberus

#endif