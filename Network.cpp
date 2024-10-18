#include "Network.h"

std::vector<UXTO> Network::GetClientUXTOs(const std::string& addr)
{
  std::vector<UXTO> clientUXTOs;

  for(auto& uxto: miners.front()->GetUXTOs()) {
    if(uxto.Addr == addr) clientUXTOs.push_back(uxto);
  }

  return clientUXTOs;
}

std::shared_ptr<Client> Network::GetClientByAddr(const std::string& addr)
{
  auto pos = std::find_if(clients.begin(),
                          clients.end(),
                          [addr](std::shared_ptr<Client> cli) {
                            return cli->GetAddress() == addr;
                          });
  if(pos == clients.end()) {
    auto miner = std::find_if(miners.begin(),
                              miners.end(),
                              [addr](std::shared_ptr<Client> cli) {
                                return cli->GetAddress() == addr;
                              });
    if(miner == miners.end()) return {};

    return *miner;
  }

  return *pos;
}

std::shared_ptr<Client> Network::GetClientByName(const std::string& name)
{
  auto pos = std::find_if(
      clients.begin(),
      clients.end(),
      [name](std::shared_ptr<Client> cli) { return cli->GetName() == name; });
  if(pos == clients.end()) {
    auto miner = std::find_if(
        miners.begin(),
        miners.end(),
        [name](std::shared_ptr<Client> cli) { return cli->GetName() == name; });
    if(miner == miners.end()) return {};

    return *miner;
  }

  return *pos;
}

void Network::Update()
{
  switch(state) {

  case State::ChoosingMiner:
    std::cout << "Choosing random miner\n";
    choosenMiner = miners.at(rand() % miners.size());
    std::cout << "Miner " << choosenMiner->GetName() << " was selected\n";
    state = State::SendingTransaction;
    break;
  case State::SendingTransaction: {

    std::string userInput;
    std::shared_ptr<Client> currentUser;
    do {
      if(!currentUser) {
        std::cout << "Curret user is empty\n";
      } else {
        std::cout << "Curret user is " << currentUser->GetName() << "\n";
      }
      std::cout << "1) Transfer from too\n"
                   "2) Check amount\n"
                   "3) Switch current user\n"
                   "4) Force error \n"
                   "5) Output current miner uxto\n"
                   "6) Finish\n>>";

      std::cin >> userInput;

      if(userInput == "6") break;
      if(userInput == "3") {
        std::cout << "Input name\n>>";
        std::cin >> userInput;
        currentUser = GetClientByName(userInput);
        while(currentUser == nullptr) {
          std::cerr << "Incorrect name\n";
          std::cout << "Input name\n>>";
          std::cin >> userInput;
          auto currentUser = GetClientByName(userInput);
        }
      }
      if(userInput == "2" && currentUser) {
        auto uxtos = currentUser->GetUXTOs();

        for(auto& uxto: uxtos) {
          std::cout << uxto.Amount << " : "
                    << GetClientByAddress(uxto.Addr)->GetName() << "\n";
        }
      }
      if(userInput == "5" && choosenMiner) {
        auto uxtos = choosenMiner->GetUXTOs();

        for(auto& uxto: uxtos) {
          std::cout << uxto.Amount << " : "
                    << GetClientByAddress(uxto.Addr)->GetName() << "\n";
        }
      }
      if(userInput == "1" && currentUser) {
        std::string to;
        std::string amount;
        std::cout << "To >>";
        std::cin >> to;

        std::cout << "Amount >>";
        std::cin >> amount;

        choosenMiner->AddTransaction(
            currentUser->CreateTransaction(std::stoi(amount),
                                           GetClientByName(to)->GetAddress()));
      }
      if(userInput == "4") {
        if(choosenMiner) choosenMiner->SetForceError();
      }
    } while(userInput != "6");

    state = State::MiningNewBlock;
    break;
  }
  case State::MiningNewBlock:
    std::cout << "Creating new block\n";
    currentBlock = choosenMiner->CreateNewBlock();
    state = State::Validating;
    break;
  case State::Validating: {
    int count = 1;// мы согласны с собой
    for(auto miner: miners) {
      if(miner == choosenMiner) continue;

      std::cout << "Miner " << miner->GetName() << " validating block "
                << currentBlock.height << "\n";

      if(miner->ValidateBlock(currentBlock)) { count++; }
    }
    std::cout << count << "/" << miners.size() << " are agreeing\n";
    state = State::Accepting;
    break;
  }
  case State::Accepting:
    state = State::ChoosingMiner;
    for(auto client: clients) {

      std::cout << "Sending to " << client->GetName() << " his "
                << GetClientUXTOs(client->GetAddress()).size() << "\n";
      client->UpdateUxto(GetClientUXTOs(client->GetAddress()));
    }
    break;
  }
  std::cout << "===================================\n";
}
