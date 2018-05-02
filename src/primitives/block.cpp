// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "primitives/block.h"

#include "hash.h"
#include "tinyformat.h"
#include "utilstrencodings.h"
#include "crypto/common.h"
//#include "streams.h"

uint256 CBlockHeader::GetHash() const
{
        //CDataStream ss(SER_GETHASH, 0);
        //ss << nVersion << hashPrevBlock << hashMerkleRoot << nTime << nBits << nNonce << bnPrimeChainMultiplier;
        //return Hash(ss.begin(), ss.end());
		//DATACOIN CHANGED Считаем хеш по новому
		return SerializeHash(*this);
}

std::string CBlock::ToString() const
{
    std::stringstream s;
    s << strprintf("CBlock(hash=%s, hashBlockHeader=%s, ver=0x%08x, hashPrevBlock=%s, hashMerkleRoot=%s, nTime=%u, nBits=%08x, nNonce=%u, bnPrimeChainMultiplier=%s, vtx=%u)\n",
        GetHash().ToString(),
		GetHeaderHash().ToString().c_str(),
        nVersion,
        hashPrevBlock.ToString(),
        hashMerkleRoot.ToString(),
        nTime, nBits, nNonce,
		bnPrimeChainMultiplier.ToString().c_str(),
        vtx.size());
    for (const auto& tx : vtx) {
        s << "  " << tx->ToString() << "\n";
    }
    return s.str();
}
