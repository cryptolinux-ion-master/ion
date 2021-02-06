// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Copyright (c) 2018-2020 The Ion Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <primitives/block.h>

#include "hash.h"
#include "streams.h"
#include "tinyformat.h"
#include "utilstrencodings.h"
#include "versionbits.h"
#include "crypto/common.h"

uint256 CBlockHeader::GetHash() const
{
    // CVectorWriter grows vch when necessary
    std::vector<unsigned char> vch(80);
    CVectorWriter ss(SER_NETWORK, PROTOCOL_VERSION, vch, 0);
    ss << *this;
    if ((nVersion & BLOCKTYPEBITS_MASK) == BlockTypeBits::BLOCKTYPE_MINING) {
        return HashX11((const char *)vch.data(), (const char *)vch.data() + vch.size());
    } else {
        return Hash((const char *)vch.data(), (const char *)vch.data() + vch.size());
    }
}

std::string CBlock::ToString() const
{
    std::stringstream s;
    s << strprintf("CBlock(hash=%s, ver=0x%08x, hashPrevBlock=%s, hashMerkleRoot=%s, nTime=%u, nBits=%08x, nNonce=%u, vtx=%u)\n",
        GetHash().ToString(),
        nVersion,
        hashPrevBlock.ToString(),
        hashMerkleRoot.ToString(),
        nTime, nBits, nNonce,
        vtx.size());
    for (const auto& tx : vtx) {
        s << "  " << tx->ToString() << "\n";
    }
    return s.str();
}
