#ifndef ENCRYPTERROR_H
#define ENCRYPTERROR_H

#include <iostream>

#include "utils.h"

using namespace Utilities;

namespace SSLWrapper {

class EncryptError: public GenericException
  {
  public:
    EncryptError (const std::string &message):
      GenericException("SSLWrapper", message) {}
  };

}
#endif // ENCRYPTERROR_H
