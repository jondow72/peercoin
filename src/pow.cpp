// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2022 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <pow.h>

#include <arith_uint256.h>
#include <chain.h>
#include <consensus/amount.h>
#include <primitives/block.h>
#include <uint256.h>

#include <bignum.h>
#include <chainparams.h>
#include <kernel.h>
#include <atomic>
#include <util/system.h> // For fTestNet
#include <rpc/blockchain.h>
#include <inttypes.h>


static std::atomic<const CBlockIndex *> cachedAnchor{nullptr};
static int64_t nDAAHalfLife = 24 * 60 * 60;

/**
 * ASERT caches a special block index for efficiency. If block indices are
 * freed then this needs to be called to ensure no dangling pointer when a new
 * block tree is created.
 * (this is temporary and will be removed after the ASERT constants are fixed)
 */

void ResetASERTAnchorBlockCache() noexcept {
    cachedAnchor = nullptr;
}

arith_uint256 CalculateASERT(const arith_uint256 &refTarget,
                             const int64_t nPowTargetSpacing,
                             const int64_t nTimeDiff, const int64_t nHeightDiff,
                             const arith_uint256 &powLimit,
                             const int64_t nHalfLife) noexcept;

uint32_t GetNextASERTWorkRequired(const CBlockIndex *pindexPrev,
                                  const CBlockIndex *pindex,
                                  const Consensus::Params &params) noexcept;

uint32_t
GetNextASERTWorkRequired(const CBlockIndex *pindexPrev,
                         const CBlockIndex *pindex,
                         const Consensus::Params &params,
                         const CBlockIndex *pindexAnchorBlock) noexcept;

/**
 * Returns a pointer to the anchor block used for ASERT.
 * As anchor we use the last POW block for which IsProtocolV14() returns false.
 * This block happens to be the last block which was mined under the old
 * rules.
 *
 * This function is meant to be removed some time after the upgrade, once
 * the anchor block is deeply buried, and behind a hard-coded checkpoint.
 *
 * Preconditions: - pindex must not be nullptr
 *                - pindex must satisfy: IsProtocolV14(pindex) == true
 * Postcondition: Returns a pointer to the last (highest) POW block for which
 *                IsProtocolV14 is false.
 */
static const CBlockIndex *GetASERTAnchorBlock(const CBlockIndex *const pindex,
                                              const Consensus::Params &params) {
    assert(pindex);

    // - We check if we have a cached result, and if we do and it is really the
    //   ancestor of pindex, then we return it.
    //
    // - If we do not or if the cached result is not the ancestor of pindex,
    //   then we proceed with the more expensive walk back to find the ASERT
    //   anchor block.
    //
    // CBlockIndex::GetAncestor() is reasonably efficient; it uses
    // CBlockIndex::pskip Note that if pindex == cachedAnchor, GetAncestor()
    // here will return cachedAnchor, which is what we want.
    const CBlockIndex *lastCached = cachedAnchor.load();
    if (lastCached && pindex->GetAncestor(lastCached->nHeight) == lastCached) {
        return lastCached;
    }

    // Slow path: walk back until we find the first PoW block for which
    // IsProtocolV14 == false.
    const CBlockIndex *anchor = pindex;

    while (anchor->pprev) {
        // first, skip backwards testing IsProtocolV14
        // The below code leverages CBlockIndex::pskip to walk back efficiently.
        if (anchor->pskip && IsProtocolV14(anchor->pskip)) {
            // skip backward
            anchor = anchor->pskip;
            // continue skipping
            continue;
        }
        // cannot skip here, walk back by 1
        if (!IsProtocolV14(anchor->pprev) && anchor->IsProofOfWork()) {
            // found it -- highest block where ASERT is not enabled is
            // anchor->pprev, and anchor points to the last POW block for which
            // IsProtocolV14() == false
            break;
        }
        anchor = anchor->pprev;
    }

    // Overwrite the cache with the anchor we found. More likely than not, the
    // next time we are asked to validate a header it will be part of same /
    // similar chain, not some other unrelated chain with a totally different
    // anchor.
    cachedAnchor = anchor;

    return anchor;
}

uint32_t GetNextASERTWorkRequired(const CBlockIndex *pindexPrev,
                                  const CBlockIndex *pindex,
                                  const Consensus::Params &params) noexcept {
    return GetNextASERTWorkRequired(pindexPrev, pindex, params,
                                    GetASERTAnchorBlock(pindexPrev, params));
}

/**
 * Compute the next required proof of work using an absolutely scheduled
 * exponentially weighted target (ASERT).
 *
 * With ASERT, we define an ideal schedule for block issuance (e.g. 1 block
 * every 600 seconds), and we calculate the difficulty based on how far the most
 * recent block's timestamp is ahead of or behind that schedule. We set our
 * targets (difficulty) exponentially. For every [nHalfLife] seconds ahead of or
 * behind schedule we get, we double or halve the difficulty.
 */
uint32_t
GetNextASERTWorkRequired(const CBlockIndex *pindexPrev,
                         const CBlockIndex *pindex,
                         const Consensus::Params &params,
                         const CBlockIndex *pindexAnchorBlock) noexcept {
    // This cannot handle the genesis block and early blocks in general.
    assert(pindexPrev != nullptr);

    // Anchor block is the block on which all ASERT scheduling calculations are
    // based. It too must exist, and it must have a valid parent.
    assert(pindexAnchorBlock != nullptr);

    // We make no further assumptions other than the height of the prev block
    // must be >= that of the anchor block.
    assert(pindexPrev->nHeight >= pindexAnchorBlock->nHeight);

    const arith_uint256 powLimit = UintToArith256(params.powLimit);

    // For nTimeDiff calculation, the timestamp of the parent to the anchor
    // block is used, as per the absolute formulation of ASERT. This is somewhat
    // counterintuitive since it is referred to as the anchor timestamp, but as
    // per the formula the timestamp of block M-1 must be used if the anchor is
    // M.
    assert(pindexPrev->pprev != nullptr);
    // Note: time difference is to parent of anchor block (or to anchor block
    // itself iff anchor is genesis).
    //       (according to absolute formulation of ASERT)
    const auto anchorTime = pindexAnchorBlock->pprev
                                ? pindexAnchorBlock->pprev->GetBlockTime()
                                : pindexAnchorBlock->GetBlockTime();
    const int64_t nTimeDiff = pindex->GetBlockTime() - anchorTime;
    // Height difference is from current block to anchor block
    const int64_t nHeightDiff =
        pindexPrev->nHeight - pindexAnchorBlock->nHeight -
        (pindexPrev->nHeightStake - pindexAnchorBlock->nHeightStake);
    const arith_uint256 refBlockTarget =
        arith_uint256().SetCompact(pindexAnchorBlock->nBits);
    // Do the actual target adaptation calculation in separate
    // CalculateASERT() function
    arith_uint256 nextTarget =
        CalculateASERT(refBlockTarget, params.nStakeTargetSpacing * 6, nTimeDiff,
                       nHeightDiff, powLimit, nDAAHalfLife);

    // CalculateASERT() already clamps to powLimit.
    return nextTarget.GetCompact();
}

// ASERT calculation function.
// Clamps to powLimit.
arith_uint256 CalculateASERT(const arith_uint256 &refTarget,
                             const int64_t nPowTargetSpacing,
                             const int64_t nTimeDiff, const int64_t nHeightDiff,
                             const arith_uint256 &powLimit,
                             const int64_t nHalfLife) noexcept {
    // Input target must never be zero nor exceed powLimit.
    assert(refTarget > 0 && refTarget <= powLimit);

    // We need some leading zero bits in powLimit in order to have room to
    // handle overflows easily. 28 leading zero bits should be enough.
    assert((powLimit >> 228) == 0);

    // Height diff should NOT be negative.
    assert(nHeightDiff >= 0);

    // It will be helpful when reading what follows, to remember that
    // nextTarget is adapted from anchor block target value.

    // Ultimately, we want to approximate the following ASERT formula, using
    // only integer (fixed-point) math:
    //     new_target = old_target * 2^((blocks_time - IDEAL_BLOCK_TIME *
    //     (height_diff + 1)) / nHalfLife)

    // First, we'll calculate the exponent:
    assert(llabs(nTimeDiff - nPowTargetSpacing * nHeightDiff) <
           (1ll << (63 - 16)));
    const int64_t exponent =
        ((nTimeDiff - nPowTargetSpacing * (nHeightDiff + 1)) * 65536) /
        nHalfLife;

    // Next, we use the 2^x = 2 * 2^(x-1) identity to shift our exponent into
    // the [0, 1) interval. The truncated exponent tells us how many shifts we
    // need to do Note1: This needs to be a right shift. Right shift rounds
    // downward (floored division),
    //        whereas integer division in C++ rounds towards zero (truncated
    //        division).
    // Note2: This algorithm uses arithmetic shifts of negative numbers. This
    //        is unpecified but very common behavior for C++ compilers before
    //        C++20, and standard with C++20. We must check this behavior e.g.
    //        using static_assert.
    static_assert(int64_t(-1) >> 1 == int64_t(-1),
                  "ASERT algorithm needs arithmetic shift support");

    // Now we compute an approximated target * 2^(exponent/65536.0)

    // First decompose exponent into 'integer' and 'fractional' parts:
    int64_t shifts = exponent >> 16;
    const auto frac = uint16_t(exponent);
    assert(exponent == (shifts * 65536) + frac);

    // multiply target by 65536 * 2^(fractional part)
    // 2^x ~= (1 + 0.695502049*x + 0.2262698*x**2 + 0.0782318*x**3) for 0 <= x <
    // 1 Error versus actual 2^x is less than 0.013%.
    const uint32_t factor =
        65536 + ((+195766423245049ull * frac + 971821376ull * frac * frac +
                  5127ull * frac * frac * frac + (1ull << 47)) >>
                 48);
    // this is always < 2^241 since refTarget < 2^224
    arith_uint256 nextTarget = refTarget * factor;

    // multiply by 2^(integer part) / 65536
    shifts -= 16;
    if (shifts <= 0) {
        nextTarget >>= -shifts;
    } else {
        // Detect overflow that would discard high bits
        const auto nextTargetShifted = nextTarget << shifts;
        if ((nextTargetShifted >> shifts) != nextTarget) {
            // If we had wider integers, the final value of nextTarget would
            // be >= 2^256 so it would have just ended up as powLimit anyway.
            nextTarget = powLimit;
        } else {
            // Shifting produced no overflow, can assign value
            nextTarget = nextTargetShifted;
        }
    }

    if (nextTarget == 0) {
        // 0 is not a valid target, but 1 is.
        nextTarget = arith_uint256(1);
    } else if (nextTarget > powLimit) {
        nextTarget = powLimit;
    }

    // we return from only 1 place for copy elision
    return nextTarget;
}

/*
unsigned int GetNextTargetRequired(const CBlockIndex* pindexLast, bool fProofOfStake, const Consensus::Params& params)
{
    if (pindexLast == nullptr || params.fPowNoRetargeting)
        return UintToArith256(params.powLimit).GetCompact(); // genesis block

    const CBlockIndex* pindexPrev = GetLastBlockIndex(pindexLast, fProofOfStake);
    if (pindexPrev->pprev == nullptr)
        return UintToArith256(params.bnInitialHashTarget).GetCompact(); // first block
    const CBlockIndex* pindexPrevPrev = GetLastBlockIndex(pindexPrev->pprev, fProofOfStake);
    if (pindexPrevPrev->pprev == nullptr)
        return UintToArith256(params.bnInitialHashTarget).GetCompact(); // second block

    if (!fProofOfStake && IsProtocolV14(pindexPrev))
        return GetNextASERTWorkRequired(pindexPrev, pindexLast, params);

    int64_t nActualSpacing = pindexPrev->GetBlockTime() - pindexPrevPrev->GetBlockTime();

    // rfc20
    int64_t nHypotheticalSpacing = pindexLast->GetBlockTime() - pindexPrev->GetBlockTime();
    if (!fProofOfStake && IsProtocolV12(pindexPrev) && (nHypotheticalSpacing > nActualSpacing))
        nActualSpacing = nHypotheticalSpacing;

    // peercoin: target change every block
    // peercoin: retarget with exponential moving toward target spacing
    CBigNum bnNew;
    bnNew.SetCompact(pindexPrev->nBits);
    if (Params().NetworkIDString() != CBaseChainParams::REGTEST) {
        int64_t nTargetSpacing;

        if (fProofOfStake) {
            nTargetSpacing = params.nStakeTargetSpacing;
        } else {
            if (IsProtocolV09(pindexLast->nTime)) {
                nTargetSpacing = params.nStakeTargetSpacing * 6;
            } else {
                nTargetSpacing = std::min(params.nTargetSpacingWorkMax, params.nStakeTargetSpacing * (1 + pindexLast->nHeight - pindexPrev->nHeight));
            }
        }

        int64_t nInterval = params.nTargetTimespan / nTargetSpacing;
        bnNew *= ((nInterval - 1) * nTargetSpacing + nActualSpacing + nActualSpacing);
        bnNew /= ((nInterval + 1) * nTargetSpacing);
        }

    if (bnNew > CBigNum(params.powLimit))
        bnNew = CBigNum(params.powLimit);

    return bnNew.GetCompact();
}
*/

// Replace:
// static CBigNum bnProofOfWorkLimit(~uint256(0) >> 20);
// static CBigNum bnProofOfStakeLimit(~uint256(0) >> 20);

// static CBigNum bnProofOfWorkLimitTestNet(~uint256(0) >> 20);
// static CBigNum bnProofOfStakeLimitTestNet(~uint256(0) >> 20);

static CBigNum bnProofOfWorkLimit(ArithToUint256(~UintToArith256(uint256()) >> 20));
static CBigNum bnProofOfStakeLimit(ArithToUint256(~UintToArith256(uint256()) >> 20));

static CBigNum bnProofOfWorkLimitTestNet(ArithToUint256(~UintToArith256(uint256()) >> 20));
static CBigNum bnProofOfStakeLimitTestNet(ArithToUint256(~UintToArith256(uint256()) >> 20));

// Debug flag for Magi
static bool fDebug = false;
static bool fDebugMagiPoS = false; // Set via -debug=MagiPoS

#define HEIGHT_LOOKUP_DEPTH 10
unsigned int GetNextTargetRequired_v1(const CBlockIndex* pindexLast, bool fProofOfStake)
{
    CBigNum bnTargetLimit = bnProofOfWorkLimit;

    if(fProofOfStake)
    {
        // Proof-of-Stake blocks has own target limit since nVersion=3 supermajority on mainNet and always on testNet
        bnTargetLimit = bnProofOfStakeLimit;
    }

    if (pindexLast == NULL)
        return bnTargetLimit.GetCompact(); // genesis block

    const CBlockIndex* pindexPrev = GetLastBlockIndex(pindexLast, fProofOfStake);
    if (pindexPrev->pprev == NULL)
        return bnTargetLimit.GetCompact(); // first block
    const CBlockIndex* pindexPrevPrev = GetLastBlockIndex(pindexPrev->pprev, fProofOfStake);
    if (pindexPrevPrev->pprev == NULL)
        return bnTargetLimit.GetCompact(); // second block

    int64_t nTargetSpacing = fProofOfStake? GetStakeTargetSpacing(pindexLast->nHeight+1): GetTargetSpacingWork(pindexLast->nHeight+1);
    int64_t nActualSpacing = pindexPrev->GetBlockTime() - pindexPrevPrev->GetBlockTime();
	if (nActualSpacing < 0)
    {
        if (IsProtocolV3(pindexLast->nHeight+1))
        {
            int nBlks = 1;
            do {
                pindexPrevPrev = GetLastBlockIndex(pindexPrevPrev->pprev, fProofOfStake);
                if (pindexPrevPrev->pprev == NULL) break;
                nActualSpacing = pindexPrev->GetBlockTime() - pindexPrevPrev->GetBlockTime();
                ++nBlks;
            } while ( (nActualSpacing < 0) && (nBlks <= HEIGHT_LOOKUP_DEPTH) );
            {
                if (nActualSpacing < 0) 
                    nActualSpacing = 1;
                else
                    nActualSpacing = nActualSpacing / nBlks;
            }
        } else
            nActualSpacing = 1;
    } else if (nActualSpacing > nTargetTimespan)
		nActualSpacing = nTargetTimespan;

    // ppcoin: target change every block
    // ppcoin: retarget with exponential moving toward target spacing
    CBigNum bnNew;
    bnNew.SetCompact(pindexPrev->nBits);
    int64_t nInterval = nTargetTimespan / nTargetSpacing;
    bnNew *= ((nInterval - 1) * nTargetSpacing + nActualSpacing + nActualSpacing);
    bnNew /= ((nInterval + 1) * nTargetSpacing);

	/*
	printf(">> Height = %d, fProofOfStake = %d, nInterval = %"PRId64", nTargetSpacing = %"PRId64", nActualSpacing = %"PRId64"\n",
		pindexPrev->nHeight, fProofOfStake, nInterval, nTargetSpacing, nActualSpacing);
	printf(">> pindexPrev->GetBlockTime() = %"PRId64", pindexPrev->nHeight = %d, pindexPrevPrev->GetBlockTime() = %"PRId64", pindexPrevPrev->nHeight = %d\n",
		pindexPrev->GetBlockTime(), pindexPrev->nHeight, pindexPrevPrev->GetBlockTime(), pindexPrevPrev->nHeight);
	*/
    if ( IsProtocolV3(pindexLast->nHeight+1) && (bnNew <= 0 || bnNew > bnTargetLimit) )
        bnNew = bnTargetLimit;
    else if (bnNew > bnTargetLimit)
        bnNew = bnTargetLimit;

    /// debug print
    if (fDebugMagiPoS)
    {
        printf("GetNextTargetRequired RETARGET\n");
        printf("nTargetSpacing = %" PRId64"    nActualSpacing = %" PRId64"    nInterval = %" PRId64"\n", nTargetSpacing, nActualSpacing, nInterval);
        printf("Before: %08x  %s\n", pindexPrev->nBits, CBigNum().SetCompact(pindexPrev->nBits).getuint256().ToString().c_str());
        printf("After:  %08x  %s\n", bnNew.GetCompact(), bnNew.getuint256().ToString().c_str());
    }

    return bnNew.GetCompact();
}

#define HEIGHT_DIFF_ADJ_TARGET_SPACKING_WORK_V3_INIT 1482000
int64_t GetTargetSpacingWork(int nHeight)
{
    return ( (nHeight >= HEIGHT_DIFF_ADJ_TARGET_SPACKING_WORK_V3_INIT) ? 
        nTargetSpacingV3Work : nTargetSpacingWork );
}

int64_t GetTargetTimespanV3(bool fProofOfStake)
{
    return ( fProofOfStake? nTargetTimespanV3Stake : nTargetTimespanV3Work );
}

int64_t GetTargetSpacingV3(bool fProofOfStake)
{
    return ( fProofOfStake? nTargetSpacingV3Stake : nTargetSpacingV3Work );
}

unsigned int GetNextTargetRequired_v3(const CBlockIndex* pindexLast, bool fProofOfStake)
{
    CBigNum bnTargetLimit = bnProofOfWorkLimit;

    int64_t nTargetTimespan0 = GetTargetTimespanV3(fProofOfStake);
    int64_t nTargetSpacing0 = GetTargetSpacingV3(fProofOfStake);

    if(fProofOfStake)
    {
        // Proof-of-Stake blocks has own target limit since nVersion=3 supermajority on mainNet and always on testNet
        bnTargetLimit = bnProofOfStakeLimit;
    }

    if (pindexLast == NULL)
        return bnTargetLimit.GetCompact(); // genesis block

    const CBlockIndex* pindexPrev = GetLastBlockIndex(pindexLast, fProofOfStake);
    if (pindexPrev->pprev == NULL)
        return bnTargetLimit.GetCompact(); // first block
    const CBlockIndex* pindexPrevPrev = GetLastBlockIndex(pindexPrev->pprev, fProofOfStake);
    if (pindexPrevPrev->pprev == NULL)
        return bnTargetLimit.GetCompact(); // second block

    int64_t nActualSpacing = pindexPrev->GetBlockTime() - pindexPrevPrev->GetBlockTime();
    if(nActualSpacing < 0)
    {
        // printf(">> nActualSpacing = %"PRId64" corrected to 1.\n", nActualSpacing);
        nActualSpacing = 1;
    }
    else if(nActualSpacing > nTargetTimespan0)
    {
        // printf(">> nActualSpacing = %"PRId64" corrected to nTargetTimespan0 (900).\n", nActualSpacing);
        nActualSpacing = nTargetTimespan0;
    }

    // no adjustment
    if (IsBlockInvalid(pindexPrev->nHeight, pindexPrev->GetBlockTime(), fProofOfStake, pindexPrev->pprev))
        nActualSpacing = nTargetSpacing0;

    // ppcoin: target change every block
    // ppcoin: retarget with exponential moving toward target spacing
    CBigNum bnNew;
    bnNew.SetCompact(pindexPrev->nBits);

    int64_t nInterval = nTargetTimespan0 / nTargetSpacing0;
    bnNew *= ((nInterval - 1) * nTargetSpacing0 + nActualSpacing + nActualSpacing);
    bnNew /= ((nInterval + 1) * nTargetSpacing0);

    /*
    printf(">> Height = %d, fProofOfStake = %d, nInterval = %"PRId64", nTargetSpacing0 = %"PRId64", nActualSpacing = %"PRId64"\n",
        pindexPrev->nHeight, fProofOfStake, nInterval, nTargetSpacing0, nActualSpacing);
    printf(">> pindexPrev->GetBlockTime() = %"PRId64", pindexPrev->nHeight = %d, pindexPrevPrev->GetBlockTime() = %"PRId64", pindexPrevPrev->nHeight = %d\n",
        pindexPrev->GetBlockTime(), pindexPrev->nHeight, pindexPrevPrev->GetBlockTime(), pindexPrevPrev->nHeight);
    */

    if (bnNew <= 0 || bnNew > bnTargetLimit)
        bnNew = bnTargetLimit;

    return bnNew.GetCompact();
}

#define MQW_TIME_COEFF_TESNT 1.0
#define MQW_AVER_COEFF_TESNT 1.0
#define MQW_EXPON_COEFF_TESNT 2.3
#define WEIGHT_SCALE_TESNT 100.0
unsigned int MagiQuantumWave_TESNT(const CBlockIndex* pindexLast, bool fProofOfStake)
{
    /* Magi Quantum Wave (MQW) for XMG - Coin Magi, written by Joe Lao */
    if (fProofOfStake) return GetNextTargetRequired_v1(pindexLast, fProofOfStake);

    int64_t nActualBlockSpacing, nActualTimeSpanMQW;
    int64_t nAveragedBlocks = 1, nTotPastBlocks = 15;
    CBigNum bnAverage;
    CBigNum bnAveragePrev;

    CBigNum bnTargetLimit = bnProofOfWorkLimit;
    if (fProofOfStake)
    {
        // Proof-of-Stake blocks has own target limit since nVersion=3 supermajority on mainNet and always on testNet
        bnTargetLimit = bnProofOfStakeLimit;
    }

    if (pindexLast == NULL)
        return bnTargetLimit.GetCompact(); // genesis block

    const CBlockIndex* pindexPrev = GetLastBlockIndex(pindexLast, fProofOfStake);
    if (pindexPrev->pprev == NULL)
        return bnTargetLimit.GetCompact(); // first block

    const CBlockIndex* pindexPrevPrev = GetLastBlockIndex(pindexPrev->pprev, fProofOfStake);
    if (pindexPrevPrev->pprev == NULL)
        return bnTargetLimit.GetCompact(); // second block

    nActualBlockSpacing = pindexPrev->GetBlockTime() - pindexPrevPrev->GetBlockTime();
    if(nActualBlockSpacing < 0) { nActualBlockSpacing = 1; }
    nActualTimeSpanMQW = nActualBlockSpacing;
    double fw = exp_n(-double(nActualBlockSpacing)*MQW_EXPON_COEFF_TESNT*MQW_TIME_COEFF_TESNT/double(GetTargetSpacingWork(pindexLast->nHeight+1))) * MQW_AVER_COEFF_TESNT;
    bnAverage.SetCompact(pindexPrev->nBits);
    bnAverage = bnAverage * ((int64_t)(fw*WEIGHT_SCALE_TESNT));
    
    int64_t nWeightTot = ((int64_t)(fw*WEIGHT_SCALE_TESNT));
    double rWeight = 1.-fw;

    for(unsigned int i = 1; pindexPrevPrev; i++)
    {
        if (i >= nTotPastBlocks) { break; }
	pindexPrev = pindexPrevPrev;
	pindexPrevPrev = GetLastBlockIndex(pindexPrev->pprev, fProofOfStake);
        if (pindexPrevPrev == NULL) { assert(pindexPrev); break; }
	nActualBlockSpacing = pindexPrev->GetBlockTime() - pindexPrevPrev->GetBlockTime();
	if (nActualBlockSpacing > 0)
	{
	    nAveragedBlocks++;
	    nActualTimeSpanMQW += nActualBlockSpacing;
	    fw = exp_n(-double(nActualBlockSpacing)*MQW_EXPON_COEFF_TESNT*MQW_TIME_COEFF_TESNT/double(GetTargetSpacingWork(pindexLast->nHeight+1))) * MQW_AVER_COEFF_TESNT;
	    bnAverage += (CBigNum().SetCompact(pindexPrev->nBits)) * ((int64_t)(fw*rWeight*WEIGHT_SCALE_TESNT));
	    nWeightTot += ((int64_t)(fw*rWeight*WEIGHT_SCALE_TESNT));
	    rWeight *= (1.-fw);
	}
    }
    bnAverage /= nWeightTot;

    CBigNum bnNew(bnAverage);

    int64_t nTargetTimeSpanMQW = nAveragedBlocks*GetTargetSpacingWork(pindexLast->nHeight+1);

    if (nActualTimeSpanMQW < nTargetTimeSpanMQW/3)
        nActualTimeSpanMQW = nTargetTimeSpanMQW/3;
    if (nActualTimeSpanMQW > nTargetTimeSpanMQW*3)
        nActualTimeSpanMQW = nTargetTimeSpanMQW*3;

    // Retarget
    bnNew *= nActualTimeSpanMQW;
    bnNew /= nTargetTimeSpanMQW;

    if (bnNew > bnProofOfWorkLimit){
        bnNew = bnProofOfWorkLimit;
    }
     
    return bnNew.GetCompact();
}

#define MQW_TIME_COEFF 1.0
#define MQW_AVER_COEFF 1.0
#define MQW_EXPON_COEFF 0.15
#define WEIGHT_SCALE 100.0
#define WEIGHT_MIN 0.005
#define WEIGHT_MAX 0.8
unsigned int MagiQuantumWave(const CBlockIndex* pindexLast, bool fProofOfStake)
{
    /* Magi Quantum Wave (MQW) for XMG - Coin Magi, written by Joe Lao */
    if (fProofOfStake) return GetNextTargetRequired_v1(pindexLast, fProofOfStake);

    int64_t nActualBlockSpacing, nActualTimeSpanMQW;
    int64_t nAveragedBlocks = 1, nTotPastBlocks = 15;
    CBigNum bnAverage;
    CBigNum bnAveragePrev;

    CBigNum bnTargetLimit = bnProofOfWorkLimit;
    if (fProofOfStake)
    {
        // Proof-of-Stake blocks has own target limit since nVersion=3 supermajority on mainNet and always on testNet
        bnTargetLimit = bnProofOfStakeLimit;
    }
    if (pindexLast == NULL) {
        return bnTargetLimit.GetCompact(); // genesis block
    }

    const CBlockIndex* pindexPrev = GetLastBlockIndex(pindexLast, fProofOfStake);

    if (pindexPrev->pprev == NULL) {
        return bnTargetLimit.GetCompact(); // first block
    }

    const CBlockIndex* pindexPrevPrev = GetLastBlockIndex(pindexPrev->pprev, fProofOfStake);

    if (pindexPrevPrev->pprev == NULL) {
        return bnTargetLimit.GetCompact(); // second block
    }

    nActualBlockSpacing = pindexPrev->GetBlockTime() - pindexPrevPrev->GetBlockTime();

    if(nActualBlockSpacing < 0) {
        nActualBlockSpacing = 1;
    }

    nActualTimeSpanMQW = nActualBlockSpacing;
    double fw = ( 1. - exp_n(-double(nActualBlockSpacing) * MQW_EXPON_COEFF*MQW_TIME_COEFF / double(GetTargetSpacingWork(pindexLast->nHeight+1))) ) * MQW_AVER_COEFF;
    if (fw < WEIGHT_MIN) {
        fw = WEIGHT_MIN;
    } else if (fw > WEIGHT_MAX) {
        fw = WEIGHT_MAX;
    }

    bnAverage.SetCompact(pindexPrev->nBits);
    bnAverage *= ((int64_t)(fw * WEIGHT_SCALE));

    int64_t nWeightTot = ((int64_t)(fw * WEIGHT_SCALE));
    double rWeight = 1.-fw;

    for(unsigned int i = 1; pindexPrevPrev; i++)
    {
        if (i >= nTotPastBlocks) {
            break;
        }

        pindexPrev = pindexPrevPrev;
        pindexPrevPrev = GetLastBlockIndex(pindexPrev->pprev, fProofOfStake);

        if (pindexPrevPrev == NULL) { assert(pindexPrev); break; }
        nActualBlockSpacing = pindexPrev->GetBlockTime() - pindexPrevPrev->GetBlockTime();

        if (nActualBlockSpacing > 0)
        {
            nAveragedBlocks++;
            nActualTimeSpanMQW += nActualBlockSpacing;
            fw = ( 1. - exp_n(-double(nActualBlockSpacing) * MQW_EXPON_COEFF*MQW_TIME_COEFF / double(GetTargetSpacingWork(pindexLast->nHeight+1))) ) * MQW_AVER_COEFF;

            if (fw < WEIGHT_MIN) {
                fw = WEIGHT_MIN;
            } else if (fw > WEIGHT_MAX) {
                fw = WEIGHT_MAX;
            }

            bnAverage += (CBigNum().SetCompact(pindexPrev->nBits)) * ((int64_t)(fw*rWeight*WEIGHT_SCALE));
            nWeightTot += ((int64_t)(fw * rWeight * WEIGHT_SCALE));

            rWeight *= (1.-fw);
        }
    }

    bnAverage /= nWeightTot;

    CBigNum bnNew(bnAverage);

    int64_t nTargetTimeSpanMQW = nAveragedBlocks * GetTargetSpacingWork(pindexLast->nHeight+1);

    if (nActualTimeSpanMQW < nTargetTimeSpanMQW / 3) {
        nActualTimeSpanMQW = nTargetTimeSpanMQW / 3;
    }

    if (nActualTimeSpanMQW > nTargetTimeSpanMQW * 3){
        nActualTimeSpanMQW = nTargetTimeSpanMQW * 3;
    }

    // Retarget
    bnNew *= nActualTimeSpanMQW;
    bnNew /= nTargetTimeSpanMQW;

    if (bnNew > bnProofOfWorkLimit){
        bnNew = bnProofOfWorkLimit;
    }

    return bnNew.GetCompact();
}

#define MQW_DUMMY_NUMBER 100
unsigned int MagiQuantumWave_v2(const CBlockIndex* pindexLast, bool fProofOfStake)
{
    /* Magi Quantum Wave (MQW) for XMG - Coin Magi, written by Joe Lao */
    if (fProofOfStake) return GetNextTargetRequired_v1(pindexLast, fProofOfStake);

    int64_t nActualBlockSpacing, nActualTimeSpanMQW;
    int64_t nAveragedBlocks = 1, nTotPastBlocks = 13;
    CBigNum bnAverage;
    CBigNum bnAveragePrev;

    CBigNum bnTargetLimit = bnProofOfWorkLimit;
    if (fProofOfStake)
    {
        // Proof-of-Stake blocks has own target limit since nVersion=3 supermajority on mainNet and always on testNet
        bnTargetLimit = bnProofOfStakeLimit;
    }
    if (pindexLast == NULL) {
        return bnTargetLimit.GetCompact(); // genesis block
    }

    const CBlockIndex* pindexPrev = GetLastBlockIndex(pindexLast, fProofOfStake);

    if (pindexPrev->pprev == NULL) {
        return bnTargetLimit.GetCompact(); // first block
    }

    const CBlockIndex* pindexPrevPrev = GetLastBlockIndex(pindexPrev->pprev, fProofOfStake);

    if (pindexPrevPrev->pprev == NULL) {
        return bnTargetLimit.GetCompact(); // second block
    }

    nActualBlockSpacing = pindexPrev->GetBlockTime() - pindexPrevPrev->GetBlockTime();

    if(nActualBlockSpacing < 0) {
        nActualBlockSpacing = 1;
    }

    nActualTimeSpanMQW = nActualBlockSpacing;
    double fw = ( 1. - exp_n(-double(nActualBlockSpacing) * MQW_EXPON_COEFF*MQW_TIME_COEFF / double(GetTargetSpacingWork(pindexLast->nHeight+1))) ) * MQW_AVER_COEFF;
    if (fw < WEIGHT_MIN) {
        fw = WEIGHT_MIN;
    } else if (fw > WEIGHT_MAX) {
        fw = WEIGHT_MAX;
    }

    bnAverage.SetCompact(pindexPrev->nBits);
    bnAverage *= ((int64_t)(fw * WEIGHT_SCALE * MQW_DUMMY_NUMBER));

    double rWeightTot = fw * WEIGHT_SCALE * MQW_DUMMY_NUMBER;
    double rWeight = 1.-fw;

    for(unsigned int i = 1; pindexPrevPrev; i++)
    {
        if (i >= nTotPastBlocks) {
            break;
        }

        pindexPrev = pindexPrevPrev;
        pindexPrevPrev = GetLastBlockIndex(pindexPrev->pprev, fProofOfStake);

        if (pindexPrevPrev == NULL) { assert(pindexPrev); break; }
        nActualBlockSpacing = pindexPrev->GetBlockTime() - pindexPrevPrev->GetBlockTime();

        if (nActualBlockSpacing > 0)
        {
            nAveragedBlocks++;
            nActualTimeSpanMQW += nActualBlockSpacing;
            fw = ( 1. - exp_n(-double(nActualBlockSpacing) * MQW_EXPON_COEFF*MQW_TIME_COEFF / double(GetTargetSpacingWork(pindexLast->nHeight+1))) ) * MQW_AVER_COEFF;

            if (fw < WEIGHT_MIN) {
                fw = WEIGHT_MIN;
            } else if (fw > WEIGHT_MAX) {
                fw = WEIGHT_MAX;
            }

            bnAverage += (CBigNum().SetCompact(pindexPrev->nBits)) * ((int64_t)(fw * rWeight * WEIGHT_SCALE * MQW_DUMMY_NUMBER));
            rWeightTot += (fw * rWeight * WEIGHT_SCALE * MQW_DUMMY_NUMBER);
            rWeight *= (1.-fw);
        }
    }

    int64_t nWeightTot = (int64_t)rWeightTot;

    if (nWeightTot < 1) {
        nWeightTot = 1;
    }
    if (fDebug) printf("nWeightTot: %d\n", nWeightTot);

    bnAverage /= nWeightTot;

    CBigNum bnNew(bnAverage);

    int64_t nTargetTimeSpanMQW = nAveragedBlocks * GetTargetSpacingWork(pindexLast->nHeight+1);

    if (nActualTimeSpanMQW < nTargetTimeSpanMQW / 3) {
        nActualTimeSpanMQW = nTargetTimeSpanMQW / 3;
    }

    if (nActualTimeSpanMQW > nTargetTimeSpanMQW * 3){
        nActualTimeSpanMQW = nTargetTimeSpanMQW * 3;
    }

    // Retarget
    bnNew *= nActualTimeSpanMQW;
    bnNew /= nTargetTimeSpanMQW;

    if (bnNew > bnProofOfWorkLimit){
        bnNew = bnProofOfWorkLimit;
    }

    return bnNew.GetCompact();
}

unsigned int GetNextTargetRequired(const CBlockIndex* pindexLast, bool fProofOfStake, const Consensus::Params& params)
{
    if (fDebug) printf("nHeight: %d\n", pindexLast->nHeight);
    int DiffMode = 1;
    if (fTestNet) DiffMode = 1;
    else if (pindexLast->nHeight+1 >= 33500 && pindexLast->nHeight+1 < HEIGHT_DIFF_ADJ_TARGET_SPACKING_WORK_V3_INIT) DiffMode = 2;
    else if (pindexLast->nHeight+1 >= HEIGHT_DIFF_ADJ_TARGET_SPACKING_WORK_V3_INIT && pindexLast->nHeight+1 < HEIGHT_CHAIN_SWITCH-2) DiffMode = 3;
    else if (pindexLast->nHeight+1 >= HEIGHT_CHAIN_SWITCH-2 && pindexLast->nHeight+1 < 1606988) DiffMode = 2;
    else if (pindexLast->nHeight+1 >= 1606988) DiffMode = 4;
    
    if (DiffMode == 1) return GetNextTargetRequired_v1(pindexLast, fProofOfStake);
    else if (DiffMode == 2) return MagiQuantumWave(pindexLast, fProofOfStake);
    else if (DiffMode == 3) return GetNextTargetRequired_v3(pindexLast, fProofOfStake);
    else if (DiffMode == 4) return MagiQuantumWave_v2(pindexLast, fProofOfStake);
    return GetNextTargetRequired_v1(pindexLast, fProofOfStake);
}

bool CheckProofOfWork(uint256 hash, unsigned int nBits, const Consensus::Params& params)
{
    bool fNegative;
    bool fOverflow;
    arith_uint256 bnTarget;

    bnTarget.SetCompact(nBits, &fNegative, &fOverflow);

    // Check range
    if (fNegative || bnTarget == 0 || fOverflow || bnTarget > UintToArith256(params.powLimit))
        return false;

    // Check proof of work matches claimed amount
    if (UintToArith256(hash) > bnTarget)
        return false;

    return true;
}
