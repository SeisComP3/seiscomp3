/***************************************************************************
 *   Copyright (C) by gempa GmbH                                           *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/


#ifndef SEISCOMP_UTILS_CERTIFICATESTORE_H
#define SEISCOMP_UTILS_CERTIFICATESTORE_H


#include <seiscomp3/core.h>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/datetime.h>
#include <seiscomp3/utils/mutex.h>

#include <openssl/x509.h>
#include <string>
#include <map>


namespace Seiscomp {
namespace Util {


DEFINE_SMARTPOINTER(CertificateContext);

/**
 * @brief An agency related certificate context.
 * This class manages information for a particular subject hash.
 * It saves the last accessed certificate for caching, the list of available
 * certificates and the certificate revocation list.
 *
 * It provides functions to lookup certificates based on a reference time
 * or a given digest and a reference signature.
 *
 * While looking up certificates the revocation list is considered as well.
 * Revoked certificates will be ignored.
 */
class SC_SYSTEM_CORE_API CertificateContext : public Core::BaseObject {
	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	protected:
		//! C'tor
		CertificateContext();

	public:
		//! D'tor
		~CertificateContext();


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:

		/**
		 * @brief Returns a certficate valid for a given reference time.
		 * @param referenceTime The reference time
		 * @return An X509 certificate or null
		 */
		const X509 *findCertificate(const Core::Time &referenceTime) const;

		/**
		 * @brief Returns a certificate by signing the digest and comparing it
		 *        against the reference signature.
		 * @param digest Address pointing to the digest
		 * @param nDigest Number of digest bytes
		 * @param signature OpenSSL ECDSA signature
		 * @return A certficate or null
		 */
		const X509 *findCertificate(const char *digest, size_t nDigest,
		                            const ECDSA_SIG *signature) const;

		// /**
		// * @brief Checks if an certificate has been revoked
		// *        Omit the reference time to disable the time check.
		// * @param cert The X509 certificate
		// * @param referenceTime The reference time
		// * @return
		// */
		// bool isRevoked(const X509 *cert,
		//                const Core::Time &referenceTime = Core::Time()) const;


	// ----------------------------------------------------------------------
	//  Members
	// ----------------------------------------------------------------------
	protected:
		typedef std::map<std::string, X509*> Certs;
		typedef std::map<std::string, X509_CRL*> CRLs;
		// Cache
		mutable X509            *_cert;
		mutable ASN1_TIME       *_begin;
		mutable ASN1_TIME       *_end;

		// Certificates and revocation list
		Certs                    _certs;
		CRLs                     _crls;

	friend class CertificateStore;
};


DEFINE_SMARTPOINTER(CertificateStore);

/**
 * @brief An OpenSSL certificate store.
 *
 * @code
 * CertificateStore store;
 * if ( !store.init("/path/to/store") ) {
 *   exit(1);
 * }
 *
 * // Get context for authority "gempa"
 * const CertificateContext *ctx = store.getContext("792241d4");
 * if ( !ctx ) {
 *   exit(1);
 * }
 *
 * const X509 *cert = ctx.findCerticiate(Core::Time::GMT());
 * if ( !cert ) {
 *   exit(1);
 * }
 *
 * // Do something with the certificate
 * @endcode
 */
class SC_SYSTEM_CORE_API CertificateStore : public Core::BaseObject {
	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		CertificateStore();
		//! D'tor
		~CertificateStore();


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		/**
		 * @brief Returns the global certificate store.
		 * The global store is going to be used by components that do not
		 * accept a certificate store as part of their interface.
		 * @return The certificate store reference.
		 */
		static CertificateStore &global();

		//! Initializes the store and opens the passed directory.
		bool init(const std::string &baseDirectory);

		/**
		 * @brief Checks if the certificate store is valid
		 * @return True, if the store is valid
		 */
		bool isValid() const;

		/**
		 * @brief Returns a certificate context for a given authority.
		 * @param hash Pointer to OpenSSL X509 name hash
		 * @param len Number of bytes of authority string
		 * @return The context or null.
		 */
		const CertificateContext *getContext(const char *hash, size_t len);

		/**
		 * @brief Returns a certificate context for a given authority.
		 * @param hash The given OpenSSL X509 name hash
		 * @return The context or null.
		 */
		const CertificateContext *getContext(const std::string &hash);

		/**
		 * @brief Validates an ECDSA signature against the certificates in
		 *        the store.
		 * @param authority The authority
		 * @param len Then length of the authority string in bytes
		 * @param digest The digest block
		 * @param nDigest The length of the digest block in bytes
		 * @param signature The ECDSA signature to check against
		 * @param matchedCertificate The matched certificate if requested
		 * @return Success flag
		 */
		bool validate(const char *authority, size_t len,
		              const char *digest, size_t nDigest,
		              const ECDSA_SIG *signature,
		              const X509 **matchedCertificate = 0);

		bool validate(const std::string &authority,
		              const char *digest, size_t nDigest,
		              const ECDSA_SIG *signature,
		              const X509 **matchedCertificate = 0);

		/**
		 * @brief Loads certificates for a specific hash from directory
		 * @param List in which matching certs should be added to
		 * @param hash The given OpenSSL X509 name hash
		 * @param baseDirectory The base directory used for the
		 *        recursive search
		 * @return True on success.
		 */
		bool loadCerts(CertificateContext::Certs &certs, const std::string &hash,
		               const std::string &baseDirectory);

		/**
		 * @brief Loads crls for a specific hash from directory
		 * @param crls List in which matching crls should be added to
		 * @param hash The given OpenSSL X509 name hash
		 * @param baseDirectory The base directory used for the
		 *        recursive search
		 * @return True on success
		 */
		bool loadCRLs(CertificateContext::CRLs &crls, const std::string &hash,
		              const std::string &baseDirectory);

	// ----------------------------------------------------------------------
	//  Members
	// ----------------------------------------------------------------------
	protected:
		typedef std::map<std::string, CertificateContextPtr> Lookup;
		static CertificateStore  _global;
		Lookup                   _lookup;
		mutex                    _storeMutex;
		std::string              _baseDirectory;
};


inline CertificateStore &CertificateStore:: global() {
	return _global;
}

inline bool CertificateStore::isValid() const {
	return !_baseDirectory.empty();
}


}
}


#endif
