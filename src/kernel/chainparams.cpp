// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2021 The Bitcoin Core developers
// Copyright (c) 2011-2025 The Peercoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <kernel/chainparams.h>

#include <chainparamsseeds.h>
#include <consensus/amount.h>
#include <consensus/merkle.h>
#include <consensus/params.h>
#include <hash.h>
#include <chainparamsbase.h>
#include <logging.h>
#include <primitives/block.h>
#include <primitives/transaction.h>
#include <script/interpreter.h>
#include <script/script.h>
#include <uint256.h>
#include <util/strencodings.h>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <type_traits>

static CBlock CreateGenesisBlock(const char* pszTimestamp, const CScript& genesisOutputScript, uint32_t nTimeTx, uint32_t nTimeBlock, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    CMutableTransaction txNew;
    txNew.nVersion = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    txNew.vin[0].scriptSig = CScript() << 486604799 << CScriptNum(9999) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
    txNew.vout[0].nValue = genesisReward;
    txNew.vout[0].scriptPubKey = genesisOutputScript;
    txNew.nTime = nTimeTx;

    CBlock genesis;
    genesis.nTime    = nTimeBlock;
    genesis.nBits    = nBits;
    genesis.nNonce   = nNonce;
    genesis.nVersion = nVersion;
    genesis.vtx.push_back(MakeTransactionRef(std::move(txNew)));
    genesis.hashPrevBlock.SetNull();
    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
    return genesis;
}

/**
 * Build the genesis block. Note that the output of its generation
 * transaction cannot be spent since it did not originally exist in the
 * database.
 *
 * CBlock(hash=000000000019d6, ver=1, hashPrevBlock=00000000000000, hashMerkleRoot=4a5e1e, nTime=1231006505, nBits=1d00ffff, nNonce=2083236893, vtx=1)
 *   CTransaction(hash=4a5e1e, ver=1, vin.size=1, vout.size=1, nLockTime=0)
 *     CTxIn(COutPoint(000000, -1), coinbase 04ffff001d0104455468652054696d65732030332f4a616e2f32303039204368616e63656c6c6f72206f6e206272696e6b206f66207365636f6e64206261696c6f757420666f722062616e6b73)
 *     CTxOut(nValue=50.00000000, scriptPubKey=0x5F1DF16B2B704C8A578D0B)
 *   vMerkleTree: 4a5e1e
 */
static CBlock CreateGenesisBlock(uint32_t nTimeTx, uint32_t nTimeBlock, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    const char* pszTimestamp = "Super fracking, Physics Today 67(8), 34 (2014); doi: 10.1063/PT.3.2480";
    const CScript genesisOutputScript = CScript();
    return CreateGenesisBlock(pszTimestamp, genesisOutputScript, nTimeTx, nTimeBlock, nNonce, nBits, nVersion, genesisReward);
}

/**
 * Main network on which people trade goods and services.
 */
class CMainParams : public CChainParams {
public:
    CMainParams() {
        strNetworkID = CBaseChainParams::MAIN;
        consensus.signet_blocks = false;
        consensus.signet_challenge.clear();
/*
        consensus.nSubsidyHalvingInterval = 210000;
        consensus.script_flag_exceptions.emplace( // BIP16 exception
            uint256S("0x00000000000002dc756eebf4f49723ed8d30cc28a5f108eb94b1ba88ac4f9c22"), SCRIPT_VERIFY_NONE);
        consensus.script_flag_exceptions.emplace( // Taproot exception
            uint256S("0x0000000000000000000f14c35b2d841e986ab5441de8c585d5ffe55ea1e395ad"), SCRIPT_VERIFY_P2SH | SCRIPT_VERIFY_WITNESS);
        consensus.BIP34Height = 227931;
        consensus.BIP34Hash = uint256S("0x000000000000024b89b42a942fe0d9fea3bb44ab7bd1b19115dd6a759c0808b8");
        consensus.BIP65Height = 388381; // 000000000000000004c2b624ed5d7756c508d90fd0da2c7c679febfa6c4735f0
        consensus.BIP66Height = 363725; // 00000000000000000379eaa19dce8c9b722d46ae6a57c2f1a988119488b50931
        consensus.CSVHeight = 419328; // 000000000000000004a1b34462cb8aeebd5799177f7a29cf28f2d1961716b5b5
        consensus.SegwitHeight = 481824; // 0000000000000000001c8018d9cb3b742ef25114f27563e3fc4a1902167f9893
        consensus.MinBIP9WarningHeight = 483840; // segwit activation height + miner confirmation window
        consensus.powLimit = uint256S("00000000ffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nTargetTimespan = 7 * 24 * 60 * 60; // two weeks
        consensus.nPowTargetSpacing = 10 * 60;
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 1815; // 90% of 2016
        consensus.nMinerConfirmationWindow = 2016; // nPowTargetTimespan / nPowTargetSpacing
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = Consensus::BIP9Deployment::NEVER_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].min_activation_height = 0; // No activation delay

        // Deployment of Taproot (BIPs 340-342)
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].bit = 2;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nStartTime = 1619222400; // April 24th, 2021
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nTimeout = 1628640000; // August 11th, 2021
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].min_activation_height = 709632; // Approximately November 12th, 2021

        consensus.nMinimumChainWork = uint256S("0x000000000000000000000000000000000000000044a50fe819c39ad624021859");
        consensus.defaultAssumeValid = uint256S("0x000000000000000000035c3f0d31e71a5ee24c5aaf3354689f65bd7b07dee632"); // 784000
*/
        consensus.BIP34Height = 339994;
        consensus.BIP34Hash = uint256S("000000000000000237f50af4cfe8924e8693abc5bd8ae5abb95bc6d230f5953f");
        consensus.powLimit =            uint256S("00000fffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"); // ~arith_uint256(0) >> 20;
        consensus.bnInitialHashTarget = uint256S("00000fffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"); //  ~arith_uint256(0) >> 20;

        consensus.nTargetTimespan = 60 * 30;   // 30 min
        consensus.nStakeTargetSpacing = 90;		// 90 sec PoS block spacing
        consensus.nTargetSpacingWorkMax = 12 * consensus.nStakeTargetSpacing; // 2-hour
        consensus.nPowTargetSpacing = consensus.nStakeTargetSpacing;
        consensus.nStakeMinAge = 60 * 60 * 2;	// minimum age for coin age: 8hr for block# > 1446800, or 2hr
        consensus.nStakeMaxAge = 60 * 60 * 24 * 30;	// stake age of full weight: 30 days
        consensus.nModifierInterval = 10 * 60; // 10 min
        consensus.nCoinbaseMaturity = 100;

        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 1815; // 90% of 2016
        consensus.nMinerConfirmationWindow = 2016; // nPowTargetTimespan / nPowTargetSpacing

        consensus.SegwitHeight = 455470;

        consensus.nMinimumChainWork = uint256S("0x0"); // 750000
        consensus.defaultAssumeValid = uint256S("0x0");  // 750000

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 32-bit integer with any alignment.
         */
        pchMessageStart[0] = 0xf0;
        pchMessageStart[1] = 0xb9;
        pchMessageStart[2] = 0xb3;
        pchMessageStart[3] = 0xd6;
        nDefaultPort = 8233;
        m_assumed_blockchain_size = 2;

        genesis = CreateGenesisBlock(1407209706, 1410566399, 1780637, 0x1e0fffff, 1, 0);
        consensus.hashGenesisBlock = genesis.GetHash();

        printf("genesis.GetHash = %s\n", genesis.GetHash().ToString().c_str());

        assert(consensus.hashGenesisBlock == uint256S("0x000004c91ca895a8c63176b1671eff34291ad671e59ae46630ffd8f985dd56cc"));
        assert(genesis.hashMerkleRoot == uint256S("0x70070d9e41ffd85685f8017fa8620fb5572ed8443822d799015d01d39e7fd4af"));

        // Note that of those which support the service bits prefix, most only support a subset of
        // possible options.
        // This is fine at runtime as we'll fall back to using them as an addrfetch if they don't support the
        // service bits we want, but we should get them updated to support all service bits wanted by any
        // release ASAP to avoid it where possible.
        vSeeds.emplace_back("magi-seed.checkbug.com");

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,55);  // peercoin: addresses begin with 'P'
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,117); // peercoin: addresses begin with 'p'
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,183);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x88, 0xB2, 0x1E};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x88, 0xAD, 0xE4};

        // human readable prefix to bench32 address
        bech32_hrp = "pc";

        vFixedSeeds = std::vector<uint8_t>(std::begin(chainparams_seed_main), std::end(chainparams_seed_main));

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        m_is_test_chain = false;
        m_is_mockable_chain = false;

        checkpointData = {
            {
                {       0, uint256S("0x000004c91ca895a8c63176b1671eff34291ad671e59ae46630ffd8f985dd56cc")},
                {    5000, uint256S("0x000000003bf169c8256a1cecca19a2a25c30b176eac2fdb137263f4a9dbbdf62")},
                {   10000, uint256S("0x0000000014919d2e69816bdfc8243b623444f8fdbed61c9299e8b894527f0358")},
                {   15000, uint256S("0x6c01e269f56bcb553b75928b6ed9a180b3e1244661e2aea6f0a244b0d8366756")},
                {   20000, uint256S("0xc68b0259c0fd31f00dcb779960d1f85df74eac5dec65abc2432d13e8b0ae6846")},
                {   25000, uint256S("0x000000003eeaf5d5948bec2e65b1487c4ae38d5b05d9463b8b9cbe4b98b3d95b")},
                {   30000, uint256S("0x000000004236f010a3ac5b9282ab9b47bb1c463d9c700bfbc7e6d2d2a6e26ba9")},
                {   35000, uint256S("0x000000007e30362f41fd9b0b59694a375269068e2f5d7c0dca85689a3788af4e")},
                {   40000, uint256S("0x88bab2e99e501747a35b82cbba85d4bf7f146b37fa6f79aca2b7010473b79e7b")},
                {   45000, uint256S("0xe33b1b4091c7ae47bb8f9b5fbf7dfc721333edb679c81720ee1e56e2349f7d48")},
                {   50000, uint256S("0x9e5657b00d0fca5f8e906d816f9f2ae8e9bf1987114daa4a5abdfb9dd66d789a")},
                {   55000, uint256S("0x0000000089847f29ba32e22374bea9211d9543914688566d59f96789a17a5593")},
                {   60000, uint256S("0x0000000160abd41740b02bb50dc2e6a32d45e40b12cd6302d915262052161126")},
                {   65000, uint256S("0x00000000289907bc066fc1e5b845dc8bdec53f52e21a307bccf8c7e8fecea0ef")},
                {   70000, uint256S("0x461c3e2bca8d0126055f057517fb3a6c9967ff1280ba68e74679c788a9f65a95")},
                {   75000, uint256S("0x23b982b206c782fbce8a3611e98d3a0f5e53f16ebdba15e84b09e0722439f50f")},
                {   80000, uint256S("0xb1d67fc234d6d988b5b3850feed7bedfe09ef173f74feb4dbe067611c99e1da1")},
                {   85000, uint256S("0xd7cc877aa2bb20a573cfd4411624315586c05959525a4f2551ebcb47614e6ffe")},
                {   90000, uint256S("0xea6d620d0122e10279121edb14e31d8f2ef1c777267e471ed8a5ba78dbaaeb06")},
                {   95000, uint256S("0x631db336088c21f056f51a419e8a393da7fbacb5c7e52871d706b9f6f0c54e6c")},
                {  100000, uint256S("0x000000005516bc2003aa2a222cc026348d5b09e71786ee26d9fb1451882c3ef0")},
                {  105000, uint256S("0x000000002825864c8f1d93ad232b56931847b8f3f3009fbd9eb6817ee90105c1")},
                {  110000, uint256S("0x0000000006e47e5b4de925405318d68bd263b5988462e1d850d134b219a8c7a1")},
                {  115000, uint256S("0x000000004b97a7031b0f0717d51be35aad64240ff71ea47528a9dc142c041b41")},
                {  120000, uint256S("0x77ec4b20b8bb45cb3615fdd6dddebd1124f10a71bc0cd921f5f79e511e27149d")},
                {  125000, uint256S("0x00000000547a29a80ebca8800fe4954d7b4938e0eeb3eb76782ab2ed72f152dc")},
                {  130000, uint256S("0xf5d91957848c4950bb6eedcb762768bab8120be60623711e6c5cb868fd0c01a0")},
                {  135000, uint256S("0x00000000fe73e05e2faeb6342e986bef06ce75ec1e0c41e8fe66b898e88784d9")},
                {  140000, uint256S("0x0000000068c52da14958eded023b46b33f48b257fa1cc91da3e54fece452fa06")},
                {  145000, uint256S("0x54cf5f0bda4fd9f10de95caa5093c13a9747cd08aea9fac70a9a12fa52adbcfa")},
                {  150000, uint256S("0x597c6f5f74d01a3ed571e9eac1a520590f0718a3fe3080a8610f9b541c184f85")},
                {  155000, uint256S("0x0000000096d386212f81686657a13b02909e8a32d1a4e75fa877715dc8b4cb64")},
                {  160000, uint256S("0xbc6f719147a001c511737cee84be504dfc0aa7ba7e654904c54e9abd797b9315")},
                {  165000, uint256S("0x1346a4d10d74babade92415011287e6b3aaf7c036c87d5e84bf047252a7669f8")},
                {  170000, uint256S("0xfbe841e74f87bcb08ea9071a11a0aac7dc85dab539994e9f90c3f6ca3ee93e4e")},
                {  175000, uint256S("0x00000000572e48d784883248b11e4ceb53868d3444ab9f4f4c3ee3268eb001b4")},
                {  180000, uint256S("0x4e492f07631bc6653ee6a1227565e04ec3a2d2c81351508629a559da8d707a86")},
                {  185000, uint256S("0x0000000050af8dcc507c92a1e8bbc0e3d3065772899b15240a1b55650f950466")},
                {  190000, uint256S("0x6f982bf6325e92fcefd789022088498a615793658e4d2c82b02b8f7b370e758d")},
                {  195000, uint256S("0xb09d58142f882a1c4987bd96d818a3b2a0a3f002067b1ba125acf5fe61a6c8f7")},
                {  200000, uint256S("0x4d79738e5a16ba831fceed7ad784ef0b101707adbf77355192e70f82e623883f")},
                {  205000, uint256S("0x00000000298ec0cb2b246a290f3ba26d886710d767dc4c8f583d89ac65e00ac1")},
                {  210000, uint256S("0x00000000196c324d1c19e81773c1daa5bb825b7374ce744f059aaa218295913f")},
                {  215000, uint256S("0x4afe420a617c57ba548c19648278a9f65adab119c0c12efe305bdbf8d8bf41d8")},
                {  220000, uint256S("0x000000003d1f4b82ee64d28f9b05a310f374a948ba5dd81b939e1af030c17941")},
                {  225000, uint256S("0x99f84d8cead5a1d44d4f68853b7c46f9074d54c59b166bd1f2279178e9861fe3")},
                {  230000, uint256S("0xce93880dbea089be6bcbd1c665d6f860341e2f3412895a35046e8160be0f6a15")},
                {  235000, uint256S("0x00000000965ad6b6d6b92a3c125b41bd9bd35f2104a9afc51968c0ab3acb107e")},
                {  240000, uint256S("0x8aa68bb57f04521971ce479cfa6140b367462116ddeb55c523b2ec29b439a5e5")},
                {  245000, uint256S("0x000000005a6c93b6ea45e13c4446d22ea89cf3f2e82fc555bb22566cc8761413")},
                {  250000, uint256S("0x00000000331c9591f8fde2303565bab5a7d2ebf3f7834dccb079b5f23d6aa0e7")},
                {  255000, uint256S("0x2f27e027a27300359eac26d7c1f34a7c086f326620979469192acae3e1afd5de")},
                {  260000, uint256S("0x979d5173ad642aa0f8166c9a3c2b351de0e7ec381f2465659de31287e0fb5ad7")},
                {  265000, uint256S("0x0000000031377da7a7255e739d22d66ddc665cf9acea2c2100699900e2dd3f36")},
                {  270000, uint256S("0x9245486ccb4cd9efea6e764828d70a06c304264d6b460af65ccce216067cb9b6")},
                {  275000, uint256S("0x000000006fd2868bd4f3779c81138f986e53ec9999b44f2abf714cf38846679e")},
                {  280000, uint256S("0x9b7f2a068f29983821b562b26390b4bfd56e37ba4a3873c45b2b826c24137755")},
                {  285000, uint256S("0xf786bd1f1a1e3be5aa0b01dbfca0794b6b38e5a8de9dea0d3c20526f68cbdcb8")},
                {  290000, uint256S("0xb6faddb02a4efa0568cbdc64b5cfd7d0b3fa8e2854995791a4d73cbc106d1e1d")},
                {  295000, uint256S("0x7eaa65ace48c0a3a06d58e3ca031f2228ad6cf3bb2192c92c29b7a2c2edf00a4")},
                {  300000, uint256S("0x0000000085d96ac62f6208a3520ced06102cef49a607a2550cd4126e82091a00")},
                {  305000, uint256S("0x25f05a08f2d084e6e0ba5c1ca9541f5245a763137f22e518a2d07d6807ca3e87")},
                {  310000, uint256S("0x797ee4acbef92adc9cbfdd188a68b8b0d29c8a4610742da9ee85d6e5af1a386e")},
                {  315000, uint256S("0x24b2f2f5974bd4b66f13f44531260a3432e04205e81956da0e12d0673a13564c")},
                {  320000, uint256S("0x9dbf3c825efbcf7328287a8b4634fb02be246481e205c87fb389a0ba1ad51d59")},
                {  325000, uint256S("0x224ceaf779764effa6ffe2ed43076a9db8d2e244e91a492b60fb06edd464c95a")},
                {  330000, uint256S("0x4d31d159c991d6877eca70ae889fd19b4eec2ec851b7138042b98adcb1906368")},
                {  335000, uint256S("0x0000000078d9b7a387c6036df3ad5cd06e6fdf02ce19e7ee40cfebac6272e879")},
                {  340000, uint256S("0x4f0b1db359b33b4e56d71efd8c7283c9035b7670d8f523d3eee369f98fa0b415")},
                {  345000, uint256S("0xd4f53d353d76ce85a4a5923d079e4253eea66cc9469a5ce56e7a5505162d445c")},
                {  350000, uint256S("0x000000005f2959514e33e69d8a879ddb82b0f860f0f2bba5dd4cc4c9115b20c4")},
                {  355000, uint256S("0x46728e7e5d7fa07324d945ecc1fb150e0f6e7a2b07fb71fa927c2215b2f3118d")},
                {  360000, uint256S("0xa82201db11741ebdcf1cf25254faf5b058dc850dc7d67be77b1e5837116c63cf")},
                {  365000, uint256S("0x000000000a813d8f62cdf73b1557bbf0bb3b56f78ec27e2db4fb374b79361584")},
                {  370000, uint256S("0x51020a49905bd44a070a6e8311a09e67908b1bdcc901d2800ddf8a0f6350eaf6")},
                {  375000, uint256S("0xed79b0a5bbef2663d262ff6cd03e0e636495fddedf072f3d5c5dc47129f8bea4")},
                {  380000, uint256S("0x000000001eafd4b5d92620f4413487c021889ed1749718373a5bd5c4fb65c798")},
                {  385000, uint256S("0x0000000017abb63486565445af5f881c6d6e4de2129d8a5d933cc6f41228bece")},
                {  390000, uint256S("0x000000002d5a54a0b29a28af1abe0e019ff58d07a50273753854fc58d03f8d40")},
                {  395000, uint256S("0x6b66cc84f71b2e7fc59085134050e2a9eaed01dd8c410fcedf977c6750214fa5")},
                {  400000, uint256S("0x846c39d7ae5b9f9e7c1564f75fe8ef9565cd7fee4f4791a7a599c3a4f09fc6fc")},
                {  405000, uint256S("0xba939dfe2e16da17f61f1a9cae8d80a26c1b6a11d23f59af97231ecf025681cd")},
                {  410000, uint256S("0x05c6824f77e7bc8da74bad16bd4e9bd1671d7b02f3c3bda8d01f6b445b500085")},
                {  415000, uint256S("0x5dbfaba5fe86a41d73d0d968ab3c272e43e85c620ef4e54b4196c50127c59ac7")},
                {  420000, uint256S("0x000000000cccddb818253378e2e4e6da433c0b1ce6f0c6f44f0e5ed616b06233")},
                {  425000, uint256S("0x2bf1acc70faeb5d99d31c8d32d166b67a0b8007c92486a468b320c3009bbb419")},
                {  430000, uint256S("0x8b9e56c9fb1046f73eabe8370055bf17d58707e6ef0cce21907f8d1215bb75f8")},
                {  435000, uint256S("0x0000000086c6b16750fa5c6e0660c803e02de730e9e90cd237a5c0f19f8f7fb8")},
                {  440000, uint256S("0xc494084e6da3c640bfd4c328f5b265f01781d602543f423d975c840424d4ca1c")},
                {  445000, uint256S("0xc171669051b02e7677fb3f86d40dc7dc2a0a7efef7ee6bd7752ae012ac49af11")},
                {  450000, uint256S("0xd9b19fa6d10cf25ec5f1e2dde5561feb290b109d80f63fed0ca7adb8ba336443")},
                {  455000, uint256S("0xfd0d636e7b0e43fc650ce2768af95465831771c2d569d0bc630755659941ab20")},
                {  460000, uint256S("0xa17a30cd900821d04d06a429539bd8430f2a04dfa89dcd44f2a330e59245e952")},
                {  465000, uint256S("0x0000000038f6ee5ea559e7f04bee27277c65d77d12cc1092ff5f8d351b328820")},
                {  470000, uint256S("0x5a5f5c6aae5b66a94386c85ddaacc17653a8d58811e593232feaf974dc0e7df0")},
                {  475000, uint256S("0xe4c66a1dc103888926b4e5d6d57d9b9cfdff63527fc36cdf5892422ab9cd650d")},
                {  480000, uint256S("0xa85b586328f146a868b9b9a676ee752c84b31d077a070adb42b00eca53e7e80b")},
                {  485000, uint256S("0x0f3609cd0e3646e319ecf82944a925f7e397b1447523c8469aaa85af2033bb2e")},
                {  490000, uint256S("0x0000000037f74aca9330c59c9cc5cd7cc3fb44b39befcefe3102ef30e066f993")},
                {  495000, uint256S("0x000000002b60658ddea8b4035dfa2feb27d8eb2f459be11ae1a2b3847d11151f")},
                {  500000, uint256S("0x27df3bcf626c226b05344e4ca660cd85ba4381d728d270df295c1e9cb3e513d7")},
                {  505000, uint256S("0x000000002b43972f8066ae419200b5d1841d722df5965a9742db65541308ec30")},
                {  510000, uint256S("0x1e6352e80a13c2c334695d9c1eb8c61c64fd8290370c4740b504615f433b5836")},
                {  515000, uint256S("0xb288335e440f040a7fb010acef99374d84db98c03d59d8f68d2ded345a48b620")},
                {  520000, uint256S("0x0000000015953740968fe5b74cf2daaf1cb53908e96543d1acf2720fce44b5dd")},
                {  525000, uint256S("0xa18857c32ceef29abfbac35acf18069685d417a764636272119c39083a6c08f4")},
                {  530000, uint256S("0x000000001bc96941499a9b80c55ae14ce3d63868cfb525df73f4b818b9d859fb")},
                {  535000, uint256S("0xac6963f8d41ec86f88a1afe4dbe380fe54cbe9ffc24b374c979b3116926b901c")},
                {  540000, uint256S("0x54b3ec012c2accf100142cf5b7ac3f0041e5fbc09f98ad677777787a66b48638")},
                {  545000, uint256S("0x000000007bdaf00133e1f407bf51766360216ee7af1dc85add3f8d61fd98d526")},
                {  550000, uint256S("0x033182b3c28797a41475dca4e5d50abdb6ec8e4fd2385d61a0b9b239199d2bc3")},
                {  555000, uint256S("0xa35a9fe524bcf645dc7e1f020e787b24f270c6d711925d58d14578883cdd2128")},
                {  560000, uint256S("0x000000006a921d29c9e2aa05373efc2ed5a3fd3e2121671f9b9f5364c7167318")},
                {  565000, uint256S("0xf545bdd7ef8f6a7120b55aa41f3257d1a4aec5a9940ed2bce8ef9de05ab29a6b")},
                {  570000, uint256S("0x0000000013c34e530489697f1784706be6ea9759270895f1b095fedc4e630386")},
                {  575000, uint256S("0x000000005505cd1ede6a71804d39febbcdb82ac2aafd9ed44628b4059f518e65")},
                {  580000, uint256S("0x6d836ad3329ed1b1bc2e040660c00c05729f5c3ec463054cfb83fb5ca040d7d9")},
                {  585000, uint256S("0xfbfdd71aa7c8cb14c33dc94a903ffa8fc9156cb672e414065f53b103e745b027")},
                {  590000, uint256S("0x68ab47adf17377590521cadf3c9d18cd4ce5801a7e057cec2aeb5cd86f0401dd")},
                {  595000, uint256S("0xc0a256aba48c9b2b929f9bdcd9f6d84ad20f590bd3cc8991848f2e346b143994")},
                {  600000, uint256S("0xd2c345905688e9bd3afa55dcbd393d3e8f707c8b6785099261647c47c29c7207")},
                {  605000, uint256S("0x870272463cf916c90f0c08119dd16a8c275b1fe511a4f9317573e4783174f655")},
                {  610000, uint256S("0xbe5d15967f1685d3d0c79bbb9f05d603c4338d81f5a528809739dae489208c1e")},
                {  615000, uint256S("0xe645f7b9b289e62f0b8565667930d0cd8a0e433650cd56ef34fae6561a28078b")},
                {  620000, uint256S("0xabce50e4d3252accd635aa4e43e93bf895170ab1b186ae4a41823ca2c808a869")},
                {  625000, uint256S("0x000000003c138c6f863fd679125d3916b82fdd4e647413b75f344efde3e960a4")},
                {  630000, uint256S("0xe6d0a38c358271852872430376c9fc2c99992d7a5f92734fce08829125233ea9")},
                {  635000, uint256S("0x0000000004bd4a7a7d755599e44f159124c0b9db0c473f9a31cf021bcbb701f3")},
                {  640000, uint256S("0x000000000a685a603743d34eb58e0b9ce83144acf3be112e9691f3c1e88089e1")},
                {  645000, uint256S("0x9e5038f8b26b6b7a2bb9fc97c39d22a946eb4487418f48c0e1c30080ef57cbde")},
                {  650000, uint256S("0x4a997604fa4efb05edf5a400393d2a719c2ca73025f486627fc43548f3315f65")},
                {  655000, uint256S("0xc173fadc0ac9bffe9bc623598a34787e2bde8b31c7cccc3d963cf618a268c173")},
                {  660000, uint256S("0x000000009b7e29cbe432e56d1b4f489207e92f8f6b183a49fb860477ff66ee8a")},
                {  665000, uint256S("0x3fafab51dc2acf171bd21f7a4c8f9c821e9e5a03e6dff4a5da6d025b557e1481")},
                {  670000, uint256S("0x3d38090091062700bb026d6b2fd41903b3c51397b9ffe2dac1de2155a3b39091")},
                {  675000, uint256S("0x391df12552fd3f5d677d18328e63dd9f4c1da4cd5b87eda729f5a41328d61517")},
                {  680000, uint256S("0x085863d7ef723877211f8f278d23a00dcb760342b28f2b9ac00f64cb8022be0e")},
                {  685000, uint256S("0x0000000075df5825767cc3c7204ff0d9397ef489eaf4c1154e96e1c767646cde")},
                {  690000, uint256S("0x000000001900352da16843ab7cc145fc5a47b5f9b7699d11029dfca857ac5012")},
                {  695000, uint256S("0x000000007cbebe6bf71c2e2e4c3c26f28714f7d5b0dc33a0a5f77a0b780f9615")},
                {  700000, uint256S("0xda8312f75d326e0efb8ddb17e111650e965df3067163589990d8870936f6ebc3")},
                {  705000, uint256S("0x5251dc6c207504b09dd6c21d53fee70b0d78afa7f9b400ffde77b66fac44a404")},
                {  710000, uint256S("0x72b67f0dcb46d7e2a8eaa550fc3dc33e6c3935bd51dde7f4754935ef0ce95548")},
                {  715000, uint256S("0x000000003e0c6d1d8b25f31bb30ecbc421e5da2d306ad4c99b09e332317cff93")},
                {  720000, uint256S("0x000000000a0adbe725d8271677dead60ff6a67963f0cd417c824ae02b1851f19")},
                {  725000, uint256S("0x4d24573ec005cabc5633004454e2fea753904ac3ec8ed4f8ca3d35fd28770663")},
                {  730000, uint256S("0xedcfa9029fdf44ed52dd1608e50da9581d652c075e39b323941712e4d0c28543")},
                {  735000, uint256S("0x000000003143c21737a9f59396707681a42912a59d0585e2cd82932aceede201")},
                {  740000, uint256S("0x000000004201fd820e883ea22cecbf62e83f67ae0c0836156a15f218829cae63")},
                {  745000, uint256S("0x000000002eac45dc72910b4fb65f78dd2b63d4265856911ff1416d1ecdcc7224")},
                {  750000, uint256S("0x5819a0aecac390adfee554c7254abcdd79e5da331a3c8c160544f500caa14135")},
                {  755000, uint256S("0xc25eb92c23e943601e98afee1e85d7d47fad8cb0fda11878a8072067f7a0ef8e")},
                {  760000, uint256S("0x77b180e3a244f3e538411ca066a0416aedb04e6e90980a024ff0e5635578fe0d")},
                {  765000, uint256S("0xee60f373223efb6d9fad681a609dafad7fc0340d36ffe6f42e1d037b6d74765a")},
                {  770000, uint256S("0xa7f4a6dc49d3f8ae6717e796a19d187aba52af91f59d16e9fb488501def225f8")},
                {  775000, uint256S("0x869e40c81eb7460384133d80a19f1784e3a6221c6bbfbf33929a6b87adb14e1b")},
                {  780000, uint256S("0xfdd2c0c951ba0399277dcba885b4e4ffa55c5ae8f9f4fdb0e151b6410e532203")},
                {  785000, uint256S("0x000000003e0a8cc7806914ac75b00aca961fca0b7dd8aeb0fbcc648583a9a43d")},
                {  790000, uint256S("0x2b65f1c512d1c262467b2b356b4d64018518e80b7332cb17353a4facb4110e51")},
                {  795000, uint256S("0x710c017791d41cc7332881ef0109cb93779a745a63db8179d6a06178e1ebd5a8")},
                {  800000, uint256S("0xed61e9ff2169f9ebde62a1e70e1a26b313629b2a71965370d13d67b14c5e0b13")},
                {  805000, uint256S("0xce5b0f4646def86ba959e14c3bc195832f6cf360c2812fa1b529ca492b18263d")},
                {  810000, uint256S("0x3fbbd7ad9c93f3922b9ef34b87e8afbff903d240e3dc6703a7dc54adbfdca99c")},
                {  815000, uint256S("0x00000000903abdbb8a041102d7973068aadacca0b5b757f6baa12a15ed38ce4c")},
                {  820000, uint256S("0xed2083b6322fdbd540865c287e20af398e08c05c96c65b6291432c839968cdb2")},
                {  825000, uint256S("0xe313a63069671ed4db5fc5c29a179f9b30e6da7c8e3840d74e243dd1cb2350c2")},
                {  830000, uint256S("0xc86f32387abce55936a0d4192ca9bd775287d233e1da868c5fc0029df3e4bc14")},
                {  835000, uint256S("0xa74e252a974f194a7c69e1c354743cc227dbc2b3d8e73002969eed2355947404")},
                {  840000, uint256S("0x849c9044daf4bdc61776dca520e581fd23aee875c3a03d64b08654f6db08453c")},
                {  845000, uint256S("0x9e931d25fc9bc660ac5e6d6b708900b89dd3a16a5741ba322b2cfede333620d7")},
                {  850000, uint256S("0x000000000673429678a563c04876eb1798b001fa1676ced30d5ab08ac8c60a05")},
                {  855000, uint256S("0x000000001bae740b0dc502a15505c6081718d3df0bc1cf70adac3528d24a2109")},
                {  860000, uint256S("0x92418fa6c1809c4c45e6b81b316dfbf3799119905975a31c504c05e58bb1b7e3")},
                {  865000, uint256S("0x000000002b8ddd5cace9645df03d067e34461b97755983fe7bb3af1b2123f5ac")},
                {  870000, uint256S("0x48a8e512a9658e28828ad888b089231b15e1d7415937851db5353d9418b3c347")},
                {  875000, uint256S("0x3acb26bfcc3806eb08ba31369184a279e6b8f9630def9e3118d78345cb531dae")},
                {  880000, uint256S("0xf6063e3c70b4a0094b4e44f03ac466cc92df5c62fd431708ff1741e1acfabd2f")},
                {  885000, uint256S("0xde16e2b9f42c91d7183cfb53303bb3598110a3c9933e442c3c11b5fd13d12ece")},
                {  890000, uint256S("0x86ab9ea2e40071043a26665d35258b4411ca60a6e05266bf8b5eaf2235dfaf5a")},
                {  895000, uint256S("0x000000004cd90cfcd09edd3b2979e5b0e5c2332e7ddb6d9d2f8ee5051af143e3")},
                {  900000, uint256S("0x0000000069e6c211cf7d73e0751d8ce3b3084338723be0f7a368e2568b9b3c6b")},
                {  905000, uint256S("0xaec2c2d82d5b3384b5de98a9f14d976aee8bb312bd1fe96ee9b79594ef610a2d")},
                {  910000, uint256S("0x5ed72d5e0acdb64cb27125c863fa6dece852a66682abb56ca855f13b54f5f099")},
                {  915000, uint256S("0x43a05d15903ce837c017f73f61508978f6209570219df62fc24cddc9b8ee4006")},
                {  920000, uint256S("0x599d0411dd7df97443d009fa34a497bea77fb8bce3599d3c06fdf85a0560e285")},
                {  925000, uint256S("0x177b5a018214ca9ffe6ca416fb00ff7a1fab231618559d098a21bebf4c93310a")},
                {  930000, uint256S("0x0000000051c4ee500c26db67fea6ac22b01c402a095f4c1763490239e3770f1d")},
                {  935000, uint256S("0x2e2fb6282b39d2dc306c65c7833504077e6cf399ee768478d3e16f9440dbf85f")},
                {  940000, uint256S("0xab3625f5ea75990a3bcaebd88742d28f52cc34600f6ec24b3bf2929e87334365")},
                {  945000, uint256S("0x000000002a43675dddcc3ca34ed3cd9e293f84b3a43bb7bcb9651f5f36cd29d6")},
                {  950000, uint256S("0x000000002072150bff32b75cf5b0ab3a38fff1da9e5b8ce273363c24b79d4235")},
                {  955000, uint256S("0x000000001d098397ee935840eba39af37a789d9e8491c44be5cf57689dd8595c")},
                {  960000, uint256S("0x000000003ed6d962012db46e6e0f8123acefedc78be52bf45d5fdc658c936725")},
                {  965000, uint256S("0xa6677ad1fa5c3f58fd6328866d3c07ba9b51156465dbd20e699b2dc018cf642f")},
                {  970000, uint256S("0x43d8fbbea18a0999a6cf2aaa5ff5730abfe22394f3d25147c076f8a1c8e4baa5")},
                {  975000, uint256S("0x000000001d10dcedd4200f9aee8be4b10430bf311d7a1632a4a4d4cf22c4f916")},
                {  980000, uint256S("0x1009f5f810ca62873a2e9853d2ea18434379d051b5e7b1066159cfc23370964e")},
                {  985000, uint256S("0x88ad861d74859ad470dd9cb5ec6ee7f12d4ad1ef2a71e2a7124bb7a7dbc4739f")},
                {  990000, uint256S("0xcf7f7c80b3ff95e5dd6272b90d860f35b9a301a3a2086b04e451bac9cfd21a7c")},
                {  995000, uint256S("0x132c4135d11dca7d52cfc5153aeddc96c20f0303fe61e4248a5b9be35a3cee21")},
                { 1000000, uint256S("0x2233161a327d185300428ddeaf4e0872a5a8adf7d66768cc0e3e777515fdc2f5")},
                { 1005000, uint256S("0xe591b11ae781f912f66026c659d545d58cfa0047a47f5dc7d2cb0f7910a1487b")},
                { 1010000, uint256S("0x0000000039cbb1b7189fd0b94e709a6ff5fc529d45f586f022aa6021173659b0")},
                { 1015000, uint256S("0x23592ca5676f09ad2f7858295e07811614dc962077d30577a2588afe30de1c79")},
                { 1020000, uint256S("0x854b84c47b27c2d731b06655f611f263cb9f7ba34453db397b117917ce5a70f8")},
                { 1025000, uint256S("0xe400a000c29e39163e1563a0c5979e33a8556277247991b423323da08447e998")},
                { 1030000, uint256S("0xb0177663ef4b47fbdf178005e42196f7e7620a4530dc21e1f15b5adf92593ef8")},
                { 1035000, uint256S("0xc4185076ff6d6e916452b20dfb8d415cdd0b81a27350f8655cba7ec9852276a9")},
                { 1040000, uint256S("0x54ab112054b686854aa5f7c47ebc4b0d2ee2e57a2da6ae09adc4d5a51bb8da4d")},
                { 1045000, uint256S("0xab7f13211c16672c3ac87babcb28a3d3bef581eaf1a262ac14754d91aa086407")},
                { 1050000, uint256S("0x0f4a8293f1a099b493697f2e298392a79d0dd9b3b1abdb766aa2958dd51501dc")},
                { 1055000, uint256S("0x5b058892552fd89ade18947f74cc61a263b93d014c2064aa082d1b789cf5e33d")},
                { 1060000, uint256S("0xd0203d686e795eaa780c048fa10435906c98712ebbbaf23a7bb5c9722dbad92e")},
                { 1065000, uint256S("0x85becd12b0169f41d34e491bfa4ae98c333bf061cbf476e9200d193cfd374183")},
                { 1070000, uint256S("0xfd727ad0677931a16406e9a67f92fc0a81d2ce31d355277efb30ecd553ceb667")},
                { 1075000, uint256S("0x65dfb30483c813061ca70bd60397b7485a00492d1017af0590aaf315325d70c8")},
                { 1080000, uint256S("0x00000000569c821928f6268a61587e62e81369cb311d5992724d95a05a27c94a")},
                { 1085000, uint256S("0x0000000016157dfc4f9b243fcc60c5b64bc836bd01aa53edf8c4769071909c44")},
                { 1090000, uint256S("0xaf39cf1238b2784e9b8f60c9489d16c8dff07f9bac4ca8ef19ec1e0676c259ee")},
                { 1095000, uint256S("0x62d2d924120763000db68bec03dd5f63dcd2d1e9ab19ed414530fc4027c0600d")},
                { 1100000, uint256S("0xec84f8e51316ad3dc45ba93b31bbe2b23eaa1240a4a6763e5779aa83f793970d")},
                { 1105000, uint256S("0x2d566388defaa09413b5ec5aac02f81c4cacec5c20428db1c11499d3aeb40477")},
                { 1110000, uint256S("0x0000000040b4cb9d5fe2a5d95b175be33f82265ca765e547c7c8e4d77b74840c")},
                { 1115000, uint256S("0xfad8fba507b213060af5ea70ccb3a456f42eff6ba86fc10edc4677a32da018f5")},
                { 1120000, uint256S("0x0000000005b69d29a8d361e9d13feb4fc316ae650b7cf5aa3b9a0d975eefca11")},
                { 1125000, uint256S("0x89d563eb4fff5393a46db536828353a00d7aee8839655115d9cbc562c1c504dd")},
                { 1130000, uint256S("0xa8af4e54a8d1fc36fea79551c7b067d30697f8f5495f97455fa2f0bbc0507aa9")},
                { 1135000, uint256S("0x0000000018fd1205940f801373643b89323a0350a118c8c491d0f5a106e86b34")},
                { 1140000, uint256S("0xaa66ec406adfc97b4a22f7ada99725fd6bcfd4372edcb2192a15d7f834c09849")},
                { 1145000, uint256S("0x0000000000bf999c8205f21ea29b95ed8b4290e50f3d6f33c2aeaaf49b6d1463")},
                { 1150000, uint256S("0x00000000017e4db7e26e6bf94d2d789335ffb95b264d342d81cf4244678db4a5")},
                { 1155000, uint256S("0xfadc01a5a73397cb60f0fdbca645ba62032a30bf4594bbc6b5ff5b57d10d08c6")},
                { 1160000, uint256S("0xc914ec2594f84dc3330a382b80ae51569caa3770e0d60e6b53d86a5f9080d0d3")},
                { 1165000, uint256S("0x7cd1449bb65f7231aae3d298a70744e88b6ff17d88bb50c6c7434816e74faf46")},
                { 1170000, uint256S("0xd28e5beeb4be8a98b8a5b152f408559d7d466c18fce48bc595a475abf217fb26")},
                { 1175000, uint256S("0x0000000010805ef0400e92078e233b8858df89632d729554f566dbd75276aef6")},
                { 1180000, uint256S("0xc63ac9ceeefc096d30e6247ba67935d2e7dd465d472986bc6fc88bff17749e09")},
                { 1185000, uint256S("0x8408fb0f3fee29d9546938bcf5477b6ffb0a522d38f519039b337e223084804a")},
                { 1190000, uint256S("0xc5e990c329dafc03de4b524496b861e60211ee1fbc5e7e506106f1ea2341410d")},
                { 1195000, uint256S("0x3d6c3c2524e488b3916d24767db23feb566b4144406881464dc3bd10d775bcc3")},
                { 1200000, uint256S("0xe2700cc7c23c3e2a7ea5ef3b236a8cc5ce82758e9b21bce6a66d6b25b42cd5ce")},
                { 1205000, uint256S("0x000000000a3ce9d8ce959412d654ab496c68f81cf28a280db7cd02a9c0fe34d0")},
                { 1210000, uint256S("0x000000003db7d6577653baf075433e7025fa8344887f8b8d72fb895e9d0371a0")},
                { 1215000, uint256S("0x79aac2aaed0fcdd012120b6e460e3acda86218e7b02ef7509fabdf489de7f688")},
                { 1220000, uint256S("0xa8549ccb8a50ee48d78f99e8831d0c03bd1e5bcd52748895f5731ed44d52cf23")},
                { 1225000, uint256S("0x0000000044796ba95e62c4e965af51c1a0edfddd40ac946bbc1e0b0cbe8f86b8")},
                { 1230000, uint256S("0xf7a9dafde0b762bd7c4838b687adbc9201dd764b80fad42014d330b05fafa05f")},
                { 1235000, uint256S("0x30285dd9865c8607f9338f95e6f742cb4533fb78cba874edb0b9ad9973fbba94")},
                { 1240000, uint256S("0x9a8cf876f87c1976fe1663463e12efcda3fcdac1b2bff76262edc34a1a192c1a")},
                { 1245000, uint256S("0x847c2dbd5dce7927ba9ac0fda2a568ba7ee660019287971a08439d797adc614b")},
                { 1250000, uint256S("0xac2c13e8575cb63c8c6ca8e55e39fef7a990e2be16f34f184d096830603ef499")},
                { 1255000, uint256S("0x1d55cc47bd89c00696e7b977b71e951195db025dcd3270ba74265d1ed7965721")},
                { 1260000, uint256S("0x684c07e86dc93ffe47368c33c7e47e58ff50b4924a5dd4b85978d452b0af73ce")},
                { 1265000, uint256S("0x029ed3613a9647f8164003f09e40e19fff091719a7db984afb86d1a8a4f2ae5b")},
                { 1270000, uint256S("0xe1f2ec55d0961ec0edfa9fc5d7e853f5106160b6147ad204c92a8ae6a60a0818")},
                { 1275000, uint256S("0x000000001d9954f7b191fd806ad85dc169828af3ba32c87991481790edfa6ffe")},
                { 1280000, uint256S("0x00000000156f7a28352290ea8e34b9de6e43b1cb907de51c0ae34b3b4b83fedf")},
                { 1285000, uint256S("0x87949846c21015ba6dbc9c9eb2c15a344b50568386267ae2a8e7cd99f9d5c40d")},
                { 1290000, uint256S("0x0000000034f3fa659c9776c82ea5261fb41d4dc9bb161b7d3c21979a86b9735c")},
                { 1295000, uint256S("0x0000000052a9a79250eedbdf96f84d60676534b6399fc6980c57a9854d055cc4")},
                { 1300000, uint256S("0x00000000278806dad8d8d61e048fa2f4ee7560431af13317740edac5e36a8833")},
                { 1305000, uint256S("0x000000007566ce9435c86cf4d03949155f718e9a8bb4d4499a40302eb4f7b977")},
                { 1310000, uint256S("0x2cb0c12cb2f553d4321538bf28761f38243c1fa8948ffa1cbf5c6415a232e3d5")},
                { 1315000, uint256S("0xb34fd9e97af6d87216c42149193c0b3c2ede0de8a55b9ecb6e3f608a4364bb9a")},
                { 1320000, uint256S("0x00000000172f9ab32457dcb091c109cb080451a5c53a976d6505df184837f7ba")},
                { 1325000, uint256S("0x0000000019af79bbe25e53a8e573bcc7d412481ada594ed7f6d3edc73b60ce3c")},
                { 1330000, uint256S("0x00000000399c99cc3f68ecf2fb0311154feb6dcb4b320c0cfad24eeb01c09c9b")},
                { 1335000, uint256S("0x0000000099e5207bdac7e4deb3bf16aa26974287490e27e170a2097068d9cea7")},
                { 1340000, uint256S("0x00000000475856cdcc246f72f8838350a8784fe3831090c657f88ebe78c1fdc0")},
                { 1345000, uint256S("0x48c517dc16e5f9afb2b91a628e9b971e745718d693a7cc614c3526bd065d3704")},
                { 1350000, uint256S("0x3978239371147244d5f0812f78abf150204449cec5ac6b1849b78215415c438f")},
                { 1355000, uint256S("0x7a5f78b06badf010e7c5cd60d4d70c36f148c8cfadb98ee0414cd819554196db")},
                { 1360000, uint256S("0x9081f37c0f12641ad723bbed41816ec27bcc6192624d23287654b53a0764d198")},
                { 1365000, uint256S("0x3d190bf6f0019c4ac382fa1eacf3ac88bd018993dbc0979bda71f009851683a3")},
                { 1370000, uint256S("0xe8c50179fc772d0a071f6355a9487baaebe8185c589f8516296c38453e1de29a")},
                { 1375000, uint256S("0xa7485dfe4a714324bb8f0532210532925d3301f66589f43415d1ae8d3e053962")},
                { 1380000, uint256S("0x09a822ded922c534a14c212494742184e92e8308312345c4c03853bfa34fe30e")},
                { 1385000, uint256S("0x0000000004ec6ea3e655b1fae862b9e259a192c1f68f3ef9cc4c8513828164f7")},
                { 1390000, uint256S("0x00000000861c8d0339e494004349cab97ed0ca039d6f5f6f0015d56244a39438")},
                { 1395000, uint256S("0x0000000002dbca4d29f0021c0e5bbac1f32970dd5eb44980d7724cc463a1b474")},
                { 1400000, uint256S("0x9b003cd849abad4c5ebcc77f630ead36dced88696249bb67a527cf570d86e8a9")},
                { 1405000, uint256S("0x0000000013663aa0af959bcdc00cac1a7e5c3a2fc93bce2f7101040a3b2a58c9")},
                { 1410000, uint256S("0x2fcf3bf92c66e96e36f60bccb09cd4d32e5ea6e2f2d0eaf92beab97b35a7d15a")},
                { 1415000, uint256S("0x0000000056e34b4addb64976feaba86e0c6c0637cc5099a23bc04d07a1e4b8b1")},
                { 1420000, uint256S("0x10ba37fdea42b74a9b298fb8ab91bffa1682098e94ddfe111c322f0dbdab1192")},
                { 1425000, uint256S("0x0000000011afc5f7f482d4b417acaff71d7cf7f7364d9edb4d1e2e3452dc4a5f")},
                { 1430000, uint256S("0x92f609d8b0f5707c6beb52009102b8fb47e7c26ff23bba47eb1fd7a6ee003279")},
                { 1435000, uint256S("0x0000000054e4cf2932873ba68ff9b4a947b10fcf9a21f18bffa5d8db76adc32b")},
                { 1440000, uint256S("0x4bb183ac42416587b899cb14a9b3f1aec355122dec72ae8efa3a7a7dafcc70bc")},
                { 1445000, uint256S("0x000000002874893a11f86d1dbd1116f81cd8731d221ba7057e82df1d17438992")},
                { 1450000, uint256S("0x4b44109d0dbf0bdc58d12b44876ec928328048581f23cdf5cf947fc1f687041a")},
                { 1455000, uint256S("0x56eaf23e80ccaffe09f5565a2f330e0f48ebe524a901c74c1d57fd0caa5e517a")},
                { 1460000, uint256S("0xed20589c63692f049f40c2c1ea4457fd06eb7cacda304d4b783d5278354dd0fb")},
                { 1465000, uint256S("0x5a53dc4e4c5e28a60ea25ec1c95a2c2347936d116d7b85e073148a28fa73149d")},
                { 1470000, uint256S("0x000000003faef4809669cfbd1884c4f224ee7a45c8c51fda7dcc9dcf94a63c18")},
                { 1475000, uint256S("0xe84df391f8706fa8074195be617b83e24a0f02ce234e53f5048ab835eff9b847")},
                { 1480000, uint256S("0x189a255470840cf3cd7e68c199cd0eb851941ede6bce2c65aaa41721f7334492")},
                { 1485000, uint256S("0x000000005aac6754b41111c37d8e69065638461feae44d954ee8a8d1f594f712")},
                { 1490000, uint256S("0x92007d331e7535dbbeba2927bc4d028a1a5613713e1d15cfeccbc39f1a21cef7")},
                { 1495000, uint256S("0xa7a405b0a40f45611dbe259c1b497564993f0e1c1f1e827dc8decd1f888c0a0a")},
                { 1500000, uint256S("0x999fab9fb95d1a8c344f00e9632131746e2815bb235b0bd3e26871f65c9cabf9")},
                { 1505000, uint256S("0xda9647d2c61bf82cbabfa71cd24d3bf7ea571d6dab9d0c6f591c634ebb03e0c0")},
                { 1510000, uint256S("0x41db31c07af3cbc7ba1cae7aa28b4857f78eabbba4e919f77b7c7deceba365f3")},
                { 1515000, uint256S("0x0000000007fa10ba2efbebcdcb1c5d6d4a5ccf8d7d16eed87b0e895ac37c4534")},
                { 1520000, uint256S("0x3e3d942041e0f814f532698f225b7188edb5af78b2105568c50e1fe68941a510")},
                { 1525000, uint256S("0x000000004fc64e720ab7bb2d9f7e70c8e8e9ec7fbcc5d7c89d2df8dbcacbe9a9")},
                { 1530000, uint256S("0x0000000024661e8eb168c9cd7a0770ddc2814a246d3fa3bb4328008936e908f5")},
                { 1535000, uint256S("0x0000000085c585578781b1be24199aff44d9d10058f2b45ddf83b96bc595d3cf")},
                { 1540000, uint256S("0x3b5c595b5aa1eee3e8ab37285b97279ece1b69f465abe81fde99a57e4ba72ab6")},
                { 1545000, uint256S("0x0000000007e05abf30ffcf355737c0413ba084a6538fcaa33ceb0beccae05c36")},
                { 1550000, uint256S("0x871c6170f98d7d3f282c6215ea940393f3cc06991a35ccad0b2db9d310aefbbd")},
                { 1555000, uint256S("0xb9bbc9480903dfda85b28e882e8b9cbffd5a78671f3ed377d093364cc90498d0")},
                { 1560000, uint256S("0x0000000040b3f15267c0b18a93ec5bd43dc0b31858ce2ac5eeeccdb975ec286e")},
                { 1565000, uint256S("0x820e94e45b2eee639675dd5a7e5e3fa7ea8c38ea5883667ae02d557d22302055")},
                { 1570000, uint256S("0xe92164972b2efe4ab9d62e839e1e90f9d6b004a244f0af1f6f312bb2dbdc7c37")},
                { 1575000, uint256S("0x31ea18b876539c8490056515a7d5fcd21ce1555f31941b9ec6006349a21c9c34")},
                { 1580000, uint256S("0xdd8a24ee6f0b67b125dd05e60f56165d0fc98674a09bc4cb4cb272f165e0bea9")},
                { 1585000, uint256S("0xe27a8a95c88714353cdc0fb2d9195c2efd580db05f0df209b91de93783ff8034")},
                { 1590000, uint256S("0xd3139535da70ade6a05fa485ab6eea119b6538535c26308f81754adc90439e44")},
                { 1595000, uint256S("0x0000000028bb667c606c849e3128ee0388e0258ada44efafd6c5fe8e39ff32cc")},
                { 1600000, uint256S("0xbdba936ee5877931e2c77d5c0f8f0139759ee74a51a8c9ce8afdc735c388b039")},
                { 1605000, uint256S("0x00000284a2e40827fd8dfb1855070b04e41ffbb5b30b03a58b568a6fba63fa1c")},
                { 1610000, uint256S("0x0b0242a70ca58cb7e5c8b8ca6d67d78754419bae7fc309e0f38bdbe97caddbe8")},
                { 1615000, uint256S("0x00000000228f5c04a90d16d37339193df299953967dbec889e11698a9c2402c4")},
                { 1620000, uint256S("0x0000000010087a5c1bc799f7ed2899300048cda9ef0e30747cc1df65f59d8b5d")},
                { 1625000, uint256S("0xe09258139b93aae5098dc75505f86ed70947518db8363182a1c105f00c80fb9b")},
                { 1630000, uint256S("0xe914830eabfd39c48925c7dfcb35970275eafdf179491f10d42d91f5daa0cb53")},
                { 1635000, uint256S("0x227352dbc327a03b77ae33057b21914dc763b3ea8ed5c3a64603cb519fbfb092")},
                { 1640000, uint256S("0x0000000022272bcbb928ffdba97df934ca59d5ba32961dcd7f516dbd7de8a0fe")},
                { 1645000, uint256S("0x8841626354d70a8a1b71d9f675fb24b523f481f8aba0bb1c4f68c31d0ee47c01")},
                { 1650000, uint256S("0x207b57974849095590ea979490429d0ee6ee62e0a3eed99d178c24f5778913fa")},
                { 1655000, uint256S("0x0000000012214a2e72a5ac56010858d8621da0267e06aea3f2ee8ca0273ca37d")},
                { 1660000, uint256S("0x0000000006186fd985fa694f911099aed0aae53b3441ced0e6b30b237995c450")},
                { 1665000, uint256S("0x0000000007483cc0b80bc6dfb7e7f4fac7d2c8959f77e50ae33a3ced4d8d6574")},
                { 1670000, uint256S("0x00000000243087ae346c33137d760bce18e63d75d38649ad4a85fff2dc573d96")},
                { 1675000, uint256S("0xa5aaf05f20277d4e72e03b523c33ac2b277fd3259432d80e9b364a04f9ef8b75")},
                { 1680000, uint256S("0x00000000205d5c181fbcdd2335bc28f78d7f8709e583b556e595ce59a69414ea")},
                { 1685000, uint256S("0x870c2dc613a6937c01152269ed434b68cb0deefd021d6f3bc1b3c1a186f4c9b3")},
                { 1690000, uint256S("0x00c469f08ff03af00b345454a2b7e68ba2163591f06dbe30545593d757c7eb18")},
                { 1695000, uint256S("0x0000000009418f5741ed8823b48afda208f37430da2a356b7f3799577180f586")},
                { 1700000, uint256S("0x00000000348963d119d735aba9299830b137fe32793230883facc24c51a0986c")},
                { 1705000, uint256S("0xbf27b5dd50637d9eabe0cf7bc8841fa8bf8328963f736e410b5d9bcbccdb6580")},
                { 1710000, uint256S("0x00000000284581dd3c75ce92e82a5afee3951788702e74ef9c74413e57717ec8")},
                { 1715000, uint256S("0x0000000029191637c2b8c8abd7d2f8a926381eef5e8d97c94af7c487b3c05b1b")},
                { 1720000, uint256S("0x709f31fc61dfc57f828ca8a58e5cc84ca7cf92db369570a9f9e628e4a1d65aad")},
                { 1725000, uint256S("0xf8716479d6c4a4f3d650b3a407cb6754bd3a5c73328947f1082239dfc39955c7")},
                { 1730000, uint256S("0x661bdc8f1d24af7ef8814940f3d5391caeaedcae638d296726312566e14b4aab")},
                { 1735000, uint256S("0xdbd0dfd45efe5ec72d5cb9b16c554e4a1644ee37b77d0648deabd162c9d0795e")},
                { 1740000, uint256S("0xecdfa8d711c5604e126b2e8d3f7f86e3b8f724eb4215ab90aff13755efa5219c")},
                { 1745000, uint256S("0xc838abd7eaf36850375cc41f833be6fd9a81ff5f439229ac7693ff5d6283be5e")},
                { 1750000, uint256S("0x27ef1486de8c4e40b7da3e5da7c24bc07c2e8c88bb00fefd0d711fba72d010a9")},
                { 1755000, uint256S("0x0cdac13103cb90f7ea307c911447d0073e2f6066c6bcf17118697494b66c28cc")},
                { 1760000, uint256S("0xdb6fb7a7f2ff6315a5ff559ff657ec28817adebe28a56bab73dfe48ef5495ccc")},
                { 1765000, uint256S("0x6c3c24688a24540b145b9f5773a044fad014db85b1675f8ceb15e831d0aec2f9")},
                { 1770000, uint256S("0x5eb968eb2f153ae7936ce094dc091ffc274ee3a2e771c79f64c3914097ef4875")},
                { 1775000, uint256S("0xe0eff83a944b17cd9a4e51e91942fd5bc060abf398a7d6888eee0dc0284dd659")},
                { 1780000, uint256S("0x6e5109abecfa735e4a213b8b76964709c6a7cd7d958d403f08ab1e34501a730b")},
                { 1785000, uint256S("0x241ac38c20d6cbc80dd732d63922f77015555bb31d6dfbd06971b36c78c9a9b9")},
                { 1790000, uint256S("0xbdaa611dd5b205f3b29a85cd90a70ae1b6768239eebce78dfb13841c9a9d134b")},
                { 1795000, uint256S("0xe4cb11a67288f9103a692584886017724a7ec90b678b575fa630919d5b3ca25c")},
                { 1800000, uint256S("0xc44d9c18adaec999219fc2bf6b635818f167b6bdc7d37855bc7aad3f5e25bdbd")},
                { 1805000, uint256S("0xe381a6130dd4104e94809b1773e23141f721637af5ae06b3e2dfa47fa4fae582")},
                { 1810000, uint256S("0x4000186449d79f20a8ba9a5b6cf83f7527bd88ac1ab7b0c653864a00c2392ce0")},
                { 1815000, uint256S("0x2a73638c79adbd9df6987a5f1f72cc39716e1b4e1a44033a1e3709a381ab13df")},
                { 1820000, uint256S("0x0000000017104321c21dba35165203edbec7a1ae9bec463202d272b3d4797139")},
                { 1825000, uint256S("0x0000000018e2c583c70c2452b187a4680f69e3927176231e4cdc6e75cec21542")},
                { 1830000, uint256S("0x63746ad605a4f7458e72427f1b5e987f59beb0f27f94fb091d9ad184b1de122d")},
                { 1835000, uint256S("0x6c52f81aef419923ef5278ddb275992ee8fe92546250e657e34b2b26b0315915")},
                { 1840000, uint256S("0x93f1e86a645ba4119f6d9d608849fe44c05abba7ada627cb9ab450659c4974ad")},
                { 1845000, uint256S("0x358b8a3959ce8ad0cbc610e46da9379c71c1002910f200d44c80a15b8e890e48")},
                { 1850000, uint256S("0xb85db13af757976ba0d77a0d65eacbcdda9a62eb3b60da1663387b24230f42b0")},
                { 1855000, uint256S("0x3ec07e9af581e5aef02e20fecbb78732569879b2ce95a4813b243e43411ab181")},
                { 1860000, uint256S("0x000000002198f21e1478cf8e158bf4999af351b5e1fd486ed208825541ace10c")},
                { 1865000, uint256S("0x0000000024908969103dadf0d3c3d8af468118b1af0877875f3b847f6d320262")},
                { 1870000, uint256S("0x000000003ca785a3f00c65eaebf6180b9585b8192e0b97e772a986074c6fef32")},
                { 1875000, uint256S("0x96790effb84f020a0b369fc25b9b9c21d3bebf8508d21b83471fc9e9488a3ace")},
                { 1880000, uint256S("0xccd109f4c03ab53f47ea41080c6619173d4dd06f0d3f96aec1054227cbd961d4")},
                { 1885000, uint256S("0x0000000052dc8eff765d1669757d65bd38e6a7d59c9a67f7857a928c0b53456f")},
                { 1890000, uint256S("0xee208b84983d7bbb6c4eac46cd49a17d6550c6017432bce8783659707a3dc814")},
                { 1895000, uint256S("0x4820326d61d0ebf9def7f81710277b9d18074757b33f92041f8bb99d81025bb2")},
                { 1900000, uint256S("0x3b26dcaea7405b1e148e2595c9b8b11f0a4e4c5ba4596491b1961299fa9981f5")},
                { 1905000, uint256S("0xa90ca7dd3afd43a9f6470c59f858df808d6e40eaaea90a4687253220182f7511")},
                { 1910000, uint256S("0xf08137604342395536ee84af625448f1e06ba8dce8f62974db2292afb6778469")},
                { 1915000, uint256S("0x0cf03654797e75a729dea784640546e4e9056e78622e54ae5f958801bb55e1b3")},
                { 1920000, uint256S("0x00000000098d516c66fec6776c2ba6de9d541bc34ed97cd54d3c907857ed4b93")},
                { 1925000, uint256S("0x83c0f534792eeec044940b95cd3a7ff602ad803a70fc54b2b8b62afef08708a1")},
                { 1930000, uint256S("0x89cf1fd312da23e33a892971fcdea29bc81e9861a07bd3f31bf754a1a6aeec30")},
                { 1935000, uint256S("0x055a21762b63c4118ded07eff1da167c253c6b57948e56ebdaec01f75edfbcdd")},
                { 1940000, uint256S("0xdc4c987d1af30ee862b6f282e63ab99996e9072e88ae8025f763f8b0f4c40169")},
                { 1945000, uint256S("0x0000000011ef33a07263b93a62e2a3d77ba172db413620a6a0f601fee7509c40")},
                { 1950000, uint256S("0x56d2aa130a5a76bedf6b5caed1212546ab1020dce682dd873cc9646450bd4b1b")},
                { 1955000, uint256S("0x0000000023b80fefe78e8a16824e45d39a5532fe0f783fbbabc10b3805659ffe")},
                { 1960000, uint256S("0x000000004cba335869c54e98e97408fc843380969c6479af1cbd9fff0ffc657c")},
                { 1965000, uint256S("0x000000003bf1c3b4ae816f0c1d8c31c90bfc4057d337ec00a5952247d05fef55")},
                { 1970000, uint256S("0x000000002b3c4d231c662ee6733cf44fd06fb8702920f3ff1b387489b2c2472b")},
                { 1975000, uint256S("0x68fc711b4fe8543bfba25e64a90d7fc257ebbc3871690b5dd8ae28e129022ad7")},
                { 1980000, uint256S("0x757399b3ce621819e357526ce5d6dcc1b5f76ae36bbf56af0c687f8eeccc91c0")},
                { 1985000, uint256S("0x5e6ef6395378d27f1a9caa25689e94e9d36639fb4ddde119be0417502465fcc2")},
                { 1990000, uint256S("0x5d9ee6b672c64bcd90510fc66dabf1d713305a16672ceea76924c296c22c68b1")},
                { 1995000, uint256S("0xe5c3264b0344adf21eaf57c59cd81e14d95a1f4d97f511dbb22f6d2df2c8d3c3")},
                { 2000000, uint256S("0xee2a0eca216e312741faea4b657f63a83fe152c0a615deed306d4e900054b9be")},
                { 2005000, uint256S("0xc070d92edef4f52ce22788d1290c865aba37aa2f5fc72f600e5c9ebaa4c2f444")},
                { 2010000, uint256S("0x5196fb13253a8778c9a78558c62601ba7152cfea4dab705290ac0c3537518538")},
                { 2015000, uint256S("0x000000003ece5bdf8194b0c702ea98d3e73d34126c9f8da83cc7d751c927a68c")},
                { 2020000, uint256S("0x000000006dc11af0348a09b8ff6e43d583534527d7d9ab2ebcb52f255cb0bedb")},
                { 2025000, uint256S("0x0000000011cefc34d1626040a784de8d96049083fcda3f0e22f14a83e3244c9a")},
                { 2030000, uint256S("0x000000000b5fc4987f96b1c115cca4c54808efc718975249218134c85b699e4a")},
                { 2035000, uint256S("0xead2d3860b1352c4b1707ffe4e2bb82ea40cae76068f7e1ac5a9f189d865cd05")},
                { 2040000, uint256S("0xfd2a0a04c79db77cb942d92da0990b379b0e897687af0f056ac49467b0e362c8")},
                { 2045000, uint256S("0x98b2ab8a80163873aa93fb5eb6a611eaf81f151fdf28090ecb649bd6f4730865")},
                { 2050000, uint256S("0x838d12f56fb54f4907dc60639d63f02bb9767a2d48d823c971cb33203cd7143f")},
                { 2055000, uint256S("0x59d2f75d99da8e06bdb365e6deb4539f1e3e8bd278976cae64f772b272166925")},
                { 2060000, uint256S("0x753509d36517032fbfb9bd378d3c15b10693b1835c498678d8562dee3a4f15c6")},
                { 2065000, uint256S("0x1c89428fe4311194a9fb72e826a047b2dde01b90b60928dba8884564cb78c84e")},
                { 2070000, uint256S("0x5ea212ce8efb2022d31e3447e9707bd62597d67e91053ab69aa878b424934677")},
                { 2075000, uint256S("0x4b27be2843ea32c9cad8319d99e6b574d9bb9b2f6ef72c44c7c15085a023ba54")},
                { 2080000, uint256S("0xdfb5b3eda633b36507c771446216c32df3c716b68f73a82fa2dc41432cdd6542")},
                { 2085000, uint256S("0xf52d64aeafe4a82fac096b81b318a686330882bd4a741a1878a97f932bde7fb9")},
                { 2090000, uint256S("0x263e76b5768dc46e2b94d2df316b081fb8615a473431d4bfaea173e666d4f1c1")},
                { 2095000, uint256S("0x0000000004f2173234cdba42b55a133abcd2312a2642cb94f2a87584a4e5b0ab")},
                { 2100000, uint256S("0xb340eafad04b914f1e7e381d811bc9c37195f8b3c83e849c69789991f3fd53f2")},
                { 2105000, uint256S("0x000000006758ab80d7bf2c182b00774e29b8fb1ab860b6ee3422168745e7331d")},
                { 2110000, uint256S("0x000000000b6c5d6ce426187d6b1807f43893afa9df3c0369c3a1fcd76b1817c2")},
                { 2115000, uint256S("0xc91255809dd53f4e804443c48fe3b9d3b363e0b6a287ef2e4127dfd8c1a2cf0e")},
                { 2120000, uint256S("0xa8132642a20e03bca8ae6d572dfdd5228e0e9bf48eb33bda1b4adeb23bbe973a")},
                { 2125000, uint256S("0x0000000018cf14528c54e470f80c35d8f697a56fcba627f9d7335f57b86425df")},
                { 2130000, uint256S("0x029371af624dedb571e2fb494b68fd5e4b18082ae4e9de63fc977e590c71e82b")},
                { 2135000, uint256S("0x000000002f99af29120ced55fbeb0a2daa7580957dd3ecd13019cf77ad44c1aa")},
                { 2140000, uint256S("0xe3ad9d4afb23fb123b555bd6797d657a6f112c119af7421d74018d0d2719bccf")},
                { 2145000, uint256S("0xebc7196effa6b7f47f8a6a01987b70bf964008d0111dcabbefc72be387678782")},
                { 2150000, uint256S("0xaf7b1a4f71ca77d55f89b6e0803fc6a2ed2778827e000e6b8855ee42a5681c20")},
                { 2155000, uint256S("0x0000000015990fc71a8ceead78c2dc8913dc7b1caf2925bffe019dd16dbf97c9")},
                { 2160000, uint256S("0x00000000069a1cca32e4e369638dd543dd39b2a48b006fd8360a64a3c14ba212")},
                { 2165000, uint256S("0x000000001dfbe3000ecdc1668125fcc40e50ee4017f3e7b5b4a749ccda568cd5")},
                { 2170000, uint256S("0xa61ac60032212b7a4bc3fab0ea11b54b2c151fa41dc20c2a9c95f0eddd8ce60b")},
                { 2175000, uint256S("0x7643b254b5563177cd521fa4c89f3032bf9c983711719196261f9fd12d763f5f")},
                { 2180000, uint256S("0x49db1343984d8f35d271757e0f9b2466c34cff73976567198f1d44a6892d1571")},
                { 2185000, uint256S("0x5383b4b88ad5953d6780c6fd9d7e1ebab3c8ec3ab1ddf08e7c697de29b2386fd")},
                { 2190000, uint256S("0xcfb46763439cd4b3edfd048a709ca1fefb062f3c5840fdeb9c97c5bace860acd")},
                { 2195000, uint256S("0x00000000307875839d54caf874d6ec0f14c5b62739d2e725e4f07b58bcdbd663")},
                { 2200000, uint256S("0x0eb403f9172847b753ffef06eb9fcb8a7ad22cd873830ce124f0f84f4a210bcd")},
                { 2205000, uint256S("0x6cb15009641bf618335f9c4a8397111b1318e89bdd0bb39f19f7b07e873acbf3")},
                { 2210000, uint256S("0x0000000030eb894d37a5074d28d9ad4fce573a58497c5879a0087d07204b92b0")},
                { 2215000, uint256S("0x0000000005c1b1ad34d5d7169a27127a54da1977d5233ee821d62e7ede54dfb1")},
                { 2220000, uint256S("0x7da66bfcbbad480e2573fb5ad58da8d90b89e47a4068d2130ba12a48701e8fae")},
                { 2225000, uint256S("0x00000000a018b8d06ce496be46a60c454da941a0f0b6d38c509b582f60896a1b")},
                { 2230000, uint256S("0x00000000443d121c4e525a93e0f7a3b287c1e14bae6dacb202631ef396cdd36f")},
                { 2235000, uint256S("0x00000000279af5199d07aa1a66a7ea3a3ceac586ed29431b5d8a0b98cb13656b")},
                { 2240000, uint256S("0xc643b5a95bc76a6441873408fca89a06763fd8fe48b520079ef158fa88ba862f")},
                { 2245000, uint256S("0x61c0169dc0b0fa4a849936ef71296d870f7ccd740fde705b62404bdf9fc4b509")},
                { 2250000, uint256S("0xa397a86be788ec8c41362cf242dfc52a0f9c20324baac103fbccde731973a1a4")},
                { 2255000, uint256S("0xeec8b1cea667a81063e8e35769abfcd68ee9b0d36cf9cefc781f934814bf341c")},
                { 2260000, uint256S("0x0000000058c89562823f8555dd124c719bbc8e3c6112cb518fa6d6dacd289350")},
                { 2265000, uint256S("0xa133a00810b659a27200200e434f2ae09c75bd2372d4d213110d2900d83ac42f")},
                { 2270000, uint256S("0x0538b3b42885252a484844e65585a6cd5450ee7cc84a93a8a4d844f5bc156f16")},
                { 2275000, uint256S("0x0000000008f763f25c783555809257a04df15a2666a9fa4f5401427398624905")},
                { 2280000, uint256S("0x5c6d2a6176d570b308a22aa21ede40b65e84379b1c6642ffa1c35055a0bd4334")},
                { 2285000, uint256S("0xdd1b05a6d6b91dec381bbacd97669548afde5e2bdeba38d2ad48caf7607b69be")},
                { 2290000, uint256S("0xff6d18c7ad0970da1cdb29ff6e0cdb2ef3770f9096b9e0ed84c512217e9ee06d")},
                { 2295000, uint256S("0x00000000294d76e1dfc3a625ce02433b42c993cc928bda99a6ccb46d74356d72")},
                { 2300000, uint256S("0xf2d1fffa948ba4409d72d529b3305d31267d04d4378d80b525af819a94a1a9b3")},
                { 2305000, uint256S("0xe7114db8f5f48c5cc0a7bab2c7feb6ed77ff508be713404fa4bb76ccf355bbb2")},
                { 2310000, uint256S("0x48b6bb3e352d6a131a3f7a09d30543235b0faf5d140b519edf074861d547cc4b")},
                { 2315000, uint256S("0x9a7a9d3125777cebe6f2d7bff189a66d8454daa6da4669f804f568860bc8d2cb")},
                { 2320000, uint256S("0x0000000012619a9d9588e50584a6f6a25ea7a91f5a17c861533cdbe63f49ad42")},
                { 2325000, uint256S("0x0000000065340d6459f5e2077612eb6648e142c4fb137671ab8ab5657d452c4d")},
                { 2330000, uint256S("0x000000002fd812a160cbd37655d6faac2678b79522df6ff5c721960a1dc9347e")},
                { 2335000, uint256S("0xb3be58aead06ebc05675bee893bc954cbfb2af39d188f1a2b446c9605f673d84")},
                { 2340000, uint256S("0x00000000061c543f97ed794454cc263c000f8909d4dd2876e148714b3a8e9663")},
                { 2345000, uint256S("0x000000003ce319f384bcf870168bde9d219bf75c6256c47114d685315b8592f6")},
                { 2350000, uint256S("0x7cd3c1cabf49257459222e7a74cff3c640acad9fd174b7fc599f05121c3e56fd")},
                { 2355000, uint256S("0x023cd7a4f286bf3c9a9e3356761b4690605af70cb856a7e09488f796c14b1d24")},
                { 2360000, uint256S("0x00000000e7f37240cf87a1e99933d973082d8e09f977791f7c347a4ba689410a")},
                { 2365000, uint256S("0x000000001ec53b70f0fd6c50ebb7cd546b03ff55462993899a0ba0896aeed74c")},
                { 2370000, uint256S("0xc2cdfed0d8955640e68d046c939d0320704296b692704c157fb6197f9491dba5")},
                { 2375000, uint256S("0xc9e30406c0e29b8d5042caf2a8078e25ad4eb1683f200f56ab657f721920b229")},
                { 2380000, uint256S("0x00000000494a112aa6c570054c9c083caf3cabff87a570c1ddfb34caca556f1a")},
                { 2385000, uint256S("0x539d176f766284e4ba72241ffda2ddc22fc1d6287467c779337c54510f2a2007")},
                { 2390000, uint256S("0xb0186d98964a7ca27cf42290f06b242f4218df68adb24731b2f17629013818bd")},
                { 2395000, uint256S("0x829b77f96b81193f481865b043db1a2ae628eb6bcf64d83121fb0ed9b8895b59")},
                { 2400000, uint256S("0xc95a762d79dcfb1544a0e9b4929e9738f991e21d28dc97b9823f4b45ff96fd97")},
                { 2405000, uint256S("0x000000001865ec061e9f58caac39ce5832da2f245558e9318b83648a9b14ffe4")},
                { 2410000, uint256S("0x1f9b527030657eb2d4cb9dd3243c3e61471cb85a12518f8166bb7265b87df848")},
                { 2415000, uint256S("0xa1df2ccba2faa8ca8e67bd9c81b234c3a1982545d5171248df0b255b6e0dcf36")},
                { 2420000, uint256S("0xfde7039103fa500d244b8da84ee3e7b24170be957b6934abc6b2540e7af0a062")},
                { 2425000, uint256S("0xdb2277026c593e22a2e2f3acc04e40f87721d77f4d9848398954fa6b38a0178f")},
                { 2430000, uint256S("0x000000007de931154ca601cc376420c712e9e17f30ad537836131562739b6f86")},
                { 2435000, uint256S("0x000000001504fb1c80443c073706073e92ef917f85c84d38cbf5627e00080dea")},
                { 2440000, uint256S("0xd393c2b80086f65eddf033a0b70924bf365cd7bea31f1d2318c10b73759ba55a")},
                { 2445000, uint256S("0xe6351c14dd0522cb5b83a0d723c45d4eb19778280c2f976c13fb9b5057c1fe08")},
                { 2450000, uint256S("0x9afbc0e47d2f21e080b4333effe98e727c179e12a72cb60c67f0f3737bd857c2")},
                { 2455000, uint256S("0x000000002a825837591d8bb814d6f4383f404f1b4b38e48d1505086ca67f5954")},
                { 2460000, uint256S("0x44d5e886944e122539455ef36f9b32f50bbb401b489cd010b0c7b8b5b2326c86")},
                { 2465000, uint256S("0x8d9ba04cc39368576cd576b6c049eb233d2a29bfc7d85270c399337c15eb3152")},
                { 2470000, uint256S("0x09ef5c62402a6beff3d085c835f33e3d6dbbc98ecbdcf6eb9a90b470e7370768")},
                { 2475000, uint256S("0xc509ffcb9e3b41044e878c94b7d27fd70442a4c3a86de4c9d602c5af5077e7b4")},
                { 2480000, uint256S("0x0000000000c45d961eb544ecf98606ef296a183f0d0ad2f1461503d424b702cb")},
                { 2485000, uint256S("0xdab430f285e861ef516db0b5c459c5997ce19e8b59b1604c00789dfaaa376b32")},
                { 2490000, uint256S("0x000000004f04557fa3ad8b11c6c5da47f8646a7d65044f2784d1b728ad4aa7a1")},
                { 2495000, uint256S("0x00000000025541afabe8a3f7838e7c83876e0e1a722498f45f14720c987ce2ed")},
                { 2500000, uint256S("0xaf111ff7efacc45506b2616ce4838ab3365acd5916206d0b41eb47dfebe6eed7")},
                { 2505000, uint256S("0x000000002873c6d3a0c7fbd8b049a9cdc36d0c4cd1f4ca48ba755605f9e9da16")},
                { 2510000, uint256S("0x89b5e19e7429743879567a8d9e26035df856386e5a0a9e9ad61f279b84279691")},
                { 2515000, uint256S("0x5ba058f262ac27c09d68496731b1b5bca7fb46653a2327047bb4de3db68d051c")},
                { 2520000, uint256S("0xe7fdd657ea44a0bbd82f2ddeaaa78c182cb84342797cabf63b1057016564828a")},
                { 2525000, uint256S("0x47b0f22b6c46718a62cc0187bbf5d8fd152ba0e3312a30dda78cb36cd4eb07b5")},
                { 2530000, uint256S("0xea41d27928fd7f0d6ec2a049932bdaed05e16c770023866c0ad0b95b328da18f")},
                { 2535000, uint256S("0x2fd2d2f100195de08b19bbbee4acaca4fde0c195db88a0e2df0c7b68415283fe")},
                { 2540000, uint256S("0x32082c1d609d37c7ed877bb2ff8c028a44abf85ce504dca861ad8d28bc89cba1")},
                { 2545000, uint256S("0x3132c8ced26dfcd6f246f2c057fd50e2b6bf62933a6e35986d4abb17520ed2bd")},
                { 2550000, uint256S("0x00000000c283ad4ca0a8bcbf03b35eda469e3acb9ba7ddb02c122aa58904e76a")},
                { 2555000, uint256S("0xf13486ddf80ae65ce904d06d6f349fd3bfa6b4dfc72cf55c9a426c734f89f32a")},
                { 2560000, uint256S("0x000000009b02c87d18a1226f253441c4d7cbd28f4b7a4c40cb813f79c468b7d4")},
                { 2565000, uint256S("0x4c6fbfeea0bce5feed6053ff57d6f6a4d7993285b555ba3d6f97dff6ec0ca3e6")},
                { 2570000, uint256S("0x000000007edf32604e9014ca2668802b3e5a7b36ae062086ddcce7e651e2d5be")},
                { 2575000, uint256S("0x0000000001694a3451973f715794fc118eab4ec5a34d42db2e1132913718052d")},
                { 2580000, uint256S("0x3f1db05eba1812949292b70882d598c4ca13b4dc05484992d4dafeae269ed30f")},
                { 2585000, uint256S("0x9fa92a2966add1bd5a4aadeb7475c3e5d51d12874083dea4d1ea2d0ee162d94c")},
                { 2590000, uint256S("0xd3be3f6396cdbde78a8cf46331edc79473ac021903b41adfe441c0d701a2d8d6")},
                { 2595000, uint256S("0x00000000573559f544226528c49261358afb1e8d5faa502a5364ddc19e2ce496")},
                { 2600000, uint256S("0x00000000539b5208f623f989781a570ea5182817d76edbe0968f3218690b65bf")},
                { 2605000, uint256S("0xb18fadc5c2012c41c5ee07c2f79047ededcb12584d59bca597270c4b8dd5715e")},
                { 2610000, uint256S("0x0cecaa574b26a41e370bdf9b527bfffd8d77929f5303c25b804e192edaebbd23")},
                { 2615000, uint256S("0x7120b04eee60bf6eac7c4f946297cb4c5bb19827a1eac8a9f038f63eb50e4618")},
                { 2620000, uint256S("0xb4586e63cb38a4729afeec6d4a73b2faad3ad14e05f1dfef342dd23f09190e46")},
                { 2625000, uint256S("0x000000004c1dae26fab98f3157a72b80c8aa4c6e67caa62d898498f6b18b9231")},
                { 2630000, uint256S("0x000000005868b391369b896ed3e75878e3b41d0744163225607648dc7870e206")},
                { 2635000, uint256S("0xcb9df4e75d44a23632da7b81b8f87f510ee08842bf5e90a4688de6cffc542489")},
                { 2640000, uint256S("0x000000008236f74cae853acbf7693a80f11bc7fab5aed19c3278602949f61caa")},
                { 2645000, uint256S("0xc1c40f51bf0878ca76369b368f6196b9d492b905e1bc866cbea384c0f13fab60")},
                { 2650000, uint256S("0xd264d67121331f674c8ca7e01a1f7baf46651de27b06627f310a5942cb2376e5")},
                { 2655000, uint256S("0xbcac3cf167715a359a6ce4486a0a14464dadbc8badbd096a463c3dccec1685ec")},
                { 2660000, uint256S("0x000000004b6756ee984661c34a62615abd57f07b13bcfd179a7ecfdfe9af507f")},
                { 2665000, uint256S("0xe1b940b357d12bf5355beca32e170f7bb683df1941eb48a999e3f76640b06e78")},
                { 2670000, uint256S("0x00000000128b72147587d56360be19fabaeaa2a70bbfc14f97926541255208eb")},
                { 2675000, uint256S("0x419afb49471dd8e5a4191d8d6b7f797ec155884bc7e530c2a2b2c3ab12c592ce")},
                { 2680000, uint256S("0xbd07fcde554b2b90d36253531b3ec43fe6fd4009c9300cc2d55713ed61ed3c31")},
                { 2685000, uint256S("0xb9057b760eb5a093b52d16d451aa2ab19cc4e1c973da5598fb208e07b6101336")},
                { 2690000, uint256S("0x974ac7b21b00ad4c082cc1c63f0f4ecf38efd9120ba13674eda16660609d41f4")},
                { 2695000, uint256S("0x8b12a9c7d9381e1d329801622df58ff0fa2a5019d1d36ca4d8c7da5706149d52")},
                { 2700000, uint256S("0xe75706806d39e0ac37bc8ed39272a57244fc67281e6c97bb18adcd1257c56418")},
                { 2705000, uint256S("0x9197e523ffc06bd29b2e019d2ba92dab582746652fc6961eb087830377fe82c3")},
                { 2710000, uint256S("0x13485853375dc3909833f7faf6ce95d9506f5f7836a60dc87b07aa23bddbc0a9")},
                { 2715000, uint256S("0x59c9c5cfa39314285b7df42614441be45dd50d4fb28893836581b0c14f29ab60")},
                { 2720000, uint256S("0x5c04e556614c205a5e6eb8e41128bb552c58178c7e86f9bd8714177f421db51c")},
                { 2725000, uint256S("0x2d57719b304a1b15802086fb28b3f70f0fc11ffa952a01b7e5026752a6203043")},
                { 2730000, uint256S("0x000000004a35d1463aaa4a6c17fbba0b1d4c8de76caf2a919a8fde6c80b5a396")},
                { 2735000, uint256S("0x9d5b3d5f1aa959b4f1da2a167b37a7fca7a24ed2ebe6869470401237e0dcedf1")},
                { 2740000, uint256S("0x0000000065e70d40ce4506113ad3853aa91342f464235587ff011944d6ef03b2")},
                { 2745000, uint256S("0x00000000583b0227ff7845e99beb7b76f06bd1bb4f914caba4b893e7b77b7056")},
                { 2750000, uint256S("0x000000005038d5c0a84375d69078a26b7ce53a4ac356c47738d292f3603536e3")},
                { 2755000, uint256S("0x01ae11e9c5b386bbbcae5740afedd827bb2ba7f1679dc89fba794ff4550fa28b")},
                { 2760000, uint256S("0xfc0a7338ff89cd2230a90f73677c7c7b57836fdd73acaa14c1a01c025e120854")},
                { 2765000, uint256S("0x000000005ba3ac9ae335b9213d9e91bf56d1a929d47d9d0e61b19062254dc379")},
                { 2770000, uint256S("0x0000000015f9d7bd8e57bd5cb170449ab4cd60884625a769a022e03f0c307762")},
                { 2775000, uint256S("0x000000000a26dade336858c0421573c8145426ff5c9d0ffca720cccd97e32463")},
                { 2780000, uint256S("0x8229d399f626bab224e5a9cec2f42bb09a659d259379573123e16dc1e428b2b5")},
                { 2785000, uint256S("0xae68f92740c6024edffccd12bbfe899423ceeafe8aa220920908d66b36f3128e")},
                { 2790000, uint256S("0x97c62dc595ce90819337e9d16b8668ce2d74fac748749fe021f40c0f4a128381")},
                { 2795000, uint256S("0xa3567edd2a8b1bfe30a0c818b255e0554a7779df5be684b59a7abe2f58a75a3b")},
                { 2800000, uint256S("0x00000000270e95ce37c6b8b122671125b8d3939a30c7fbab82f81879e00626f1")},
                { 2805000, uint256S("0xdd287f9b2342d019d2446c93ad397b07d49903eb696ca41b25a85734e136e836")},
                { 2810000, uint256S("0x000000000d3e18fbf750d4a5c2ca4a19dac1ccf028e9970ab2820d9d15278861")},
                { 2815000, uint256S("0xfba15f17ef610e3ffc2bc6e2f5a43aab948f1bdcedccc58b9aafc3f8a479cef7")},
                { 2820000, uint256S("0xea3233a9453b14550bb7f47bbf8c32aba5362da268f8b20097aec4a1fc0188c3")},
                { 2825000, uint256S("0xdae1c8ab28ea789b1c8cce0fbb779ba157c2a8f65a9747332497871bc4b0963e")},
                { 2830000, uint256S("0xd4ae1b9f24d77e3e4f6b2bf5b556c4695b4f8492a63251423cc313b7915217a8")},
                { 2835000, uint256S("0x0000000071628e28ebfd492afd939967fda4f9414f603faea6620efbe7583d60")},
                { 2840000, uint256S("0xcce6cf4dd2918247f789523194908d7568509e4db6ac76351266fb9ce8906eeb")},
                { 2845000, uint256S("0x5b4566f8603998312b119c30f310c85e77b6981d7292688713330e227ebd6dfc")},
                { 2850000, uint256S("0x00000000413f6c20fc7ddf422b033920c0713973949e7e249e107f3c23c0aa1d")},
                { 2855000, uint256S("0x2d65fd09e1e120e2d85170cdd7aea58162f6ef26e97e03e58df74f61ebc2e290")},
                { 2860000, uint256S("0xecf7c704d7e013518f1caf1562eb60421fadc5ec66df40a5be973d112c7ebe3b")},
                { 2865000, uint256S("0xc8b82a6c23920f162f3d9752cc04502b9bd5689b909256510f2e4e9ceb22cf66")},
                { 2870000, uint256S("0x000000003d2156aa1785aefa235957d5dc70fe4beb7fe2865cb2b687709d9ddf")},
                { 2875000, uint256S("0x0260d6e2e7f7b5622048bca8a22584fdfc4273915fdac34ddd5b51c759dd175a")},
                { 2880000, uint256S("0xc86ece54659ebc70c3d05db523799420cb86bcf87ed4f50a70699c2e13ee0090")},
                { 2885000, uint256S("0x7e1b9e2a46d0ffa53a57c2f565452bea0b80618589d4b5a397b9868966cf2f58")},
                { 2890000, uint256S("0x00000000171ea3ed851d5fd939feaabb335b220cb4a1eee3e2f0de941f2ebc0e")},
                { 2895000, uint256S("0x0000000046964a2b1dd7736c1daca9d153d73efb8a64c7c77bbffac9fea39cc3")},
                { 2900000, uint256S("0xb1b713a52a4afde38419b9ec57f1f6b5a7b2c92641c393428787692672a649a9")},
                { 2905000, uint256S("0xfc0e23f4cecc8bb6f0925c55aae0a3d7ad82eab8df09da2e4389240821f89973")},
                { 2910000, uint256S("0x7a09e8de3ec793f9d718b2a8ab044a71872275a6b99d9213c63ca20cf0f3465f")},
                { 2915000, uint256S("0xbe97c5615039c6f778bcc9e065a72539c1ef79c5253ffbe19ecc29f860dbe882")},
                { 2920000, uint256S("0x000000003bba8303e29864fa25aa1758b5e0e58da1fc0cb2215b357c26aafe26")},
                { 2925000, uint256S("0x000000001b6713936b3256461fe377a7b8c3e867d7fe38419833ca8e8d804081")},
                { 2930000, uint256S("0xe05e57dec4fb3c7a77d844ac5c1a9508f5f0fa81a2cee642ce87ae19c3dbc2e0")},
                { 2935000, uint256S("0x00000000924234b196b49891e43d5606661d05bcae95257b7fc86b22a29ac4f0")},
                { 2940000, uint256S("0x45ab542d728ec95c7050b61670fab873e0ebb9f1b78792119acf363d8e9351e2")},
                { 2945000, uint256S("0x00000000880dddc2ddc432bf149017ff7009e667a0ae0601aa6af680d1589108")},
                { 2950000, uint256S("0x23954f1cab5ed60319e677f1e573d307d471b9b99c5327d5f2d35ec9448349c0")},
                { 2955000, uint256S("0xfa1662f11dfedc735ec1d8b932610d2a95fd00776c69973356aacfd817476a15")},
                { 2960000, uint256S("0xf9e5c785b7dd3c641e46935aad0fc312e793ac8059bea056a0a209d4ddd58019")},
                { 2965000, uint256S("0x39c7bfab195aea6f2457c0b7d3016b142460ebe68e202d1488fdcea57489095e")},
                { 2970000, uint256S("0x1ec73ea0e57ec0ed17f28542d26cc55e8d09282865c79cb27bd66eff65740856")},
                { 2975000, uint256S("0x000000003a749ab6068c3dbaab44335921bce8a1b6c9ad2c54f3f63d819b8171")},
                { 2980000, uint256S("0xd1c053ba0d33dcacfe8511f176121226d6be5d487ae7748f04f4d6767b43a8bb")},
                { 2985000, uint256S("0x000000023f46e01a020c33656c85a108314d3d8c4db3b886a4a00093e590de82")},
                { 2990000, uint256S("0x0000000291b617ecdc3b3f161c0456f859e0ef01a6579ba302c7c3dff582ca1f")},
                { 2995000, uint256S("0x0000000160531220ed52f1335f48067af808b1e373cf615aa5cbd1ab30aaf4c1")},
                { 3000000, uint256S("0xf2fa99d797c2d8717f42c1accb1669757387aecb606689abe78746f649fbcf9d")},
                { 3005000, uint256S("0x0000000056b040bf87aa9cab9e923cbb637d7fc02d9386905dfc2fbebeadb443")},
                { 3010000, uint256S("0xdca3854b3fae7beb914fdd898d2834ec6b01f16c90685b763f29d53991eac537")},
                { 3015000, uint256S("0xf8288b71a0fad22c7764882d6ee4b04e55cce95f347e86947a271d563c37cd77")},
                { 3020000, uint256S("0xdf5a7fba78a0ea2fca2d50a680d76656f9bb163f995ce6deadfb1926c62846a6")},
                { 3025000, uint256S("0x0000000116c1ec8bc8c0ffc3b52761dc64d2cff9748066892bfaa077df6848f1")},
                { 3030000, uint256S("0x0000000075dc24ad8fc15de2a9243ded0a660a74c9740af5f540d72558b270f0")},
                { 3035000, uint256S("0x2fba3ee3d274ba5cd596cab463e7a49b8d76fbf22cb960269795fead13e9daa6")},
                { 3040000, uint256S("0x6d63a14235307d3bb6592c37ab8337c310204c109a4ee47bf2b37f2c5202e319")},
                { 3045000, uint256S("0x00000000278375fc14cfa0928fb0c2c31396be0ec3c3b16cd777fc82afed17b9")},
                { 3050000, uint256S("0x0000000053d18d91fc7d8cf1a858dca10702b945c93fe8b08f0e652422b04b8f")},
                { 3055000, uint256S("0x21c5f24c64b0ae015dfd5b1b65f852814d511ffd9cc1b2f1609bd0d74af0654f")},
                { 3060000, uint256S("0x5eca1b4b5d0dda0d79ee15e4a875197b256c930e654ac4acc6ce69af792643b5")},
                { 3065000, uint256S("0xad7658c74dddc1bc55c36da6ba255ddde8f8309ce5b26d68a7c707381171fd87")},
                { 3070000, uint256S("0x0000000024a89200cfad7fb3cead79255b9210513abd3244042f4fb296ce5755")},
                { 3075000, uint256S("0x4dee504bdace2f9938322245a3956467a7cccaac95e7195aa61d8146fbe3f69a")},
                { 3080000, uint256S("0xe25d22f4672f3af2a566c5e04c0300a1f5e10e909b81ab5664774ca96a606cb1")},
                { 3085000, uint256S("0x8fd62d7e63d573c66f0249f028af368cc7e1b560fcc6cca1802ef39e4cd7c221")},
                { 3090000, uint256S("0xac5afb897db5b6236484c024c24909c4374fe6a53acf054bd3af9e89613558ea")},
                { 3095000, uint256S("0xe0e649c3792f1f225f71fcb40189baf3fe8f048560699c3e6008f36f82aa7438")},
                { 3100000, uint256S("0x58401cd178b6c45c419fe2213ea318f2f1530ea35c420af8664c2f4597f94002")},
                { 3105000, uint256S("0x95277763b014ca1b7fd864a6e3af86b3c78ad81a4cd60e6be46a89e388fe3c1b")},
                { 3110000, uint256S("0x000000001466a9bf394e0dae12b78fe03f8cc99d9b9fa57c9fa6c51d14973346")},
                { 3115000, uint256S("0xbe99f91acdcad18a7e99cbae5cbb1757ec86177df5281a7ef88ec443784ab2bf")},
                { 3120000, uint256S("0x32c5ad16d4c38b25e4543c51b7d53b3a84969bd9d1b121d84c42d9d11a24dbed")},
                { 3125000, uint256S("0x1b7d1fc8007120da085c632a8ded36befa5efa60d79ee3468923a7b143710035")},
                { 3130000, uint256S("0x15c820f83636bee011de4981bf72f4d58ded279ec080f0744bbb8d5efebfda49")},
                { 3135000, uint256S("0xb4a59d91e1c797b989519b67646738814c0b9d115b5674a1418d8a3d8928a87b")},
                { 3140000, uint256S("0xc997e20dd1e44b51ae646f05dcbd2d51748fbed9e64c81734aec941b468735be")},
                { 3145000, uint256S("0x375d264e45604f01d097cf8378b91643b98b80012e3f9dd35493262574e062a3")},
                { 3150000, uint256S("0x0eb997468bddc5f14a208aa7d18efea7aabc0b97fda2cd2e07438c84d994104f")},
                { 3155000, uint256S("0xd41f3e6f9be3daf835dece37dfe4948126d0102c5328bc84c744329d04a1e90b")},
                { 3160000, uint256S("0x000000002c4532a41094fd0e73984bc7bda8f0f3f5563da3aed7f82d68cb0266")},
                { 3165000, uint256S("0x29963901c6d50d499c9cf53ed0c3ef38701e8ac838dc2b834a58950cffe91096")},
                { 3170000, uint256S("0x65167a39d2fb8737c515504289881077f4de907399b7f23278b3e4762d3901be")},
                { 3175000, uint256S("0x9223c9f33b67f6e12469fadcd04504fb19639b77db3c5cb08ca92570144ea33d")},
                { 3180000, uint256S("0xf9b7b8f31e2fe97a44c85cf114b7f39804f3682ad23883449aa72dc995eb384d")},
                { 3185000, uint256S("0xa5b9efd44fe476cfb2f020590e215daa282684a66a2390eb6865a1f2daeb789e")},
                { 3190000, uint256S("0x000000000b044eef6f71734199bc6082f898b7e219f3e52133eaf742bd79d5a9")},
                { 3195000, uint256S("0x00000000895848f9a944b6d638bf29346e41dffc9e57f2deb4daf268a68afacb")},
                { 3200000, uint256S("0x0000000046820fcfea522b55a23c0d24767d85877e0423c5fbc1cac9083283b1")},
                { 3205000, uint256S("0x9438334daad1ea5c086c29926c91c34a5e69269b853d7b88a20ce5dda6a7d604")},
                { 3210000, uint256S("0x00000000357710e28417508be4ef35cfb9d8bd310f106ce0cba9f0d4883ac993")},
                { 3215000, uint256S("0xcdcc2cb3c45ded872bf712f09dac45da5c2c1a9cf84dd3faad348379cb613dd9")},
                { 3220000, uint256S("0xd7d6d0023543186dbafd82189a2d2ebae99af37ce2cbffd459d852b9824386b8")},
                { 3225000, uint256S("0x68143b43f278507401cb9cd2f566a853f06bf4f91cc79a0cc6c845d28fa1b8e5")},
                { 3230000, uint256S("0x000000007fa1da8da1d27132d52edd0af9ee677ee902f1120caf32f9b7c79d30")},
                { 3235000, uint256S("0x48c8c7f9430bbe4af8080296319c52dbe8ffdfd48d831917b176c9d6c08b26e7")},
                { 3240000, uint256S("0xe0dc69f87f521d8a3e6f73b3c526770655c567b487529d55f276195fd74cb97a")},
                { 3245000, uint256S("0xd6b79a6ea93b7831a9418cdc8660be65bef4f13bed2b7fd94975547a53b5ac11")},
                { 3250000, uint256S("0x309f41e8404de76e98e3adc9cc82c388ff9e87b27861f09cc30048b44ec5256c")},
                { 3255000, uint256S("0x000000002feb6b77dab082cfdf147074bfda4a0734d344500475fcbdc5fc9175")},
                { 3260000, uint256S("0x000000001a19b7a1778b0e68511d725c3f6fa092ecd6fed57a245904a03723c4")},
                { 3265000, uint256S("0xeba20adb110f252218a9cee967cb42b21c39b6c7859163dc2a470c4d2007e30f")},
                { 3270000, uint256S("0x002848c121af2dd84a9f9d3e2ce19d1725b8dc6b749c8829bb219fcd984d18ba")},
                { 3275000, uint256S("0x949701768f1ac1eb50f10cbafc35a9cf39bd5854ce77b27f30a430da22b9e2a3")},
                { 3280000, uint256S("0x000000009522b09f39d082f0830bb3a4b9da4b19fce0ee6c851a9004798eb0a6")},
                { 3285000, uint256S("0x1306008af5eee68e4fa2e2a6501af33d671ef72f40e56ebdf65e618aa7408848")},
                { 3290000, uint256S("0x2afe25f5bec54ce51f9018b34b046ce48f3817580d6a7fdd4fdfdf7a8a19643b")},
                { 3295000, uint256S("0x5f8b9b57b1836c41db801fbe44b2170189cad7af01828ab0e5f231ebb6e7473d")},
                { 3300000, uint256S("0x00000001ee9eabc5a3e5ed9c53ef23a353cb23c68a72fc14cd6ca220ee49effa")},
                { 3305000, uint256S("0x05d4709b11456568da1e606925965d2fcdd7aa0e84f9c69787baa7c297d9767a")},
                { 3310000, uint256S("0x58cffb9b54604a5f0477b0e544ff02a217593c01c7dd42b88c7d640b8cab08d0")},
                { 3315000, uint256S("0x4b6c75c3d95d492809704fa5c5a2d2e5b1e02c3d5dc0f8f8a15bc91713836866")},
                { 3320000, uint256S("0x0258de3b2e3e426adca17c3c4f734209824d3fc3951ca64070c5d0e23bb4ba64")},
                { 3325000, uint256S("0x34c1850e54b1bce766f1ccfab12434456656d42385d32f14f21ee373b52aff96")},
                { 3330000, uint256S("0x000000000a2d5a66ab84138b7f3548e99b324792c68955af51aafe80280fd305")},
                { 3335000, uint256S("0x9f6c5b843a6d328207d63d5dfc9a4917ac0500b869cf9c17168cf6ca801418c5")},
                { 3340000, uint256S("0xecce67fdce01eeec90b7f09111673a1912f8b1b4692d2cee825041f3dfa0691e")},
                { 3345000, uint256S("0x000000000160e71b09871b24041e280083b08e488ab6d4e0df5ae9b5bde33252")},
                { 3350000, uint256S("0x40fa7c5939d78879bfd331d3b41bbfef88da697f0dbd0c15a2c735922e49faad")},
                { 3355000, uint256S("0x9180f750f270d3da85c6c6924624929df1e8b4c6c6ff9e3aa2cc8b1d17190553")},
                { 3360000, uint256S("0x15068e8b389155844225ece3427654986940947db548b1572a70711d00b51bb2")},
                { 3365000, uint256S("0x00000000728862b22ac05245171a62bf9d7c010aad8033754c3a6883176c5926")},
                { 3370000, uint256S("0xbfaaf938d860f979f451216d69255e3bdfe97bb219fe86f9d1757804f5d60946")},
                { 3375000, uint256S("0x4cc7820806a778e67900a5c9a774e608f9f53a63262e6ee04f8b024e3165c5f4")},
                { 3380000, uint256S("0x000000025a24bc2f653d33c830d76f2027140d2bed928c2c79d019451695524a")},
                { 3385000, uint256S("0x103d99f8d8dc9eb791688e715b7c29796f24b5264c514d90a3c5556100ae4323")},
                { 3390000, uint256S("0x00000000985ebff9985223b98d6158c5f8a0b9b81fe7eedb100e38341bc61964")},
                { 3395000, uint256S("0x6e243f77c5d8fa44e7f468b57e9c1e3af9d87f1154e6d9a37105796ff5462999")},
                { 3400000, uint256S("0xc644964df8b5280d8bbb9dd79b85998d264e0e3aaac2626ed40c851ac6265952")},
                { 3405000, uint256S("0x7fdbb1a8f74988d02c97d387e814177005b618468775a3d9b77d69e9bdfec217")},
                { 3410000, uint256S("0x000000009e11a922e275bed1304d7c5af7ab9f0cc6ad0286ee1b28cefad4b854")},
                { 3415000, uint256S("0xb988915c8fda05ee7ea151268dcc8ad168fff41a4a60f8147d5348e81960889a")},
                { 3420000, uint256S("0x27f36927fb85fb65e6ac171776387b90e0618a4b38fd9a335b88b8e9b5c7d15d")},
                { 3425000, uint256S("0xbb37bac5984fd2d6cb79dc476a0e11f78e7bdfe69ff36ebebc9db37b2eb630cb")},
                { 3430000, uint256S("0xda8d85d17c002dde1650d8f2ae5eea0328c6578baac560ba874e948b4e3e2dc7")},
                { 3435000, uint256S("0x656fbebb32487e9dc991b103563ffb748fc5a2e0239f9bc20455299d10868649")},
                { 3440000, uint256S("0x00000002dbb61da10645c568ac48fb47592688c57f762db06e1f1ee43c288139")},
                { 3445000, uint256S("0x5fb48ad4948cb84fa992a8749a3039bbbf4ad9f7b8385769d88fac5307e09428")},
                { 3450000, uint256S("0x8ce65dfa5af783f38cf476bb5c9abcba962ede13606c3a29dce610dd5c74e5e4")},
                { 3455000, uint256S("0x00000000644c84427acb3dfdd5d9570554861acbb266268ab0057a9e9c835175")},
                { 3460000, uint256S("0x5159b3f85133cbcf859f27209bde8d1c7820a50edfe9cc0300c6c52daedc5f1b")},
                { 3465000, uint256S("0x00000000f21f3aebbd5bf894b86de6b4ad85977c125ac366eaf002e3f273b29a")},
                { 3470000, uint256S("0x3d1610d2b56ea94c2413c8fd52ab03e96de74adcdbcefcb7f5f87eed0f5665f0")},
                { 3475000, uint256S("0x1b472a82ac2c75bbabf0cf05d815bc77a3aee82b88020eaa2fd1c76bcae29852")},
                { 3480000, uint256S("0x6fbdf98f3901dbe38f80930babbb06b7c6b1025ce68e906e00e194c7d6df72de")},
                { 3485000, uint256S("0xca5926d3999ee3952de5275af00759152ad329a966b8bf220b4357426f654d7c")},
                { 3490000, uint256S("0x6a409410b26129fa364b2bc1603a9caaed1e48bef5966eb30d18fe3e4acefb58")},
                { 3495000, uint256S("0x293cab69407b3b17df1d848b07ad156101b54b79e038dafad8272a4a9b7dea04")},
                { 3500000, uint256S("0x000000033a59fc2c35e41e718d9daf447faabe294759ee8ca1a0e79287ac9d3a")},
                { 3505000, uint256S("0x00000000f232c81b87f181984175e9a1d270cf83b46f6ed60f686839e448d6a1")},
                { 3510000, uint256S("0x03c974a054f6c1f53b378dd0cfaedcbdfe65bf18eb52218b95271a1af8309076")},
                { 3515000, uint256S("0x3c0f7d5634d1dc40ebaf4fd408c98e5c93057a451fd7060c3e90f52f045a51fe")},
                { 3520000, uint256S("0x3d08a34193e5c86399b96478374f42e2bdf8a7a3009f76df4418034500f7cf05")},
                { 3525000, uint256S("0x2bea8889fd5c58e1602a2623906b630e93b18a5f9d0d14acfc3526baf5757b1d")},
                { 3530000, uint256S("0xc18bc65703f80842768c32e5a7820d61fcd56c5e2172b023aec7a2868868ba64")},
                { 3535000, uint256S("0xda63ea629f6e3cdd9564711086e82d801cb3b595484c66a630414bac9d442fe9")},
                { 3540000, uint256S("0x9ee49d45a398bfb4db3a3bf284f753123a5b29af21919ca3f1e903121bfd16a5")},
                { 3545000, uint256S("0x48b37932db10c9e49dda14c213a28fd6a4e7a82b1c63c80f3bd438522ec01ab4")},
                { 3550000, uint256S("0xfe90451e4a5e97e2336174e0933c5a7f03887794e48b4cf49a14580f9b56541f")},
                { 3555000, uint256S("0x1596a053f4104d4286fc60737d91d129ca9069e3124587188f202b564d4a03b9")},
                { 3560000, uint256S("0xebd19d9f3e9c54e52cec3e0988b0ebd2bc2e65d97bb70efdf2012afdbab76442")},
                { 3565000, uint256S("0xf6ba49c82383d61031de35bfdf614cf3d262df69008564f0c97098098e8434cd")},
                { 3570000, uint256S("0x08c756e4b84aad8079f926dcf364f58f876a8a8f3d13c61db1ce5c2002404a84")},
                { 3575000, uint256S("0x00000002047cae3fc52e178b9271ebea9dfd919e1684b5a9760885aea1e517cd")},
                { 3580000, uint256S("0x22e2e4809cddcbc077c56ea4201bb3f9338141d848cd4f7aef14113ab463d7e7")},
                { 3585000, uint256S("0x8bd10ff1ddd9441bb91bb5d416240080f846693b3744b26e6deeb6c26cd824f0")},
                { 3590000, uint256S("0x00000000968642e5556d558083627398d526819cc9f7d081e0a539aaee29b4de")},
                { 3595000, uint256S("0x0000000223435efc8ecf156a5c2dbaa655489ed90f90ad30540fea8f708f873d")},
                { 3600000, uint256S("0xa0617736127c8eed20403cea0f329dbad25cf0a5e0b304263d968d1068662bd1")},
                { 3605000, uint256S("0x00000001eeeadc88ebdf758c57ad356e6463d2be03d88c3e0fe69c018577b169")},
                { 3610000, uint256S("0x000000022deb316c8ea8e3d73f5622b6e39588c99a480ff7bbbe3764594d7fe9")},
                { 3615000, uint256S("0x3c97e8877bfd446413e8e2f62cf62fc4cbbb64e010bc83d9b6f77e8c9a95b14f")},
                { 3620000, uint256S("0x40797254fc29132768c7a0f9d9b2b525276d4236f87615dc187a060d0cfab11a")},
                { 3625000, uint256S("0x4550cb94b50178560dcc4f3e79ef1bfa212a3ff38a8d02af3b822e30efb1a65b")},
                { 3630000, uint256S("0x21086b16c8dd5d6fd7a2b897a3353b888476870e5147173d5c60cd17ff2f5dbf")},
                { 3635000, uint256S("0x80a02f4f1d95703631172230ac02d15e16321ece72c1404f0c65df1602e5d9bc")},
                { 3640000, uint256S("0x000000024db4f21bb19374d0d2f5e23bbb3630d1b106841994533d1e71f2c16e")},
                { 3645000, uint256S("0x16b996b1c4daab313413d5fdfc181d71b50d23e8df6ab04c9c1b3d82f23b985f")},
                { 3650000, uint256S("0x7a2cb97f0585e7d9894c1d6559c58a8df36fae4e8bc3b680d2d5941d3e4f04ad")},
                { 3655000, uint256S("0x794098a3dafeca2f0174e875851d81cffab7f0895c1359b6ce87982023b1b289")},
                { 3660000, uint256S("0x4a2ee58fa47e50326bb0701cf224dcdd786a0b5fb337a0dac66559661deacf81")},
                { 3665000, uint256S("0x00000003e820d5ead3e8e97be07d649afe84ff688c8c938375cc14caf0e9afd4")},
                { 3670000, uint256S("0x00000004f51624603549a3ae20c915c7a091496e8503438b4d442e89e9a088ff")},
                { 3675000, uint256S("0x8980b2cf2b700bd8a7a4be7289cc4a525f4002253b8adac4d7d078d84a57b662")},
                { 3680000, uint256S("0x000000052e0c77ec79da3f1e000364cb23983fe41c1e4268c69057f4cf4701a3")},
                { 3685000, uint256S("0x199bb9aeaf88c38160f61a74a097f912114f6d08307573f792d0cc8457d7e758")},
                { 3690000, uint256S("0x8c1d22fd25b9db025be7ce32c02c851265942714092c1a052fcb230d233d02d5")},
                { 3695000, uint256S("0x2a9eb631e538d81252c33c488b7352cc84ef1769d9cd0133cf46df302af2b786")},
                { 3700000, uint256S("0x00000003ab580f6a9e817222ef602e7a07417eaa764edf1d418d8b0b5ff6c77d")},
                { 3705000, uint256S("0x519336b3117936fe07a16298c16a972b683fe4e60a0d100b6cb6389a06f59f8b")},
                { 3710000, uint256S("0x31f8a28cc32ddc2324a2fce5654efc3228c1dabb3f97e417b587c30820e492aa")},
                { 3715000, uint256S("0x40b07eafae4c66ef85566a23326297e7ac66f0154e69861c2dfed48316a7b76f")},
                { 3720000, uint256S("0x000000005b5c29d2aeb990263aec699ff1ba6391439248ff81ddbd197bfc89b3")},
                { 3725000, uint256S("0x855f20c30e4a445dc985f54c66c3bac324b34d6c5cdd3ef3a5e9f2c97031c563")},
                { 3730000, uint256S("0x3d6acf97fd9e0af6634be3a92750a338c054cd936e6ad0e2d620cdfca7e0f0eb")},
                { 3735000, uint256S("0x8bddb1bdcaeff2b8502afa6d690fe5ff40c2d40d94df4973377dabeb1cb43abf")},
                { 3740000, uint256S("0x22ac3c2709bc1ba69583d2c4a2abdbbaeec5808b044c8b2900848d6008aa43c4")},
                { 3745000, uint256S("0x8d70ef5f13f2d2101d728274a0c879b4a546fc740602ab35b36dbd631fd153d9")},
                { 3750000, uint256S("0x5e7dc5d405ca488a3b0b01d4f76571b04b9eeb654c6716a38177e8f6050a3e5c")},
                { 3755000, uint256S("0x66fd92cc0de0ce2da57cf746e6d24ad63efeda5784e9cda4f65fabbb6d98f5ba")},
                { 3760000, uint256S("0x6adae1020cec6bed609751f763b68af73c49ce4a8d620578ed496d0a93bb794f")},
                { 3765000, uint256S("0x6cc4580f4d09582e6e80e4c94008a2084781d2bc0be80e90a82fa6d25eb68997")},
                { 3770000, uint256S("0xf55938084fb243d0d790b84ef7a03546a6797a76b07510855c2c6d883b979018")},
                { 3775000, uint256S("0x00000000d7cd118c905f60f5b1f8f9a1fe1a2b3fef6dd8284e0c92858e962ad2")},
                { 3780000, uint256S("0x0000000293e7a7855917d41f9f6be862d659d7ba21e6c5e13e94a4dcf9158c1c")},
                { 3785000, uint256S("0x2ddf7dd81c3690dda974d165844d388c617cd070603d26dd5b36fcb2d3bfc359")},
                { 3790000, uint256S("0x16269364d61094948098933df854baea1ab9ea54ce37becaf4d9cc8764763666")},
                { 3795000, uint256S("0xf255e887eb1bb4e1b4a484a5382b6e511628d6e6516224e8516ba1b9aac33bf0")},
                { 3800000, uint256S("0x0f39e6054f02650291fbace84d3dd276d21f156224925d3ed19968904a93de30")},
                { 3805000, uint256S("0x042ada6424a7a1f1718dc71da86e755808113731f53c2c8f35799c91789631b8")},
                { 3810000, uint256S("0xa81efd1c0a53dfb7716b6c42c59cf67bd7ef125d51e0246d540c5e9f225888d4")},
                { 3815000, uint256S("0x0000000148088b12e2af20ed1638c0fa4d080aec4d8eedc176b93f255a1b73da")},
                { 3820000, uint256S("0xbf556a6c12e0189fb6017d9a0ec8403afeb38958fadaed9edb66bed02cf6a308")},
                { 3825000, uint256S("0xae04bf226447c5080c72ebb5115c97e25b47a6e7da8e3e483582908f388dae4d")},
                { 3830000, uint256S("0x00000000b54f9dad6816db6d97b44c4c10359e131aff536ddc42c33263120448")},
                { 3835000, uint256S("0x0000000066b8099198822d3b37e6c265efa58c631053744bc382372338e2d4a5")},
                { 3840000, uint256S("0x48da8663d2bd2897ca1421b12bfa0f896c2e30f6bb2a8c8bf956be048b4109d1")},
                { 3845000, uint256S("0xb0ba1878a095792563b39ea6898cc5547b397f4308bb22ae31be8650acfa4dec")},
                { 3850000, uint256S("0x985a0c1aab2af4b73a015019b618a758c0f412bd190ff596c7b29cd6faf27569")},
                { 3855000, uint256S("0x42a8d50c6c6f017ac5f3998cadcfae687595e75867f5771d20d71c6bd18c2df0")},
                { 3860000, uint256S("0x9ebcca9dd95b06e725ce3c010522946ca72f04749b53ff20636804e4c52b1ab2")},
                { 3865000, uint256S("0x36ae7907f7467897067061e1bb86dbcf7ca9f5d1fcb47cf5bd1692c7e8508f0e")},
                { 3870000, uint256S("0x8f839eba6a911b754ad468d3d0bc4318cd50c9939641dc508278a7d08edcab24")},
                { 3875000, uint256S("0x0000000217f9c25dc143e3981dd746d054058f3c53556fa24e277ff017f04c81")},
                { 3880000, uint256S("0xfd093efdff723a28849c03ef7a5664cdfe6d217950795cf2c7b64aef88bb7330")},
                { 3885000, uint256S("0x0000000250888b2444c1834012f031b74783d4fcf92630f3a1577c12d8054d24")},
                { 3890000, uint256S("0xf53ed76a272db4bf4c08c00d810d46ba4affcb86dc0b499ef7b755bc773b9b27")},
                { 3895000, uint256S("0x05271fb92a3e5778b90495fde5b4dbf451a81aeb80aa83832b1e558e26f8ea5e")},
                { 3900000, uint256S("0xa191fbaf34667cf7fe902c40c24b0fac70957bdfc9cae4036d721e814afb4b51")},
                { 3905000, uint256S("0x0000000074321c4ae819e08c653141c14d70a866218295be0b35c79a2c1de95f")},
                { 3910000, uint256S("0x00000001ed83c439ecd6b49573e1d415d645ddd05b6e331cbaabe943ec1c3090")},
                { 3915000, uint256S("0x000000016b34ea0f578c2d0eddcbcf6fbee19dad073edcc3cb5a5b315421d831")},
                { 3920000, uint256S("0x75fc76a400fc3bd13b3c39da97e4a1abed3f625ee6647671d5a3ec44e794b0c3")},
                { 3925000, uint256S("0x0a8ffd821fada6419afcba2c622be867eab401d65755d0732258f335f22a584b")},
                { 3930000, uint256S("0x558c4359d3827f120b87830de045215564701118f381dd6335d63d57b932d5d6")},
                { 3935000, uint256S("0x223f865412de76254e034bd6f7c6a62467c4e4f36e28cdfbd8c4fb772a647dc3")},
                { 3940000, uint256S("0x00000000306ae6c905539d42e15fd014cfddc77372cb65a3f88abd6c9f6351ef")},
                { 3945000, uint256S("0x893e94b28f0fd0fe84549906a7a99783405a442287f799d1ddd2e7a9f1627c3b")},
                { 3950000, uint256S("0x463fa496dc860f2b9804716c088bae08625ab0dc3fbe2555ebdc9f302103c4d1")},
                { 3955000, uint256S("0x1a68169ecbff0be0493894b21216d50eef302cba7582b6546559af9ab539cc33")},
                { 3960000, uint256S("0x032649eb1847a99586438ad71657aa936fd02a39217a8a382aaae9b8184bea4f")},
                { 3965000, uint256S("0x737c857b89b55adc271fd077e504aa7c721ae8b83c0e971041f39cb61969e958")},
                { 3970000, uint256S("0xa0a986e449318e30ccf3287d4af19b7d6acc38e538e80ae06eb7947d76decb04")},
                { 3975000, uint256S("0x00ad242bf9461c7e1db551edf35fe627e3160cf3d8c68d897f7c09e8637248d5")},
                { 3980000, uint256S("0xbf6c101cdf320c3bc526dc84b8f18286b8cd090d2b5f5b3aa59523eeb248d239")},
                { 3985000, uint256S("0xadc68dc49299d84e0fc435aae9b9bbcc6e80bec2036b19d6ab6c018f7cc14601")},
                { 3990000, uint256S("0x6a033efa1e44a50d92f48ca804bf11a5dcd0f778f0dc22e823e886b7f7a29dcd")},
                { 3995000, uint256S("0x93f370800c08fa990295aef119197183c418d0b03d5040791f6d424301598577")},
                { 4000000, uint256S("0x92c1e79830fe00bf8c9d31d6cff072c89c395482aeac5563ebe8ca18d509e963")},
                { 4005000, uint256S("0x0000000237a6d78c1d17b602ef8c9a601fcf2061b7354e4dcbb9002ec917016b")},
                { 4010000, uint256S("0x0000000434b46b6eb434da6568155d51f5864a372482b73b04d118b9902685e3")},
                { 4015000, uint256S("0xa1e3261bf2d42d64fddabdab3920e1c5a33610b5ae49b02e3cb33698d46e1c78")},
                { 4020000, uint256S("0x25e7b3d53069de4a755b60b070786d83dc2885314d466877194a9d054e15a5a7")},
                { 4025000, uint256S("0xcbaac56c05882e104644fcc7c2aadb5138794a3327f12afb9ce776d80726beee")},
                { 4030000, uint256S("0x00000001fc5565e6fade4fcfda6fe17d37a03a271733e4e9a61ef0afea03c4c1")},
                { 4035000, uint256S("0x6af1431df4b07498f677d6dbaafa28d80d2f7789e1861d0c94d68d1d223c9adc")},
                { 4040000, uint256S("0x000000004f0366bd7aa17061698045c79c24f9ebc1e760d7f556b7e07b3d5488")},
                { 4045000, uint256S("0x00000003fecd14e9e49b8af3b58f962f1c6459d9c0554613e4bd6119db27effd")},
                { 4050000, uint256S("0x00000001b7c2c1f37f9526b0abdc26357c474c49a376e6347db24de6b75eed8f")},
                { 4055000, uint256S("0x000000025202da9dabe301b66283466d903b62facca187e7f82ce2b71629fb4d")},
                { 4060000, uint256S("0x9bd0a0349a9e33ff745b0f923b0f5402ed62815d44b2409f8d5cad8f9100d995")},
                { 4065000, uint256S("0x82207a6d9e37e7453bcdd6e6f85c494697c451571028e03b09b2ac02edcd1f58")},
                { 4070000, uint256S("0x0000000280614a8f3f8c25da8d59c880687ddf8c52e96c32253fab4a93ddb8c0")},
                { 4075000, uint256S("0x076f55f527dba074677f227250b1261140f9cd79e934a5c379e7d4163203c700")},
                { 4080000, uint256S("0xaf3eba9661375e8b4a601b9a40098f5e947d2aa32dce52ae9f62a5390fa60781")},
                { 4085000, uint256S("0xc5fd055550991d99384c045f0582a9f472b7216b5e3f77780db8305db6c77959")},
                { 4090000, uint256S("0x73e21a2bed7a3e00f3e920d6238459c00a8652e287a5ddabdee4bc0e485ef1ba")},
                { 4095000, uint256S("0x00000002d0a411cc56dd2aac3fccbb708311453fe9888f802c08d3c893ccdb11")},
                { 4100000, uint256S("0x88effe3c212ce782facb58aa609af076a369200e2562dd1823ad9b0224b828dd")},
                { 4105000, uint256S("0x000000001a83c481d07e7e304714065979f409b2f9a0c8b17599e3a38c1d911e")},
                { 4110000, uint256S("0x00000000c4de2603bfd086e1a382d04b84c8514ea7837bb5210346d04b35f914")},
                { 4115000, uint256S("0xf6d17a59c13754ba87b7f59dffec8832cf735f4b8b9ac599eae779f5de31701f")},
                { 4120000, uint256S("0xb347bd384994cf9d2eba89754118e2c452f5eb0f9d249ee0b35fe4f36727353e")},
                { 4125000, uint256S("0x30e62fd7b4df22692dcf8fda62c15a8cceb2cef2a5098d6eb6c3c8543cd3cffc")},
                { 4130000, uint256S("0x6051cf08f7ec9731da1f4b12489dfaab8d343ea88fd797409efa5cd90c0e2eee")},
                { 4135000, uint256S("0x7ce1a106bd4cfb3ef4cdea086bb2ac0de987c742d2310e4319eaf376f912bbdb")},
                { 4140000, uint256S("0x2874d104d4ec3134d23cd8ede1eb5c90f71243c0ebc5a713929271c635fa5a47")},
                { 4145000, uint256S("0x00000000b9edbb712f6f8d2f7936827da322f261a29ecede9e2b4a36fee70adf")},
                { 4150000, uint256S("0xe03dc8067559858f49f870cf7b07bef6d05c5f47e7bbcfc791b9808fe541580f")},
                { 4155000, uint256S("0xa040fbb12e5f5e02f285933e97d705ea4ccf92ebab75c2df06d0dbcc72c5316c")},
                { 4160000, uint256S("0x00000000990b6bff3507a89cb6427463ff5714db7fdccb4b205d406baa2810e8")},
                { 4165000, uint256S("0x0000000197cc66c44e256d59d81255a1958f909fd542ccd6a46019d7d68d7ea2")},
                { 4170000, uint256S("0x53c76977b6553b487013de18f23d4872d29536de4ab2f12fcb3ea32f48e55e0b")},
                { 4175000, uint256S("0x79cdc62ca0d3b8fc093100c819364f58e05d93880ce477df70a3500fa50cda2f")},
                { 4180000, uint256S("0x10691477d701e71a88c7d5c977eaf73e211869c3d2abb36cd8e609ade05b77f2")},
                { 4185000, uint256S("0x962796a13cb3d83bd812c582161441366b8fec6e29534fbee0cde0fb6edf1031")},
                { 4190000, uint256S("0x6c277821c97411c94675c0901ae3eac629531178a164bc291d30ccd5967ebc29")},
                { 4195000, uint256S("0x06df57f4d002340a488ac6706f045adaa63de1b9ed453eb698fe3fb14af89bc1")},
                { 4200000, uint256S("0xb67d921c83079f0de6f1a061a6624c0d226b6bba5262a29f268f7402713e1ea2")},
                { 4205000, uint256S("0x0000000010b7e73d36d7ae998d4d7893e357d54c01729f5adfc669918543fae7")},
                { 4210000, uint256S("0x00000000fc1e85cb48c7aa4ee2f89119d2a748b88396317a41222eaad5e76b9a")},
                { 4215000, uint256S("0x6e75cf617babf87c6b1a23b5b0e089db67e29c3934f01e52c38955053ed540e2")},
                { 4220000, uint256S("0xc4e3111c9c9ab5a2f3e3f230d20b45830ac3e42ae08bb834be97f8f30e1e5a4f")},
                { 4225000, uint256S("0x3df566c38c8511e1ddac202ed472fde1331b41dc7fae82194f9b397431f6bfb1")},
                { 4230000, uint256S("0x00000001d870209c46fe4e1741ced689db93f68fc31f482830bc45569500ec8a")},
                { 4235000, uint256S("0x00000002a1b548fdb26d6c7d972ed6247e3421e05fd71fcc7c5f99c80e3606af")},
                { 4240000, uint256S("0x80ba36ea1bad56fb6c94877813af8ca1c355eac5194064841ca483af71bb05b7")},
                { 4245000, uint256S("0xac7ebda9e9c2c24035a939235d6038541a29c3b34d58006766548bdb14852598")},
                { 4250000, uint256S("0x59bcbae6a803b506a62ea9dc4fdb6f69768e044a9fd7122d017cf2000fe41a8b")},
                { 4255000, uint256S("0x087d69803228fd30291ae2dcc51e8ac6cf664527bce7b663905a15bc4e67a353")},
                { 4260000, uint256S("0x9c23edf99477845766977ed10a2408d4750d8a7611582c5cac4a340fa7cb32ea")},
                { 4265000, uint256S("0xcaa3d6289d5ec0b1b23223b24d14f75ea6afeb6e6e3bcd60d3182be05c51da99")},
                { 4270000, uint256S("0xa1d1bff52984cd9c19d2de7183ee9c38bf57adf76b797761faf3ddf71633676e")},
                { 4275000, uint256S("0x00000001b1b8be5eb471368012c2046b6aee76a8f4d7f12c858d9f8f42a40357")},
                { 4280000, uint256S("0x172f50325fef92aac2f21e6b22cc447d30a6af758c14337d68f112e4902f67bb")},
                { 4285000, uint256S("0x00000001ce98f0032cfb36e54c0bdbe30aab1917209f5ea43b634fb094bbc85c")},
                { 4290000, uint256S("0x2047e1e7b89bcc9a0611406666f8a1147be6d403469063178d886e15ee322621")},
                { 4295000, uint256S("0xda13cdf5f15c193a75b3ece541d38f34dcc355f6d9f5ce638f8541512f7af964")},
                { 4300000, uint256S("0x104eb1da413ff98389ac918246af91dc4eeb98aa9827a3ad1c581ba4d7acc279")},
                { 4305000, uint256S("0x000000009d47db512e401519cf37d7dc07367cd2d75d719c48b3f122fb8e8770")},
                { 4310000, uint256S("0xb8a6bc46f05bd0dc88887c293d60519738250adf05eb49262759d0f4d97a7bee")},
                { 4315000, uint256S("0x1c9c60b5addf0ca34cace4b0314f93b680a2ac53197482eeec28ad86f71e73be")},
                { 4320000, uint256S("0xb4ad1dd11005a9a122ca3cdc7a83d77313f3d7b70c7192a3dca945d00815bbc5")},
                { 4325000, uint256S("0xa27ca720e4cfa912cd897be12c53daa81c1c11589df63ade06e90b43b0970449")},
                { 4330000, uint256S("0xd416a9164f679cbf564f54e52224109000eb055378f472ccac06f9d673c43480")},
                { 4335000, uint256S("0x9ac6d1d4d5ffa73832fefb73b154012214a0b0393eafecc76ce0fc12cfecb3c4")},
                { 4340000, uint256S("0xc7b8dd9814d7f3af669e4beb8719eb8e6b3ed346b0bd17e21c4128cf2780ef66")},
                { 4345000, uint256S("0x3e64260f372734f005b283f8c54c561a03c7ca013e1f070321638a9cf000ed93")},
                { 4350000, uint256S("0x3c51bbbbcf05a581a4161ac8f755122eacee3a535b77b147be44bdb6bc84cd01")},
                { 4355000, uint256S("0xc8dadf37793baf4e70a15e9c7320e1e3758737705b53a636f1121bd5a3ea3a5f")},
                { 4360000, uint256S("0xa2a6335c5fee7ec3e820963679497fa23c86c28c6af6623a6f2f0b6f46a0b199")},
                { 4365000, uint256S("0x97cb7c6eb460f4f57d8b096cfeae4a1b1bca47fe0b468df93c8e002b39df3956")},
                { 4370000, uint256S("0x91146decb3cccf14c9b97c80acf6b71cbd2e27c8c23b14fd923c41909232e174")},
                { 4375000, uint256S("0x6c67256a88ddb6c0a091bd82b14b427c6c1a619379c60577515017063fc604c8")},
                { 4380000, uint256S("0x931fcb8aa86a295fce3f164b9614ee138d8c8165330f0079c5af268de77f78a2")},
                { 4385000, uint256S("0x3a578bf97cc26523d672b293b1cc284e1040a2a1b889f53591bf024688cedf9e")},
                { 4390000, uint256S("0x30a77513cafefbdf40112a3eb607f5b7385103cbcd7b5e6ac90e8c5df27b1677")},
                { 4395000, uint256S("0x1e373a3eac2b1f12f5360df6979b24d9838c11e3541b9872be0be9e925829864")},
                { 4400000, uint256S("0xcca2919d0287497a94ae2ce0af57bc195fef8fb27ebbb693e36f0b7cf1d5fe04")},
                { 4405000, uint256S("0x31ce560eda31689b46c41f1aafe9f9dd88d421fa4ca1c676c8bb723443ffbf4c")},
                { 4410000, uint256S("0xe770b1f21d2c099aa366cf7896cdd19a671f253c76b70c1946979cde21fb5b59")},
                { 4415000, uint256S("0x8a7b05a522da73ebb5ce728eef30944d39a82cad384c867a2b64e6e3c9c41493")},
                { 4420000, uint256S("0x6827fd518866b36ab4fcbf1cbf5b3c1a262a75001dd52ecffbc637e4e3448861")},
                { 4425000, uint256S("0x8bffc60e44d410ad5603918cb19e495b6cd57f57007f7b950575109fc8a73344")},
                { 4430000, uint256S("0x8b5ba0f42a5f69f280b10b2ed499bd2018b3eaad7d87241b072d54f6b7f33447")},
                { 4435000, uint256S("0xcc5856e1b86ad97714a052b84a34f0bf0c3ac0a678be581d81af1e03e3a31d73")},
                { 4440000, uint256S("0x3659f43722780eb59493633618f60cba8c98198a3d11275e232914f8e3930bb4")},
                { 4445000, uint256S("0x00000001d35c39441612db8e7f4fec6b8a29c648b116613b31f728d27404a08e")},
                { 4450000, uint256S("0x4cea6b30d91fdfa097132183e6e1b9fd58b756a9e0e146fc83dc80de601e2b41")},
                { 4455000, uint256S("0xcb404d09e811ef10b4b06fd30dee15559e2dc3803a99b830c0b9b97e0b58048a")},
                { 4460000, uint256S("0x000000044b08ee073da34781706082a34383d966980b350b9b05d9a7c1e56d52")},
                { 4465000, uint256S("0x0000000b35b112d72d79da16d99105c0bb18bafbfa053a6c529026778446f89b")},
                { 4470000, uint256S("0x846ae3f65d058e5d4a445d4a0655ace16f5d5ac9b198657a0d79960cefec943d")},
                { 4475000, uint256S("0xe1b38abe78d666a11087e7b6e3005c82020ca3220a4330a52d0a44ad3e67ff9e")},
                { 4480000, uint256S("0x000000009b2c43d7e8ba21c0c71cbdd86b61e53d68c1e11308c4f6478a6cf43c")},
                { 4485000, uint256S("0x00000004d1963239a654120bd596d4fed000f4cad89827ad5e26ea71630d0d8c")},
                { 4490000, uint256S("0x50cbdbb9bb839bbdb72154372b25f3dd9075af7043695c6301bcd5d3b73611ba")},
                { 4495000, uint256S("0x000000025b205aafea300b5ebbc0feae809c2f8c5e75c7567436962076405a90")},
                { 4500000, uint256S("0x00000001f47ae8f0ecddafae6e5407c646655b672e2e814d9e18ff4cc32a383f")},
                { 4505000, uint256S("0x2d1c9ecb07be994dc4745319db7b16b25be5ff11ab32efd488aaeb5136c249e9")},
                { 4510000, uint256S("0x00000001dc0ac47ec7e2fc6dac4f48256f74cf3b850714e12a98d13a57feff54")},
                { 4515000, uint256S("0x000000016fb3fdb499900e996009625930fbde23bed45a8ae8c91e77716bbc88")},
                { 4520000, uint256S("0x14eb36e6e96dcaf781df66dfa05a330bc2aa08a0412d03b690f5ef1de98a3055")},
                { 4525000, uint256S("0xc14c970572f370abea118e13e4536f59e936df248caa8d001721d060b246df7c")},
                { 4530000, uint256S("0x0582499e953ef2c39c3100b56fe3304a8d0f0a89042434365cd2776c0ff35d88")},
                { 4535000, uint256S("0x00000000ddeb427cd4f88fd164ce1bcf84915daf595406f0d17795422538590c")},
                { 4540000, uint256S("0x3c06b163791a95df83a5e8ae8ed877d2bbe606aaaf64b15bbe38c040bd6ed25a")},
                { 4545000, uint256S("0x00000002c4f029c61da847511d5b73bda2a96bfe7e65b7861f63693e53ecae36")},
                { 4550000, uint256S("0x660ad6b80adb8e6ee9dca8a38349b2a30f85aae8f3bd6293b38006986e504541")},
                { 4555000, uint256S("0x9971bf63211a14146842405b13245e4ed6e37df463e451fde4ebadfea1bfdd72")},
                { 4560000, uint256S("0x530f2cbfdc517bdb2a2bc21c9474dbce2409c56b65f55ed5870f3c885359e51d")},
                { 4565000, uint256S("0x6e341bc8ae0d95bdb0d74b91963a1d903451e71e1acff1be5e2c6c7247237308")},
                { 4570000, uint256S("0x434f7bdf597df62c07b64ed5f548a3473bfef24905f7d530f515fa498f883ca3")},
                { 4575000, uint256S("0x000000030790c82ed45af9e0e1843eb74d2d384b3f679c6f2a2f308536add237")},
                { 4580000, uint256S("0x000000033e65704408317812cba7e71aaf4c64801e9cf84104681e6f29224113")},
                { 4585000, uint256S("0x1727e62d48239e37ee880a8c35308fc1be42ecc50653a34980a082dfcf915334")},
                { 4590000, uint256S("0x2b65e3225e5dac7000d984595cafdf8c265b5fc14d8687ca7767995067b447e2")},
                { 4595000, uint256S("0x23160beff32ecab07c0b4d63beb4223128cddea721f09c7a5b959a61a3be47c2")},
                { 4600000, uint256S("0xd8c6fe494f567f40b3a6c74f6d4d26c076e717319a5c6722f0a24b988af50507")},
                { 4605000, uint256S("0x9d8ed3715b4a02d384fc66e21a5ce1ed445d6405c8935ca8afc02d899811242b")},
                { 4610000, uint256S("0xedb648bd7d12e10c75fa0e16a9b0d80fe47cae99f3b62fe4376946fce83b2318")},
                { 4615000, uint256S("0x2314c06468bf33582524c5242e600bd18127ed0add7a1892d9f905c6a420eb4c")},
                { 4620000, uint256S("0xc7d6ba15eaed4435e295bed5d17c327c64951a757cbf6f1a76d76841067c591a")},
                { 4625000, uint256S("0x992c8973e4b955566a9db69f4f2fc333a8356397d7b7a4701c4e98a71f03d375")},
                { 4630000, uint256S("0x3883958beae66e1a874fa3cf8e77e633d52656ed621634b260ba44137a24e71f")},
                { 4635000, uint256S("0xe85d546bcfcd71312de8dc9b70bc8744e227001bf1e08e151f7e21142c144076")},
                { 4640000, uint256S("0x8ca48ae006d2d50ef52efb5c0f513141294b7278f881f5264fe2b7b699bc5668")},
                { 4645000, uint256S("0x0000000309d86c4fae090e96d314811b6c11ecf2b096f1907830bdce3290d7b0")},
                { 4650000, uint256S("0xcbc9f17d2554e02785781f5a40ca4a2111c2520347345e14908679a7e0f17451")},
                { 4655000, uint256S("0x520ccdba3e0c9a8ac3f2304b50d0816e9e1110ec192d4e5e168583ec1b36b581")},
                { 4660000, uint256S("0x0eb7957bbab4e61c0f27bc97de810735519d0af5c1263e142260a8c4570a4223")},
                { 4665000, uint256S("0xda86face699d96bd10ac9c50f8064719946f3bbaa492b97844461bdbf3c121fd")},
                { 4670000, uint256S("0x05a0e87e4f574d9cefcb6fe9d86479ce222e44303e5f8e7d24eaf01f77f91635")},
                { 4675000, uint256S("0x00000001082d754ad2ce6930eec7155065a9c83496c6b426ba7dbd0a6846f650")},
                { 4680000, uint256S("0x00000001f8dd37769f45acd1471e4822ba8103b4e5009092550a6478f6388d89")},
                { 4685000, uint256S("0xb742f0c02ba48c174201b616e5d5af90fd7fdb02ef2f527403f5338bcf84b1c8")},
                { 4690000, uint256S("0x57379fbd8832634514de4cd512c80a346118ccd0bc620bfdf36ce12450b55329")},
                { 4695000, uint256S("0x2110e0a5efa545d4514b9d634cce937029dab521618782545d2fba8c7f5ac333")},
                { 4700000, uint256S("0x35b85841c6a46edcf8f2740b831d02b20276858dd0d4e4f218dfba472deb7ec6")},
                { 4705000, uint256S("0xd612629f8229a46f3a4c9736209242bb5f9afa840cf9ba8a39eb20f3dd5df963")},
                { 4710000, uint256S("0x8f3ab0a7d76cad65cd26f072f5d49ec6f6860a89baa85e0d17dfca68eaabd946")},
                { 4715000, uint256S("0x0000000431f809e2de201b3f7fb25e4a1e266065635eccb417ad6e2e3bc6d518")},
                { 4720000, uint256S("0x00000008fd033187fb40ad63a4aacea1656d7b011936b7c26416bc1fdb7a1f7f")},
                { 4725000, uint256S("0x6a3de11a9a7776d5d9bbf624f2f93a4b925945eafb84d416ecc56a1cb291753d")},
                { 4730000, uint256S("0x00000001a4dec99ec4e0352ef29b81ff1c8da77a062e6890b877a14f99e3a807")},
                { 4735000, uint256S("0x76a470f1a501801ce6a0457aceb28d57f5b15bab593d62fb528f633fc0ec5f8c")},
                { 4740000, uint256S("0xd591bb680cdfda688435345d8987d0cf8b980e5fe5208fb392bee5037b2ce615")},
                { 4745000, uint256S("0x7571e92327e9b1c41901e2f48dc675da32202ff767a799255b125807566abd59")},
                { 4750000, uint256S("0x00000003ef34092c23512b18b4df995ead3d6135ef574f0362dc47634d26a04e")},
                { 4755000, uint256S("0xaa215ebff7828464eb39ad6c4b8baa55fafa2ada15276f5479b62db9ca839fa8")},
                { 4760000, uint256S("0x522c70b59adeaad2089ead7ba722f5c2ff965d043e12aa439533db6314424e22")},
                { 4765000, uint256S("0x00000013a2886581c3d00f4e2a3d18d0aabd8cdf45d6beaf03ec683bf2424018")},
                { 4770000, uint256S("0x000000201178e565aad644037c389aa5d77eaf4e678f7952da292ec7a7c00d14")},
                { 4775000, uint256S("0x00000007e6bfdb487a9c7c6ae3e5606c56698d870fe6dfc916460cc74c5e47e2")},
                { 4780000, uint256S("0x00000002f209d3b2d3e23fb6cb7755040e523d971985302b2f2044dacd6f6f14")},
                { 4785000, uint256S("0x000000040718418545a1d6bb5dc6f7bdaee9063df7490b16e4145e23fa3b5f10")},
                { 4790000, uint256S("0x7646b447e1b3c1406a4561d851136cf9107fa4f991b512272cb5df042903933b")},
                { 4795000, uint256S("0xd5d31700ed92fedd41597a347e00a3985613dd95e321697d424b8f5e19f7a201")},
                { 4800000, uint256S("0x000000019454fd90502046d3819b5655d16458388d19f95dfbd418225a13a842")},
                { 4805000, uint256S("0xdf0ccbef41a76d85b559b4997a5ec163850ad9da821d3607a7ba6ad13d98e0ed")},
                { 4810000, uint256S("0x3ed5ebaae7eb935ff8d38940582982f78b6e72fea4e85bd3a6d4cd791583bd6d")},
                { 4815000, uint256S("0x0000000253c42c15b29da47fa1b87e3f73704664f8397dac6244ebbfd2f5b8c2")},
                { 4820000, uint256S("0x0000000061ac855420301f669af1aa27f5870eb8cc6cb6fcf9b9d978d2e53064")},
                { 4825000, uint256S("0x8363d803979aa52b2854d0aa3c343bc6ab6c6b3d2f13b05b46b22abddbad6aff")},
                { 4830000, uint256S("0x0000000141fb46c6398e859c3568ea997a0c7d010c1071d0301ebf42562bf712")},
                { 4835000, uint256S("0x8ae3aeb3122915f9db387acd709171510f8bac958706a43c5d2b5153bc127118")},
                { 4840000, uint256S("0xcb81e6ef4ac9777d2751d5c51bc9366f892510dea1c764869743fe1588e7b5e4")},
                { 4845000, uint256S("0x0000000442ac09abea129e936c7051c59aecc7cd5bc017cef93b0fb00f48329b")},
                { 4850000, uint256S("0x9f9ccfd8f413f6664030fba6821e22cd7046020afc8d2f8e975f6a27893cdf16")},
                { 4855000, uint256S("0x88f939d555dfa1ecff70584c852a246e9aca9948801801d65ada9b13fc9bd411")},
                { 4860000, uint256S("0x000000006f5d9741ddcdf1fcd068d0f73b086ba69e60efce7ce50ef801921b21")},
                { 4865000, uint256S("0x00000009157259a0a8096a6196448cb3eefdd607af65e5e24fa1025614cacb56")},
                { 4870000, uint256S("0x0b45988a37e09f7a3846dccf34ad72b74b764c9f4b7f650323f5d4aab07e0e03")},
                { 4875000, uint256S("0xb2e4b6c325cf54a99ac5ca30d546570079fae734c8a3ac266e05c1b010eba989")},
                { 4880000, uint256S("0xc3b7ce3a1e237be064a496022e47f9137f423a16eba38e0c7e9db71e03ff0195")},
                { 4885000, uint256S("0x9c11aec0007ff7791fad37c18a82d4163c6c83e97e9963de751c22395ef2bfdd")},
                { 4890000, uint256S("0x000000033bc0319bf28f9a84c5d82dd2edf5f0c816b520fcdbacb2b3887e751d")},
                { 4895000, uint256S("0xcf5367282e47a497b875cc6ef54af90d6ce935015a212e782be98893e84ae053")},
                { 4900000, uint256S("0x469e923d6a79ba92f378085c7390d13704616ef261ab476ef9d24aa0c4f1b229")},
                { 4905000, uint256S("0x44934050cc664653a0dfa6e071b8f32eb09120e084be80f0c92299865b5d965f")},
                { 4910000, uint256S("0xcb699e65041107c1cb740b0b07b19b19bc5eb9ba45af7f7b944d2049c68986b2")},
                { 4915000, uint256S("0x08a28de85597eae16a7744e549d96871beac86b4c7c25c57c4e4004104d7a9bc")},
                { 4920000, uint256S("0xa52534300d57e6669c5cab7362acb2c15531585f466fef5fa4f8d67766db69b9")},
                { 4925000, uint256S("0x0000000718a2cfb59521f6d5a1679716b80b0053614df0031310ca06c5da8de0")},
                { 4930000, uint256S("0x36cbdb44d211cc42114fc68c6f4ed2e81423cd414d0c4889e8ba6148ee402c9b")},
                { 4935000, uint256S("0xac7c657acdc0ea1c3bd2c1eab71b01a42000f8b264c49020e66f3fe1fcf8ea2c")},
                { 4940000, uint256S("0x5fd454bd11623505285967e7b17e124eed14a65ace7ca2e5fb0e3f4f1943ab7e")},
                { 4945000, uint256S("0x000000041e57a8a9cd8a291e011e586c76d74ef45f8bfaa0fd007f2d250c65c1")},
                { 4950000, uint256S("0x7bb4906ebe178b147166c86ac7e6fc07c50d2ceb1ba26a01fe9272d5d86c1cad")},
                { 4955000, uint256S("0xc872df357b2949ed1bcf000cd26c2817b1e81787011a9aa8c4410a1e682d4a1d")},
                { 4960000, uint256S("0x9cb44780a03c7b8f0ddba28c931fb5b8dfdfcf76dce608d921259821a51c94c7")},
                { 4965000, uint256S("0xd4508273e541d3543564fa50397773f10d1abc8f1b4a88a7bf9187437109b557")},
                { 4970000, uint256S("0xfa6b51f0c01e3709596531f83ea9e437bb8a38d9777f14b35797389565d6c0c1")},
                { 4975000, uint256S("0x7a5b3000d9c57cf1cee6b5f2473ef700dabba4448f26e60df7b9ac380b9831db")},
                { 4980000, uint256S("0x0000000a2b47ce46af65faa96c3d85d24783a57d8c27c9c57aed16975ffff92a")},
                { 4985000, uint256S("0x6470147472f02affd303d462fd6ee1fcb2e9333936b4ea78efafba038abf4558")},
                { 4990000, uint256S("0xe2735eb9bd0d068e20479e174a3322f33077d778d6272d850b3df584d7bdff19")},
                { 4995000, uint256S("0x50593809aab8f77b46d0ab2d8eac9fd845f12cf006004238e40927915689c2a0")},
                { 5000000, uint256S("0x00000006ad7284c54708eaeeea76de306fc8aeb83486024bd710161060f89481")},
                { 5005000, uint256S("0x7b3eb7846a00196a4b3fb2d9fd291399e8fe874307427b89e75236f870ac7b60")},
                { 5010000, uint256S("0x99163225095d8732e73b8585cae35979d037fc41d1da8fc5e37450b418ad9296")},
                { 5015000, uint256S("0x346e9cfbb53852637d7b4e633636690aaaf1c06f142a87d2118a13f866279fc0")},
                { 5020000, uint256S("0x000000053eda063d2b2a7cc63609311240558f42d0c2c4884293954404059929")},
                { 5025000, uint256S("0x0452bfcf7ee588b6550061c313f1ac07b5548316284f755b9916387a39f68761")},
                { 5030000, uint256S("0x6420830b71a845c6f2b59ed9dfed33f67c9694ee8fc252e888ccd594563be9b1")},
                { 5035000, uint256S("0xe7f034df957c0a6782ae6d2de51bcbe15820b5bf0e4c7ad5a32f95669f224c99")},
                { 5040000, uint256S("0xb5b105d75a3ce97cc4c0fc55acc91c967a5892707c6560a0a48d421fb1afd933")},
                { 5045000, uint256S("0x05a92d6fbd00266fffe5eaa346efb9c2a2a8a0f8de9589ce9d6cd782beb7c274")},
                { 5050000, uint256S("0xa8ae629dd4d28766c87cafd8eb02713dd911edec4d4034e90840f06532dac712")},
                { 5055000, uint256S("0x5941753c69c827fc53446a780eb7e9860392ec849c92df877e6194fb8039bf22")},
                { 5060000, uint256S("0x0ce96ca0665956eb3256f7d422ae6a9bbc815f52fd0f047188fbe2461d011f55")},
                { 5065000, uint256S("0x0000000423680c1f2c8db098ba7eea9a485ea662fcc289807e8b70d6dd931b28")},
                { 5070000, uint256S("0x00000008fbd39a50b6ddd3fd64fb8390cfdbe33704637c2015f8d3355d9ae5e1")},
                { 5075000, uint256S("0x8621a57fafe472b7979dde830773487fc2c10138874849ff990df123cc1fde65")},
                { 5080000, uint256S("0x0000000be9db98b99ea72d640843e035a3a5c197bd9f8cdd1d65f034cd6a542c")},
                { 5085000, uint256S("0xf4889d75ebab9d4bc3ba479fdf49a49c8c207ef8c04b7a2aeaab64c7bf6ef626")},
                { 5090000, uint256S("0xe23dd6098f7631b492ec425964eb29d4a8d675d9e3d038fc5f164ea0ef35176e")},
                { 5095000, uint256S("0x93c48a4e496a467f837a59f9f3dbd043305d52f5b492278b548b4f97ef037fc4")},
                { 5100000, uint256S("0x3158ba971f2b17c5a53c679c912b541ddfc86879d11a0c0bd1b866b70e927b83")},
                { 5105000, uint256S("0x9fc5da775e34012212ff16cf172e0c4ec2b720f32cd10cf76e2298b4976e0e49")},
                { 5110000, uint256S("0x8261a30bd047da577d1232a966935323de880bcdfefb5a92df3e728d5f02eb82")},
                { 5115000, uint256S("0x00000009ce614d7e0f4c178c3dbebdd834ade2d4541da41e9e78fd188b23f120")},
                { 5120000, uint256S("0xee107d30189b52782fa7be61dc1714b515c3b915aff005128586dadec717e4c3")},
                { 5125000, uint256S("0x42316512587b6a9d6e8f2e13ac052c1b15c99587274e905c1c3a81aaf540230d")},
                { 5130000, uint256S("0x000000059ed22adc8214a3d30fd90514cf2bd29c8cd5d3b080963cff0ddcd854")},
                { 5135000, uint256S("0x0000000ae4537f4ace12d109c6bb70baf960a6127cbe9aa6728f4315cf7fb3bc")},
                { 5140000, uint256S("0x1b6039c7243583988329c8fd216a4c953d1f3f215b37b37e290bbdf9e5164071")},
                { 5145000, uint256S("0x3a727d79d5cfeea06f47125371938fdf1a7034276fe10a3d09d79e24fc4c61f2")},
                { 5150000, uint256S("0x85e246f617dd19edc32702fb03ce23c598bd40770bbb25560cf65a2a2dedeba2")},
                { 5155000, uint256S("0x000000026f58cd97b373711c3db39373f9072d020e4eaf07c998f006045a34f8")},
                { 5160000, uint256S("0x8f7ec043ddffe290734a10fd5a83cc0e5f05bd183daaf3fa248a7dc08c6254d0")},
                { 5165000, uint256S("0x0000001ac826374484d69fc348469b9a0b6ed2f8be9c3c422b35f305db1e4737")},
                { 5170000, uint256S("0xfbce285579811663be4c9178d8d8c4646183a837cfc15ee04900a884f6e016a6")},
                { 5175000, uint256S("0xdee86e9c1c4405e97f34820dc1d1f259a7a6999c329371e228fc551960d56e9b")},
                { 5180000, uint256S("0x000000095e79986cd246c37814c3b65e65d2c569f611a5671eb21c2048e32448")},
                { 5185000, uint256S("0x06098769a950a2c70fdb34d74825344af68fae89fb7dbb0d994953df1c1aad6c")},
                { 5190000, uint256S("0xfdf1d8c260cb61d1113a05998e239eff9849936689e09ece56b457e3245bec06")},
                { 5195000, uint256S("0x00000025091362606fabd59ca5ecf29c0d2192e8f5c98373ffa5b76068c11f78")},
                { 5200000, uint256S("0x0000001b41302bd37ede1798d1d60542f97fdc7d02a393a7b775ff27ef30774f")},
                { 5205000, uint256S("0xf57fcfa5c05bfda65f170d8b9da954622c1e0ff84df79fe45518f3635d71b11d")},
                { 5210000, uint256S("0xab7a574b506046666bae97d27c43223208e16ff0ec6d01a58dd9cca88ea16253")},
                { 5215000, uint256S("0x7a6d7ab0b8c4ea4bd3c4cbc670ab92482a2c72883e294d9280fcae73a5183507")},
                { 5220000, uint256S("0x308ae75ae18665aaebe6e661e98422b1167ac96d070ececaf404c9feede3b800")},
                { 5225000, uint256S("0x00000022c5ec6efb19788300c7372fce65bed2e00a5643a65adb295a0eb08925")},
                { 5230000, uint256S("0xbe830ddfa6eec58bd44e9c824ea9d149f81576c46239326b1dd6a2d71ba5f4a3")},
                { 5235000, uint256S("0x0000001396438281ccdbb15d7761ddd5b32225f95d43df6b5cb109b40f096599")},
                { 5240000, uint256S("0x45d8b03ad05c1aaa7b90759290f5a17c513e91c2e8736ea7c9431a8332998677")},
                { 5245000, uint256S("0x0000002aed95e30212dc7faae92c6c2ed7fca78bc7f16f8158f90bcb974cd9c3")},
                { 5250000, uint256S("0x00000037e6c97970bb5d146e102124ecb9aa649081505b0afd8d7c774863ff7f")},
                { 5255000, uint256S("0x8adfa0cf934da98582ac0ecc1081ec6360030a59497395ee6cb7dbbc2fb61f10")},
                { 5260000, uint256S("0xee8c0b7e515678445da888f9826f7a912b4f1d2bee41f0bedc25a99ce0f5589f")},
                { 5261473, uint256S("0xcde41c63c4f55c69333a72af01fdc9c700ad15ffb7599c445b9ef42a477612d4")},
            }
        };

        m_assumeutxo_data = MapAssumeutxo{
         // TODO to be specified in a future patch.
        };

        chainTxData = ChainTxData{
            // Data as of block 0fc7bf7f0e830eea0bc367c76f9dcfc70d42d5625d93b056354dc23049de6e29 (height 770396).
            1410566399, // * UNIX timestamp of last known number of transactions
            0,    // * total number of transactions between genesis and that timestamp
                        //   (the tx=... number in the ChainStateFlushed debug.log lines)
            0.000001 // * estimated number of transactions per second after that timestamp
                        //   2551705/(1727128008-1345400356) = 0.006684622
        };
    }
};

/**
 * Testnet (v3): public test network which is reset from time to time.
 */
class CTestNetParams : public CChainParams {
public:
    CTestNetParams() {
        strNetworkID = CBaseChainParams::TESTNET;
        consensus.signet_blocks = false;
        consensus.signet_challenge.clear();
        consensus.BIP34Height = 293368;
        consensus.BIP34Hash = uint256S("00000002c0b976c7a5c9878f1cec63fb4d88d68d614aedeaf8158c42d904795e");
        consensus.powLimit =            uint256S("0000000fffffffffffffffffffffffffffffffffffffffffffffffffffffffff"); // ~arith_uint256(0) >> 28;
        consensus.bnInitialHashTarget = uint256S("00000007ffffffffffffffffffffffffffffffffffffffffffffffffffffffff"); // ~arith_uint256(0) >> 29;

        consensus.nTargetTimespan = 7 * 24 * 60 * 60;  // one week
        consensus.nStakeTargetSpacing = 10 * 60;  // 10-minute block spacing
        consensus.nTargetSpacingWorkMax = 12 * consensus.nStakeTargetSpacing; // 2-hour
        consensus.nPowTargetSpacing = consensus.nStakeTargetSpacing;
        consensus.nStakeMinAge = 60 * 60 * 24; // test net min age is 1 day
        consensus.nStakeMaxAge = 60 * 60 * 24 * 90;
        consensus.nModifierInterval = 60 * 20; // Modifier interval: time to elapse before new modifier is computed
        consensus.nCoinbaseMaturity = 60;

        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 1512; // 75% for testchains
        consensus.nMinerConfirmationWindow = 2016; // nPowTargetTimespan / nPowTargetSpacing

        consensus.SegwitHeight = 394215;

        consensus.nMinimumChainWork = uint256S("0x00000000000000000000000000000000000000000000000000a39348f70f067a");  // 500000
        consensus.defaultAssumeValid = uint256S("0xa40f64181ee4a3bedda2eae0107d9da0e049fe285b6e6e2a7f1f11697f22c7ed"); // 500000

        pchMessageStart[0] = 0xcb;
        pchMessageStart[1] = 0xf2;
        pchMessageStart[2] = 0xc0;
        pchMessageStart[3] = 0xef;
        nDefaultPort = 9903;
        m_assumed_blockchain_size = 1;

        genesis = CreateGenesisBlock(1407209706, 1410566399, 1780637, 0x1e0fffff, 1, 0);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x000004c91ca895a8c63176b1671eff34291ad671e59ae46630ffd8f985dd56cc"));
        assert(genesis.hashMerkleRoot == uint256S("0x70070d9e41ffd85685f8017fa8620fb5572ed8443822d799015d01d39e7fd4af"));

        vFixedSeeds.clear();
        vSeeds.clear();
        // nodes with support for servicebits filtering should be at the top
        vSeeds.emplace_back("test-magi-seed.checkbug.com");

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,111);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,196);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};

        // human readable prefix to bench32 address
        bech32_hrp = "tpc";

        vFixedSeeds = std::vector<uint8_t>(std::begin(chainparams_seed_test), std::end(chainparams_seed_test));

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = false;
        m_is_test_chain = true;
        m_is_mockable_chain = false;

        checkpointData = {
            {
                {       0, uint256S("0x0000036df26f4d11af604f86b7bdc5ce5f8bee17a3c6f57e9e6e800ef21d8447")},
            }
        };

        m_assumeutxo_data = MapAssumeutxo{
            // TODO to be specified in a future patch.
        };

        chainTxData = ChainTxData{
            // Data as of block 00000003216118fab90ea268ce526e1fcce67dc97b74c5b62119cc3d244d8f71 (height 612778)
            1739494055, // * UNIX timestamp of last known number of transactions
            1221679,    // * total number of transactions between genesis and that timestamp
                        //   (the tx=... number in the SetBestChain debug.log lines)
            0.003104928 // * estimated number of transactions per second after that timestamp
                        //   1221679/(1739494055-1346029522) = 0.003104928

        };
    }
};

/**
 * Signet: test network with an additional consensus parameter (see BIP325).
 */
class SigNetParams : public CChainParams {
public:
    explicit SigNetParams(const SigNetOptions& options)
    {
        std::vector<uint8_t> bin;
        vSeeds.clear();

        if (!options.challenge) {
            bin = ParseHex("512103ad5e0edad18cb1f0fc0d28a3d4f1f3e445640337489abb10404f2d1e086be430210359ef5021964fe22d6f8e05b2463c9540ce96883fe3b278760f048f5189f2e6c452ae");
            vSeeds.emplace_back("seed.signet.bitcoin.sprovoost.nl.");

            // Hardcoded nodes can be removed once there are more DNS seeds
            vSeeds.emplace_back("178.128.221.177");
            vSeeds.emplace_back("v7ajjeirttkbnt32wpy3c6w3emwnfr3fkla7hpxcfokr3ysd3kqtzmqd.onion:38333");

            consensus.nMinimumChainWork = uint256S("0x000000000000000000000000000000000000000000000000000001291fc22898");
            consensus.defaultAssumeValid = uint256S("0x000000d1a0e224fa4679d2fb2187ba55431c284fa1b74cbc8cfda866fd4d2c09"); // 105495
            m_assumed_blockchain_size = 1;
            m_assumed_chain_state_size = 0;
            chainTxData = ChainTxData{
                // Data from RPC: getchaintxstats 4096 000000d1a0e224fa4679d2fb2187ba55431c284fa1b74cbc8cfda866fd4d2c09
                .nTime    = 1661702566,
                .nTxCount = 1903567,
                .dTxRate  = 0.02336701143027275,
            };
        } else {
            bin = *options.challenge;
            consensus.nMinimumChainWork = uint256{};
            consensus.defaultAssumeValid = uint256{};
            m_assumed_blockchain_size = 0;
            m_assumed_chain_state_size = 0;
            chainTxData = ChainTxData{
                0,
                0,
                0,
            };
            LogPrintf("Signet with challenge %s\n", HexStr(bin));
        }

        if (options.seeds) {
            vSeeds = *options.seeds;
        }

        strNetworkID = CBaseChainParams::SIGNET;
        consensus.signet_blocks = true;
        consensus.signet_challenge.assign(bin.begin(), bin.end());
        //consensus.nSubsidyHalvingInterval = 210000;
        consensus.BIP34Height = 1;
        consensus.BIP34Hash = uint256{};
        consensus.BIP65Height = 1;
        consensus.BIP66Height = 1;
        consensus.CSVHeight = 1;
        consensus.SegwitHeight = 1;
        //consensus.nTargetTimespan = 7 * 24 * 60 * 60; // two weeks
        consensus.nPowTargetSpacing = 10 * 60;
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 1815; // 90% of 2016
        consensus.nMinerConfirmationWindow = 2016; // nPowTargetTimespan / nPowTargetSpacing
        consensus.MinBIP9WarningHeight = 0;
        consensus.powLimit = uint256S("00000377ae000000000000000000000000000000000000000000000000000000");
/*
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = Consensus::BIP9Deployment::NEVER_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].min_activation_height = 0; // No activation delay

        // Activation of Taproot (BIPs 340-342)
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].bit = 2;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nStartTime = Consensus::BIP9Deployment::ALWAYS_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].min_activation_height = 0; // No activation delay
*/
        // message start is defined as the first 4 bytes of the sha256d of the block script
        HashWriter h{};
        h << consensus.signet_challenge;
        uint256 hash = h.GetHash();
        memcpy(pchMessageStart, hash.begin(), 4);

        nDefaultPort = 38333;

        genesis = CreateGenesisBlock(1407209706, 1410566399, 1780637, 0x1e0fffff, 1, 0);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x000004c91ca895a8c63176b1671eff34291ad671e59ae46630ffd8f985dd56cc"));
        assert(genesis.hashMerkleRoot == uint256S("0x70070d9e41ffd85685f8017fa8620fb5572ed8443822d799015d01d39e7fd4af"));

        vFixedSeeds.clear();

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,111);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,196);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};

        bech32_hrp = "tb";

        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        m_is_test_chain = true;
        m_is_mockable_chain = false;
    }
};

/**
 * Regression test: intended for private networks only. Has minimal difficulty to ensure that
 * blocks can be found instantly.
 */
class CRegTestParams : public CChainParams
{
public:
    explicit CRegTestParams(const RegTestOptions& opts)
    {
        strNetworkID =  CBaseChainParams::REGTEST;
        consensus.signet_blocks = false;
        consensus.signet_challenge.clear();
        //consensus.nSubsidyHalvingInterval = 150;
        consensus.BIP34Height = 1; // Always active unless overridden
        consensus.BIP34Hash = uint256();
        consensus.BIP65Height = 1;  // Always active unless overridden
        consensus.BIP66Height = 1;  // Always active unless overridden
        consensus.CSVHeight = 1;    // Always active unless overridden
        consensus.SegwitHeight = 0; // Always active unless overridden
        consensus.MinBIP9WarningHeight = 0;
        consensus.powLimit = uint256S("7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.bnInitialHashTarget = uint256S("00000007ffffffffffffffffffffffffffffffffffffffffffffffffffffffff"); // ~arith_uint256(0) >> 29;

        consensus.nTargetTimespan = 7 * 24 * 60 * 60; // two weeks
        consensus.nStakeTargetSpacing = 10 * 60; // 10-minute block spacing
        consensus.nTargetSpacingWorkMax = 12 * consensus.nStakeTargetSpacing; // 2-hour
        consensus.nPowTargetSpacing = consensus.nStakeTargetSpacing;

        consensus.nStakeMinAge = 60 * 60 * 24; // test net min age is 1 day
        consensus.nStakeMaxAge = 60 * 60 * 24 * 90;
        consensus.nModifierInterval = 60 * 20; // Modifier interval: time to elapse before new modifier is computed
        consensus.nCoinbaseMaturity = 60;

        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = true;
        consensus.nRuleChangeActivationThreshold = 108; // 75% for testchains
        consensus.nMinerConfirmationWindow = 144; // Faster than normal for regtest (144 instead of 2016)

        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].min_activation_height = 0; // No activation delay

        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].bit = 2;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nStartTime = Consensus::BIP9Deployment::ALWAYS_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].min_activation_height = 0; // No activation delay

        consensus.nMinimumChainWork = uint256{};
        consensus.defaultAssumeValid = uint256{};

        pchMessageStart[0] = 0xcb;
        pchMessageStart[1] = 0xf2;
        pchMessageStart[2] = 0xc0;
        pchMessageStart[3] = 0xef;
        nDefaultPort = 9903;
        m_assumed_blockchain_size = 0;
        m_assumed_chain_state_size = 0;

        for (const auto& [dep, height] : opts.activation_heights) {
            switch (dep) {
            case Consensus::BuriedDeployment::DEPLOYMENT_SEGWIT:
                consensus.SegwitHeight = int{height};
                break;
            case Consensus::BuriedDeployment::DEPLOYMENT_HEIGHTINCB:
                consensus.BIP34Height = int{height};
                break;
            case Consensus::BuriedDeployment::DEPLOYMENT_DERSIG:
                consensus.BIP66Height = int{height};
                break;
            case Consensus::BuriedDeployment::DEPLOYMENT_CLTV:
                consensus.BIP65Height = int{height};
                break;
            case Consensus::BuriedDeployment::DEPLOYMENT_CSV:
                consensus.CSVHeight = int{height};
                break;
            }
        }

        for (const auto& [deployment_pos, version_bits_params] : opts.version_bits_parameters) {
            consensus.vDeployments[deployment_pos].nStartTime = version_bits_params.start_time;
            consensus.vDeployments[deployment_pos].nTimeout = version_bits_params.timeout;
            consensus.vDeployments[deployment_pos].min_activation_height = version_bits_params.min_activation_height;
        }

        genesis = CreateGenesisBlock(1407209706, 1410566399, 1780637, 0x1e0fffff, 1, 0);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x000004c91ca895a8c63176b1671eff34291ad671e59ae46630ffd8f985dd56cc"));
        assert(genesis.hashMerkleRoot == uint256S("0x70070d9e41ffd85685f8017fa8620fb5572ed8443822d799015d01d39e7fd4af"));

        vFixedSeeds.clear(); //!< Regtest mode doesn't have any fixed seeds.
        vSeeds.clear();
        vSeeds.emplace_back("dummySeed.invalid.");

        fDefaultConsistencyChecks = true;
        fRequireStandard = true;
        m_is_test_chain = true;
        m_is_mockable_chain = true;

        fMiningRequiresPeers = false;

        checkpointData = {
            {
                {0, uint256S("0x00000001f757bb737f6596503e17cd17b0658ce630cc727c0cca81aec47c9f06")}
            }
        };

        m_assumeutxo_data = MapAssumeutxo{
            {
                110,
                {AssumeutxoHash{uint256S("0x1ebbf5850204c0bdb15bf030f47c7fe91d45c44c712697e4509ba67adb01c618")}, 110},
            },
            {
                200,
                {AssumeutxoHash{uint256S("0x51c8d11d8b5c1de51543c579736e786aa2736206d1e11e627568029ce092cf62")}, 200},
            },
        };

        chainTxData = ChainTxData{
            0,
            0,
            0
        };

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,111);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,196);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};

        bech32_hrp = "pcrt";
    }
};

std::unique_ptr<const CChainParams> CChainParams::SigNet(const SigNetOptions& options)
{
    return std::make_unique<const SigNetParams>(options);
}

std::unique_ptr<const CChainParams> CChainParams::RegTest(const RegTestOptions& options)
{
    return std::make_unique<const CRegTestParams>(options);
}

std::unique_ptr<const CChainParams> CChainParams::Main()
{
    return std::make_unique<const CMainParams>();
}

std::unique_ptr<const CChainParams> CChainParams::TestNet()
{
    return std::make_unique<const CTestNetParams>();
}
