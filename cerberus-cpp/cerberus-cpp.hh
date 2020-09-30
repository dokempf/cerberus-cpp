#ifndef CERBERUS_CPP_HH

#include<yaml-cpp/yaml.h>

#include<exception>
#include<functional>
#include<map>
#include<string>

#include<iostream>


namespace Cerberus {

  class ValidationError
    : public std::exception
  {
    public:
    ValidationError(const std::string& message)
      : message_(message)
    {}

    virtual const char* what() const noexcept override
    {
      return message_.c_str();
    }

    private:
    std::string message_;
  };

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
      , document_(YAML::Node())
    {
      rulemapping["meta"] = [](Validator&, const YAML::Node&, const YAML::Node&){};

      rulemapping["type"] = [](Validator& v, const YAML::Node& schema, const YAML::Node& data)
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
      };

      rulemapping["required"] = [](Validator& v, const YAML::Node& schema, const YAML::Node& data)
      {
        if((schema.as<bool>()) && (data.IsNull()))
          v.raiseError({"Error: Missing required field!"});
      };

      // Normalization rules
      normalizationmapping["default"] = [](Validator&, const YAML::Node& schema, const YAML::Node& data, YAML::Node& vdata)
      {
        if(data.IsNull())
          vdata = schema;
      };

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

    bool validate(const YAML::Node& data)
    {
      return validate(data, schema_);
    }

    bool validate(const YAML::Node& data, const YAML::Node& schema)
    {
      // If the schema is ill-formed we throw an exception instead
      // of gathering as many errors as possible.
      if(!schema.IsMap())
        throw ValidationError("All schemas are expected to be mappings!");

      // TODO: Here: Validate the schema against the schema schema
      
      // Clean errors from previous runs
      errors.clear();

      // Clean return document from previous runs
      document_ = data;

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
        document_[fieldrules.first] = normalized_subdata;
      }

      // Perform validation
      for(auto fieldrules : schema)
      {
        auto field = fieldrules.first.as<std::string>();
        auto rules = fieldrules.second;

        if(!rules.IsMap())
          throw ValidationError("A mapping is expected in schemas for each field");
        
        auto subdata = document_[field];

        for(auto ruleval : rules)
        {
          try {
            applyRule(ruleval.first.as<std::string>(), ruleval.second, subdata);
          }
          catch(std::bad_function_call)
          {
            if(!normalizationmapping[ruleval.first.as<std::string>()])
            {
              std::cerr << "Unknown rule " << ruleval.first.as<std::string>() << std::endl;
              std::cerr << "This will be replaced with validation in the future";
            }
          }
        }
      }

      // We succeed iff there are no errors
      return errors.empty();
    }

    YAML::Node getDocument() const
    {
      return document_;
    }

    // TODO: This should be replaced with a more general error handling concept
    template<typename Stream>
    void printErrors(Stream& stream)
    {
      for(auto error: errors)
        stream << error.message << std::endl;
    }

    void raiseError(ValidationErrorItem error)
    {
      errors.push_back(error);
    }

    void applyRule(const std::string& name, const YAML::Node& schema, const YAML::Node& data)
    {
      rulemapping[name](*this, schema, data);
    }

    void applyNormalization(const std::string& name, const YAML::Node& schema, const YAML::Node& data, YAML::Node& vdata)
    {
      normalizationmapping[name](*this, schema, data, vdata);
    }

    bool applyType(const std::string& name, const YAML::Node& data)
    {
      return typesmapping[name](data);
    }

    private:
    YAML::Node schema_;
    YAML::Node document_;
    std::vector<ValidationErrorItem> errors;

    // The registered rules: The arguments these functions take are in this order
    // * The schema entry
    // * The data
    // * The normalized return data
    std::map<std::string, std::function<void(Validator&, const YAML::Node&, const YAML::Node&)>> rulemapping;
    std::map<std::string, std::function<void(Validator&, const YAML::Node&, const YAML::Node&, YAML::Node&)>> normalizationmapping;
    std::map<std::string, std::function<bool(const YAML::Node&)>> typesmapping;
  };

} // namespace Cerberus

#endif