#include "Block.h"
#include "Chain.h"
#include "CryptoUtils.h"

void Block::Mine()
{
  std::string result = std::to_string(height);
  result.append(std::to_string(time));
  result.append(root);
  result.append(prev);

  std::string currentHash;
  nonce = 0;
  do {
    nonce++;
    currentHash = result;
    currentHash.append(std::to_string(nonce));
    currentHash = Crypto::sha256(currentHash);
  } while(currentHash.substr(0, 2) != "00");

  hash = currentHash;
}

bool Block::CheckMiningResult()
{
  std::string result = std::to_string(height);
  result.append(std::to_string(time));
  result.append(root);
  result.append(prev);

  result.append(std::to_string(nonce));
  result = Crypto::sha256(result);

  return result == hash && result.substr(0, 2) == "00";
}

std::string HashTransaction(const Transaction& tr)
{
  std::string res = tr.GetInput().Addr;
  res.append(std::to_string(tr.GetInput().Amount));

  for(auto& uxto: tr.GetOutput()) {
    res.append(uxto.Addr);
    res.append(std::to_string(uxto.Amount));
  }

  return Crypto::sha256(res);
}

std::string HashTwoHashes(const std::string& a, const std::string& b)
{
  return Crypto::sha256(a + b);
}

void Block::CalculateRoot()
{
  std::vector<std::string> hashes;
  for(auto& tr: transactions) hashes.push_back(HashTransaction(tr));
  if(hashes.size() % 2 != 0)
    hashes.push_back(HashTransaction(transactions.back()));

  while(hashes.size() != 1) {
    std::vector<std::string> newHashes;
    for(int i = 0; i < hashes.size(); i += 2)
      newHashes.push_back(HashTwoHashes(hashes[i], hashes[i + 1]));

    hashes = newHashes;
  }

  root = hashes.front();
}

bool operator==(const Transaction& lhs, const Transaction& rhs)
{
  return lhs.inputUXTO == rhs.inputUXTO && lhs.outputUXTO == rhs.outputUXTO
      && lhs.pk == rhs.pk;
}
bool operator!=(const Transaction& lhs, const Transaction& rhs)
{
  return !(lhs == rhs);
}
