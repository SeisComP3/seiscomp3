/***************************************************************************
* Copyright (C) 2009 BY GFZ Potsdam,                                       *
* EMail: seiscomp-devel@gfz-potsdam.de                                     * 
* The term "Non-Commercial Entity" is limited to the following:            *
* - Research institutes                                                    *
* - Public institutes dealing with natural hazards warnings                * 
*                                                                          *
* Provided that you qualify for a Non-Commercial Version License as        *
* specified above, and subject to the terms and conditions contained       *
* herein,GFZ Potsdam hereby grants the licensee, acting as an end user,    *
* an institute wide, non-transferable, non-sublicensable license, valid    *
* for an unlimited period of time, to install and use the Software, free   *
* of charge, for non-commercial purposes only. The licensee is allowed     *
* to develop own software modules based on the SeisComP3 libraries         *
* licensed under GPL and connect them with software modules under this     *
* license. The number of copies is for the licensee not limited within     *
* the user community of the licensee.                                      *
*                                                                          *
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS  *
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF               *
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.   *
* IN NO EVENT SHALL THE CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR    * 
* ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, * 
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE        *
* SOFTWARE OR THE USE OR OTHER DEALINGS WITH THE SOFTWARE.                 *
*                                                                          * 
* GFZ Potsdam reserve all rights not expressly granted to the licensee     *
* herein. Find more details of the license at geofon.gfz-potsdam.de        *
***************************************************************************/

#define SEISCOMP_COMPONENT License

#include "license.h"

#include <seiscomp3/logging/log.h>
#include <seiscomp3/core/strings.h>
#include <seiscomp3/system/environment.h>

#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/md5.h>

#include <string.h>

#include <iostream>
#include <fstream>



/* Type of key */
#define PUBLIC   1
#define PRIVATE  2

using namespace std;

namespace Seiscomp{
namespace License {


namespace {


/*
unsigned char gfz_der[] = {
  0x30, 0x82, 0x02, 0x22, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86,
  0xf7, 0x0d, 0x01, 0x01, 0x01, 0x05, 0x00, 0x03, 0x82, 0x02, 0x0f, 0x00,
  0x30, 0x82, 0x02, 0x0a, 0x02, 0x82, 0x02, 0x01, 0x00, 0xbd, 0x08, 0x99,
  0x17, 0x76, 0xfd, 0xa6, 0x9e, 0x94, 0x4f, 0x34, 0xad, 0x6b, 0xb6, 0x08,
  0x5a, 0xd3, 0x71, 0x58, 0xf0, 0x9d, 0x19, 0xfa, 0xb6, 0xc4, 0x0f, 0x5d,
  0x53, 0x27, 0x74, 0x8e, 0xa4, 0x8e, 0x6f, 0x9b, 0xc2, 0x00, 0x9c, 0x91,
  0x73, 0x11, 0xd2, 0xbc, 0x56, 0x41, 0xb3, 0x41, 0xc6, 0x3a, 0x33, 0x2f,
  0xb4, 0x3d, 0x6f, 0x3c, 0xec, 0x77, 0x4d, 0x13, 0x87, 0xff, 0x20, 0x09,
  0x4b, 0x43, 0x14, 0x13, 0xb7, 0xcc, 0x57, 0xff, 0x8c, 0x82, 0x9a, 0x55,
  0xd1, 0xc8, 0x77, 0x14, 0x40, 0xb3, 0x8c, 0x5c, 0xf1, 0x86, 0xe1, 0x7d,
  0xa0, 0xe0, 0x02, 0xe1, 0xd2, 0x70, 0xef, 0x95, 0x9f, 0xe6, 0xa8, 0xee,
  0xa9, 0x98, 0xca, 0x6e, 0xe3, 0x42, 0xf7, 0xa0, 0x46, 0x8a, 0x83, 0xca,
  0xff, 0x78, 0x61, 0x5d, 0xc5, 0x46, 0x3b, 0x1a, 0x2a, 0xd7, 0x70, 0x2c,
  0x4a, 0x72, 0x1f, 0x21, 0xd2, 0x50, 0x1b, 0x63, 0x7f, 0x8b, 0x53, 0xf3,
  0x2e, 0x31, 0xdc, 0x0f, 0x27, 0xec, 0x67, 0xcf, 0xdc, 0xa7, 0x8b, 0xae,
  0xb9, 0xdc, 0x26, 0x30, 0x69, 0x9e, 0x43, 0x46, 0xf6, 0xfb, 0xbe, 0x6c,
  0xf5, 0x92, 0xbf, 0x15, 0x52, 0xbe, 0x77, 0x46, 0x07, 0x12, 0x94, 0xe4,
  0x3c, 0x4a, 0x0e, 0xa0, 0x23, 0x16, 0xc2, 0x34, 0x0f, 0xbb, 0xac, 0x7e,
  0x99, 0x99, 0xfc, 0x6d, 0x51, 0x31, 0x17, 0xb0, 0xac, 0xd0, 0xb3, 0xf1,
  0x22, 0xa8, 0x3d, 0x4a, 0xaa, 0x4b, 0xc8, 0x55, 0x06, 0x5a, 0xd3, 0xea,
  0x09, 0xd8, 0x7e, 0x95, 0xcf, 0xed, 0xe5, 0x84, 0xe0, 0xc4, 0x8a, 0x74,
  0x20, 0x1b, 0xfb, 0xbc, 0x33, 0xcf, 0x78, 0x4a, 0x18, 0xfd, 0x33, 0xee,
  0xe1, 0x37, 0xd7, 0x51, 0x97, 0x32, 0x0d, 0x1b, 0x35, 0x97, 0xcb, 0x9b,
  0xec, 0xfa, 0x6f, 0x56, 0xd6, 0xe9, 0x75, 0x90, 0x53, 0x95, 0xe5, 0x7a,
  0xb3, 0x49, 0x06, 0xc3, 0x55, 0x10, 0x83, 0xfb, 0xcd, 0x36, 0x65, 0x80,
  0x19, 0xa9, 0x1e, 0x7b, 0x56, 0x23, 0x5f, 0xc7, 0xc2, 0x8b, 0xc8, 0x16,
  0x01, 0xf6, 0x24, 0xbf, 0xb5, 0x3c, 0x51, 0x61, 0xcb, 0x1c, 0x43, 0x1c,
  0x4f, 0xca, 0x4c, 0x50, 0x9c, 0x51, 0x1b, 0x5f, 0x21, 0x93, 0x6a, 0x93,
  0x27, 0x29, 0x81, 0x1a, 0x36, 0x5f, 0x18, 0x73, 0xae, 0xc8, 0x9c, 0xa0,
  0xba, 0xea, 0x1b, 0xa9, 0x8a, 0xe0, 0xbc, 0x89, 0x0f, 0x4a, 0xa1, 0x2b,
  0x86, 0x17, 0x04, 0x90, 0x48, 0x2e, 0xe3, 0xde, 0x22, 0xc9, 0xd3, 0x24,
  0xd4, 0x5f, 0x61, 0xe6, 0xf0, 0xa3, 0x98, 0xc5, 0x3c, 0x40, 0x9a, 0xfc,
  0xbe, 0x03, 0x9c, 0x77, 0x86, 0x0f, 0x7f, 0x94, 0x13, 0xe4, 0xea, 0xb5,
  0x92, 0xdb, 0xc9, 0x88, 0xf5, 0x7a, 0x89, 0xe2, 0xaa, 0x72, 0x90, 0xcd,
  0xed, 0x20, 0x29, 0xd1, 0xc5, 0x16, 0x08, 0x45, 0xb3, 0x65, 0x51, 0x2b,
  0x2d, 0x6d, 0x79, 0x57, 0x8e, 0x2c, 0x62, 0x36, 0xbc, 0x33, 0x5e, 0x9a,
  0x1e, 0x2a, 0x66, 0x72, 0x93, 0x2b, 0xc0, 0xd1, 0x69, 0x3e, 0x1a, 0x91,
  0x5c, 0x4f, 0x6e, 0x9c, 0xec, 0x0e, 0x67, 0x78, 0x96, 0x82, 0xe9, 0x66,
  0xcd, 0x40, 0xc9, 0xd1, 0x80, 0xae, 0x37, 0x78, 0xe2, 0x98, 0x80, 0x0e,
  0x4e, 0xed, 0x37, 0x33, 0x6b, 0xab, 0x15, 0x7f, 0x49, 0x70, 0xa4, 0x1b,
  0x99, 0x51, 0x1f, 0x14, 0x5f, 0x11, 0x1f, 0x40, 0x48, 0x6c, 0x66, 0xe4,
  0xbd, 0xc5, 0x85, 0x58, 0x99, 0x0a, 0x11, 0xb8, 0x83, 0xd3, 0x95, 0xd3,
  0x57, 0xa6, 0xad, 0x1e, 0xc5, 0x8f, 0xb7, 0x38, 0x94, 0x64, 0xe7, 0xbd,
  0xc2, 0x73, 0xb8, 0xc9, 0x6e, 0xf9, 0x24, 0x63, 0x7e, 0x01, 0x5b, 0xfe,
  0x60, 0x32, 0xf5, 0x25, 0xf9, 0x0e, 0x16, 0x94, 0x05, 0x78, 0x86, 0x35,
  0x26, 0xd8, 0x70, 0x03, 0x31, 0x02, 0x03, 0x01, 0x00, 0x01
};

unsigned int gfz_der_len = 550;
*/

namespace {

string errorStr;
string licenseText;

}

RSA *readKeyFromBio(BIO *in, int which) {
	RSA *rsa = NULL;

	if ( which == PUBLIC )
		rsa = PEM_read_bio_RSA_PUBKEY(in, NULL, NULL, NULL);
		//rsa = d2i_RSA_PUBKEY_bio(in, NULL);
	else
		rsa = PEM_read_bio_RSAPrivateKey(in, NULL, NULL, NULL);
	
	if ( rsa ) RSA_up_ref(rsa);

	return rsa;
}


RSA *readKey(const char *fn, int which, int minbits, int maxbits, int &strength) {
	RSA *rsa = NULL;
	BIO *in = BIO_new(BIO_s_file());
	//BIO *in = BIO_new_mem_buf(gfz_der, gfz_der_len);
	
	if ( !in ) return NULL;
	
	(void)BIO_set_close(in, BIO_NOCLOSE);
	if ( BIO_read_filename(in, fn) == 1 ) {
		rsa = readKeyFromBio(in, which);
		if ( rsa == NULL )
			errorStr = string("Unable to get public RSA key from file: ") + fn;
	}
	else
		errorStr = string("Unable to read from file: ") + fn;
	
	(void)BIO_free_all(in);
	
	if ( rsa != NULL ) {
		strength = BN_num_bits(rsa->n);
		if ( strength < minbits || strength > maxbits ) {
			RSA_free(rsa);
			rsa = NULL;
			errorStr = string("Invalid key strength: ") + Core::toString(strength);
		}
	}
	
	return rsa;
}

X509* readCertificate(const string& filename) {
	BIO *in = BIO_new(BIO_s_file());

	if ( !in ) return NULL;

	if ( BIO_read_filename(in, filename.c_str()) != 1 ) {
			(void)BIO_free_all(in);
			return NULL;
	}

	X509* x509 = PEM_read_bio_X509(in, NULL, NULL, NULL);

	(void)BIO_free_all(in);

	return x509;
}

bool readNID(char** str, X509* x509, int nid) {
	ASN1_IA5STRING* asn1 = (ASN1_IA5STRING*)X509_get_ext_d2i(x509, nid, NULL, NULL);
	if ( asn1 == NULL ) return false;

	int length = asn1->length;
	char* data = new char[length + 1];
	strncpy(data, (char*)asn1->data, length);
	data[length] = '\0';

	ASN1_STRING_free(asn1);

	*str = data;
	return true;
}

}


bool isValid() {
	static bool hasValidated = false;
	static bool validates = false;

	if ( hasValidated ) return validates;

	Environment *env = Environment::Instance();
	if ( env == NULL ) {
		cerr << "FATAL ERROR: No environment available" << endl;
		return false;
	}

	hasValidated = true;

	string licenseDir = env->configDir() + "/key";
	string licenseFile = licenseDir + "/License";
	string licenseKeyfile = licenseDir + "/License.key";
	string licenseSignature = licenseDir + "/License.signed";
	string licenseCert = licenseDir + "/License.crt";

	X509 *x509 = readCertificate(licenseCert);
	if ( x509 ) {
		ASN1_TIME* notAfter = X509_get_notAfter(x509),
		         * notBefore = X509_get_notBefore(x509);
		time_t ptime = time(NULL);

		int res = X509_cmp_time(notBefore, &ptime);
		if ( res == 0 || res > 0 ) {
			X509_free(x509);
			cerr << "FATAL ERROR: License has expired: " << licenseCert << endl;
			return false;
		}

		res = X509_cmp_time(notAfter, &ptime);
		if ( res == 0 || res < 0 ) {
			X509_free(x509);
			cerr << "FATAL ERROR: License has expired: " << licenseCert << endl;
			return false;
		}

		OpenSSL_add_all_algorithms();
		OpenSSL_add_all_ciphers();
		OpenSSL_add_all_digests();

		EVP_PKEY* pkey=X509_get_pubkey(x509);
		if ( !pkey ) {
			X509_free(x509);
			EVP_cleanup();
			cerr << "FATAL ERROR: License verification has failed: " << licenseCert << endl;
			return false;
		}

		res = X509_verify(x509, pkey);
		if ( res != 1 ) {
			X509_free(x509);
			EVP_PKEY_free(pkey);
			EVP_cleanup();
			cerr << "FATAL ERROR: License verification has failed: " << licenseCert << endl;
			return false;
		}

		char *buf;
		if ( readNID(&buf, x509, NID_netscape_comment) ) {
			licenseText = buf;
			delete buf;
		}

		EVP_PKEY_free(pkey);
		X509_free(x509);

		EVP_cleanup();

		return true;
	}

	// Read license file
	MD5_CTX ctx;
	MD5_Init(&ctx);

	unsigned char digest[MD5_DIGEST_LENGTH];
	char data[64];
	size_t len;

	ifstream f;

	f.open(licenseFile.c_str(), ios_base::in);

	if ( !f.good() ) {
		cerr << "FATAL ERROR: Invalid license file: " << licenseFile << endl;
		validates = false;
		return false;
	}

	licenseText.clear();

	while ( (len = f.rdbuf()->sgetn(data, 64)) > 0 ) {
		licenseText.append(data, len);
		MD5_Update(&ctx, data, len);
	}

	f.close();

	MD5_Final(digest, &ctx);

	int strength = 0;
	RSA *publicKey = readKey(licenseKeyfile.c_str(), PUBLIC, 1024, 8192, strength);
	if ( publicKey == NULL ) {
		cerr << "FATAL ERROR: Invalid key file: " << licenseKeyfile << endl;
		validates = false;
		return false;
	}

	BIO *bio_file = NULL, *b64_file;
	b64_file = BIO_new(BIO_f_base64());
	bio_file = BIO_new_file(licenseSignature.c_str(), "r");
	bio_file = BIO_push(b64_file, bio_file);

	int sigLength = strength / 8;
	unsigned char *signature = new unsigned char[sigLength];

	sigLength = BIO_read(bio_file, signature, sigLength);

	BIO_free_all(bio_file);

	if ( sigLength <= 0 ) {
		delete [] signature;
		cerr << "FATAL ERROR: Empty signature" << endl;
		validates = false;
		return false;
	}

	validates = RSA_verify(NID_md5, digest, MD5_DIGEST_LENGTH, signature, sigLength, publicKey);

	delete [] signature;

	/*
	if ( validates ) {
		cerr << "-----BEGIN LICENSE-----" << endl;
		cerr << licenseText << endl;
		cerr << "-----END LICENSE-----" << endl << endl;
	}
	*/

	return validates;
}


const char *text() {
	return licenseText.c_str();
}


void printWarning(ostream &os) {
	os << "You have no valid license to run this software." << endl;
	os << "To obtain a license contact <geofon_dc@gfz-potsdam.de>." << endl;
}


}
}
