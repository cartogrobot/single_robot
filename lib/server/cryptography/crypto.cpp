/*
 * crypto.cpp
 * Author: Aven Bross
 * Date: 8/30/2015
 * 
 * Description:
 * Encryption classes wrapping openssl functionality.s
*/

#include "crypto.h"

/*
 * class ECDH
 * Wraps openssl and performs ECDH exchange
 */

// Initialize context and generate key pair
ECDH::ECDH(): _done(false), _dead(false){
    // Create an Elliptic Curve Group object and set it up to use the secp256k1 curve
	if(NULL == (group = EC_GROUP_new_by_curve_name(NID_secp256k1))) handleErrors();
	std::cout << "created ec group\n";
	
	// Create an Elliptic Curve Key object
	if(NULL == (key = EC_KEY_new())) handleErrors();
	std::cout << "created ec\n";
	
	// Set the key to use the group we created
	if(1 != EC_KEY_set_group(key, group)) handleErrors();
	std::cout << "set ec group\n";
	
	if(NULL == (bn_ctx = BN_CTX_new())) handleErrors();
	std::cout << "created bignum context\n";
	
	// Generate the private and public key
	if(1 != EC_KEY_generate_key(key)) handleErrors();
	
	std::cout << "generated keys\n";
}

// Returns the public key for sharing with peer
std::string ECDH::getPublicKey() const{
    if(_dead){
        return std::string("");
    }
    
    unsigned char buffer[65];
    
    const EC_POINT * point = EC_KEY_get0_public_key(key);
    std::cout << "grabbed public key\n";
    
    std::cout << EC_POINT_point2oct(group, point, POINT_CONVERSION_UNCOMPRESSED, buffer, 65, bn_ctx) << "\n";
    
    std::cout << "public key converted\n";
    
    return std::string((char*)(buffer+1), 64);
}

// Recieve public key from peer
void ECDH::recieveKey(const std::string & otherKey){
    if(_done || _dead) return;
    
    // Grab the keys
    EC_POINT * peerkey = EC_POINT_new(group);
    
    // 
    std::string octetString;
    octetString.push_back((char)0x4);
    octetString += otherKey;
    
    if(1 != EC_POINT_oct2point(group, peerkey, (unsigned char*)octetString.c_str(), octetString.size(), bn_ctx)) handleErrors();
    std::cout << "converted string to ec point\n";

    /* Calculate the size of the buffer for the shared secret */
	field_size = EC_GROUP_get_degree(group);
	secret_len = (field_size+7)/8;
	std::cout << "calculated field size\n";

	/* Allocate the memory for the shared secret */
	if(NULL == (secret = (unsigned char*)OPENSSL_malloc(secret_len))) handleErrors();
	std::cout << "allocated space for shared secret\n";

	/* Derive the shared secret */
	secret_len = ECDH_compute_key(secret, secret_len, peerkey, key, NULL);
	std::cout << "calculated shared secret\n";
	
	EC_POINT_free(peerkey);
	std::cout << "free peerkey\n";
	
	_done = true;
}

// Retrieve shared secret
std::string ECDH::getSecret() const{
    if(!_done || _dead){
        return std::string("");
    }
    else{
        return std::string((char*)secret, secret_len);
    }
}

// Handle errors reported by openssl API
void ECDH::handleErrors(){
    _dead = true;
}

// Free up memory
ECDH::~ECDH(){
    if(key != NULL) EC_KEY_free(key);
    if(group != NULL) EC_GROUP_free(group);
	if(bn_ctx != NULL) BN_CTX_free(bn_ctx);
	if(secret != NULL) delete secret;
}



/*
 * Crypto Functions
 */

static const char b64_table[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static const char reverse_table[128] = {
   64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
   64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
   64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
   52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
   64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
   15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
   64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
   41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64
};

// Encode the given binary data in base64
std::string base64Encode(const std::string &bindata)
{
   using std::string;
   using std::numeric_limits;

   if (bindata.size() > (numeric_limits<string::size_type>::max() / 4u) * 3u) {
      throw std::length_error("Converting too large a string to base64.");
   }

   const std::size_t binlen = bindata.size();
   // Use = signs so the end is properly padded.
   string retval((((binlen + 2) / 3) * 4), '=');
   std::size_t outpos = 0;
   int bits_collected = 0;
   unsigned int accumulator = 0;
   const string::const_iterator binend = bindata.end();

   for (string::const_iterator i = bindata.begin(); i != binend; ++i) {
      accumulator = (accumulator << 8) | (*i & 0xffu);
      bits_collected += 8;
      while (bits_collected >= 6) {
         bits_collected -= 6;
         retval[outpos++] = b64_table[(accumulator >> bits_collected) & 0x3fu];
      }
   }
   if (bits_collected > 0) { // Any trailing bits that are missing.
      assert(bits_collected < 6);
      accumulator <<= 6 - bits_collected;
      retval[outpos++] = b64_table[accumulator & 0x3fu];
   }
   assert(outpos >= (retval.size() - 2));
   assert(outpos <= retval.size());
   return retval;
}

// Decode the given base64 string into binary data
std::string base64Decode(const std::string &ascdata)
{
   using std::string;
   string retval;
   const string::const_iterator last = ascdata.end();
   int bits_collected = 0;
   unsigned int accumulator = 0;

   for (string::const_iterator i = ascdata.begin(); i != last; ++i) {
      const int c = *i;
      if (std::isspace(c) || c == '=') {
         // Skip whitespace and padding. Be liberal in what you accept.
         continue;
      }
      if ((c > 127) || (c < 0) || (reverse_table[c] > 63)) {
         throw std::invalid_argument("This contains characters not legal in a base64 encoded string.");
      }
      accumulator = (accumulator << 6) | reverse_table[c];
      bits_collected += 6;
      if (bits_collected >= 8) {
         bits_collected -= 8;
         retval += (char)((accumulator >> bits_collected) & 0xffu);
      }
   }
   return retval;
}

// Compute sha1 hash of string
std::string sha1(const std::string & str){
    // Buffer to store hash result
    unsigned char obuf[20];

    // Compute sha1 hash of input string
    SHA1((unsigned char *)str.c_str(), str.size(), obuf);
    
    // Convert results to string
    return std::string((char *)obuf, 20);
}
