// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2021 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_CONSENSUS_AMOUNT_H
#define BITCOIN_CONSENSUS_AMOUNT_H

#include <cstdint>
#include <string>

/** Amount in satoshis (Can be negative) */
typedef int64_t CAmount;

static constexpr CAmount COIN = 100000000;
static constexpr CAmount CENT = 1000000;

static const CAmount MIN_TX_FEE_PREV7 = CENT;
static const CAmount MIN_TX_FEE = .0001 * COIN;
static const CAmount PERKB_TX_FEE = MIN_TX_FEE;
static const CAmount MIN_TXOUT_AMOUNT = MIN_TX_FEE;
static const CAmount MAX_MINT_PROOF_OF_WORK = 112500 * COIN;
static const CAmount MAX_MINT_PROOF_OF_WORK_V10 = 50 * COIN;
static const std::string CURRENCY_UNIT = "PPC";
static const std::string CURRENCY_ATOM = "sat"; // One indivisible minimum value unit


// Magi-specific reward functions
extern int nBestHeight;
static const int MAX_MAGI_POW_HEIGHT = 25000000;
static const int PRM_MAGI_POW_HEIGHT = 80000;
static const int PRM_MAGI_POW_HEIGHT_V2 = 50000; // re-cal PoW-I end block
static const int END_MAGI_POW_HEIGHT = 500000;
static const int END_MAGI_POW_HEIGHT_V2 = 5000000; // PoW-II aims to issue 12 mil and more than 10 years

static const int BLOCK_REWARD_ADJT = 2700;
static const int BLOCK_REWARD_ADJT_M7M_V2 = 32750;

static const unsigned int MAX_BLOCK_SIZE = 1000000;
static const unsigned int MAX_BLOCK_SIZE_GEN = MAX_BLOCK_SIZE/2;
static const unsigned int MAX_BLOCK_SIGOPS = MAX_BLOCK_SIZE/50;
static const unsigned int MAX_ORPHAN_TRANSACTIONS = MAX_BLOCK_SIZE/100;
// net_processing.cpp static const unsigned int MAX_INV_SZ = 50000;
// consensus/amount.h static static const int64_t COINS_BURNED = 720000 * COIN; // Notes: https://bitcointalk.org/index.php?topic=735170.msg9475622#msg9475622
// consensus/amount.h static static const int64_t MIN_TX_FEE = .0001 * COIN;
static const int64_t MIN_RELAY_TX_FEE = MIN_TX_FEE;
// consensus/amount.h static static const int64_t MAX_MONEY = 25000000 * COIN + COINS_BURNED;  // NOte: the amount of COINS_BURNED is unspendable
//static const int64_t MAX_MONEY_POW_PRM = 10000000 * COIN;	// 10 mil; 5.5 mil in 1st magipow
//static const int64_t MAX_MONEY_POW_END = 15000000 * COIN;	// 15 mil; 5 mil in 2nd magipow
static const double MAX_MAGI_PROOF_OF_STAKE = 0.05;		// dynamic annual interest, max 5%
static const double MAX_MAGI_BALANCE_in_STAKE = 0.15;		// balance/money supply, max 15%
static const int64_t MAX_MONEY_STAKE_REF = 5000000 * COIN;	// 5 mil
static const int64_t MAX_MONEY_STAKE_REF_V2 = 500000 * COIN;	// 0.5 mil

// consensus/amount.h static const int64_t MIN_TXOUT_AMOUNT = MIN_TX_FEE;

static const int nCoinbaseMaturity = 100;            // 100 blocks
static const int nCoinbaseMaturityADJ = 500;            // 500 blocks

// consensus/amount.h inline bool MoneyRange(int64_t nValue) { return (nValue >= 0 && nValue <= MAX_MONEY); }
// Threshold for nLockTime: below this value it is interpreted as block number, otherwise as UNIX timestamp.
// script/script.h static const unsigned int LOCKTIME_THRESHOLD = 500000000; // Tue Nov  5 00:53:20 1985 UTC
//-------------------------------------------------------------------------------------------------------------------------------------

// kernel/chainparams.cpp unsigned int nStakeMinAge = 60 * 60 * 2;	// minimum age for coin age: 8hr for block# > 1446800, or 2hr 
// kernel/chainparams.cpp unsigned int nStakeMaxAge = 60 * 60 * 24 * 30;	// stake age of full weight: 30 days
// kernel/chainparams.cpp unsigned int nStakeTargetSpacing = 90;		// 90 sec PoS block spacing

// ?  int64_t nStakeSplitThreshold = 500; // PoS stake splitting threshold
// ?  int64_t nStakeCombineThreshold = nStakeSplitThreshold / 2; // PoS stake combining threshold

static const int64_t nTargetTimespan = 60 * 30;   // 30 min

static const int64_t nTargetTimespanV3Stake = 60 * 30;   // 30 min
static const int64_t nTargetTimespanV3Work = 60 * 16;   // 16 min

static const int64_t nTargetSpacingV3Stake = 90;   // 1.5 min
static const int64_t nTargetSpacingV3Work = 60 * 4;   // 4 min

static const int64_t nTargetSpacingWork = 2 * 90; // 3 min PoW block spacing


//-------------------------------------------------------------------------------------------------------------------------------------

/** No amount larger than this (in satoshi) is valid.
 *
 * Note that this constant is *not* the total money supply, which in Bitcoin
 * currently happens to be less than 21,000,000 BTC for various reasons, but
 * rather a sanity check. As this sanity check is used by consensus-critical
 * validation code, the exact value of the MAX_MONEY constant is consensus
 * critical; in unusual circumstances like a(nother) overflow bug that allowed
 * for the creation of coins out of thin air modification could lead to a fork.
 * */
static constexpr CAmount COINS_BURNED = 720000 * COIN; // Notes: https://bitcointalk.org/index.php?topic=735170.msg9475622#msg9475622
static constexpr CAmount MAX_MONEY = 25000000 * COIN + COINS_BURNED;  // NOte: the amount of COINS_BURNED is unspendable
inline bool MoneyRange(const CAmount& nValue) { return (nValue >= 0 && nValue <= MAX_MONEY); }

#endif // BITCOIN_CONSENSUS_AMOUNT_H
