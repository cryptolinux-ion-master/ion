// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Copyright (c) 2014-2019 The Dash Core developers
// Copyright (c) 2018-2020 The Ion Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "staker.h"

#include "chain.h"
#include "chainparams.h"
#include "miner.h"
#include "pos/blocksignature.h"
#include "pos/kernel.h"
#include "pos/rewards.h"
#include "pos/staking-manager.h"
#include "pos/stakeinput.h"
#include "rpc/protocol.h"
#include "script/tokengroup.h"
#include "tokens/tokengroupmanager.h"
#include "validation.h"
#include "wallet/wallet.h"

//////////////////////////////////////////////////////////////////////////////
//
// IONStaker
//

//#if ENABLE_MINER

UniValue generateHybridBlocks(std::shared_ptr<CReserveKey> coinbaseKey, int nGenerate, uint64_t nMaxTries, bool keepScript, CWallet * const pwallet)
{
    const auto& params = Params().GetConsensus();
    const bool fRegtest = Params().NetworkIDString() == CBaseChainParams::REGTEST;
    static const int nInnerLoopCount = 0x10000;
    bool fPosPowPhase;
    bool fPosPhase;
    bool fCreatePosBlock;
    int nHeightEnd = 0;
    int nHeight = 0;

    {   // Don't keep cs_main locked
        LOCK(cs_main);
        nHeight = chainActive.Height();
        nHeightEnd = nHeight+nGenerate;
    }
    unsigned int nExtraNonce = 0;
    UniValue blockHashes(UniValue::VARR);
    while (nHeight < nHeightEnd)
    {
        fPosPowPhase = nHeight + 1 >= params.POSPOWStartHeight;
        fPosPhase = fRegtest && (nHeight + 1 >= params.POSStartHeight && nHeight + 1 < params.POSPOWStartHeight);
        // If nHeight > POS start, wallet should be enabled.

        // Create coinstake if on regtest, in POS phase and not in POW phase, or if in POS phase and in POW phase during alternating (odd) blocks
        fCreatePosBlock = fRegtest && (fPosPhase || (fPosPowPhase && nHeight % 2));
        std::unique_ptr<CBlockTemplate> pblocktemplate = nullptr;
        if (fCreatePosBlock) {
            std::shared_ptr<CMutableTransaction> coinstakeTxPtr = std::shared_ptr<CMutableTransaction>(new CMutableTransaction);
            std::shared_ptr<CStakeInput> coinstakeInputPtr = std::shared_ptr<CStakeInput>(new CIonStake);
            if (stakingManager->CreateCoinStake(chainActive.Tip(), coinstakeTxPtr, coinstakeInputPtr)) {
                // Coinstake found. Extract signing key from coinstake
                pblocktemplate = BlockAssembler(Params()).CreateNewBlock(CScript(), coinstakeTxPtr, coinstakeInputPtr);
            };
        } else {
            if (fPosPowPhase) {
                if (!tokenGroupManager->ElectronTokensCreated()) {
                    throw JSONRPCError(RPC_MISC_ERROR, "Error: Mining in hybrid mode, but the Electron token group is not yet created");
                }
                CScript coinbase_script;
                CBlockReward reward(nHeight + 1, 0, false, params);
                if (!pwallet->GetScriptForHybridMining(coinbase_script, coinbaseKey, reward.GetCoinbaseReward())) {
                    throw JSONRPCError(RPC_WALLET_KEYPOOL_RAN_OUT, "Error: Keypool ran out, please call keypoolrefill first");
                }
                pblocktemplate = BlockAssembler(Params()).CreateNewBlock(coinbase_script);
            } else {
                CScript coinbase_script;
                if (!pwallet->GetScriptForPowMining(coinbase_script, coinbaseKey)) {
                    throw JSONRPCError(RPC_WALLET_KEYPOOL_RAN_OUT, "Error: Keypool ran out, please call keypoolrefill first");
                }
                pblocktemplate = BlockAssembler(Params()).CreateNewBlock(coinbase_script);
            }
        }
        if (!pblocktemplate.get())
            throw JSONRPCError(RPC_INTERNAL_ERROR, "Couldn't create new block");
        CBlock *pblock = &pblocktemplate->block;
        {
            LOCK(cs_main);
            IncrementExtraNonce(pblock, chainActive.Tip(), nExtraNonce);
        }
        while (!fCreatePosBlock && nMaxTries > 0 && pblock->nNonce < nInnerLoopCount && !CheckProofOfWork(pblock->GetHash(), pblock->nBits, Params().GetConsensus())) {
            ++pblock->nNonce;
            --nMaxTries;
        }
        if (nMaxTries == 0) {
            break;
        }
        if (pblock->nNonce == nInnerLoopCount) {
            continue;
        }

        if (fCreatePosBlock) {
            CKeyID keyID;
            if (!GetKeyIDFromUTXO(pblock->vtx[1]->vout[1], keyID)) {
                LogPrint(BCLog::STAKING, "%s: failed to find key for PoS", __func__);
                continue;
            }
            CKey key;
            if (!pwallet->GetKey(keyID, key)) {
                LogPrint(BCLog::STAKING, "%s: failed to get key from keystore", __func__);
                continue;
            }
            if (!key.Sign(pblock->GetHash(), pblock->vchBlockSig)) {
                LogPrint(BCLog::STAKING, "%s: failed to sign block hash with key", __func__);
                continue;
            }
        }

        std::shared_ptr<const CBlock> shared_pblock = std::make_shared<const CBlock>(*pblock);
        if (!ProcessNewBlock(Params(), shared_pblock, true, nullptr))
            throw JSONRPCError(RPC_INTERNAL_ERROR, "ProcessNewBlock, block not accepted");
        ++nHeight;
        blockHashes.push_back(pblock->GetHash().GetHex());

        //mark script as important because it was used at least for one coinbase output if the script came from the wallet
        if (keepScript)
        {
            coinbaseKey->KeepKey();
        }
    }
    return blockHashes;
}
//#endif // ENABLE_MINER
