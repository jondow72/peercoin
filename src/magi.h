// src/magi.h
#ifndef MAGI_H
#define MAGI_H

#include <map>
#include <uint256.h>

// Forward declaration
class CBlockIndex;

// Global block index map
extern std::map<uint256, CBlockIndex*> mapBlockIndex;

#endif // MAGI_H