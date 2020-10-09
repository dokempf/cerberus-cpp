#ifndef CERBERUS_CPP_ERROR_HH
#define CERBERUS_CPP_ERROR_HH

#include<exception>
#include<sstream>
#include<string>

namespace Cerberus {

  //! A base class for exceptions thrown from Cerberus
  class CerberusError
    : public std::exception
  {};

  /** @brief An exception indicating a faulty schema input
   * 
   * This exception is thrown when the schema given to the @c Validator
   * class was not correct.
   */
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

  //! A struct representing an error during validation
  struct ValidationErrorItem
  {
    std::string path;
    std::string message;
  };

} // namespace Cerberus

#endif