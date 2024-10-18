#ifndef CRYPTO_CLIENTS_H
#define CRYPTO_CLIENTS_H

#include "Block.h"
#include "Chain.h"
#include "CryptoUtils.h"

class Client
{
public:
  Client(const std::string& n)
      : name(n)
      , balance(0)
  {
    auto keys = Crypto::genKeyPair();
    sk = keys.first;
    pk = keys.second;
    addr = Crypto::sha256(pk);
  }

  inline void UpdateUxto(std::vector<UXTO> newUxtos)
  {
    uxtos = newUxtos;
    UpdateBalance();
  }

  inline void UpdateBalance()
  {
    balance = 0;
    for(auto& uxto: uxtos) { balance += uxto.Amount; }
  }

  Transaction CreateTransaction(uint32_t amount, std::string target);

  inline const std::string& GetName() { return name; }
  inline const std::string& GetAddress() { return addr; }

  inline std::vector<UXTO> GetUXTOs() { return uxtos; };

protected:
  std::string name;
  std::string sk;
  std::string pk;
  std::string addr;
  uint32_t balance;

  std::vector<UXTO> uxtos;
};

extern std::shared_ptr<Client> GetClientByAddress(const std::string& add);
extern bool IsTransactionValid(const Transaction& tr);

class Miner : public Client// Miner является и клиентом
{
public:
  // В рамках демонстрации можно просто хранить ссылку на общий реестр
  inline Miner(const std::string name)
      : Client(name)
  {}
  inline Miner(const std::string name, Chain c, std::vector<UXTO> u)
      : Client(name)
      , chain(c)
  {
    uxtos = u;
  }

  Block CreateNewBlock();

  bool ValidateBlock(Block block);

  bool IsTransactionValid(const Transaction& tr);
  inline void AddTransaction(Transaction tr) { transactions.push_back(tr); }

  inline const std::vector<UXTO>& GetUXTOs() const { return uxtos; }

  inline void SetForceError() { forceError = true; }
  inline Chain GetChain() { return chain; }

private:
  std::vector<Transaction> transactions;

  bool forceError = false;
  Chain chain;
};

#endif// CRYPTO_CLIENTS_H
