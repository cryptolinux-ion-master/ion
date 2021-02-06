// Copyright (c) 2012-2014 The Bitcoin developers
// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2015-2018 The PIVX developers
// Copyright (c) 2018-2019 The Ion developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_VERSION_H
#define BITCOIN_VERSION_H

/**
 * network protocol versioning
 */

static const int PROTOCOL_VERSION = 95704;

//! initial proto version, to be increased after version/verack negotiation
static const int INIT_PROTO_VERSION = 901;

//! In this version, 'getheaders' was introduced.
static const int GETHEADERS_VERSION = 96000;

//! disconnect from peers older than this proto version
static const int MIN_PEER_PROTO_VERSION_BEFORE_ENFORCEMENT = 95702;
static const int MIN_PEER_PROTO_VERSION_AFTER_ENFORCEMENT = 95702;

//! masternodes older than this proto version use old strMessage format for mnannounce
static const int MIN_PEER_MNANNOUNCE = 95700;

//! nTime field added to CAddress, starting with this version;
//! if possible, avoid requesting addresses nodes older than this
static const int CADDR_TIME_VERSION = 95700;

//! BIP 0031, pong message, is enabled for all versions AFTER this one
static const int BIP0031_VERSION = 95700;

//! "filter*" commands are disabled without NODE_BLOOM after and including this version
static const int NO_BLOOM_VERSION = 95700;

//! "sendheaders" command and announcing blocks with headers starts with this version
static const int SENDHEADERS_VERSION = 96000;

//! DIP0001 was activated in this version
static const int DIP0001_PROTOCOL_VERSION = 96000;

//! short-id-based block download starts with this version
static const int SHORT_IDS_BLOCKS_VERSION = 96000;

//! introduction of DIP3/deterministic masternodes
static const int DMN_PROTO_VERSION = 96000;

//! introduction of LLMQs
static const int LLMQS_PROTO_VERSION = 96000;

//! introduction of SENDDSQUEUE
//! TODO we can remove this in 0.15.0.0
static const int SENDDSQUEUE_PROTO_VERSION = 96000;

//! protocol version is included in MNAUTH starting with this version
static const int MNAUTH_NODE_VER_VERSION = 70218;

#endif // BITCOIN_VERSION_H
