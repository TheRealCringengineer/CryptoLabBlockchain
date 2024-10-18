#include "Clients.h"

#include <algorithm>

bool IsAddressExist(const std::string& add)
{
  return GetClientByAddress(add) != nullptr;
}

bool Miner::IsTransactionValid(const Transaction& tr)
{
  std::cout << "Checkin transaction\n";
  auto sender = GetClientByAddress(tr.GetInput().Addr);
  if(!sender) {
    std::cout << "Invalid sender address\n";
    return false;
  }

  auto res = std::find(uxtos.begin(), uxtos.end(), tr.GetInput());
  if(res == uxtos.end()) {
    std::cout << "Sender doesn't have UXTO\n";
    return false;// Нет UXTO
  }

  if(res->Amount != tr.GetInput().Amount) {
    std::cout << "Only complete UXTO\n";
    return false;// Только полные UXTO
  }

  uint32_t sum = 0;
  for(auto& output: tr.GetOutput()) {
    sum += output.Amount;
    if(!IsAddressExist(output.Addr)) {
      std::cout << "Nobody to recieve\n";
      return false;
    }
  }

  return Crypto::verify(tr, tr.GetSignature(), tr.GetKey());
}

Block Miner::CreateNewBlock()
{
  Block block;

  if(chain.GetLastBlock()) {
    block.height = chain.GetLastBlock()->GetHeight() + 1;
    block.prev = chain.GetLastBlock()->GetHash();
    if(forceError) {
      block.prev = "incorrect";
      forceError = false;
    }
    block.time = chain.GetLastBlock()->time + 1;
    block.nonce = 0;
    block.coinbase = chain.GetLastBlock()->coinbase - 5;
  } else {
    block.height = 0;
    block.time = 0;// TODO : Later
    block.nonce = 0;
    block.coinbase = Block::INITIAL_COINBASE;
    block.prev = Block::EMPTY_HASH;
  }

  std::cout << "Adding reward to transaction list\n";

  block.AddTransaction(Transaction({"coinbase", 5}, {{addr, 5}}, "", ""));

  std::cout << "Adding transactions list\n";
  for(auto& tr: transactions) {

    if(IsTransactionValid(tr)) {
      block.AddTransaction(tr);
    } else {
      std::cerr << "Invalid transaction found\n";
    }
  }
  transactions.clear();

  std::cout << "Calculating Merkle tree\n";
  block.CalculateRoot();

  std::cout << "Adding UXTOs\n";
  for(auto& tr: block.GetTransactions()) {
    auto res = std::find(uxtos.begin(), uxtos.end(), tr.GetInput());
    if(res != uxtos.end()) { uxtos.erase(res); }

    for(auto& output: tr.GetOutput()) { uxtos.push_back(output); }
  }

  UpdateBalance();

  std::cout << "Mining\n";
  block.Mine();

  std::cout << "Mining finished with nonce " << block.nonce << "\n";

  chain.AddBlock(block);

  return block;
}

bool Miner::ValidateBlock(Block block)
{
  std::string originalRoot = block.root;
  block.CalculateRoot();
  if(originalRoot != block.root) {
    std::cout << "Merkle root is incorrect\n";
    return false;
  }

  if(!block.CheckMiningResult()) {
    std::cout << "Mining result is incorrect\n";
    return false;
  }

  if(chain.GetLastBlock()) {
    if(block.height - chain.GetLastBlock()->GetHeight() != 1) {
      std::cout << "Height is incorrect " << block.height << "\n";
      return false;
    }

    if(block.prev != chain.GetLastBlock()->GetHash()) {
      std::cout << "Prev is incorrect\n";
      return false;
    }

    if(block.time <= chain.GetLastBlock()->time) {
      std::cout << "Time is incorrect\n";
      return false;
    }

    if(chain.GetLastBlock()->coinbase - block.coinbase != 5) {
      std::cout << "Incorrect coinbase amount " << block.coinbase << "\n";
      return false;
    }
  } else {
    if(block.height != 0) {
      std::cout << "Height is not zero\n";
      return false;
    }
    if(block.prev != Block::EMPTY_HASH) {
      std::cout << "Prev is not zero\n";
      return false;
    }
    if(block.time != 0) {
      std::cout << "Time is not zero\n";
      return false;
    }
    if(block.coinbase != Block::INITIAL_COINBASE) {
      std::cout << "Incorrect initial coinbase\n";
      return false;
    }
  }

  bool isFirst = true;
  for(auto& tr: block.GetTransactions()) {
    if(isFirst) {
      if(tr.GetInput().Addr != "coinbase" || tr.GetInput().Amount != 5) {
        std::cerr << "Invalid reward\n";
      }
      isFirst = false;
      uxtos.push_back(tr.GetOutput().front());
      continue;
    }
    if(!IsTransactionValid(tr)) {
      std::cout << "Some transaction is incorrect\n";
      return false;
    }
    auto res = std::find(uxtos.begin(), uxtos.end(), tr.GetInput());
    if(res != uxtos.end()) { uxtos.erase(res); }

    for(auto& output: tr.GetOutput()) { uxtos.push_back(output); }
  }

  UpdateBalance();

  std::cout << "LGTM\n";
  // добавляем блок в свою копию реестра
  chain.AddBlock(block);

  return true;
}

Transaction Client::CreateTransaction(uint32_t amount, std::string target)
{
  for(auto& uxto: uxtos) {
    if(uxto.Amount == amount) {
      Transaction tr({addr, amount}, {{target, amount}});
      tr.Sign(Crypto::sign(tr, sk), pk);

      auto temp = std::remove_if(
          uxtos.begin(),
          uxtos.end(),
          [amount](const UXTO& uxto) { return uxto.Amount == amount; });
      return tr;
    }
  }

  for(auto& uxto: uxtos) {
    if(uxto.Amount > amount) {
      Transaction tr({addr, uxto.Amount},
                     {{target, amount}, {addr, uxto.Amount - amount}});
      tr.Sign(Crypto::sign(tr, sk), pk);

      auto temp = std::remove_if(
          uxtos.begin(),
          uxtos.end(),
          [amount](const UXTO& uxto) { return uxto.Amount == amount; });
      return tr;
    }
  }

  // чтобы можно было явно ошибится
  return {{addr, amount}, {{target, amount}}};
}
