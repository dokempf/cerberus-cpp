#define CATCH_CONFIG_MAIN
#include"catch2/catch.hpp"

#include<cerberus-cpp/validator.hh>
#include<yaml-cpp/yaml.h>

static const YAML::Node testdata = YAML::LoadFile("testdata.yml");
static const YAML::Node illschemas = YAML::LoadFile("illformedschemas.yml");

// For now this is only a different class - nothing fancy
class CustomValidator
  : public cerberus::Validator
{};

TEMPLATE_TEST_CASE("Performing standard validation", "[validate]", cerberus::Validator, CustomValidator) {
  TestType validator;
  for(auto testcase : testdata)
  {
    auto name = testcase.first;
    SECTION(name.as<std::string>()) {
      auto spec = testcase.second;

      validator.setAllowUnknown(spec["allow_unknown"].as<bool>(false));
      validator.setPurgeUnknown(spec["purge_unknown"].as<bool>(false));
      validator.setRequireAll(spec["require_all"].as<bool>(false));
      for (auto schema : spec["registry"])
        validator.registerSchema(schema.first.as<std::string>(), schema.second);
      
      std::vector<std::pair<YAML::Node, bool>> cases;
      for (auto data : spec["success"])
        cases.push_back({data, true});
      for (auto data : spec["failure"])
        cases.push_back({data, false});

      for (auto dataset : cases)
      {
        bool result = validator.validate(dataset.first, spec["schema"]);
        INFO("Validating the following document\n" << dataset.first << "\n\nThe validator says:\n");
        INFO(validator);
        REQUIRE(result == dataset.second);
      }
    }
  }
}

TEST_CASE("Faulty schema throws error", "[error]") {
  cerberus::Validator validator;
  for(auto testcase : illschemas)
  {
    auto name = testcase.first;
    SECTION(name.as<std::string>()) {
      REQUIRE_THROWS_AS(validator.validate(YAML::Node(), testcase.second), cerberus::SchemaError);
    }
  }
}