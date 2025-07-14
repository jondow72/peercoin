// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2019 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <primitives/block.h>

#include <hash.h>
#include <tinyformat.h>

#include <../chainparams.h>
#include <../serialize.h>
#include <../crypto/m7m.h>

bool fTestNet = Params().NetworkIDString() == CBaseChainParams::TESTNET;

uint256 CBlockHeader::GetHash() const {
    std::vector<unsigned char> vch;
    CDataStream ss(SER_DISK, PROTOCOL_VERSION);
    ss << *this;
    vch.assign(ss.begin(), ss.end());
    if (fTestNet) {
        return hash_M7M_v2(vch.begin(), vch.end(), nNonce);
    } else {
        if (nTime < 1414330200) {
            return hash_M7M(vch.begin(), vch.end());
        } else {
            return hash_M7M_v2(vch.begin(), vch.end(), nNonce);
        }
    }
}

std::string CBlock::ToString() const
{
    std::stringstream s;
    s << strprintf("CBlock(hash=%s, ver=0x%08x, hashPrevBlock=%s, hashMerkleRoot=%s, nTime=%u, nBits=%08x, nNonce=%u, nFlags=%08x, vtx=%u)\n",
        GetHash().ToString(),
        nVersion,
        hashPrevBlock.ToString(),
        hashMerkleRoot.ToString(),
        nTime, nBits, nNonce,
        nFlags, vtx.size());
    for (const auto& tx : vtx) {
        s << "  " << tx->ToString() << "\n";
    }
    return s.str();
}
