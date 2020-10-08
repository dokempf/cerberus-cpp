#ifndef CERBERUS_CPP_ERROR_HH
#define CERBERUS_CPP_ERROR_HH

#include<exception>
#include<sstream>
#include<string>

namespace Cerberus {

  class CerberusError
    : public std::exception
  {};

  class SchemaError
    : public CerberusError
  {
    public:
    template<typename V>
    SchemaError(const V& v)
    {
      std::stringstream sstream;
      v.printErrors(sstream);
      message = sstream.str().c_str();
    }

    virtual const char* what() const noexcept override
    {
      return message;
    }

    private:
    const char* message;
  };

  struct ValidationErrorItem
  {
    std::string message;
  };

} // namespace Cerberus

#endif