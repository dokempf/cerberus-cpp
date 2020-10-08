#ifndef CERBERUS_CPP_STACK_HH
#define CERBERUS_CPP_STACK_HH

#include<yaml-cpp/yaml.h>

#include<memory>
#include<vector>

namespace Cerberus {

  class DocumentPathItem
  {};

  class DictLookupItem
    : public DocumentPathItem
  {};

  class ListEntryItem
    : public DocumentPathItem
  {};

  class DocumentPath
    : std::vector<std::shared_ptr<DocumentPathItem>>
  {};

  class DocumentStack
    : public std::vector<YAML::Node>
  {
    public:

    private:
    DocumentPath path;
  };

} // namespace Cerberus

#endif