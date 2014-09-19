/***************************************************************************** 
 * encrypt.h/.cc
 *
 * SSLWrapper for the Arclink server
 *
 * (c) 2011 Marcelo Bianchi, GFZ Potsdam
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any later
 * version. For more information, see http://www.gnu.org/
 *****************************************************************************/
#ifndef ENCRYPT_H
#define ENCRYPT_H

#include <iostream>
#include <exception>
#include <fstream>
#include <vector>

#include <openssl/rand.h>
#include <openssl/evp.h>

#include "encrypterror.h"
#include "encryptpasswordhandle.h"

namespace SSLWrapper {

/** Encrypt Class usage help
  
  This class wraps the OpenSSL library to be used with the Arclink. 
  
  The normal usage consist in (a,b,c) are optionals.
  
  1) Create the class
  2) Initialize the class with initContext()
  2a) Find out the total output size givin a certain number of blocks using expectedSize()
  2b) Find out if the class is ready for encrypt with method isReady()
  3) Encrypt every block of information using update()
  3a) Find out if the class has encrypted anything using hasStarted()
  4) Finish the encryption with the finish()
  5) Reset the class that will be ready for a new initContext() with method reset()
  
  Something like:
  
  const unsigned char *oBuffer;
  int outputSize;
  SSLWrapper::Encrypt encryptor;
  
  try {
   encryptor.initContext("User name -- Normally an email address");
   
   foreach BLOCK OF DATA = D
   do
    oBuffer = encryptor.update(&outputSize, D, sizeof(D));
    writeout CONTENTS of "oBuffer" up to "outputSize"
   done
   
   oBuffer = encryptor.finish(&outputSize);
   writeout CONTENTS of "oBuffer" up to "outputSize"
  } catch (exception &e) {
    HANDLE EXCEPTION
  }
  
  The oBuffer, is a class internal buffer that should not be overwritten and 
  also it is not necessary to free its memory after usage. Just keep in mind 
  that the content of the pointer oBuffer will change after every call to methods
  update() and finish().
  
**/
class Encrypt
{
private:
	// Magic value written to openssl files, folowed by the salt.
	static const char magic[8];

	// Encryption specific Context & Salt
	EVP_CIPHER_CTX *ctx;
	unsigned char salt[PKCS5_SALT_LEN];

	// how many blocks has been received for encryption
	bool started;

	// Output buffer used for returning the encrypted value.
	unsigned char* inBuffer;
	int inBufferSize;

	// Resize the internal Buffer to fit the output
	void resizeOutput(int size);
	
	// This is the maximum block size expected to come out from the cipher.
	int maxBlockSize(int block);

public:
	Encrypt();
	~Encrypt();

public:
	// Prepare the context (the encrypt) to work
	void initContext (std::string filename, std::string masterpassword, std::string User, std::string dcid, std::string organization, std::string contactemail);

	// Check if the start has already been sent
	bool hasStarted();

	// Check if encrypt is ready to work
	bool isReady();

	// Those two are the encryption methods. They returns a pointer to a buffer.
	// This buffer should not be freed but it is overwritted at every call to the update
	// or finish methods. I.e. if you need to keep the results you should copy it to 
	// another buffer before calling those methods again.
	
	// Encode the supplied data considering the block size of the cipher.
	const unsigned char* update(int *outputSize, unsigned char* inbuf, int inbufl);

	// Flush the rest of the buffer on the cipher..
	const unsigned char* finish(int *outputSize);

	// Returns how many bytes will be the encrypted file size given by insize
	static int64_t expectedSize(int64_t insize);

	// Reset the encrypt class to start again
	void reset();
};

}
#endif // ENCRYPT_H
