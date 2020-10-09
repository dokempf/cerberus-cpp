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
      return prefix + ((prefix != "^") ? "." : "") + key;
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

  /** @brief An object that represents a stack of nested YAML documents */
  class DocumentStack
    : public std::vector<YAML::Node>
  {
    public:
    /** @brief reset the document stack to a new document
     * 
     * @param node The new YAML document. The stack will hold a deep copy
     *             of this document.
     */
    void reset(const YAML::Node& node)
    {
      path.clear();
      this->clear();
      this->push_back(node);
    }

    /** @brief Push a subdocument onto the stack according to a mapping key
     * 
     * This will add an item onto the stack by looking up a key in the current
     * top item dictionary.
     */
    void pushDictItem(const std::string& key)
    {
      path.push_back(std::make_shared<DictLookupItem>(key));
      this->push_back(this->back()[key]);
    }

    /** @brief Push a subdocument onto the stack according to a list index lookup
     * 
     * This will add an item onto the stack by looking up an item in the current
     * top item list.
     */
    void pushListItem(std::size_t i)
    {
      path.push_back(std::make_shared<ListEntryItem>(i));
      this->push_back(this->back()[i]);
    }

    //! Pops the top item of the stack
    void pop()
    {
      path.pop_back();
      this->pop_back();
    }

    /** @brief Accesses an item of the stack
     * 
     * The data structure isn't technically a stack because you can access
     * more item's than just the top one by adjusting the @c level parameter.
     * 
     * @param level The stack item index that we are interested in.
     */
    YAML::Node& get(std::size_t level = 0)
    {
      return *(this->rbegin() + level);
    }

    /** @brief Accesses an item of the stack
     * 
     * The data structure isn't technically a stack because you can access
     * more item's than just the top one by adjusting the @c level parameter.
     * 
     * @param level The stack item index that we are interested in.
     */
    const YAML::Node& get(std::size_t level = 0) const
    {
      return *(this->rbegin() + level);
    }

    /** @brief Extract a string describing the path from the root document through the stack
     * 
     * This can be e.g. used to print information about the subdocument we
     * are dealing with.
     */
    std::string stringPath() const
    {
      std::string result = "^";
      for (auto element: path)
        result = element->stringify(result);
      return result;
    }

    private:
    std::vector<std::shared_ptr<DocumentPathItem>> path;
  };

} // namespace Cerberus

#endif