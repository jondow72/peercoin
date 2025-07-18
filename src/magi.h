#ifndef MAGI_H
#define MAGI_H

#include "consensus/consensus.h"
#include "amount.h"

namespace Magi {

static const CAmount COIN = 100000000;
static const CAmount MAX_MINT_PROOF_OF_WORK = 112500 * COIN;
static const uint32_t GENESIS_TIME = 1410566399;
static const double M7Mv2_SCALE = 2.545;
static const int PRM_MAGI_POW_HEIGHT_V2 = 50000;
static const int END_MAGI_POW_HEIGHT_V2 = 5000000;
static const int BLOCK_REWARD_ADJT = 2700;
static const int BLOCK_REWARD_ADJT_M7M_V2 = 32750;

double GetDifficultyFromBits(unsigned int nBits);
int64_t GetProofOfWorkReward(unsigned int nBits, unsigned int nHeight);

} // namespace Magi

#endif // MAGI_H