#ifndef CRYPTO_CHAIN_H
#define CRYPTO_CHAIN_H

#include "Block.h"
#include <memory>

class Chain
{
public:
  inline void AddBlock(Block b)
  {
    std::shared_ptr<Block> newBlock = std::make_shared<Block>(b);
    head = newBlock;
  }

  inline std::shared_ptr<Block> GetLastBlock() { return head; }

private:
  std::shared_ptr<Block> head;
};

#endif// CRYPTO_CHAIN_H
