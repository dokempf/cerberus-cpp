#ifndef CERBERUS_CPP_RULES_HH
#define CERBERUS_CPP_RULES_HH

#include<yaml-cpp/yaml.h>

#include<regex>
#include<string>

namespace Cerberus {

  enum class RulePriority
  {
    NORMALIZATION = 0,
    VALIDATION = 1,
    TYPECHECKING = 2
  };

  namespace impl {
    
    template<typename Validator>
    void allowed_rule(Validator& validator)
    {
      validator.registerRule(
        YAML::Load(
          "allowed:\n"
          "  type: list\n"
        ),
        [](auto& v)
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
    }

    template<typename Validator>
    void contains_rule(Validator& validator)
    {
      validator.registerRule(
        YAML::Load(
          "contains: {}"
        ),
        [](auto& v)
        {
          std::vector<YAML::Node> needed;
          if(v.getSchema().IsScalar())
            needed.push_back(v.getSchema());
          else if(v.getSchema().IsSequence())
            for(auto item: v.getSchema())
              needed.push_back(item);
          else
          {
            v.raiseError({"Contains-Rule expects value or list!"});
            return;
          }
          
          for(auto item: v.document)
            for(auto it = needed.begin(); it != needed.end();)
              if (v.extractType("string")->equality(*it, item))
                it = needed.erase(it);
              else
                ++it;

          if(!needed.empty())
            v.raiseError({"Contains-Rule violated"});
        }
      );
    }

    template<typename Validator>
    void default_rule(Validator& validator)
    {
      // Normalization rules
      validator.registerRule(
        YAML::Load("default: {}"),
        [](auto& v)
        {
          if(!v.document.IsDefined())
            v.document = v.getSchema();
        },
        RulePriority::NORMALIZATION
      );
    }

    template<typename Validator>
    void forbidden_rule(Validator& validator)
    {
      validator.registerRule(
        YAML::Load(
          "forbidden:\n"
          "  type: list"
        ),
        [](auto& v)
        {
          for(auto item: v.getSchema())
            if (v.extractType()->equality(item, v.document))
              v.raiseError({"Forbidden-Rule violated: " + item.template as<std::string>()});
        }
      );
    }

    template<typename Validator>
    void items_rule(Validator& validator)
    {
      validator.registerRule(
        YAML::Load(
          "items:\n"
          "  type: list"
        ),
        [](auto& v)
        {
          auto schemait = v.getSchema().begin();
          auto datait = v.document.begin();
          while (schemait != v.getSchema().end())
          {
            decltype(v) vnew(v);
            vnew.document = YAML::Clone(*(datait++));
            vnew.validateItem(*(schemait++), vnew.document);
          }
        }
      );
    }

    template<typename Validator>
    void keysrules_rule(Validator& validator)
    {
      validator.registerRule(
        YAML::Load(
          "keysrules:\n"
          "  type: dict"
        ),
        [](auto& v)
        {
          decltype(v) vnew(v);
          for(auto item: v.document)
          {
            vnew.document = YAML::Clone(item.first);
            vnew.validateItem(v.getSchema(), vnew.document);
          }
        }
      );
    }

    template<typename Validator>
    void meta_rule(Validator& validator)
    {
      validator.registerRule(
        YAML::Load("meta: {}"),
        [](auto&){}
      );
    }

    template<typename Validator>
    void max_rule(Validator& validator)
    {
      validator.registerRule(
        YAML::Load("max: {}"),
        [](auto& v)
        {
          // Extract type information from the larger schema
          auto type = v.extractType();

          if((type->less(v.getSchema(), v.document)) || (type->equality(v.document, v.getSchema())))
            v.raiseError({"Max-Rrule violated!"});
        }
      );
    }

    template<typename Validator>
    void min_rule(Validator& validator)
    {
      validator.registerRule(
        YAML::Load("min: {}"),
        [](auto& v)
        {
          // Extract type information from the larger schema
          auto type = v.extractType();

          if(!(type->less(v.getSchema(), v.document)))
            v.raiseError({"Min-Rule violated!"});
        }
      );
    }

    template<typename Validator>
    void maxlength_rule(Validator& validator)
    {
      validator.registerRule(
        YAML::Load(
          "maxlength:\n"
          "  type: integer \n"
          "  min: 1"
        ),
        [](auto& v)
        {
          if(!((v.document.IsSequence()) || (v.document.IsMap())))
            v.raiseError({"Maxlength-Rule applied to non-iterable data container!"});
          else
          {
            unsigned int count = 0;
            for(auto item: v.document)
              ++count;
            if(count > v.getSchema().template as<int>())
              v.raiseError({"Maxlength-Rule violated!"});
          }
        }
      );
    }

    template<typename Validator>
    void minlength_rule(Validator& validator)
    {
      validator.registerRule(
        YAML::Load(
          "minlength:\n"
          "  type: integer \n"
          "  min: 0"
        ),
        [](auto& v)
        {
          if(!((v.document.IsSequence()) || (v.document.IsMap())))
            v.raiseError({"Minlength-Rule applied to non-iterable data container!"});
          else
          {
            unsigned int count = 0;
            for(auto item: v.document)
              ++count;
            if(count < v.getSchema().template as<int>())
              v.raiseError({"Minlength-Rule violated!"});
          }
        }
      );
    }

    template<typename Validator>
    void regex_rule(Validator& validator)
    {
      validator.registerRule(
        YAML::Load(
          "regex: \n"
          "  type: string"
        ),
        [](auto& v)
        {
          if(!std::regex_match(v.document.template as<std::string>(), std::regex(v.getSchema().template as<std::string>())))
            v.raiseError({"Regex-Rule violated!"});
        }
      );
    }

    template<typename Validator>
    void type_rule(Validator& validator)
    {
      validator.registerRule(
        YAML::Load(
          "type: \n"
          "  type: string"
        ),
        [](auto& v)
        {
          if(v.document.IsNull())
            return;

          auto type = v.getSchema().template as<std::string>();
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
            if (!v.extractType()->is_convertible(v.document))
              v.raiseError({"Error in type rule"});
        },
        RulePriority::TYPECHECKING
      );
    }

    template<typename Validator>
    void required_rule(Validator& validator)
    {
      validator.registerRule(
        YAML::Load(
          "required:\n"
          "  type: boolean"
        ),
        [](auto& v)
        {
          if((v.getSchema().template as<bool>()) && (v.document.IsNull()))
            v.raiseError({"Error: Missing required field!"});
        }
      );
    }

    enum class SchemaRuleType
    {
      UNSUPPORTED = 0,
      DICT = 1,
      LIST = 2
    };

    template<typename Validator>
    void schema_rule(Validator& validator)
    {
      validator.registerRule(
        YAML::Load(
          "schema:       \n"
          "  type: dict    "
        ),
        [](auto& v)
        {
          // Detect whether this is the schema(list) or schema(dict) rule by investigating
          // either the type information explicitly given or looking at the given data
          SchemaRuleType subrule = SchemaRuleType::UNSUPPORTED;
          auto typenode = v.getSchema(1)["type"];
          if(typenode)
          {
            auto type = typenode.template as<std::string>();
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
            decltype(v) vnew(v);
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
    }

    template<typename Validator>
    void valuesrules_rule(Validator& validator)
    {
      validator.registerRule(
        YAML::Load(
          "valuesrules:\n"
          "  type: dict"
        ),
        [](auto& v)
        {
          decltype(v) vnew(v);
          for(auto item: v.document)
          {
            vnew.document = YAML::Clone(item.second);
            vnew.validateItem(v.getSchema(), vnew.document);
          }
        }
      );
    }

  } // namespace impl

  template<typename Validator>
  void registerBuiltinRules(Validator& v)
  {
    impl::allowed_rule(v);
    impl::contains_rule(v);
    impl::default_rule(v);
    impl::forbidden_rule(v);
    impl::items_rule(v);
    impl::keysrules_rule(v);
    impl::max_rule(v);
    impl::min_rule(v);
    impl::maxlength_rule(v);
    impl::minlength_rule(v);
    impl::regex_rule(v);
    impl::required_rule(v);
    impl::schema_rule(v);
    impl::type_rule(v);
    impl::valuesrules_rule(v);
  }

} // namespace Cerberus

#endif