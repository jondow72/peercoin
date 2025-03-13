// Copyright (c) 2012-2014 The Bitcoin developers
// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2015-2018 The Luxcore developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once
#include "arith_uint256.h"
#include "uint256.h"

/** 512-bit unsigned big integer. */
class uint512 : public base_blob<512>
{
public:
    typedef base_blob<512> basetype;

    uint512() {}
    uint512(const base_blob<512>& b) : base_blob<512>(b) {}
    explicit uint512(const std::vector<unsigned char>& vch) : base_blob<512>(vch) {}

    uint512& operator=(const basetype& b)
    {
        memcpy(this->data(), b.data(), sizeof(this->data()));
        return *this;
    }

    uint512& operator=(uint64_t b)
    {
        uint8_t* d = this->data();
        memset(d, 0, sizeof(this->data()));  // Zero out the rest of the array
        d[0] = (uint8_t)(b & 0xFF);
        d[1] = (uint8_t)((b >> 8) & 0xFF);
        d[2] = (uint8_t)((b >> 16) & 0xFF);
        d[3] = (uint8_t)((b >> 24) & 0xFF);
        d[4] = (uint8_t)((b >> 32) & 0xFF);
        d[5] = (uint8_t)((b >> 40) & 0xFF);
        d[6] = (uint8_t)((b >> 48) & 0xFF);
        d[7] = (uint8_t)((b >> 56) & 0xFF);
        return *this;
    }

    uint256 trim256() const
    {
        return uint256(std::vector<unsigned char>(this->begin(), this->begin() + 32));
    }
};

inline bool operator==(const uint512& a, uint64_t b) { return (base_blob<512>)a == b; }
inline bool operator==(const base_blob<512>& a, const uint512& b) { return (base_blob<512>)a == (base_blob<512>)b; }

/* uint512 from const char *.
 * This is a separate function because the constructor uint512(const char*) can result
 * in dangerously catching uint512(0).
 */
inline uint512 uint512S(const char* str)
{
    uint512 rv;
    rv.SetHex(str);
    return rv;
}
