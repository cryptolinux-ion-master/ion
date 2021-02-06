// Copyright (c) 2015-2017 The Bitcoin Unlimited developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#include <algorithm>
#include "coins.h"
#include "consensus/tokengroups.h"
#include "dstencode.h"
#include "init.h"
#include "main.h"
#include "coincontrol.h"
#include "primitives/transaction.h"
#include "rpc/protocol.h"
#include "script/script.h"
#include "script/standard.h"
#include "pubkey.h"
#include "ionaddrenc.h"
#include "random.h"
#include "rpc/server.h"
#include "tokens/tokengroupmanager.h"
#include "utilmoneystr.h"
#include "wallet/wallet.h"
#include "tokens/tokengroupwallet.h"

#include <unordered_map>
#include <univalue.h>

CTokenGroupID NoGroup; // No group specified.

std::string EncodeGroupAuthority(const GroupAuthorityFlags flags) {
    std::string sflags = "none";
    if (hasCapability(flags, GroupAuthorityFlags::CTRL)) {
        sflags = "";
        if (hasCapability(flags, GroupAuthorityFlags::MINT)) {
            sflags += "mint";
        }
        if (hasCapability(flags, GroupAuthorityFlags::MELT)) {
            if (sflags != "") sflags += " ";
            sflags += "melt";
        }
        if (hasCapability(flags, GroupAuthorityFlags::CCHILD)) {
            if (sflags != "") sflags += " ";
            sflags += "child";
        } else {
            if (sflags != "") sflags += " ";
            sflags += "nochild";
        }
        if (hasCapability(flags, GroupAuthorityFlags::RESCRIPT)) {
            if (sflags != "") sflags += " ";
            sflags += "rescript";
        }
        if (hasCapability(flags, GroupAuthorityFlags::SUBGROUP)) {
            if (sflags != "") sflags += " ";
            sflags += "subgroup";
        }
        if (hasCapability(flags, GroupAuthorityFlags::CONFIGURE)) {
            if (sflags != "") sflags += " ";
            sflags += "configure";
        }
    }
    return sflags;
}

bool IsOutputGrouped(const CTxOut &txout) {
    CTokenGroupInfo grp(txout.scriptPubKey);
    if (grp.invalid)
        return true; // Its still grouped even if invalid
    if (grp.associatedGroup != NoGroup)
        return true;

    return false;
}

bool IsOutputGroupedAuthority(const CTxOut &txout) {
    CTokenGroupInfo grp(txout.scriptPubKey);
    if (grp.invalid)
        return true;
    if (grp.associatedGroup != NoGroup && grp.isAuthority())
        return true;

    return false;
}

bool IsAnyOutputGrouped(const CTransaction &tx)
{
    for (const CTxOut &txout : tx.vout)
    {
        if (IsOutputGrouped(txout)) {
            return true;
        }
    }

    return false;
}

bool IsAnyOutputGroupedAuthority(const CTransaction &tx) {
    for (const CTxOut &txout : tx.vout)
    {
        if (IsOutputGroupedAuthority(txout)) {
            return true;
        }
    }

    return false;
}

bool IsAnyOutputGroupedCreation(const CTransaction &tx, const TokenGroupIdFlags tokenGroupIdFlags)
{
    for (const CTxOut& txout : tx.vout) {
        CTokenGroupInfo grp(txout.scriptPubKey);
        if (grp.invalid)
            return false;
        if (grp.isGroupCreation(tokenGroupIdFlags))
            return true;
    }
    return false;
}

bool AnyInputsGrouped(const CTransaction &transaction, const CCoinsViewCache& view, const CTokenGroupID tgID) {
    bool anyInputsGrouped = false;
    if (!transaction.IsCoinBase() && !transaction.IsCoinStake() && !transaction.HasZerocoinSpendInputs()) {

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
                if (tokenGrp.associatedGroup == tgID) {
                    LogPrint("token", "%s - Matched a TokenGroup input: [%s] at height [%d]\n", __func__, coin.out.ToString(), coin.nHeight);
                    anyInputsGrouped = true;
                }
            }
        }
    }

    return anyInputsGrouped;
}

std::vector<unsigned char> SerializeAmount(CAmount num)
{
    CDataStream strm(SER_NETWORK, CLIENT_VERSION);
    if (num < 0) // negative numbers are serialized at full length
    {
        int64_t xSize = num;
        WRITEDATA(strm, xSize);
    }
    /* Disallow amounts to be encoded as a single byte because these may need to have special encodings if
       the SCRIPT_VERIFY_MINIMALDATA flag is set
    else if (num < 256)
    {
        unsigned char chSize = num;
        WRITEDATA(strm, chSize);
//        ser_writedata8(strm, num);
    }
    */
    else if (num <= std::numeric_limits<unsigned short>::max())
    {
        unsigned short xSize = num;
        WRITEDATA(strm, xSize);
//        ser_writedata16(strm, num);
    }
    else if (num <= std::numeric_limits<unsigned int>::max())
    {
        unsigned int xSize = num;
        WRITEDATA(strm, xSize);
//        ser_writedata32(strm, num);
    }
    else
    {
        uint64_t xSize = num;
        WRITEDATA(strm, xSize);
//        ser_writedata64(strm, num);
    }
    return std::vector<unsigned char>(strm.begin(), strm.end());
}

CAmount DeserializeAmount(opcodetype opcodeQty, std::vector<unsigned char> &vec)
{
    /* Disallow raw opcodes or single byte sizes, because having them is an unnecessary decode complication
    if ((opcodeQty >= OP_1) && (opcodeQty <= OP_16))
    {
        return opcodeQty-OP_1+1;
    }
    if (opcodeQty == OP_1NEGATE)
    {
        return 0x81;
    }

    int sz = vec.size();
    if (sz == 1)
    {
        return vec[0];
    }
    */
    int sz = vec.size();
    CDataStream strm(vec, SER_NETWORK, CLIENT_VERSION);
    if (sz == 2)
    {
        unsigned short xSize;
        READDATA(strm, xSize);
        return xSize;
//        return ser_readdata16(strm);
    }
    if (sz == 4)
    {
        unsigned int xSize;
        READDATA(strm, xSize);
        return xSize;
//        return ser_readdata32(strm);
    }
    if (sz == 8)
    {
        uint64_t v;
        READDATA(strm, v);
//        uint64_t v = ser_readdata64(strm);
        return (CAmount) v;
    }
    throw std::ios_base::failure("DeserializeAmount(): invalid format");
}

#if 0
CTokenGroupID ExtractControllingGroup(const CScript &scriptPubKey)
{
    txnouttype whichType;
    typedef std::vector<unsigned char> valtype;
    std::vector<valtype> vSolutions;
    if (!Solver(scriptPubKey, whichType, vSolutions))
        return CTokenGroupID();

    // only certain well known script types are allowed to mint or melt
    if ((whichType == TX_PUBKEYHASH) || (whichType == TX_GRP_PUBKEYHASH) || (whichType == TX_SCRIPTHASH) ||
        (whichType == TX_GRP_SCRIPTHASH))
    {
        return CTokenGroupID(uint160(vSolutions[0]));
    }
    return CTokenGroupID();
}
#endif

CTokenGroupInfo::CTokenGroupInfo(const CScript &script)
    : associatedGroup(), quantity(0), invalid(false)
{
    CScript::const_iterator pc = script.begin();
    std::vector<unsigned char> groupId;
    std::vector<unsigned char> tokenQty;
    std::vector<unsigned char> data;
    opcodetype opcode;
    opcodetype opcodeGrp;
    opcodetype opcodeQty;

    // mintMeltGroup = ExtractControllingGroup(script);

    if (!script.GetOp(pc, opcodeGrp, groupId))
    {
        associatedGroup = NoGroup;
        return;
    }

    if (!script.GetOp(pc, opcodeQty, tokenQty))
    {
        associatedGroup = NoGroup;
        return;
    }

    if (!script.GetOp(pc, opcode, data))
    {
        associatedGroup = NoGroup;
        return;
    }

    if (opcode != OP_GROUP)
    {
        associatedGroup = NoGroup;
        return;
    }
    else // If OP_GROUP is used, enforce rules on the other fields
    {
        // group must be 32 bytes or more
        if (opcodeGrp < 0x20)
        {
            invalid = true;
            return;
        }
        /* Disallow amounts to be encoded as a single byte because these may need to have special encodings if
           the SCRIPT_VERIFY_MINIMALDATA flag is set
        // quantity must be 1, 2, 4, or 8 bytes
        if (((opcodeQty < OP_1)||(opcodeQty > OP_16)) && (opcodeQty != OP_1NEGATE) && (opcodeQty != 1) && (opcodeQty != 2)
           && (opcodeQty != 4) && (opcodeQty != 8))
        {
            invalid = true;
            return;
        }
        */

        // Quantity must be a 2, 4, or 8 byte number
        if ((opcodeQty != 2) && (opcodeQty != 4) && (opcodeQty != 8))
        {
            invalid = true;
            return;
        }
    }

    try
    {
        quantity = DeserializeAmount(opcodeQty, tokenQty);
    }
    catch (std::ios_base::failure &f)
    {
        invalid = true;
    }
    associatedGroup = groupId;
}


bool CheckTokenGroups(const CTransaction &tx, CValidationState &state, const CCoinsViewCache &view, std::unordered_map<CTokenGroupID, CTokenGroupBalance>& gBalance)
{
    gBalance.clear();

    // Tokens minted from the tokenGroupManagement address can create management tokens
    bool anyInputsGroupManagement = false;

    CScript firstOpReturn;

    // Iterate through all the outputs constructing the final balances of every group.
    for (const auto &outp : tx.vout)
    {
        const CScript &scriptPubKey = outp.scriptPubKey;
        CTokenGroupInfo tokenGrp(scriptPubKey);
        if ((outp.nValue == 0) && (firstOpReturn.size() == 0) && (outp.scriptPubKey[0] == OP_RETURN))
        {
            firstOpReturn = outp.scriptPubKey; // Used later if this is a group creation transaction
        }
        if (tokenGrp.invalid)
            return state.Invalid(false, REJECT_INVALID, "bad OP_GROUP");
        if (tokenGrp.associatedGroup != NoGroup)
        {
            gBalance[tokenGrp.associatedGroup].numOutputs += 1;

            if (tokenGrp.quantity > 0)
            {
                if (std::numeric_limits<CAmount>::max() - gBalance[tokenGrp.associatedGroup].output < tokenGrp.quantity)
                    return state.Invalid(false, REJECT_INVALID, "token overflow");
                gBalance[tokenGrp.associatedGroup].output += tokenGrp.quantity;
            }
            else if (tokenGrp.quantity == 0)
            {
                return state.Invalid(false, REJECT_INVALID, "OP_GROUP quantity is zero");
            }
            else // this is an authority output
            {
                gBalance[tokenGrp.associatedGroup].ctrlOutputPerms |= (GroupAuthorityFlags)tokenGrp.quantity;
            }
        }
    }

    // Now iterate through the inputs applying them to match outputs.
    // If any input utxo address matches a non-bitcoin group address, defer since this could be a mint or burn
    for (const auto &inp : tx.vin)
    {
        const COutPoint &prevout = inp.prevout;
        const Coin &coin = view.AccessCoin(prevout);
        if (coin.IsSpent()) // should never happen because you've already CheckInputs(tx,...)
        {
            LogPrint("token", "%s - Checking token group for spent coin\n", __func__);
            return state.Invalid(false, REJECT_INVALID, "already-spent");
        }
        // no prior coins can be grouped.
        if (coin.nHeight < Params().OpGroup_StartHeight())
            continue;
        const CScript &script = coin.out.scriptPubKey;
        if (tokenGroupManager->IsManagementTokenInput(script))
            anyInputsGroupManagement = true;

        CTokenGroupInfo tokenGrp(script);
        // The prevout should never be invalid because that would mean that this node accepted a block with an
        // invalid OP_GROUP tx in it.
        if (tokenGrp.invalid)
            continue;
        CAmount amount = tokenGrp.quantity;
        if (tokenGrp.controllingGroupFlags() != GroupAuthorityFlags::NONE)
        {
            auto temp = tokenGrp.controllingGroupFlags();
            // outputs can have all the permissions of inputs, except for 1 special case
            // If CCHILD is not set, no outputs can be authorities (so unset the CTRL flag)
            if (hasCapability(temp, GroupAuthorityFlags::CCHILD))
            {
                gBalance[tokenGrp.associatedGroup].allowedCtrlOutputPerms |= temp;
                if (hasCapability(temp, GroupAuthorityFlags::SUBGROUP))
                    gBalance[tokenGrp.associatedGroup].allowedSubgroupCtrlOutputPerms |= temp;
            }
            // Track what permissions this transaction has
            gBalance[tokenGrp.associatedGroup].ctrlPerms |= temp;
        }
        if ((tokenGrp.associatedGroup != NoGroup) && !tokenGrp.isAuthority())
        {
            if (std::numeric_limits<CAmount>::max() - gBalance[tokenGrp.associatedGroup].input < amount)
                return state.Invalid(false, REJECT_INVALID, "token overflow");
            gBalance[tokenGrp.associatedGroup].input += amount;
        }
    }

    // Now pass thru the outputs applying parent group capabilities to any subgroups
    for (auto &txo : gBalance)
    {
        CTokenGroupID group = txo.first;
        CTokenGroupBalance &bal = txo.second;
        if (group.isSubgroup())
        {
            CTokenGroupID parentgrp = group.parentGroup();
            auto parentSearch = gBalance.find(parentgrp);
            if (parentSearch != gBalance.end()) // The parent group is part of the inputs
            {
                CTokenGroupBalance &parentData = parentSearch->second;
                if (hasCapability(parentData.ctrlPerms, GroupAuthorityFlags::SUBGROUP))
                {
                    // Give the subgroup has all the capabilities the parent group had,
                    // except no recursive subgroups so remove the subgrp authority bit.
                    bal.ctrlPerms |= parentData.ctrlPerms & ~(GroupAuthorityFlags::SUBGROUP);
                }

                // Give the subgroup authority printing permissions as specified by the parent group
                bal.allowedCtrlOutputPerms |=
                    parentData.allowedSubgroupCtrlOutputPerms & ~(GroupAuthorityFlags::SUBGROUP);
            }
        }
    }

    // Now pass thru the outputs ensuring balance or mint/melt permission
    for (auto &txo : gBalance)
    {
        CTokenGroupBalance &bal = txo.second;
        // If it has an authority, with no input authority, check mint
        if (hasCapability(bal.ctrlOutputPerms, GroupAuthorityFlags::CTRL) &&
            (bal.ctrlPerms == GroupAuthorityFlags::NONE))
        {
            CHashWriter mintGrp(SER_GETHASH, PROTOCOL_VERSION);
            mintGrp << tx.vin[0].prevout;
            if (firstOpReturn.size())
            {
                std::vector<unsigned char> data(firstOpReturn.begin(), firstOpReturn.end());
                mintGrp << data;
            }
            mintGrp << (((uint64_t)bal.ctrlOutputPerms) & ~((uint64_t)GroupAuthorityFlags::ALL_BITS));
            CTokenGroupID newGrpId(mintGrp.GetHash());

            if (newGrpId == txo.first) // This IS new group because id matches hash, so allow all authority.
            {
                if (bal.numOutputs != 1) // only allow the single authority tx during a create
                    return state.Invalid(false, REJECT_GROUP_IMBALANCE, "grp-invalid-create",
                         "Multiple grouped outputs created during group creation transaction");

                if (newGrpId.hasFlag(TokenGroupIdFlags::MGT_TOKEN))
                {
                    if (anyInputsGroupManagement) {
                        LogPrint("token", "%s - Group management creation transaction. newGrpId=[%s]\n", __func__, EncodeTokenGroup(newGrpId));
                    } else {
                        return state.Invalid(false, REJECT_INVALID, "grp-invalid-tx",
                            "No group management capability at any input address");
                    }
                }

                bal.allowedCtrlOutputPerms = bal.ctrlPerms = GroupAuthorityFlags::ALL;
            }
            else
            {
                if (((uint64_t)bal.ctrlOutputPerms & (uint64_t)~GroupAuthorityFlags::ALL_BITS) != 0)
                {
                    return state.Invalid(false, REJECT_INVALID, "grp-invalid-tx",
                         "Only mint transactions can have a nonce");
                }
            }
        }

        if ((bal.input > bal.output) && !hasCapability(bal.ctrlPerms, GroupAuthorityFlags::MELT))
        {
            return state.Invalid(false, REJECT_GROUP_IMBALANCE, "grp-invalid-melt",
                "Group input exceeds output, but no melt permission");
        }
        if ((bal.input < bal.output) && !hasCapability(bal.ctrlPerms, GroupAuthorityFlags::MINT))
        {
            return state.Invalid(false, REJECT_GROUP_IMBALANCE, "grp-invalid-mint",
                "Group output exceeds input, but no mint permission");
        }
        // Some output permissions are set that are not in the inputs
        if (((uint64_t)(bal.ctrlOutputPerms & GroupAuthorityFlags::ALL)) &
            ~((uint64_t)(bal.allowedCtrlOutputPerms & GroupAuthorityFlags::ALL)))
        {
                return state.Invalid(false, REJECT_GROUP_IMBALANCE, "grp-invalid-perm",
                "Group output permissions exceeds input permissions");
        }
    }

    return true;
}


bool CTokenGroupID::isUserGroup(void) const { return (!data.empty()); }bool CTokenGroupID::isSubgroup(void) const { return (data.size() > PARENT_GROUP_ID_SIZE); }
CTokenGroupID CTokenGroupID::parentGroup(void) const
{
    if (data.size() <= PARENT_GROUP_ID_SIZE)
        return CTokenGroupID(data);
    return CTokenGroupID(std::vector<unsigned char>(data.begin(), data.begin() + PARENT_GROUP_ID_SIZE));
}
std::vector<unsigned char> CTokenGroupID::GetSubGroupData() {
    std::vector<unsigned char> subgroupData;
    if (data.size() > PARENT_GROUP_ID_SIZE) {
        subgroupData = std::vector<unsigned char>(data.begin() + PARENT_GROUP_ID_SIZE, data.end());
    }
    return subgroupData;
}

bool CTokenGroupID::hasFlag(TokenGroupIdFlags flag) const {
    return data.size() >= PARENT_GROUP_ID_SIZE ? hasTokenGroupIdFlag((TokenGroupIdFlags)data[31], flag) : false;
}
