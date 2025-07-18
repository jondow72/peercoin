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

static const double MAX_MAGI_PROOF_OF_STAKE = 0.05;
static bool fDebugMagi = false;

double GetDifficultyFromBits(unsigned int nBits){
    int nShift = (nBits >> 24) & 0xff;

    double dDiff =
        (double)0x0000ffff / (double)(nBits & 0x00ffffff);

    while (nShift < 29)
    {
        dDiff *= 256.0;
        nShift++;
    }
    while (nShift > 29)
    {
        dDiff /= 256.0;
        nShift--;
    }
    return dDiff;
}

int64_t GetProofOfWorkReward(unsigned int nBits, unsigned int nHeight, int64_t nFees)
{
    bool fTestNet = gArgs.GetBoolArg("-testnet", false);
    double nDiff = GetDifficultyFromBits(nBits);

    int64 nSubsidy = 0;
    
    if (fTestNet && (nHeight%2 == 0))
    {
	if(nHeight <= 10)
	{
	    nSubsidy = 100000 * COIN;
	    return nSubsidy + nFees;
	}
	nSubsidy = (100 * COIN) >> (nHeight / 1051200); // cut in half every 1.05 mil blocks ~2 years
	if (fDebugMagi) LogPrintf("@@GPoWR-testnet nHeight = %u, nSubsidy = %" PRId64 ", nDiff = %f\n", 
	       nHeight, nSubsidy / COIN, nDiff);
	return nSubsidy + nFees;
    }
    
    /*	Notes of 11 premined blocks, totally: 1,237,505 XMG
	Coins burned: 720,000 XMG https://bchain.info/XMG/addr/93m4hAxmCcGXMfnjVPfNhWSjb69sDziGSY
				  https://bitcointalk.org/index.php?topic=735170.msg9475622#msg9475622
	Coins used to push PoM campaign: 112,505 XMG (https://bitcointalk.org/index.php?topic=802681.0)

	Remaining coins are: 404,995 (1.65%), that includes: 
	Coin swap: 233,319 XMG (0.93%)
	Leftover: 171,676 XMG (0.69%) - promotion (givaway + bounties for community members' contribution), staff salary

	Coin swap: rule of swap - total coins swapped/Coins in circulation ~ 10% or less
	Some of posts regarding the coin swap: 
	https://bitcointalk.org/index.php?topic=821170.0
	https://bitcointalk.org/index.php?topic=735170.msg8950501#msg8950501
	https://bitcointalk.org/index.php?topic=735170.msg9111697#msg9111697
	
	Details: https://bitcointalk.org/index.php?topic=735170.msg9900074#msg9900074
    */
    if(nHeight <= 10 && !fTestNet)
    {
        nSubsidy = 112500 * COIN;
    }
    else if (nHeight <= PRM_MAGI_POW_HEIGHT_V2) // difficulty dependent PoW-I mining
    {
	if (nHeight <= BLOCK_REWARD_ADJT) {
	    nSubsidy = 495.05 * pow( (5.55243*(exp_n(-0.3*nDiff/15.762) - exp_n(-0.6*nDiff/15.762)))*nDiff, 0.5) / 8.61553;
	    if (nSubsidy < 5) nSubsidy = 5;
	    nSubsidy *= COIN;
	    if (fDebugMagi) LogPrintf("@@GPoWR nHeight = %u, nSubsidy = %" PRId64 ", nDiff = %f\n", 
				nHeight, nSubsidy / COIN, nDiff);
	}
	else if (nHeight <= BLOCK_REWARD_ADJT_M7M_V2) {
	    double nDiffcu = ((nHeight <= 2700) ? 2.2 : (2.2+(nHeight-2700)*0.0000274841));
	    nSubsidy = 294.118 * pow( (5.55243*(exp_n(-0.3*nDiff/0.39) - exp_n(-0.6*nDiff/0.39)))*nDiff, 0.5) / 1.335
			   * exp_n2(nDiff/0.08, nDiffcu/0.08);
	    if (nSubsidy < 5) nSubsidy = 5;
	    nSubsidy *= COIN;
	    if (fDebugMagi) LogPrintf("@@GPoWR nHeight = %u, nSubsidy = %" PRId64 ", nDiff = %f\n", 
				nHeight, nSubsidy / COIN, nDiff);
	}
	else {
	    double nDiffcu = ((nHeight <= 2700) ? 2.2 / M7Mv2_SCALE : ( (2.2+(nHeight-2700)*0.0000183227)) / M7Mv2_SCALE );
	    nSubsidy = 294.118 * pow( (5.55243*(exp_n(-0.3*nDiff/0.39*M7Mv2_SCALE) - exp_n(-0.6*nDiff/0.39*M7Mv2_SCALE)))*nDiff, 0.5) / 0.8456
			   * exp_n2(nDiff/(0.08/M7Mv2_SCALE), nDiffcu/(0.08/M7Mv2_SCALE));
	    if (nSubsidy < 5) nSubsidy = 5;
	    nSubsidy *= COIN;
	    if (fDebugMagi) LogPrintf("@@GPoWR nHeight = %u, nSubsidy = %" PRId64 ", nDiff = %f\n", 
				nHeight, nSubsidy / COIN, nDiff);
	}
    }
    else if (nHeight <= END_MAGI_POW_HEIGHT_V2) // difficulty dependent PoW-II mining
    {
	double nDiffcu = log(nHeight)*0.1;
	nSubsidy = 50 * pow( (5.55243*(exp_n(-0.3*nDiff/0.39*M7Mv2_SCALE) - exp_n(-0.6*nDiff/0.39*M7Mv2_SCALE)))*nDiff, 0.5) / 0.8456
			* exp_n2(nDiff/(0.16/M7Mv2_SCALE), nDiffcu/(0.16/M7Mv2_SCALE));
	if (nSubsidy < 3) nSubsidy = 3;
	nSubsidy *= COIN;
	if (fDebugMagi) LogPrintf("@@GPoWR nHeight = %u, nSubsidy = %" PRId64 ", nDiff = %f\n", 
			    nHeight, nSubsidy / COIN, nDiff);
//	nSubsidy = 15. * 2500. / (pow((nDiff+500.)/10., 2.));
//	if (nSubsidy < 3) nSubsidy = 3;
//	nSubsidy *= COIN;
	for(int i = 525600; i <= nHeight; i += 525600) nSubsidy *= 0.93; // yearly decline (7%)
    }
    else {
	nSubsidy = MIN_TX_FEE;
    }

    return nSubsidy + nFees;
}

bool IsMaintenance(CBlockIndex* pindex) {
    return false; // Placeholder, implement Magi's maintenance mode
}

bool IsPoSIIProtocolV2(int nHeight) {
    return nHeight >= 50000; // Magi PoS-II-V2 fork height
}

int64_t GetPoSKernelPS(CBlockIndex* pindex) {
    return 1000000; // Placeholder, implement Magi's network weight calculation
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
