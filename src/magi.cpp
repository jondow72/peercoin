#include "magi.h"
#include "magimath.h"
#include "util/system.h"
#include <inttypes.h>

namespace Magi {

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

/*
#define BRW_BLKTIME_COEFF 0.1 // block time effect on average weight; the larger value, the less effect
#define BRW_AVER_COEFF 0.25 // the larger value, the regular moving average

#define BRW_EXPON_COEFF 0.15
#define BRW_WEIGHT_MIN 0.0001
#define BRW_WEIGHT_MAX 0.8
#define BRW_WEIGHT_SCALE 10000.0

#define DAMPINGCU 0.55
#define DAMPINGRATE 0.075
#define DAMPINMIN 0.3
#define DAMPINGAMP 2.0

#define BBLOCK 100
#define BBLOCK_AVER 2000
// diff data filter to stabilize the rewards
double GetDifficultyFromBitsV2(const CBlockIndex* pindex0, bool fPrintInfo)
{
    int64 nWeightTot, nActualBlockSpacing;
    double rDiffAverEMA, rDiffAver, rfw, rWeight;
    const CBlockIndex* pindexPrev = pindex0;

    // finding the average diff over up to 2000 backward blocks
    rDiffAver = GetDifficultyFromBits(pindexPrev->nBits);
    nWeightTot = 1;
    for(int i = 1; i <= BBLOCK_AVER-1; i++) {
    	pindexPrev = GetLastPoWBlockIndex(pindexPrev->pprev);
    	if (!pindexPrev || pindexPrev->nHeight==0) {
    	    printf("WARNING: averaged over less than BBLOCK_AVER blocks --> GetDifficultyFromBitsV2\n");
    	    break;
        }
        rDiffAver += GetDifficultyFromBits(pindexPrev->nBits);
        ++nWeightTot;
    }
    rDiffAver /= double(nWeightTot);

    pindexPrev = pindex0;
    const CBlockIndex* pindexPrevPrev = GetLastPoWBlockIndex(pindexPrev->pprev);
    if (!pindexPrevPrev || pindexPrevPrev->nHeight==0) {
	printf("ERROR: no actual average done --> GetDifficultyFromBitsV2\n");
	return rDiffAver;
    }
    nActualBlockSpacing = pindexPrev->GetBlockTime() - pindexPrevPrev->GetBlockTime();
    // moving average factor depending on block time; less rfw, smoother the diff
    rfw = (1. - exp_n(-double(nActualBlockSpacing)*BRW_EXPON_COEFF*BRW_BLKTIME_COEFF/double(GetTargetSpacingWork(pindex0->nHeight+1))) ) * BRW_AVER_COEFF;
    if (rfw < BRW_WEIGHT_MIN) { rfw = BRW_WEIGHT_MIN; }
    else if (rfw > BRW_WEIGHT_MAX) { rfw = BRW_WEIGHT_MAX; }

    rDiffAverEMA = GetDifficultyFromBits(pindexPrev->nBits) * ((int64)(rfw * BRW_WEIGHT_SCALE));
    nWeightTot = ((int64)(rfw*BRW_WEIGHT_SCALE));
    rWeight = 1.-rfw;
    for(int i = 1; i <= BBLOCK-1; i++)
    {
	pindexPrev = pindexPrevPrev;
	pindexPrevPrev = GetLastPoWBlockIndex(pindexPrev->pprev);
	if (!pindexPrevPrev || pindexPrevPrev->nHeight==0) {
	    printf("WARNING: averaged over less than BBLOCK --> GetDifficultyFromBitsV2\n");
	    break;
	}
	nActualBlockSpacing = pindexPrev->GetBlockTime() - pindexPrevPrev->GetBlockTime();
	rfw = (1. - exp_n(-double(nActualBlockSpacing)*BRW_EXPON_COEFF*BRW_BLKTIME_COEFF/double(GetTargetSpacingWork(pindex0->nHeight+1))) ) * BRW_AVER_COEFF;
	if (rfw < BRW_WEIGHT_MIN) { rfw = BRW_WEIGHT_MIN; }
	else if (rfw > BRW_WEIGHT_MAX) { rfw = BRW_WEIGHT_MAX; }
	rDiffAverEMA += GetDifficultyFromBits(pindexPrev->nBits) * ((int64)(rfw * rWeight * BRW_WEIGHT_SCALE));
	nWeightTot += ((int64)(rfw * rWeight * BRW_WEIGHT_SCALE));
	rWeight *= (1.-rfw);
    }
    rDiffAverEMA /= double(nWeightTot);
    // apply damping
    double deviation = rDiffAverEMA - rDiffAver;
    double damping;
    if (fPrintInfo) printf( "@@GetDifficultyFromBitsV2 (rDiffAverEMA, rDiffAver, deviation) = (%f, %f, %f)\n", 
      rDiffAverEMA, rDiffAver, deviation );
    if (deviation > 0.) {
	damping = DAMPINGAMP * exp_n2(DAMPINGCU/DAMPINGRATE, deviation/DAMPINGRATE) + DAMPINMIN;
    }
    else {
	damping = DAMPINGAMP * exp_n2(1.5*DAMPINGCU/DAMPINGRATE, abs(deviation)/DAMPINGRATE) + DAMPINMIN;
    }
    rDiffAverEMA = deviation * damping  +  rDiffAver;
    if (fPrintInfo) printf( "@@GetDifficultyFromBitsV2 OPM (rDiffAverEMA, damping) = (%f, %f)\n", 
      rDiffAverEMA, damping );
    return rDiffAverEMA;
}


double GetDifficultyFromBitsAver(const CBlockIndex* pindex0, int nBlocksAver0)
{
    const CBlockIndex* pindexPrev = pindex0;
    int nBlocksAver = (nBlocksAver0 > 0) ? nBlocksAver0 : 50;

    // finding the average diff over backward blocks
    double rDiffAver = GetDifficultyFromBits(pindexPrev->nBits);
    int nWeightTot = 1;
    for(int i = 1; i <= nBlocksAver-1; i++)
    {
	pindexPrev = GetLastPoWBlockIndex(pindexPrev->pprev);
	if (!pindexPrev || pindexPrev->nHeight==0) break;
      	rDiffAver += GetDifficultyFromBits(pindexPrev->nBits);
	nWeightTot++;
    }
    return rDiffAver/double(nWeightTot);
}

#define HEIGHT_INIT_MAINTENANCE 1451226
#define HEIGHT_END_MAINTENANCE 1481500
bool IsMaintenance(const CBlockIndex* pindex_)
{
    return ( (pindex_->nHeight > HEIGHT_INIT_MAINTENANCE) && (pindex_->nHeight < HEIGHT_END_MAINTENANCE) );
}

int64 GetProofOfWorkReward_OPM(const CBlockIndex* pindex0)
{
    int nHeight = pindex0->nHeight;
    double M7Mv2_move = ( (nHeight <= 75000) ? 2.85 : ( 2.85 - pow( log(nHeight) - log(75000.), 0.3 )*1.5 ) );
    double rDiff = GetDifficultyFromBitsV2(pindex0);
    double rDiffcu = 2.2 / M7Mv2_move;
    double rSubsidy = 0.;
    rSubsidy = 50. * pow( (5.55243*(exp_n(-0.3*rDiff/0.39*M7Mv2_move) - exp_n(-0.6*rDiff/0.39*M7Mv2_move)))*rDiff, 0.5)
		    / (3.02849*exp_n(-M7Mv2_move / 0.14814) + 1.794*exp_n(-M7Mv2_move / 0.89044) + 0.74536)
		    * exp_n2(rDiff/(0.16/M7Mv2_move), rDiffcu/(0.16/M7Mv2_move));
    if (rDiff > rDiffcu && rSubsidy < 3.) {
	rSubsidy = 6. * exp_n2( pow( abs( rDiff - (18.02428*exp_n(-M7Mv2_move/0.17628) + 6.58466*exp_n(-M7Mv2_move/0.71943) + 0.93489) )/(1./M7Mv2_move), 0.5 ), 0.);
    }
    if (IsMaintenance(pindex0)) rSubsidy *= 0.3;
    rSubsidy *= double(COIN);
    if (rSubsidy > 50*COIN) { rSubsidy = 50*COIN; }
    else if (rSubsidy < MIN_TX_FEE) { rSubsidy = MIN_TX_FEE; }
    for(int i = 500000; i <= nHeight; i += 500000) rSubsidy *= 0.93; // yearly decline (7%)
    return (int64)rSubsidy;
}

bool IsChainInSwitch(const CBlockIndex* pindex_)
{
    const CBlockIndex *pindex0 = pindex_;
    int nHeightIncr = 0;
    while (pindex0->nHeight >= 1443960) {
        if (!pindex0) {
            printf("ERROR: IsChainInSwitch() pindex0 null identified\n");
            break;
        }
        if (pindex0->IsProofOfWork()) ++nHeightIncr;
        pindex0 = pindex0->pprev;
    }
    return ( (pindex_->nHeight >= 1443960) && (nHeightIncr < 1000) );
}

int64 GetProofOfWorkRewardV2(const CBlockIndex* pindexPrev, int64 nFees, bool fLastBlock)
{
    const CBlockIndex* pindex0 = ( fLastBlock ? GetLastPoWBlockIndex(pindexPrev) : pindexPrev );
    int nHeight = pindex0->nHeight;
    int64 nSubsidy = 0;
    
//      double rDiff = GetDifficultyFromBitsV2(pindex0); 
//      printf("@@BLKV2-test (nHeight, rDiff, rSubsidy) = (%d, %f, %f)\n", 
//    nHeight, rDiff, double(nSubsidy)/double(COIN));
      
    if (fTestNet) {
//        if (nHeight%2 == 0) nSubsidy = 1000 * COIN;
//        else nSubsidy = GetProofOfWorkReward_OPM(pindex0);
        nSubsidy = 1000 * COIN;
        return nSubsidy + nFees;
    }

    if (nHeight <= END_MAGI_POW_HEIGHT_V2) {    // difficulty dependent PoW-II mining
       nSubsidy = GetProofOfWorkReward_OPM(pindex0);
    } else {
        nSubsidy = MIN_TX_FEE;
    }

    if (fDebugMagi) {
      double rDiff = GetDifficultyFromBitsV2(pindex0); 
      printf("@@PoWII-V2 (nHeight, rDiff, rSubsidy) = (%d, %f, %f)\n", 
      nHeight, rDiff, double(nSubsidy)/double(COIN));
    }
    if (IsChainInSwitch(pindex0)) nSubsidy = (double)nSubsidy / 25.;
    return nSubsidy + nFees;
}
*/

int64_t GetProofOfWorkReward(unsigned int nBits, unsigned int nHeight) {
    bool fTestNet = gArgs.GetBoolArg("-testnet", false);
    double nDiff = GetDifficultyFromBits(nBits);
    int64_t nSubsidy = 0;
    
    if (fTestNet && (nHeight % 2 == 0)) {
        if (nHeight <= 10) {
            nSubsidy = 100000 * COIN;
            return nSubsidy;
        }
        nSubsidy = (100 * COIN) >> (nHeight / 1051200);
        if (fDebugMagi) LogPrintf("@@GPoWR-testnet nHeight = %u, nSubsidy = %" PRId64 ", nDiff = %f\n", 
                                  nHeight, nSubsidy / COIN, nDiff);
        return nSubsidy;
    }
    
    if (nHeight <= 10 && !fTestNet) {
        nSubsidy = 112500 * COIN;
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
    return nSubsidy;
}


/*
double GetAnnualInterest_TestNet(int64 nNetWorkWeit, double rMaxAPR)
{
    double rAPR, rWeit=20000.;
    rAPR = rMaxAPR * ( ( ( 2./( 1.+exp_n(1./(nNetWorkWeit/rWeit+1.)) ) - 0.53788 ) 
           / ( 2./( 1.+exp_n(1./(rWeit+1.)) ) - 0.53788 ) ) + 1 );
    return rAPR;
}

double GetAnnualInterest(int64 nNetWorkWeit, double rMaxAPR)
{
    double rAPR, rWeit=20000.;
//    if (fTestNet) return GetAnnualInterest_TestNet(nNetWorkWeit, rMaxAPR);
    rAPR = ( ( 2./( 1.+exp_n(1./(nNetWorkWeit/rWeit+1.)) ) - 0.53788 ) * rMaxAPR 
           / ( 2./( 1.+exp_n(1./(rWeit+1.)) ) - 0.53788 ) );
    return rAPR;
}

double GetAnnualInterestV2(int64 nNetWorkWeit, double rMaxAPR, CBlockIndex* pindex0)
{
    double rAPR, rWeit=500000.;
//    if (fTestNet) return GetAnnualInterest_TestNet(nNetWorkWeit, rMaxAPR);
    rAPR = ( ( 2./( 1.+exp_n(1./(nNetWorkWeit/rWeit+1.)) ) - 0.53788 ) * rMaxAPR 
           / ( 2./( 1.+exp_n(1./(rWeit+1.)) ) - 0.53788 ) );
    if (pindex0 && IsMaintenance(pindex0)) rAPR *= 1.2;
    if (fDebugMagiPoS) printf("@PoS-APRV2 rAPR = %f\n", rAPR);
    return rAPR;
}

// miner's coin stake reward based on nBits and coin age spent (coin-days)
int64 GetProofOfStakeReward(int64 nCoinAge, int64 nFees, CBlockIndex* pindex)
{
    int64 nNetWorkWeit = GetPoSKernelPS(pindex);
    double rAPR = (IsPoSIIProtocolV2(pindex->nHeight+1)) ? 
		  GetAnnualInterestV2(nNetWorkWeit, MAX_MAGI_PROOF_OF_STAKE, pindex) : 
		  GetAnnualInterest(nNetWorkWeit, MAX_MAGI_PROOF_OF_STAKE);

    int64 nSubsidy = nCoinAge * rAPR * COIN * 33 / (365 * 33 + 8);

	if (fDebug && GetBoolArg("-printcreation"))
        printf("GetProofOfStakeReward(): create=%s nCoinAge=%"PRI64d" nBits=%d\n", FormatMoney(nSubsidy).c_str(), nCoinAge, pindex->nHeight);

	if (fDebug && fDebugMagi) printf("@@GPoSR nHeight = %d, nSubsidy = %"PRI64d", nCoinAge = %"PRI64d", rAPR = %f\n", 
				pindex->nHeight, nSubsidy/COIN, nCoinAge, rAPR);

    return nSubsidy + nFees;
}
*/

} // namespace Magi