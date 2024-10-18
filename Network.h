#ifndef CRYPTO_NETWORK_H
#define CRYPTO_NETWORK_H

#include "Clients.h"
#include <memory>
#include <vector>

class Network
{
public:
  inline Network() {}

  inline void AddMiner(std::shared_ptr<Miner> miner)
  {
    std::cout << "Miner " << miner->GetName()
              << " added to the network and got copy of the chain\n";
    miners.push_back(miner);
  }

  inline void AddClient(std::shared_ptr<Client> client)
  {
    clients.push_back(client);
  }

  // Обновлени состояние сети
  void Update();

  std::vector<UXTO> GetClientUXTOs(const std::string& addr);

  std::shared_ptr<Client> GetClientByAddr(const std::string& addr);

  std::shared_ptr<Client> GetClientByName(const std::string& name);

private:
  std::vector<std::shared_ptr<Miner>> miners;
  std::vector<std::shared_ptr<Client>> clients;

  std::shared_ptr<Miner> choosenMiner;
  Block currentBlock;

  enum class State
  {
    ChoosingMiner,
    SendingTransaction,
    MiningNewBlock,
    Validating,
    Accepting
  } state = State::ChoosingMiner;
};

#endif// CRYPTO_NETWORK_H
