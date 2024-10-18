#ifndef CRYPTO_BLOCK_H
#define CRYPTO_BLOCK_H

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

struct UXTO {
  std::string Addr;
  uint32_t Amount;

  UXTO(const std::string& add, uint32_t amount)
      : Addr(add)
      , Amount(amount)
  {}
};

inline bool operator==(const UXTO& lhs, const UXTO& rhs)
{
  return lhs.Addr == rhs.Addr && lhs.Amount == rhs.Amount;
}
inline bool operator!=(const UXTO& lhs, const UXTO& rhs)
{
  return !(lhs == rhs);
}

class Transaction
{
  // Only one UXTO
  UXTO inputUXTO;
  std::vector<UXTO> outputUXTO;
  std::string sign;
  std::string pk;

public:
  Transaction(UXTO i,
              std::vector<UXTO> o,
              const std::string& s,
              const std::string& p)
      : inputUXTO(i)
      , outputUXTO(o)
      , sign(s)
      , pk(p)
  {}

  Transaction(UXTO i, std::vector<UXTO> o)
      : inputUXTO(i)
      , outputUXTO(o)
  {}

  void Sign(const std::string& s, const std::string& key)
  {
    pk = key;
    sign = s;
  }

  friend bool operator==(const Transaction& lhs, const Transaction& rhs);
  friend bool operator!=(const UXTO& lhs, const UXTO& rhs);
  inline operator std::string() const
  {
    std::string res = inputUXTO.Addr;
    res.append(std::to_string(inputUXTO.Amount));

    for(auto& uxto: outputUXTO) {
      res.append(uxto.Addr);
      res.append(std::to_string(uxto.Amount));
    }

    return res;
  }

  inline const std::string& GetSignature() const { return sign; }
  inline const std::string& GetKey() const { return pk; }
  inline UXTO GetInput() const { return inputUXTO; }
  inline std::vector<UXTO> GetOutput() const { return outputUXTO; }
};

class Block
{
public:
  void Mine();

  bool CheckMiningResult();

  bool Verify();

  void AddTransaction(Transaction tr) { transactions.push_back(tr); }

  inline const std::vector<Transaction>& GetTransactions() const
  {
    return transactions;
  }

  inline const std::string& GetHash() const { return hash; }
  inline const uint32_t& GetHeight() const { return height; }

  void CalculateRoot();

  static inline const std::string EMPTY_HASH =
      "000000000000000000000000000000000000000000000000000000000000000000000000"
      "00000000000000000000000000000000000000000000000000000000";
  static inline const uint32_t INITIAL_COINBASE = 1000000;

  // Header
  uint32_t height;
  uint32_t time;// Timestamp
  std::string root;
  std::string prev;// Ссылка на предыдущий блок
  uint32_t nonce;
  // Body
  uint32_t coinbase;

private:
  std::string hash;

  std::vector<Transaction> transactions;
};

#endif// CRYPTO_BLOCK_H
