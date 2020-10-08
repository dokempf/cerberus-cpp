#ifndef CERBERUS_CPP_STACK_HH
#define CERBERUS_CPP_STACK_HH

#include<yaml-cpp/yaml.h>

#include<memory>
#include<string>
#include<vector>

#include<iostream>


namespace Cerberus {

  class DocumentPathItem
  {
    public:
    virtual std::string stringify(const std::string& prefix) const = 0;
  };

  class DictLookupItem
    : public DocumentPathItem
  {
    public:
    DictLookupItem(const std::string& key)
      : key(key)
    {}

    virtual std::string stringify(const std::string& prefix) const override
    {
      return prefix + ((prefix != "") ? "." : "") + key;
    }
     
    private:
    std::string key;
  };

  class ListEntryItem
    : public DocumentPathItem
  {
    public:
    ListEntryItem(std::size_t i)
      : i(i)
    {}

    virtual std::string stringify(const std::string& prefix) const override
    {
      return prefix + "[" + std::to_string(i) + "]";
    }

    private:
    std::size_t i;
  };

  class DocumentStack
    : public std::vector<YAML::Node>
  {
    public:
    void reset(const YAML::Node& node)
    {
      path.clear();
      this->clear();
      this->push_back(node);
    }

    void pushDictItem(const std::string& key)
    {
      path.push_back(std::make_shared<DictLookupItem>(key));
      this->push_back(this->back()[key]);
    }

    void pushListItem(std::size_t i)
    {
      path.push_back(std::make_shared<ListEntryItem>(i));
      this->push_back(this->back()[i]);
    }

    void pop()
    {
      path.pop_back();
      this->pop_back();
    }

    YAML::Node& get(std::size_t level = 0)
    {
      return *(this->rbegin() + level);
    }

    const YAML::Node& get(std::size_t level = 0) const
    {
      return *(this->rbegin() + level);
    }

    std::string stringPath() const
    {
      std::string result = "";
      for (auto element: path)
        result = element->stringify(result);
      return result;
    }

    private:
    std::vector<std::shared_ptr<DocumentPathItem>> path;
  };

} // namespace Cerberus

#endif