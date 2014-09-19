#include "encrypt.h"


namespace SSLWrapper {

const char Encrypt::magic[] = {'S', 'a', 'l', 't', 'e', 'd', '_', '_'};

//Private
int Encrypt::maxBlockSize(int block) {
	if (!isReady()) throw EncryptError("encrypt is not ready");
	if (block < 0) throw EncryptError("number of input blocks should be larger than 0");

	// Minimun size, writen by the start method
	int minimun = (sizeof(this->magic) + sizeof(this->salt));

	// Value get from the EVP manual page
	int cypher_size = block + EVP_CIPHER_CTX_block_size (this->ctx) - 1;

	return minimun + cypher_size;
}

void Encrypt::resizeOutput(int size) {
	if (size > this->inBufferSize)
		this->inBuffer = (unsigned char*) realloc(this->inBuffer, sizeof(unsigned char) * size);
	
	if (this->inBuffer == NULL) 
		throw EncryptError("cannot allocate memory for output buffer");
	
	return;
}

// Constructor
Encrypt::~Encrypt(){
	this->reset();
	
	if (inBuffer != NULL) 
		free(inBuffer);
	inBuffer = NULL;
	inBufferSize = 0;
}

Encrypt::Encrypt(){
	// Initialization of the class, we must zero everyone
	memset(this->salt,sizeof(this->salt),0);
	ctx = NULL;

	inBuffer = NULL;
	inBufferSize = 0;

	this->started = false;
}

// Public
void Encrypt::reset (){
	memset(this->salt,sizeof(this->salt),0);
	
	if (ctx!= NULL) {
		EVP_CIPHER_CTX_cleanup(this->ctx);
		free(ctx);
	}	
	ctx = NULL;
	
	this->started = false;
}

int64_t Encrypt::expectedSize(int64_t insize){
	int64_t blocks = insize / 8;

	// For considering the padding ? Not so sure !
	// even when the blocks are multiple of 8
	blocks++;

	return blocks*8 + sizeof(magic) + PKCS5_SALT_LEN;
}

bool Encrypt::hasStarted() {
	return (this->started);
}

bool Encrypt::isReady() {
	return (this->ctx != NULL);
}

void Encrypt::initContext (std::string filename, std::string masterpassword, std::string User, std::string dcid, std::string organization, std::string contactemail)
{
	unsigned char *key = NULL;
	unsigned char *iv = NULL;
	int key_size, result;
	std::string password;
	EncryptPasswordHandle pHandle(filename, masterpassword, dcid, organization, contactemail);
	
	// Check if we have started encryption
	if (this->hasStarted()) throw EncryptError("encryption is beeing performed");

	// Find the Password for User
	password = pHandle.findPassword (User, true, true);
	
	// Key & Iv generation
	key = (unsigned char *) calloc(sizeof(unsigned char), EVP_MAX_KEY_LENGTH);
	iv = (unsigned char *)calloc(sizeof(unsigned char), EVP_MAX_IV_LENGTH);
	
	memset(this->salt, sizeof(this->salt), 0);

	// Get a new salt
	if ((result = RAND_pseudo_bytes(this->salt, sizeof(this->salt))) != 1)
		throw EncryptError("failed to generate a new SALT");

	// Get Key and Iv from Password & Salt
	key_size = EVP_BytesToKey(EVP_des_cbc(),
						  EVP_md5(),
						  this->salt,
						  (const unsigned char*)password.c_str (),
						  password.size (),
						  1,
						  key,
						  iv);


	// Context Initialization
	this->ctx = (EVP_CIPHER_CTX*)calloc(sizeof(EVP_CIPHER_CTX),1);

	if ( (result = EVP_EncryptInit (this->ctx, EVP_des_cbc (), key, iv)) !=  1)
		throw EncryptError("failed to initialize encryption context");

	// Clean
	password.clear ();

	if (key != NULL) free(key);
	if (iv != NULL) free(iv);

	key = NULL;
	iv = NULL;
	
	return;
}

const unsigned char * Encrypt::update(int *outputSize, unsigned char* inbuf, int inbufl){
	if (inbuf == NULL) throw EncryptError("cannot read from am empty buffer");
	if (this->ctx == NULL) throw EncryptError("context was not initialized");
	
	int outbufl = 0;
	int outbufl_enc = 0;

	// Make sure that the Output Buffer fits the data
	resizeOutput (maxBlockSize (inbufl));
	
	// Add Salt magic if this is the first encode
	if (! hasStarted ()){
		
		for(unsigned int i=0; i < sizeof(this->magic); i++)
			this->inBuffer[outbufl++] = this->magic[i];
		
		for(unsigned int i=0; i < sizeof(this->salt); i++)
			this->inBuffer[outbufl++] = this->salt[i];
		
		this->started = true;
	}
	
	// Encode ...
	if (EVP_EncryptUpdate (this->ctx, &this->inBuffer[outbufl], &outbufl_enc, inbuf, inbufl) != 1){
		this->reset ();
		throw EncryptError("error in encrypt update");
	}
	
	outbufl = outbufl + outbufl_enc;
	
	// Prepare output and return
	(*outputSize) = outbufl;
	
	return this->inBuffer;
}

const unsigned char * Encrypt::finish (int *outputSize){
	int outbufl = 0;

	if (! hasStarted ())
		throw EncryptError("encryption not started");

	// Make sure that the Output Buffer fits the data
	resizeOutput (maxBlockSize (0));

	// Encode the final block
	if (EVP_EncryptFinal (this->ctx, this->inBuffer, &outbufl) != 1){
		this->reset();
		throw EncryptError("error in final encryption");
	}

	// Prepare output and return
	(*outputSize) = outbufl;

	// Inutilize the context and make the class ready for next cycle
	this->reset ();

	return this->inBuffer;
}

}
