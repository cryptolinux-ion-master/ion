// Copyright (c) 2014-2020 The Ion Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "reward-manager.h"

#include "init.h"
#include "masternode/masternode-sync.h"
#include "policy/policy.h"
#include "validation.h"
#include "wallet/wallet.h"

// fix windows build
#include <boost/thread.hpp>

std::shared_ptr<CRewardManager> rewardManager;

CRewardManager::CRewardManager() :
        fEnableRewardManager(false), fEnableAutoCombineRewards(false), nAutoCombineThreshold(0) {
}

bool CRewardManager::IsReady() {
    if (!fEnableRewardManager) return false;

    if (pwallet == nullptr || pwallet->IsLocked()) {
        return false;
    }
    bool fHaveConnections = !g_connman ? false : g_connman->GetNodeCount(CConnman::CONNECTIONS_ALL) > 0;
    if (!fHaveConnections || !masternodeSync.IsSynced()) {
        return false;
    }
    const CBlockIndex* tip = chainActive.Tip();
    if (tip == nullptr || tip->nTime < (GetAdjustedTime() - 300)) {
        return false;
    }
    return true;
}

bool CRewardManager::IsCombining()
{
    return IsReady() && IsAutoCombineEnabled();
}

void CRewardManager::AutoCombineSettings(bool fEnable, CAmount nAutoCombineThresholdIn) {
    LOCK(cs);
    fEnableAutoCombineRewards = fEnable;
    nAutoCombineThreshold = nAutoCombineThresholdIn;
}

// TODO: replace with pwallet->FilterCoins()
std::map<CBitcoinAddress, std::vector<COutput> > CRewardManager::AvailableCoinsByAddress(bool fConfirmed, CAmount maxCoinValue) {
    std::vector<COutput> vCoins;
    pwallet->AvailableCoins(vCoins, fConfirmed);

    std::map<CBitcoinAddress, std::vector<COutput> > mapCoins;
    for (COutput out : vCoins) {
        if (maxCoinValue > 0 && out.tx->tx->vout[out.i].nValue > maxCoinValue)
            continue;

        CTxDestination address;
        if (!ExtractDestination(out.tx->tx->vout[out.i].scriptPubKey, address))
            continue;

        mapCoins[CBitcoinAddress(address)].push_back(out);
    }

    return mapCoins;
}

void CRewardManager::AutoCombineRewards() {
    LOCK2(cs_main, pwallet->cs_wallet);

    std::map<CBitcoinAddress, std::vector<COutput> > mapCoinsByAddress = AvailableCoinsByAddress(true, nAutoCombineThreshold * COIN);

    //coins are sectioned by address. This combination code only wants to combine inputs that belong to the same address
    for (std::map<CBitcoinAddress, std::vector<COutput> >::iterator it = mapCoinsByAddress.begin(); it != mapCoinsByAddress.end(); it++) {
        std::vector<COutput> vCoins, vRewardCoins;
        bool maxSize = false;
        vCoins = it->second;

        // We don't want the tx to be refused for being too large
        // we use 50 bytes as a base tx size (2 output: 2*34 + overhead: 10 -> 90 to be certain)
        unsigned int txSizeEstimate = 90;

        //find masternode rewards that need to be combined
        CCoinControl coinControl;
        CAmount nTotalRewardsValue = 0;
        for (const COutput& out : vCoins) {
            if (!out.fSpendable)
                continue;

            COutPoint outpt(out.tx->GetHash(), out.i);
            coinControl.Select(outpt);
            vRewardCoins.push_back(out);
            nTotalRewardsValue += out.GetValue();

            // Combine to the threshold and not way above
            if (nTotalRewardsValue > nAutoCombineThreshold * COIN)
                break;

            // Around 180 bytes per input. We use 190 to be certain
            txSizeEstimate += 190;
            if (txSizeEstimate >= MAX_STANDARD_TX_SIZE - 200) {
                maxSize = true;
                break;
            }
        }

        //if no inputs found then return
        if (!coinControl.HasSelected())
            continue;

        //we cannot combine one coin with itself
        if (vRewardCoins.size() <= 1)
            continue;

        std::vector<CRecipient> vecSend;
        int nChangePosRet = -1;
        CScript scriptPubKey = GetScriptForDestination(it->first.Get());
        // 10% safety margin to avoid "Insufficient funds" errors
        CRecipient recipient = {scriptPubKey, nTotalRewardsValue - (nTotalRewardsValue / 10), false};
        vecSend.push_back(recipient);

        //Send change to same address
        CTxDestination destMyAddress;
        if (!ExtractDestination(scriptPubKey, destMyAddress)) {
            LogPrintf("AutoCombineDust: failed to extract destination\n");
            continue;
        }
        coinControl.destChange = destMyAddress;

        // Create the transaction and commit it to the network
        CWalletTx wtx;
        CReserveKey keyChange(pwallet); // this change address does not end up being used, because change is returned with coin control switch
        std::string strErr;
        CAmount nFeeRet = 0;

        if (!pwallet->CreateTransaction(vecSend, wtx, keyChange, nFeeRet, nChangePosRet, strErr, coinControl)) {
            LogPrintf("AutoCombineDust createtransaction failed, reason: %s\n", strErr);
            continue;
        }

        //we don't combine below the threshold unless the fees are 0 to avoid paying fees over fees over fees
        if (!maxSize && nTotalRewardsValue < nAutoCombineThreshold * COIN && nFeeRet > 0)
            continue;

        CValidationState state;
        if (!pwallet->CommitTransaction(wtx, keyChange, g_connman.get(), state)) {
            LogPrintf("AutoCombineDust transaction commit failed\n");
            continue;
        }

        LogPrintf("AutoCombineDust sent transaction\n");
    }
}

void CRewardManager::DoMaintenance(CConnman& connman) {
    if (!IsReady()) {
        MilliSleep(5 * 60 * 1000); // Wait 5 minutes
        return;
    }

    if (IsAutoCombineEnabled()) {
        AutoCombineRewards();
    }
}
