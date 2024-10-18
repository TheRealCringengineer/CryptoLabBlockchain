#include "CryptoUtils.h"

namespace Crypto {

std::pair<std::string, std::string> genKeyPair()
{
  const char* curve = "secp256k1";
  const int PRBUFFSIZE = 255;
  const int PUBUFFSIZE = 174;
  int n;

  std::pair<std::string, std::string> keys;

  /* Generate the key pair */
  EVP_PKEY* pkey = EVP_EC_gen(curve);
  if(pkey == nullptr) {
    std::cerr << "Error generating the ECC key." << std::endl;
    return keys;
  }

  // PRIVATE KEY
  char privateKeyString[PRBUFFSIZE];
  BIO* bp = BIO_new(BIO_s_mem());
  const EVP_CIPHER* cipher = EVP_get_cipherbyname(curve);

  if(!PEM_write_bio_PrivateKey(bp, pkey, cipher, nullptr, 0, nullptr, nullptr))
    std::cerr << "Error generating keypair." << std::endl;

  n = BIO_read(bp, (void*) privateKeyString, (int) sizeof(privateKeyString));
  if(n < 0) std::cerr << "Error generating keypair." << std::endl;

  keys.first = (char*) privateKeyString;
  BIO_free(bp);

  // PUBLIC KEY
  BIO* bpu = BIO_new(BIO_s_mem());
  char publicKeyString[PUBUFFSIZE];

  if(!PEM_write_bio_PUBKEY(bpu, pkey))
    std::cerr << "error generating public key" << std::endl;

  n = BIO_read(bpu, (void*) publicKeyString, (int) sizeof(publicKeyString));
  if(n < 0) std::cerr << "Error generating keypair." << std::endl;

  keys.second = (char*) publicKeyString;

  BIO_free(bpu);
  EVP_PKEY_free(pkey);

  return keys;
}

std::string sign(const std::string& str, const std::string& privateKey)
{
  /* str should be a sha256 hash */
  auto* md = (unsigned char*) str.c_str();

  /* create private key from private key string */
  const char* mKey = privateKey.c_str();
  BIO* bo = BIO_new(BIO_s_mem());
  BIO_write(bo, mKey, (int) strlen(mKey));
  EVP_PKEY* pkey = nullptr;
  PEM_read_bio_PrivateKey(bo, &pkey, nullptr, nullptr);
  BIO_free(bo);

  /* initialize our signature context object */
  EVP_PKEY_CTX* ctx;
  ctx = EVP_PKEY_CTX_new(pkey, nullptr /* no engine */);
  if(!ctx) std::cerr << "error creating ctx" << std::endl;

  if(EVP_PKEY_sign_init(ctx) <= 0)
    std::cerr << "error initializing sig ctx" << std::endl;

  /* set the context digest type to sha256 */
  if(EVP_PKEY_CTX_set_signature_md(ctx, EVP_sha256()) <= 0)
    std::cerr << "error setting signature md" << std::endl;

  /* 
    mdlen is always 32, which is the SHA256_DIGEST_LENGTH, 
    see openssl/sha.h
    */
  size_t mdlen = 32;

  /* Determine buffer length */
  size_t siglen{0};
  if(EVP_PKEY_sign(ctx, nullptr, &siglen, md, mdlen) <= 0)
    std::cerr << "error determining buffer length" << std::endl;

  /* allocate memory for the signature */
  unsigned char* sig;
  sig = (unsigned char*) OPENSSL_malloc(siglen);

  if(!sig) std::cerr << "malloc failure" << std::endl;

  if(EVP_PKEY_sign(ctx, sig, &siglen, md, mdlen) <= 0) {
    std::cerr << "error creating signature" << std::endl;
  }

  std::string hex;
  /* the easy way to translate your signature to a hex string */
  // mysig.hexsig = OPENSSL_buf2hexstr(sig, siglen);

  /* the not so easy way */
  char st[256];
  size_t strlen;
  OPENSSL_buf2hexstr_ex(st, 256, &strlen, sig, siglen, '\0');

  /* assign the result to signature structure */
  hex = st;

  /* free memory */
  EVP_PKEY_free(pkey);
  EVP_PKEY_CTX_free(ctx);

  return hex;
};

bool verify(const std::string& str,
            const std::string& signature,
            const std::string& publicKey)
{

  /* the easy way to translate your hex string back into your buffer */
  // long len;
  // unsigned char *sig = OPENSSL_hexstr2buf(signature.hexsig, &len);

  /* the not so easy way */
  unsigned char buf[256];
  const char* st = signature.c_str();
  size_t buflen;
  OPENSSL_hexstr2buf_ex(buf, 256, &buflen, st, '\0');

  /* generate the public key using the public key string */
  const char* mKey = publicKey.c_str();
  BIO* bo = BIO_new(BIO_s_mem());
  BIO_write(bo, mKey, (int) strlen(mKey));
  EVP_PKEY* pkey = nullptr;
  PEM_read_bio_PUBKEY(bo, &pkey, nullptr, nullptr);
  /* free memory */
  BIO_free(bo);

  /* create the key context */
  EVP_PKEY_CTX* ctx;
  ctx = EVP_PKEY_CTX_new(pkey, nullptr /* no engine */);
  if(!ctx) std::cerr << "ctx error" << std::endl;

  /* initialize the context */
  if(EVP_PKEY_verify_init(ctx) <= 0)
    std::cerr << "ctx verify error" << std::endl;

  /* set the message digest type to sha256 */
  if(EVP_PKEY_CTX_set_signature_md(ctx, EVP_sha256()) <= 0)
    std::cerr << "verify signature error" << std::endl;

  /* Perform operation */
  auto* md = (unsigned char*) str.c_str();
  size_t mdlen = 32;
  size_t siglen = signature.size() / 2;

  int ret = EVP_PKEY_verify(ctx, buf, siglen, md, mdlen);

  /* free memory and return the result */
  EVP_PKEY_free(pkey);
  EVP_PKEY_CTX_free(ctx);
  return ret == 1;
}

}// namespace Crypto
