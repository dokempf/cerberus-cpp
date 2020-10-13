#ifndef CERBERUS_CPP_RULES_HH
#define CERBERUS_CPP_RULES_HH

#include<yaml-cpp/yaml.h>

#include<memory>
#include<regex>
#include<string>
#include<vector>

#include<iostream>

namespace Cerberus {

  enum class RulePriority
  {
    FIRST = 0,
    NORMALIZATION = 1,
    VALIDATION = 2,
    TYPECHECKING = 3,
    POST_NORMALIZATION = 4,
    LAST = 5
  };

  namespace impl {
    
    template<typename Validator>
    void allow_unknown_rule(Validator& validator)
    {
      auto state = std::make_shared<std::vector<bool>>();

      validator.registerRule(
        YAML::Load(
          "allow_unknown:\n"
          " type: boolean"
        ),
        [state](auto& v)
        {
          state->push_back(v.getAllowUnknown());
          v.setAllowUnknown(v.getSchema().template as<bool>());
        },
        RulePriority::FIRST
      );

      validator.registerRule(
        YAML::Load(
          "allow_unknown:\n"
          " type: boolean"
        ),
        [state](auto& v)
        {
          v.setAllowUnknown(state->back());
          state->pop_back();
        },
        RulePriority::LAST
      );
    }

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
          auto type = v.getType(1);
          bool found = false;
          for(auto item: v.getSchema())
            if (type->equality(item, v.getDocument()))
              found = true;
          
          if(!found)
            v.raiseError("Value disallowed by Allowed-Rule!");
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
            v.raiseError("Contains-Rule expects value or list!");
            return;
          }
          
          for(auto item: v.getDocument())
            for(auto it = needed.begin(); it != needed.end();)
              if (v.getType("string")->equality(*it, item))
                it = needed.erase(it);
              else
                ++it;

          if(!needed.empty())
            v.raiseError("Contains-Rule violated");
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
          if(!v.getDocument().IsDefined())
            v.getDocument() = v.getSchema();
        },
        RulePriority::NORMALIZATION
      );
    }

    template<typename Validator>
    void dependencies_rule(Validator& validator)
    {
      validator.registerRule(
        YAML::Load(
          "dependencies: {}"
        ),
        [](auto& v)
        {
          // If the field is not defined, the dependency is also not necessary!
          if(!v.getDocument().IsDefined())
            return;

          if(v.getSchema().IsMap())
          {
            for(auto dep: v.getSchema())
            {
              auto lookup = v.getDocumentPath(dep.first.template as<std::string>(), 1);
              if(!lookup.IsDefined())
                v.raiseError("dependencies-Rule violated: " + dep.first.template as<std::string>() + " required!");
              if(!v.getType("string")->equality(lookup, dep.second))
                v.raiseError("dependencies-Rule violated: " + dep.first.template as<std::string>() + " requires value " + dep.second.template as<std::string>());
            }
            return;
          }
          
          YAML::Node deplist;
          if(v.getSchema().IsScalar())
            deplist[0] = v.getSchema();
          else if(v.getSchema().IsSequence())
            deplist = v.getSchema();
          else
          {
            v.raiseError("dependencies rule with unknown data");
            return;
          }

          for(auto dep: deplist)
            if(!v.getDocumentPath(dep.as<std::string>(), 1).IsDefined())
              v.raiseError("dependencies-Rule violated: " + dep.template as<std::string>() + " required!");
        }
      );
    }

    template<typename Validator>
    void empty_rule(Validator& validator)
    {
      validator.registerRule(
        YAML::Load(
          "empty:                 \n"
          "  type: boolean          "
        ),
        [](auto& v)
        {
          if(v.getDocument().IsSequence())
            if((!v.getSchema().template as<bool>()) && (v.getDocument().size() == 0))
              v.raiseError("Empty-Rule violated for sequence");
        }
      );
    }

    template<typename Validator>
    void excludes_rule(Validator& validator)
    {
      validator.registerRule(
        YAML::Load(
          "excludes: {}"
        ),
        [](auto& v)
        {
          // If the field is not defined, the exclusion is also not necessary!
          if(!v.getDocument().IsDefined())
            return;

          YAML::Node exclist;
          if(v.getSchema().IsScalar())
            exclist[0] = v.getSchema();
          else if(v.getSchema().IsSequence())
            exclist = v.getSchema();
          else
          {
            v.raiseError("excludes rule called with unknown data");
            return;
          }

          for(auto exc: exclist)
            if(v.getDocumentPath(exc.as<std::string>(), 1).IsDefined())
              v.raiseError("excludes-Rule violated: " + exc.template as<std::string>() + " is not allowed!");
        }
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
            if (v.getType(1)->equality(item, v.getDocument()))
              v.raiseError("Forbidden-Rule violated: " + item.template as<std::string>());
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
          auto datait = v.getDocument().begin();
          while (schemait != v.getSchema().end())
          {
            v.document_stack.push_back(*(datait++));
            v.schema_stack.push_back(*(schemait++));
            v.validateItem(v.getSchema(0, true));
            v.schema_stack.pop_back();
            v.document_stack.pop_back();
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
          for(auto item: v.getDocument())
          {
            v.document_stack.push_back(item.first);
            v.validateItem(v.getSchema());
            v.document_stack.pop_back();
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
          if(!v.getDocument().IsDefined())
            return;

          // Extract type information from the larger schema
          auto type = v.getType(1);

          if((type->less(v.getSchema(), v.getDocument())) || (type->equality(v.getDocument(), v.getSchema())))
            v.raiseError("Max-Rrule violated!");
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
          if(!v.getDocument().IsDefined())
            return;

          // Extract type information from the larger schema
          auto type = v.getType(1);

          if(!(type->less(v.getSchema(), v.getDocument())))
            v.raiseError("Min-Rule violated!");
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
          if(!((v.getDocument().IsSequence()) || (v.getDocument().IsMap())))
            v.raiseError("Maxlength-Rule applied to non-iterable data container!");
          else
          {
            unsigned int count = 0;
            for(auto item: v.getDocument())
              ++count;
            if(count > v.getSchema().template as<int>())
              v.raiseError("Maxlength-Rule violated!");
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
          if(!((v.getDocument().IsSequence()) || (v.getDocument().IsMap())))
            v.raiseError("Minlength-Rule applied to non-iterable data container!");
          else
          {
            unsigned int count = 0;
            for(auto item: v.getDocument())
              ++count;
            if(count < v.getSchema().template as<int>())
              v.raiseError("Minlength-Rule violated!");
          }
        }
      );
    }

    template<typename Validator>
    void nullable_rule(Validator& validator)
    {
      validator.registerRule(
        YAML::Load(
          "nullable:       \n"
          "  type: boolean \n"
          "  default: false\n"
        ),
        [](auto& v)
        {
          if ((!v.getSchema().template as<bool>()) && (v.getDocument().IsNull()))
            v.raiseError("Nullable-Rule violated!");
        }
      );
    }

    template<typename Validator>
    void purge_unknown_rule(Validator& validator)
    {
      auto state = std::make_shared<std::vector<bool>>();

      validator.registerRule(
        YAML::Load(
          "purge_unknown:\n"
          " type: boolean"
        ),
        [state](auto& v)
        {
          state->push_back(v.getPurgeUnknown());
          v.setPurgeUnknown(v.getSchema().template as<bool>());
        },
        RulePriority::FIRST
      );

      validator.registerRule(
        YAML::Load(
          "purge_unknown:\n"
          " type: boolean"
        ),
        [state](auto& v)
        {
          v.setPurgeUnknown(state->back());
          state->pop_back();
        },
        RulePriority::LAST
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
          if(!std::regex_match(v.getDocument().template as<std::string>(), std::regex(v.getSchema().template as<std::string>())))
            v.raiseError("Regex-Rule violated!");
        }
      );
    }

    template<typename Validator>
    void rename_rule(Validator& validator)
    {
      validator.registerRule(
        YAML::Load(
          "rename:       \n"
          "  type: string  "
        ),
        [](auto& v)
        {
          auto dict = v.getDocument(1);
          auto doc = v.getDocument(0);

          YAML::Node newdict;
          for(auto item : dict)
            if(item.first.template as<std::string>() != v.getCurrentField())
              newdict[item.first] = item.second;
          v.getDocument(1).reset(newdict);

          v.popCurrentField();
          v.pushCurrentField(v.getSchema().template as<std::string>());
        },
        RulePriority::POST_NORMALIZATION
      );
    }

    template<typename Validator>
    void require_all_rule(Validator& validator)
    {
      auto state = std::make_shared<std::vector<bool>>();

      validator.registerRule(
        YAML::Load(
          "require_all:\n"
          " type: boolean"
        ),
        [state](auto& v)
        {
          state->push_back(v.getRequireAll());
          v.setRequireAll(v.getSchema().template as<bool>());
        },
        RulePriority::FIRST
      );

      validator.registerRule(
        YAML::Load(
          "require_all:\n"
          " type: boolean"
        ),
        [state](auto& v)
        {
          v.setRequireAll(state->back());
          state->pop_back();
        },
        RulePriority::LAST
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
          if((v.getSchema().template as<bool>()) && (!v.getDocument().IsDefined()))
            v.raiseError("Required-Rule violated!");
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
          "schema: {}"
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
            if(v.getDocument().IsMap())
              subrule = SchemaRuleType::DICT;
            if(v.getDocument().IsSequence())
              subrule = SchemaRuleType::LIST;
          }

          if(subrule == SchemaRuleType::DICT)
          {
            v.validateDict(v.getSchema(0, true));
          }
          if(subrule == SchemaRuleType::LIST)
          {
            for(std::size_t counter = 0; counter < v.getDocument().size(); ++counter)
            {
              v.document_stack.pushListItem(counter);
              v.validateItem(v.getSchema(0, true));
              v.document_stack.pop();
            }

          }
          if(subrule == SchemaRuleType::UNSUPPORTED)
            v.raiseError("Schema-Rule is only available for type=dict|list");
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
          if((v.getDocument().IsNull()) || (!v.getDocument().IsDefined()))
            return;

          auto type = v.getSchema().template as<std::string>();
          if (type == "list")
          {
            if(!v.getDocument().IsSequence())
              v.raiseError("Expecting a list");
            }
          else if(type == "dict")
          {
            if(!v.getDocument().IsMap())
              v.raiseError("Expecting a map");
          }
          else
            if (!v.getType(type)->is_convertible(v.getDocument()))
              v.raiseError("Type-Rule violated");
        },
        RulePriority::TYPECHECKING
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
          for(auto item: v.getDocument())
          {
            v.document_stack.push_back(item.second);;
            v.validateItem(v.getSchema(0, true));
            v.document_stack.pop_back();
          }
        }
      );
    }

  } // namespace impl

  /** @brief Register all the built-in rules from cerberus 
   * 
   * This is called from the constructor of the @c Validator class.
   */
  template<typename Validator>
  void registerBuiltinRules(Validator& v)
  {
    impl::allow_unknown_rule(v);
    impl::allowed_rule(v);
    impl::contains_rule(v);
    impl::default_rule(v);
    impl::dependencies_rule(v);
    impl::empty_rule(v);
    impl::excludes_rule(v);
    impl::forbidden_rule(v);
    impl::items_rule(v);
    impl::keysrules_rule(v);
    impl::max_rule(v);
    impl::min_rule(v);
    impl::maxlength_rule(v);
    impl::minlength_rule(v);
    impl::nullable_rule(v);
    impl::purge_unknown_rule(v);
    impl::regex_rule(v);
    impl::rename_rule(v);
    impl::require_all_rule(v);
    impl::required_rule(v);
    impl::schema_rule(v);
    impl::type_rule(v);
    impl::valuesrules_rule(v);
  }

} // namespace Cerberus

#endif