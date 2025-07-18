#ifndef MAGI_H
#define MAGI_H

#include "consensus/consensus.h"
#include "primitives/block.h"
#include "chain.h"

class BlockValidationState;

// Magi-specific reward functions
int64_t GetProofOfWorkReward(unsigned int nBits, unsigned int nHeight, int64_t nFees = 0);
int64_t GetProofOfStakeReward(int64_t nCoinAge, unsigned int nBits, unsigned long nTime, CBlockIndex* pindex = nullptr);
double GetAnnualInterest(int64_t nNetWorkWeit, double rMaxAPR);
double GetAnnualInterestV2(int64_t nNetWorkWeit, double rMaxAPR, CBlockIndex* pindex);
int64_t GetPoSKernelPS(CBlockIndex* pindex = nullptr);
bool IsMaintenance(CBlockIndex* pindex);
bool IsPoSIIProtocolV2(int nHeight);

// Wrapper for compatibility with Peercoin's validation.h
int64_t GetProofOfStakeReward(int64_t nCoinAge, unsigned int nBits, unsigned long nTime);

#endif

