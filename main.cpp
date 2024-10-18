#include "Network.h"

#include <chrono>
#include <thread>

Network net;
std::shared_ptr<Client> GetClientByAddress(const std::string& add)
{
  return net.GetClientByAddr(add);
}

int main()
{
  auto initialMiner = std::make_shared<Miner>("Miner1");
  net.AddMiner(initialMiner);// Начальный майнер
  net.Update();
  net.Update();
  net.Update();
  net.Update();
  net.Update();
  net.AddMiner(
      std::make_shared<Miner>("Miner2",
                              initialMiner->GetChain(),
                              initialMiner->GetUXTOs()));// Начальный майнер
  net.AddMiner(
      std::make_shared<Miner>("Miner3",
                              initialMiner->GetChain(),
                              initialMiner->GetUXTOs()));// Начальный майнер
  net.AddClient(std::make_shared<Client>("Client1"));
  net.AddClient(std::make_shared<Client>("Client2"));

  while(true) {
    net.Update();
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}
