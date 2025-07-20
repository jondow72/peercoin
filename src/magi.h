#ifndef MAGI_H
#define MAGI_H

#include "consensus/consensus.h"
#include "primitives/block.h"
#include "chain.h"

class BlockValidationState;

// Magi-specific reward functions
int64_t GetProofOfWorkReward(unsigned int nBits, unsigned int nHeight, int64_t nFees = 0);
int64_t GetProofOfWorkRewardV2(const CBlockIndex* pindexPrev, int64_t nFees, bool fLastBlock);
int64_t GetProofOfStakeReward(int64_t nCoinAge, unsigned int nBits, unsigned long nTime, CBlockIndex* pindex = nullptr);
double GetAnnualInterest(int64_t nNetWorkWeit, double rMaxAPR);
double GetAnnualInterestV2(int64_t nNetWorkWeit, double rMaxAPR, CBlockIndex* pindex);
int64_t GetPoSKernelPS(CBlockIndex* pindex = nullptr);
bool IsMaintenance(const CBlockIndex* pindex);
bool IsPoSIIProtocolV2(int nHeight);
double GetDifficultyFromBits(unsigned int nBits);
double GetPoSKernelPSV2(const CBlockIndex* blockindex = nullptr, int lookup = 72);
double GetPoSKernelPSV3(const CBlockIndex* blockindex = nullptr);

// Wrapper for Peercoin compatibility
int64_t GetProofOfStakeReward(int64_t nCoinAge, unsigned int nBits, unsigned long nTime);

#endif

