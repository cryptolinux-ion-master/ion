// Copyright (c) 2019 The ION Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "tokengroupmanager.h"

#include "rpc/protocol.h"
#include "utilstrencodings.h"

#include <univalue.h>
#include <iostream>
#include <regex>
#include <string.h>

std::shared_ptr<CTokenGroupManager> tokenGroupManager;

bool CTokenGroupDescription::ValidateTokenDescription() {
    regex regexAlpha("^[a-zA-Z]+$");
    regex regexUrl(R"((https?|ftp)://(-\.)?([^\s/?\.#-]+\.?)+(/[^\s]*)?$)");

    smatch matchResult;
    if (!std::regex_match(strName, matchResult, regexAlpha)) {
        LogPrint("token", "Token name can only contain letters.");
        invalid = true;
        return false;
    }
    if (!std::regex_match(strTicker, matchResult, regexAlpha)) {
        LogPrint("token", "Token ticker can only contain letters.");
        invalid = true;
        return false;
    }
    if (!std::regex_match(strDocumentUrl, matchResult, regexUrl)) {
        LogPrint("token", "Token description document URL cannot be parsed.");
        invalid = true;
        return false;
    }
    invalid = false;
    return true;
}

bool CTokenGroupManager::BuildGroupDescData(CScript script, std::vector<std::vector<unsigned char> > &descriptionData) {
    std::vector<std::vector<unsigned char> > desc;

    CScript::const_iterator pc = script.begin();
    std::vector<unsigned char> data;
    opcodetype opcode;

    if (!script.GetOp(pc, opcode, data)) return false;
    if (opcode != OP_RETURN) return false;

    if (!script.GetOp(pc, opcode, data)) return false;
    uint32_t OpRetGroupId;
    if (data.size()!=4) return false;
    OpRetGroupId = (uint32_t)data[3] << 24 | (uint32_t)data[2] << 16 | (uint32_t)data[1] << 8 | (uint32_t)data[0];
    if (OpRetGroupId != 88888888) return false;

    while (script.GetOp(pc, opcode, data)) {
        LogPrintf("More data! opcode=[%d] data=[%s]\n", opcode, std::string(data.begin(), data.end()));
        desc.emplace_back(data);
    }
    descriptionData = desc;
    return true;
}

bool CTokenGroupManager::ParseGroupDescData(const std::vector<std::vector<unsigned char> > descriptionData, CTokenGroupDescription &tokenGroupDescription) {
    std::string tickerStr;
    std::string name;
    std::string url;
    uint256 docHash;

    try {
        tickerStr = std::string(descriptionData[0].begin(), descriptionData[0].end());
        name = std::string(descriptionData[1].begin(), descriptionData[1].end());
        url = std::string(descriptionData[2].begin(), descriptionData[2].end());
        docHash = uint256(descriptionData[3]);
    } catch (const std::exception& e) {
        return false;
    }
    CTokenGroupDescription tgDesc(tickerStr, name, url, docHash);
    if (tgDesc.ValidateTokenDescription()) tokenGroupDescription = tgDesc;

    return true;
}

bool CTokenGroupManager::ProcessManagementTokenGroups(CTokenGroupCreation tokenGroupCreation) {
    if (!tgMagicCreation && tokenGroupCreation.tokenGroupDescription.strTicker == "MAGIC") {
        this->tgMagicCreation = std::unique_ptr<CTokenGroupCreation>(new CTokenGroupCreation((tokenGroupCreation)));
        return true;
    } else if (!tgDarkMatterCreation && tokenGroupCreation.tokenGroupDescription.strTicker == "XDM") {
        this->tgDarkMatterCreation = std::unique_ptr<CTokenGroupCreation>(new CTokenGroupCreation((tokenGroupCreation)));
        return true;
    } else if (!tgAtomCreation && tokenGroupCreation.tokenGroupDescription.strTicker == "ATOM") {
        this->tgAtomCreation = std::unique_ptr<CTokenGroupCreation>(new CTokenGroupCreation((tokenGroupCreation)));
        return true;
    }
    return false;
}

void CTokenGroupManager::ClearManagementTokenGroups() {
    tgMagicCreation.reset();
    tgDarkMatterCreation.reset();
    tgAtomCreation.reset();
}

bool CTokenGroupManager::AddTokenGroup(CTransaction tx, CTokenGroupCreation &newTokenGroupCreation) {
    CScript firstOpReturn;
    CTokenGroupInfo tokenGroupInfo;

    bool hasNewTokenGroup = false;

    for (const auto &txout : tx.vout) {
        const CScript &scriptPubKey = txout.scriptPubKey;
        CTokenGroupInfo tokenGrp(scriptPubKey);
        if ((txout.nValue == 0) && (firstOpReturn.size() == 0) && (txout.scriptPubKey[0] == OP_RETURN)) {
            firstOpReturn = txout.scriptPubKey;
        }
        if (tokenGrp.invalid)
            return false;
        if (tokenGrp.associatedGroup != NoGroup && tokenGrp.isGroupCreation() && !hasNewTokenGroup) {
            hasNewTokenGroup = true;
            tokenGroupInfo = tokenGrp;
        }
    }
    if (hasNewTokenGroup) {
        CTokenGroupDescription tokenGroupDescription;
        if (firstOpReturn.size()) {
            std::vector<std::vector<unsigned char> > desc;
            if (BuildGroupDescData(firstOpReturn, desc)) {
                ParseGroupDescData(desc, tokenGroupDescription);
            }
        }

        CTokenGroupCreation tokenGroupCreation = CTokenGroupCreation(tx, tokenGroupInfo, tokenGroupDescription);

        ProcessManagementTokenGroups(tokenGroupCreation);

        std::pair<std::map<CTokenGroupID, CTokenGroupCreation>::iterator, bool> ret;
        ret = mapTokenGroups.insert(std::pair<CTokenGroupID, CTokenGroupCreation>(tokenGroupInfo.associatedGroup, tokenGroupCreation));

        CTokenGroupCreation& tokenGroupCreationRet = (*ret.first).second;
        bool fInsertedNew = ret.second;
        if (!fInsertedNew) {
            if (!(tokenGroupCreation == tokenGroupCreationRet)) {
                mapTokenGroups[tokenGroupInfo.associatedGroup] = tokenGroupCreation;
                // Token ID already exists. Since the hash is the same, the token specs are the same.
                // However, until reorgs are handled  properly: log this to avoid 'misplaced' token group creation transactions.
                LogPrint("token", "%s - Double token creation; updated.\n", __func__);
            } else {
                LogPrint("token", "%s - Double token creation; NOT updated.\n", __func__);
            }
        } else {
            newTokenGroupCreation = tokenGroupCreationRet;
        }

    }
    return true;
}

void CTokenGroupManager::ResetTokenGroups() {
    mapTokenGroups.clear();
}

bool CTokenGroupManager::RemoveTokenGroup(CTransaction tx, CTokenGroupID &newTokenGroupID) {
    CScript firstOpReturn;
    CTokenGroupInfo tokenGroupInfo;

    bool hasNewTokenGroup = false;

    for (const auto &txout : tx.vout) {
        const CScript &scriptPubKey = txout.scriptPubKey;
        CTokenGroupInfo tokenGrp(scriptPubKey);
        if ((txout.nValue == 0) && (firstOpReturn.size() == 0) && (txout.scriptPubKey[0] == OP_RETURN)) {
            firstOpReturn = txout.scriptPubKey;
        }
        if (tokenGrp.invalid)
            return false;
        if (tokenGrp.associatedGroup != NoGroup && tokenGrp.isGroupCreation() && !hasNewTokenGroup) {
            hasNewTokenGroup = true;
            tokenGroupInfo = tokenGrp;
        }
    }
    if (hasNewTokenGroup) {
        std::map<CTokenGroupID, CTokenGroupCreation>::iterator iter = mapTokenGroups.find(tokenGroupInfo.associatedGroup);
        if (iter != mapTokenGroups.end()) {
            mapTokenGroups.erase(iter);
            return true;
        }
    }
    return false;
}

std::string CTokenGroupManager::GetTokenGroupNameByID(CTokenGroupID tokenGroupId) {
    CTokenGroupCreation tokenGroupCreation = mapTokenGroups.at(tokenGroupId);
    return "";
}

unsigned int CTokenGroupManager::GetXDMTxCount(const CBlock &block, const CCoinsViewCache& view, unsigned int &nXDMCount) {
    int nXDMCountInBlock = 0;
    for (auto tx : block.vtx) {
        if (!tx.IsCoinBase() && !tx.ContainsZerocoins()) {
            if (IsXDMTx(tx, view)) {
                nXDMCountInBlock++;
            }
        }
    }
    nXDMCount += nXDMCountInBlock;
    return nXDMCountInBlock;
}

bool CTokenGroupManager::IsXDMTx(const CTransaction &transaction, const CCoinsViewCache& view) {
    if (!tgDarkMatterCreation) return false;

    bool anyInputsXDM = false;
    if (!transaction.IsCoinBase() && !transaction.IsCoinStake() && !transaction.IsZerocoinSpend()) {

        if (!view.HaveInputs(transaction))
            return false;

        if (((int)chainActive.Tip()->nHeight >= Params().OpGroup_StartHeight())) {
            // Now iterate through the inputs to match to DarkMatter inputs
            for (const auto &inp : transaction.vin)
            {
                const COutPoint &prevout = inp.prevout;
                const Coin &coin = view.AccessCoin(prevout);
                if (coin.IsSpent()) {
                    LogPrint("token", "%s - Checking token group for spent coin\n", __func__);
                    return false;
                }
                // no prior coins can be grouped.
                if (coin.nHeight < Params().OpGroup_StartHeight())
                    continue;
                const CScript &script = coin.out.scriptPubKey;

                CTokenGroupInfo tokenGrp(script);
                // The prevout should never be invalid because that would mean that this node accepted a block with an
                // invalid OP_GROUP tx in it.
                if (tokenGrp.invalid)
                    continue;
                if (tokenGrp.associatedGroup == tgDarkMatterCreation->tokenGroupInfo.associatedGroup) {
                    LogPrint("token", "%s - Found a XDM input: [%s] at height [%d]\n", __func__, coin.out.ToString(), coin.nHeight);
                    anyInputsXDM = true;
                }
            }
        }
    }

    return anyInputsXDM;
}
