#include<iostream>


template<typename Validator>
int check(Validator& validator, const YAML::Node& input)
{
  // Configure the validator with all the given data
  validator.setAllowUnknown(input["allow_unknown"].as<bool>(false));
  validator.setPurgeUnknown(input["purge_unknown"].as<bool>(false));
  validator.setRequireAll(input["require_all"].as<bool>(false));
  for (auto schema : input["registry"])
    validator.registerSchema(schema.first.as<std::string>(), schema.second);

  // Collect the test cases and their expected result
  std::vector<std::pair<YAML::Node, bool>> cases;
  for (auto testcase : input["success"])
    cases.push_back({testcase, true});
  for (auto testcase : input["failure"])
    cases.push_back({testcase, false});

  int failure = 0;
  for (auto testcase : cases)
  {
    auto result = validator.validate(testcase.first, input["schema"]);
    if (result != testcase.second)
    {
      std::cerr << "Failure trying to validate this data:" << std::endl;
      std::cerr << input["data"] << std::endl;
      std::cerr << "against this schema:" << std::endl;
      std::cerr << input["schema"] << std::endl;
      std::cerr << "Expected result: " << (testcase.second ? "true" : "false") << std::endl;
      std::cerr << "The following reasons were given:" << std::endl;
      std::cerr << validator << std::endl;
      ++failure;
    }
    else
    {
      if(testcase.second)
        std::cout << "The correctly validated and normalized data:" << std::endl << validator.getDocument() << std::endl << std::endl;
      else
      {
        std::cout << "Validation rightfully failed with the following reasons:" << std::endl;
        std::cout << validator << std::endl;
      }
    }
  }

  return failure;
}
