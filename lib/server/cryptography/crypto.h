/*
 * crypto.h
 * Author: Aven Bross
 * Date: 8/30/2015
 * 
 * Description:
 * Encryption classes wrapping openssl functionality.
*/

#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/ec.h>
#include <openssl/ecdh.h>
#include <iostream>
#include <string>
#include <cassert>
#include <limits>
#include <stdexcept>
#include <cctype>

#include <ios>

// Class that wraps openssl and performs ECDH exchange
class ECDH {
public:
    // Initialize context and generate key pair
    ECDH();
    
    // Returns the public key for sharing with peer
    std::string getPublicKey() const;
    
    // Recieve public key from peer
    void recieveKey(const std::string & otherKey);
    
    // Retrieve shared secret
    std::string getSecret() const;
    
    // Handle errors reported by openssl API
    void handleErrors();
    
    // Free up memory
    ~ECDH();
    
protected:
    BN_CTX *bn_ctx = NULL;
    EC_GROUP * group = NULL;
    EC_KEY *key = NULL;
    int field_size;
	unsigned char *secret = NULL;
	size_t secret_len;
	
	bool _done;
	bool _dead;
};


// Encode data in base 64
std::string base64Encode(const std::string & str);

// Decode base64 string to binary data
std::string base64Decode(const std::string & str);

// Compute sha1 hash of string
std::string sha1(const std::string & str);