// Copyright (c) 2019-2020 The Ion Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef TOKEN_GROUP_DESCRIPTION_H
#define TOKEN_GROUP_DESCRIPTION_H

#include "amount.h"
#include "script/script.h"
#include "uint256.h"

static CAmount COINFromDecimalPos(const uint8_t& nDecimalPos) {
    uint8_t n = nDecimalPos <= 16 ? nDecimalPos : 0;
    static CAmount pow10[17] = {
        1, 10, 100, 1000, 10000, 100000, 1000000, 10000000,
        100000000, 1000000000, 10000000000, 100000000000, 1000000000000, 10000000000000, 100000000000000, 1000000000000000, 10000000000000000
    };

    return pow10[n];
}

class CTokenGroupDescription
{
public:
    // Token ticker name
    std::string strTicker;

    // Token name
    std::string strName;

    // Decimal position to translate between token value and amount
    uint8_t nDecimalPos;

    // Extended token description document URL
    std::string strDocumentUrl;

    uint256 documentHash;

    CTokenGroupDescription() { };
    CTokenGroupDescription(std::string strTicker, std::string strName, uint8_t nDecimalPosIn, std::string strDocumentUrl, uint256 documentHash) :
        strTicker(strTicker), strName(strName), strDocumentUrl(strDocumentUrl), documentHash(documentHash)
    {
        SetDecimalPos(nDecimalPosIn);
    };
    CTokenGroupDescription(CScript script) {
        std::vector<std::vector<unsigned char> > desc;
        if (BuildGroupDescData(script, desc)) {
            SetGroupDescData(desc);
        }
    }

private:
    void SetDecimalPos(uint8_t nDecimalPosIn) {
        nDecimalPos = nDecimalPosIn > 16 ? 8 : nDecimalPosIn;
    }

    inline std::string GetStringFromChars(const std::vector<unsigned char> chars, const uint32_t maxChars) const {
        return std::string(chars.begin(), chars.size() < maxChars ? chars.end() : std::next(chars.begin(), maxChars));
    }

public:
    bool SetGroupDescData(const std::vector<std::vector<unsigned char> > descriptionData);
    bool BuildGroupDescData(const CScript& script, std::vector<std::vector<unsigned char> > &descriptionData);


    // Tokens with no fractional quantities have nDecimalPos=0
    // ION has has decimalpos=8 (1 ION is 100000000 satoshi)
    // Maximum value is 10^16
    CAmount GetCoin() {
        return COINFromDecimalPos(nDecimalPos);
    }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action)
    {
        READWRITE(strTicker);
        READWRITE(strName);
        READWRITE(nDecimalPos);
        READWRITE(strDocumentUrl);
        READWRITE(documentHash);
    }
    bool operator==(const CTokenGroupDescription &c)
    {
        return (strTicker == c.strTicker && strName == c.strName && nDecimalPos == c.nDecimalPos && strDocumentUrl == c.strDocumentUrl && documentHash == c.documentHash);
    }
};

#endif
