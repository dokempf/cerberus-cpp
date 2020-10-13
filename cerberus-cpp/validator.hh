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
    //! Default construct a validator instance
    Validator()
      : Validator(YAML::Node())
    {}

    /** @brief Construct a validator with a given schema
     *
     * @param schema The schema that this validator should be
     *               validating against.
     */
    Validator(const YAML::Node& schema)
      : schema_(schema)
      , state(*this, YAML::Node())
      , validate_schema(true)
    {
      registerBuiltinRules(*this);
      registerBuiltinTypes(*this);
    }

    /** @brief Register a type for use in schemas
     *
     * This method allows registering additional C++ types to be accessible
     * to the @c type rule of schemas. The given type is expected to fulfill
     * the following three properties:
     * * @c operator== exists
     * * @c operator< exists
     * * A template specialization of @c YAML::convert for the type must
     *   exist. For more information on how to write these specialization check
     *   the yaml-cpp docs: https://github.com/jbeder/yaml-cpp/wiki/Tutorial#converting-tofrom-native-data-types
     *
     * @tparam T The C++ type that should be registered - check above limitations
     * @param name The name that the type should be given
     */
    template<typename T>
    void registerType(const std::string& name)
    {
      typesmapping[name] = std::make_shared<TypeItem<T>>();
    }

    /** @brief Register a custom validation rule
     *
     * This method registers a custom valiation rule which well then be available
     * from schemas. Rules are expected to be callables that accept a reference to
     * a @c ValidationRuleInterface instance which they can use to implement their
     * custom behaviour. Furthermore, they are expected to return @c void.
     *
     * @param schema A YAML mapping that gives a schema that describes how the rule
     *               is used. This schema is used to incrementally built a schema that
     *               validates user-provided schemas.
     * @param rule The callable that implements the rule.
     * @param priority specifies when in the validation process this rule is executed.
     */
    template<typename Rule>
    void registerRule(const YAML::Node& schema, Rule&& rule, RulePriority priority = RulePriority::VALIDATION)
    {
      schema_schema[schema.begin()->first] = schema.begin()->second;
      rulemapping[std::make_pair(priority, schema.begin()->first.as<std::string>())] = std::forward<Rule>(rule);
    }

    /** @brief Register a schema to reference within larger schema
     *
     * This registers a schema with the validator in order to have it
     * be referenced by name in schemas. Whenever the value for a
     * @c schema rule is not a mapping, it is expected to be in the
     * validator's registered schemas.
     *
     * @param name The name for the registered schema
     * @param schema The YAML::Node that represents the schema
     */
    void registerSchema(const std::string& name, const YAML::Node& schema)
    {
      schema_registry[name] = YAML::Clone(schema);
    }

    /** @brief Set the validators policy regarding unknown values.
     *
     * This can be used to change the validator's policy whether it should
     * accept values that are not explicitly mentioned in the schema. By
     * default, it does not allow these. For subdocuments, this can be
     * overwritten from the schema by using the @c allow_unknown validation rule.
     *
     * @param value The new value of the allow unknowns policy
     */
    void setAllowUnknown(bool value)
    {
      state.setAllowUnknown(value);
    }

    /** @brief Set the validators policy regarding purging unknown values.
     *
     * This can be used to change the validator's policy whether it should
     * purge values that are not explicitly mentioned in the schema. By
     * default, it does not do so. For subdocuments, this can be
     * overwritten from the schema by using the @c purge_unknown normalization rule.
     *
     * @param value The new value of the purge unknowns policy
     */
    void setPurgeUnknown(bool value)
    {
      state.setPurgeUnknown(value);
    }

    /** @brief Set the validators policy regarding requiring all keys
     *
     * This can be used the change the validator's policy whether all keys
     * in a document are required, just as if they all had the required rule
     * applied. For subdocuments, this can be overwritten from the schema by
     * using the @c require_all validation rule.
     */
    void setRequireAll(bool value)
    {
      state.setRequireAll(value);
    }

    /** @brief Whether schemas given to this Validator should themselves be validated */
    void setSchemaValidation(bool value)
    {
      validate_schema = value;
    }

    /** @brief Validate a given document
     *
     * This is one of the end user entrypoints to perform validation.
     * The document will be validated against the schema passed to the
     * class constructor.
     *
     * @param document The document to validate
     */
    bool validate(const YAML::Node& document)
    {
      return validate(document, schema_);
    }

    /** @brief Validate a given document against a given schema
     *
     * This is one of the end user entrypoints to perform validation
     *
     * @param document The document to validate
     * @param schema The schema to validate against
     */
    bool validate(const YAML::Node& document, const YAML::Node& schema)
    {
      YAML::Node validated_schema;
      if(validate_schema)
      {
        Validator schema_validator(schema_schema);
        schema_validator.setSchemaValidation(false);
        for(auto entries: schema)
        {
          if(!schema_validator.validate(entries.second))
            throw SchemaError(schema_validator);
          validated_schema[entries.first] = schema_validator.getDocument();
        }
      }
      else
        validated_schema = schema;

      state.reset(document);
      state.validateDict(validated_schema);
      return state.success();
    }

    /** @brief Retrieves the normalized document after validation
     *
     * This is only valid after @ref validate has been called.
     */
    const YAML::Node& getDocument()
    {
      return state.getDocument();
    }

    /** @brief Print errors to a stream
     *
     * This also accessible through the overloaded stream operator<<.
     */
    template<typename Stream>
    void printErrors(Stream& stream) const
    {
      return state.printErrors(stream);
    }

    private:
    /** @brief The interface that validation rules can use
     *
     * This class does the actual recursive validation of data.
     * A reference to an instance of this class is handed to the
     * validation rules. If you implement a custom rule, you should
     * have a very close look at this interface, otherwise you
     * can consider it an implementation detail.
     */
    class ValidationRuleInterface
    {
      public:
      //! Construct the rule interface given the Validator instance
      ValidationRuleInterface(Validator& validator, const YAML::Node& document)
        : validator(validator)
        , allow_unknown(false)
        , purge_unknown(false)
        , require_all(false)
      {
        document_stack.reset(YAML::Clone(document));
      }

      /** @brief Report an error from the validation process
       *
       * Validation errors in cerberus-cpp do not throw exceptions or
       * otherwise terminate execution, but they are stored and reported
       * upon request. This way we are able to find more errors than just
       * the first one. Call this method to report an error from within
       * a validation rule. Additional information like the currently
       * processed subdocument will automatically be recorded.
       *
       * @param error The (telling!) error message that the validation rule reports
       */
      void raiseError(const std::string& error)
      {
        errors.push_back({document_stack.stringPath(), error});
      }

      /** @brief Validates a document item
       *
       * This is the main algorithm that actually applies validation rules.
       * A custom validation rule should call this to validate the top item
       * of the document stack against the given schema.
       *
       * @param schema Will go away in favor of the schema already being pushed on the stack
       */
      void validateItem(YAML::Node& schema)
      {
        schema_stack.push_back(schema);

        // Apply validation rules
        for(const auto priority : { RulePriority::FIRST,
                                    RulePriority::NORMALIZATION,
                                    RulePriority::VALIDATION,
                                    RulePriority::TYPECHECKING,
                                    RulePriority::POST_NORMALIZATION,
                                    RulePriority::LAST })
        {
          // Implement the require all policy of the validator
          if((priority == RulePriority::NORMALIZATION) && (require_all))
            schema["required"] = "true";

          for(auto ruleval : schema)
          {
            auto rule = validator.rulemapping.find(std::make_pair(priority, ruleval.first.as<std::string>()));
            if(rule != validator.rulemapping.end())
            {
              schema_stack.push_back(ruleval.second);
              rule->second(*this);
              schema_stack.pop_back();
            }
          }
        }

        schema_stack.pop_back();
      }

      /** @brief Validates a document dictionary
       *
       * This algorithm traverses through key/value pairs in the document
       * and applies the main validation algorithm to each such pair.
       * A custom validation rule should call this to validate the top item
       * of the document stack against the given schema - if that item is a
       * dictionary like e.g. the top-level document.
       *
       * @param schema Will go away in favor of the schema already being pushed on the stack
       */
      bool validateDict(const YAML::Node& schema)
      {
        // Store the schema in validation state to have it accessible in rules
        schema_stack.push_back(schema);

        // Perform validation
        std::vector<std::string> found;
        for(auto fieldrules : schema)
        {
          setCurrentField(fieldrules.first.as<std::string>());
          auto rules = fieldrules.second;
          document_stack.pushDictItem(field);
          validateItem(rules);
          getDocument(1)[field] = getDocument();
          found.push_back(field);
          document_stack.pop();
        }

        if((purge_unknown) && (found.size() != getDocument().size()))
        {
          YAML::Node newnode;
          for(auto item : getDocument())
            if(std::find(found.begin(), found.end(), item.first.as<std::string>()) != found.end())
              newnode[item.first] = item.second;
          document_stack.replaceBack(newnode);
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

      //! Print errors to a stream
      template<typename Stream>
      void printErrors(Stream& stream) const
      {
        for(auto error: errors)
        {
          stream << "Error validating data field " << error.path << std::endl;
          stream << "Message: " << error.message << std::endl;
        }
      }

      /** @brief Get a type implementation for an explicitly known type
       *
       * This method retrieves a type implementation as registered with the
       * @c registerType method of the @c Validator class. A validation rule
       * implementation should use this whenever it needs to compare objects.
       * If you need to extract the @c name parameter from the schema, you
       * should use the specialized overload of this function instead.
       *
       * @param name The name of the type in the @c Validator s type registry.
       * @returns an @c std::shared_ptr to the @c TypeItemBase instance
       */
      const std::shared_ptr<TypeItemBase>& getType(const std::string& name)
      {
        return validator.typesmapping[name];
      }

      /** @brief extract a type implementation from the schema
       *
       * This method retrieves a type implementation as registered with the
       * @c registerType method of the @c Validator class. A validation rule
       * implementation should use this whenever it needs to compare objects.
       *
       * @param level The schema level that the @c type information should
       *              be taken from just like in the @c getSchema method.
       * @returns an @c std::shared_ptr to the @c TypeItemBase instance
       */
      const std::shared_ptr<TypeItemBase>& getType(std::size_t level = 1)
      {
        auto type = getSchema(level)["type"].as<std::string>();
        return validator.typesmapping[type];
      }

      /** @brief Get the YAML::Node of the schema we are currently validating against.
       *
       * This is the method of choice to retrieve the schema from a validation
       * rule implementation. The recursive algorithm implemented by @c ValidationRuleInterface
       * will keep track of a stack of schema while it goes into recursion.
       * It is possible to inspect all entries of this stack by specifying the
       * @c level parameter.
       *
       * @param level The level of the schema stack that we are interested in.
       *              @c level=0 will deliver the top stack item, a.k.a. the current
       *              document.
       * @param is_full_schema Whether the returned schema snippet is expected to
       *                       be a full schema in itself as e.g. required by the
       *                       @c schema rule. This will involve potential lookup
       *                       of registered schemas.
       * @returns A reference to the node of the schema that we are currently
       *          validating against. If @c level was given, a schema
       *          further down the stack will be returned.
       */
      YAML::Node& getSchema(std::size_t level = 0, bool is_full_schema = false)
      {
        YAML::Node& schema = schema_stack.get(level);
        if((is_full_schema) && (schema.IsScalar()))
          return validator.schema_registry[schema.as<std::string>()];
        else
          return schema;
      }

      /** @brief Get the YAML::Node of the currently validated document
       *
       * This is the method of choice to retrieve the document from a validation
       * rule implementation. The recursive algorithm implemented by @c ValidationRuleInterface
       * will keep track of a stack of documents while it goes into recursion.
       * It is possible to inspect all entries of this stack by specifying the
       * @c level parameter.
       *
       * @param level The level of the document stack that we are interested in.
       *              @c level=0 will deliver the top stack item, a.k.a. the current
       *              document.
       * @returns A reference to the node of the document that currently
       *          needs to be validated. If @c level was given, a document
       *          further down the stack will be returned.
       */
      YAML::Node& getDocument(std::size_t level = 0)
      {
        return document_stack.get(level);
      }

      /** @brief Lookup a part of the document according to a key string
       *
       * This method can be used to access subdocuments for rules that cross-reference
       * data, such as e.g. the @c dependencies rule.
       *
       * @param key The key to look up
       * @param level The subdocument level just as used in the @c getDocument method.
       */
      YAML::Node getDocumentPath(const std::string& key, std::size_t level = 0)
      {
        return document_stack.pathLookup(key, level);
      }

      //! Get the validators current policy about accepting unknown values
      bool getAllowUnknown() const
      {
        return allow_unknown;
      }

      /** @brief Sets the validator's behaviour of accepting unknown values
       *
       * @param value The new value for the allow unknown policy
       *
       * This is most likely only interesting for implementation of the @c
       * allow_unknown rule.
       */
      void setAllowUnknown(bool value)
      {
        allow_unknown = value;
      }

      //! Get the validators current policy about purging unknown values
      bool getPurgeUnknown() const
      {
        return purge_unknown;
      }

      /** @brief Sets the validator's behaviour of purging unknown values
       *
       * @param value The new value for the purge unknown policy
       *
       * This is most likely only interesting for implementation of the @c
       * purge_unknown rule.
       */
      void setPurgeUnknown(bool value)
      {
        purge_unknown = value;
      }

      //! Get the validator's policy about requiring all unknown values
      bool getRequireAll() const
      {
        return require_all;
      }

      /** @brief Sets the validator's behaviour of requiring all keys
       *
       * @param value The new value for the require all policy
       *
       * This is most likely only interesting for the implementation of the @c
       * require_all rule.
       */
      void setRequireAll(bool value)
      {
        require_all = value;
      }

      bool success() const
      {
        return errors.empty();
      }

      void reset(const YAML::Node& document)
      {
        errors.clear();
        document_stack.reset(YAML::Clone(document));
      }

      void setCurrentField(const std::string& field_)
      {
        field = field_;
      }

      const std::string& getCurrentField()
      {
        return field;
      }

      // TODO: These should move to private. In order to do so, all rules implementations
      //       need to go through one of the above interfaces.
      DocumentStack schema_stack;
      DocumentStack document_stack;

      private:
      Validator& validator;
      std::vector<ValidationErrorItem> errors;
      bool allow_unknown;
      bool purge_unknown;
      bool require_all;
      std::string field;
    };

    YAML::Node schema_;
    ValidationRuleInterface state;

    std::map<std::pair<RulePriority, std::string>, std::function<void(ValidationRuleInterface&)>> rulemapping;
    std::map<std::string, std::shared_ptr<TypeItemBase>> typesmapping;
    std::map<std::string, YAML::Node> schema_registry;

    // The schema that is used to validate user provided schemas.
    // This is update with snippets as rules are registered
    YAML::Node schema_schema;
    bool validate_schema;
  };

  //! overload stream operator for easy printing of errors
  template<typename Stream>
  Stream& operator<<(Stream& stream, const Validator& v)
  {
    v.printErrors(stream);
    return stream;
  }

} // namespace Cerberus

#endif