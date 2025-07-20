#include "magi.h"
#include "consensus/consensus.h"
#include "primitives/block.h"
#include <../crypto/m7m.h>
#include <../crypto/magimath.h>
#include "util/system.h"
#include "util/strencodings.h"
#include "chain.h"
#include <inttypes.h>

#define M7Mv2_SCALE 2.545
#define PRM_MAGI_POW_HEIGHT_V2 50000
#define END_MAGI_POW_HEIGHT_V2 5000000
#define BLOCK_REWARD_ADJT 2700
#define BLOCK_REWARD_ADJT_M7M_V2 32750
#define HEIGHT_INIT_MAINTENANCE 1451226
#define HEIGHT_END_MAINTENANCE 1481500
static const uint32_t GENESIS_TIME = 1410566399;
static const double MAX_MAGI_PROOF_OF_STAKE = 0.05;
static bool fDebugMagi = false;

double GetDifficultyFromBits(unsigned int nBits) {
    int nShift = (nBits >> 24) & 0xff;
    double dDiff = (double)0x0000ffff / (double)(nBits & 0x00ffffff);
    while (nShift < 29) {
        dDiff *= 256.0;
        nShift++;
    }
    while (nShift > 29) {
        dDiff /= 256.0;
        nShift--;
    }
    return dDiff;
}

int64_t GetProofOfWorkReward(unsigned int nBits, unsigned int nHeight, int64_t nFees) {
    bool fTestNet = gArgs.GetBoolArg("-testnet", false);
    double nDiff = GetDifficultyFromBits(nBits);
    int64_t nSubsidy = 0;
    
    if (fTestNet && (nHeight % 2 == 0)) {
        if (nHeight <= 10) {
            nSubsidy = 100000 * COIN;
            return nSubsidy + nFees;
        }
        nSubsidy = (100 * COIN) >> (nHeight / 1051200);
        if (fDebugMagi) LogPrintf("@@GPoWR-testnet nHeight = %u, nSubsidy = %" PRId64 ", nDiff = %f\n", 
                                  nHeight, nSubsidy / COIN, nDiff);
        return nSubsidy + nFees;
    }
    
    if (nHeight <= 10 && !fTestNet) {
        nSubsidy = 112500 * COIN; // 11,250,000 XMG for blocks 0-10
    } else if (nHeight <= PRM_MAGI_POW_HEIGHT_V2) {
        if (nHeight <= BLOCK_REWARD_ADJT) {
            nSubsidy = 495.05 * pow((5.55243 * (exp_n(-0.3 * nDiff / 15.762) - exp_n(-0.6 * nDiff / 15.762))) * nDiff, 0.5) / 8.61553;
            if (nSubsidy < 5) nSubsidy = 5;
            nSubsidy *= COIN;
            if (fDebugMagi) LogPrintf("@@GPoWR nHeight = %u, nSubsidy = %" PRId64 ", nDiff = %f\n", 
                                      nHeight, nSubsidy / COIN, nDiff);
        } else if (nHeight <= BLOCK_REWARD_ADJT_M7M_V2) {
            double nDiffcu = (nHeight <= 2700) ? 2.2 : (2.2 + (nHeight - 2700) * 0.0000274841);
            nSubsidy = 294.118 * pow((5.55243 * (exp_n(-0.3 * nDiff / 0.39) - exp_n(-0.6 * nDiff / 0.39))) * nDiff, 0.5) / 1.335
                       * exp_n2(nDiff / 0.08, nDiffcu / 0.08);
            if (nSubsidy < 5) nSubsidy = 5;
            nSubsidy *= COIN;
            if (fDebugMagi) LogPrintf("@@GPoWR nHeight = %u, nSubsidy = %" PRId64 ", nDiff = %f\n", 
                                      nHeight, nSubsidy / COIN, nDiff);
        } else {
            double nDiffcu = (nHeight <= 2700) ? 2.2 / M7Mv2_SCALE : ((2.2 + (nHeight - 2700) * 0.0000183227)) / M7Mv2_SCALE;
            nSubsidy = 294.118 * pow((5.55243 * (exp_n(-0.3 * nDiff / 0.39 * M7Mv2_SCALE) - exp_n(-0.6 * nDiff / 0.39 * M7Mv2_SCALE))) * nDiff, 0.5) / 0.8456
                       * exp_n2(nDiff / (0.08 / M7Mv2_SCALE), nDiffcu / (0.08 / M7Mv2_SCALE));
            if (nSubsidy < 5) nSubsidy = 5;
            nSubsidy *= COIN;
            if (fDebugMagi) LogPrintf("@@GPoWR nHeight = %u, nSubsidy = %" PRId64 ", nDiff = %f\n", 
                                      nHeight, nSubsidy / COIN, nDiff);
        }
    } else if (nHeight <= END_MAGI_POW_HEIGHT_V2) {
        double nDiffcu = log(nHeight) * 0.1;
        nSubsidy = 50 * pow((5.55243 * (exp_n(-0.3 * nDiff / 0.39 * M7Mv2_SCALE) - exp_n(-0.6 * nDiff / 0.39 * M7Mv2_SCALE))) * nDiff, 0.5) / 0.8456
                   * exp_n2(nDiff / (0.16 / M7Mv2_SCALE), nDiffcu / (0.16 / M7Mv2_SCALE));
        if (nSubsidy < 3) nSubsidy = 3;
        nSubsidy *= COIN;
        if (fDebugMagi) LogPrintf("@@GPoWR nHeight = %u, nSubsidy = %" PRId64 ", nDiff = %f\n", 
                                  nHeight, nSubsidy / COIN, nDiff);
        for (int i = 525600; i <= nHeight; i += 525600) nSubsidy *= 0.93;
    } else {
        nSubsidy = MIN_TX_FEE;
    }
    return nSubsidy + nFees;
}

int64_t GetProofOfWorkRewardV2(const CBlockIndex* pindexPrev, int64_t nFees, bool fLastBlock) {
    if (!pindexPrev) return 0;
    unsigned int nHeight = fLastBlock ? pindexPrev->nHeight : pindexPrev->nHeight + 1;
    return GetProofOfWorkReward(pindexPrev->nBits, nHeight, nFees);
}

bool IsMaintenance(const CBlockIndex* pindex) {
    return (pindex->nHeight > HEIGHT_INIT_MAINTENANCE && pindex->nHeight < HEIGHT_END_MAINTENANCE);
}

bool IsPoSIIProtocolV2(int nHeight) {
    return nHeight >= 50000;
}

int64_t GetPoSKernelPS(CBlockIndex* pindex) {
    return GetPoSKernelPS(pindex, 72);
}

double GetPoSKernelPS(const CBlockIndex* blockindex, int lookup) {
    int nPoSInterval = lookup;
    double dStakeKernelsTriedAvg = 0;
    int nStakesHandled = 0, nStakesTime = 0;

    const CBlockIndex* pindex = (blockindex == nullptr) ? GetLastBlockIndex(pindexBest, true) : blockindex;
    const CBlockIndex* pindexPrevStake = nullptr;

    while (pindex && nStakesHandled < nPoSInterval) {
        if (pindex->IsProofOfStake()) {
            dStakeKernelsTriedAvg += GetDifficultyFromBits(pindex->nBits) * 4294967296.0;
            nStakesTime += pindexPrevStake ? (pindexPrevStake->nTime - pindex->nTime) : 0;
            pindexPrevStake = pindex;
            nStakesHandled++;
        }
        pindex = pindex->pprev;
    }
    if (fDebugMagi)
        LogPrintf("@GetPoSKernelPS -> stake blocks for average = %d\n", nStakesHandled);

    double result = 0;
    if (nStakesTime)
        result = dStakeKernelsTriedAvg / nStakesTime;

    if (IsProtocolV3(nBestHeight))
        result *= STAKE_TIMESTAMP_MASK + 1;

    return result;
}

double GetPoSKernelPSV2(const CBlockIndex* blockindex, int lookup) {
    int nPoSInterval = lookup;
    double diffTot = 0.;

    const CBlockIndex* pindex0 = (blockindex == nullptr) ? GetLastBlockIndex(pindexBest, true) : blockindex;
    const CBlockIndex* pindexPrev = GetLastPoSBlockIndex(pindex0);
    if (pindexPrev == nullptr || !pindexPrev->nHeight) return 0;
    const CBlockIndex* pindexPrevPrev = GetLastPoSBlockIndex(pindexPrev->pprev);
    if (pindexPrevPrev == nullptr || !pindexPrevPrev->nHeight) return 0;

    int nActualBlockTime = pindexPrev->GetBlockTime() - pindexPrevPrev->GetBlockTime(), nActualBlockTimeTot = nActualBlockTime;
    int nStakesHandled = 1;
    diffTot = GetDifficultyFromBits(pindexPrev->nBits);
    for (int i = 1; i < nPoSInterval; i++) {
        pindexPrev = pindexPrevPrev;
        pindexPrevPrev = GetLastPoSBlockIndex(pindexPrev->pprev);
        if (pindexPrevPrev == nullptr || !pindexPrevPrev->nHeight) break;
        nActualBlockTime = pindexPrev->GetBlockTime() - pindexPrevPrev->GetBlockTime();
        diffTot += GetDifficultyFromBits(pindexPrev->nBits);
        nActualBlockTimeTot += nActualBlockTime;
        nStakesHandled++;
    }
    if (nActualBlockTimeTot == 0 || nStakesHandled == 0) return 0;
    if (fDebugMagi)
        LogPrintf("@GetPoSKernelPSV2 -> aver diff = %f, block time = %f\n", diffTot / (double)nStakesHandled, (double)nActualBlockTimeTot / (double)nStakesHandled);

    return diffTot * 4294967296.0 / double(nActualBlockTimeTot);
}

double GetPoSKernelPSV3(const CBlockIndex* blockindex) {
    int nPoSInterval = 72;
    double dStakeKernelsTriedAvg = 0., diff = 0.;

    const CBlockIndex* pindex0 = (blockindex == nullptr) ? GetLastBlockIndex(pindexBest, true) : blockindex;
    const CBlockIndex* pindexPrev = GetLastPoSBlockIndex(pindex0);
    if (pindexPrev == nullptr || !pindexPrev->nHeight) return 0;
    const CBlockIndex* pindexPrevPrev = GetLastPoSBlockIndex(pindexPrev->pprev);
    if (pindexPrevPrev == nullptr || !pindexPrevPrev->nHeight) return 0;

    int nActualBlockTime = pindexPrev->GetBlockTime() - pindexPrevPrev->GetBlockTime(), nActualBlockTimeTot = nActualBlockTime;
    int nStakesHandled = 1;
    if (nActualBlockTime <= 0) {
        nActualBlockTimeTot = 0;
        nStakesHandled = 0;
    } else {
        diff = GetDifficultyFromBits(pindexPrev->nBits);
        dStakeKernelsTriedAvg = GetDifficultyFromBits(pindexPrev->nBits) * 4294967296.0 / double(nActualBlockTime);
    }
    for (int i = 1; i < nPoSInterval; i++) {
        pindexPrev = pindexPrevPrev;
        pindexPrevPrev = GetLastPoSBlockIndex(pindexPrev->pprev);
        if (pindexPrevPrev == nullptr || !pindexPrevPrev->nHeight) break;
        nActualBlockTime = pindexPrev->GetBlockTime() - pindexPrevPrev->GetBlockTime();
        if (nActualBlockTime > 0) {
            diff += GetDifficultyFromBits(pindexPrev->nBits);
            dStakeKernelsTriedAvg += GetDifficultyFromBits(pindexPrev->nBits) * 4294967296.0 / double(nActualBlockTime);
            nActualBlockTimeTot += nActualBlockTime;
            nStakesHandled++;
        }
    }
    if (nActualBlockTimeTot == 0 || nStakesHandled == 0) return 0;
    if (fDebugMagi)
        LogPrintf("@GetPoSKernelPSV3 -> aver diff = %f, block time = %f\n", diff / (double)nStakesHandled, (double)nActualBlockTimeTot / (double)nStakesHandled);

    return dStakeKernelsTriedAvg / double(nStakesHandled);
}

double GetAnnualInterest(int64_t nNetWorkWeit, double rMaxAPR) {
    double rWeit = 20000.0;
    double rAPR = (2.0 / (1.0 + exp_n(1.0 / (nNetWorkWeit / rWeit + 1.0))) - 0.53788) * rMaxAPR
                  / (2.0 / (1.0 + exp_n(1.0 / (rWeit + 1.0))) - 0.53788);
    if (fDebugMagi) LogPrintf("@PoS-APR rAPR = %f, nNetWorkWeit = %" PRId64 "\n", rAPR, nNetWorkWeit);
    return rAPR;
}

double GetAnnualInterestV2(int64_t nNetWorkWeit, double rMaxAPR, CBlockIndex* pindex) {
    double rWeit = 500000.0;
    double rAPR = (2.0 / (1.0 + exp_n(1.0 / (nNetWorkWeit / rWeit + 1.0))) - 0.53788) * rMaxAPR
                  / (2.0 / (1.0 + exp_n(1.0 / (rWeit + 1.0))) - 0.53788);
    if (pindex && IsMaintenance(pindex)) rAPR *= 1.2;
    if (fDebugMagi) LogPrintf("@PoS-APRV2 rAPR = %f, nNetWorkWeit = %" PRId64 "\n", rAPR, nNetWorkWeit);
    return rAPR;
}

int64_t GetProofOfStakeReward(int64_t nCoinAge, unsigned int nBits, unsigned long nTime, CBlockIndex* pindex) {
    bool fTestNet = gArgs.GetBoolArg("-testnet", false);
    double nDiff = GetDifficultyFromBits(nBits);
    int64_t nSubsidy = 0;

    if (fTestNet) {
        nSubsidy = 100 * COIN;
        if (fDebugMagi) LogPrintf("@@GPoSR-testnet nCoinAge = %" PRId64 ", nSubsidy = %" PRId64 ", nDiff = %f\n", 
                                  nCoinAge, nSubsidy / COIN, nDiff);
        return nSubsidy;
    }

    int64_t nNetWorkWeit = pindex ? GetPoSKernelPS(pindex) : 1000000;
    double rAPR = IsPoSIIProtocolV2(pindex ? pindex->nHeight + 1 : 0) 
                  ? GetAnnualInterestV2(nNetWorkWeit, MAX_MAGI_PROOF_OF_STAKE, pindex)
                  : GetAnnualInterest(nNetWorkWeit, MAX_MAGI_PROOF_OF_STAKE);
    nSubsidy = nCoinAge * rAPR * 33 / (365 * 33 + 8) * COIN;
    if (nSubsidy < COIN) nSubsidy = COIN;
    if (nSubsidy > 100 * COIN) nSubsidy = 100 * COIN;
    if (fDebugMagi) LogPrintf("@@GPoSR nCoinAge = %" PRId64 ", nSubsidy = %" PRId64 ", nDiff = %f, nTime = %lu, rAPR = %f\n", 
                              nCoinAge, nSubsidy / COIN, nDiff, nTime, rAPR);
    return nSubsidy;
}

int64_t GetProofOfStakeReward(int64_t nCoinAge, unsigned int nBits, unsigned long nTime) {
    return GetProofOfStakeReward(nCoinAge, nBits, nTime, nullptr);
}
