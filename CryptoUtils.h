#ifndef CRYPTO_UTILS_H
#define CRYPTO_UTILS_H

#include <algorithm>
#include <cstdio>
#include <iostream>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <ostream>
#include <sstream>

namespace Crypto {

inline std::string sha256(const std::string& str)
{
  unsigned char hash[SHA256_DIGEST_LENGTH];
  SHA256((unsigned char*) str.c_str(), str.length(), hash);
  std::stringstream ss;
  for(unsigned char i: hash) { ss << std::hex << (int) i; }
  return ss.str();
}

std::pair<std::string, std::string> genKeyPair();

std::string sign(const std::string& str, const std::string& privateKey);

bool verify(const std::string& str,
            const std::string& signature,
            const std::string& publicKey);

}// namespace Crypto

#endif// CRYPTO_UTILS_H
