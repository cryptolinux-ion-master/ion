# ION Core release notes

- [ION Core release notes](#ion-core-release-notes)
  - [ION Core version 5.0.01 is now available](#ion-core-version-5001-is-now-available)
    - [Mandatory Update](#mandatory-update)
    - [How to Upgrade](#how-to-upgrade)
    - [Compatibility](#compatibility)
    - [Noteable Changes](#noteable-changes)
      - [Migrate Travis as pipeline](#migrate-travis-as-pipeline)
      - [Hybrid Proof-of-Work / Proof-of-Stake](#hybrid-proof-of-work--proof-of-stake)
      - [Zerocoin](#zerocoin)
      - [Masternodes](#masternodes)
      - [Token implementation](#token-implementation)
    - [New RPC Commands](#new-rpc-commands)
      - [Masternodes](#masternodes-1)
    - [Deprecated RPC Commands](#deprecated-rpc-commands)
      - [Masternodes](#masternodes-2)
    - [5.0.01 Change log](#5001-change-log)

## ION Core version 5.0.01 is now available  

Download at: https://bitbucket.org/ioncoin/ion/downloads/

This is a new major version release, including a new base code, various bug fixes, performance improvements, upgrades to the Atomic Token Protocol (ATP), as well as updated translations.

Please report bugs using the issue tracker at github: https://bitbucket.org/ioncoin/ion/issues?status=new&status=open

### Mandatory Update

ION Core v5.0.99 is an experimental release, intended only for TESTNET. It is not a mandatory update. This release contains new consensus rules and improvements for TESTNET only that are not backwards compatible with older versions. Users will have a grace period of up to one week to update their testnet clients before enforcement of this update is enabled.

### How to Upgrade

- If you are running an older version, shut it down. 
- Wait until it has completely shut down (which might take a few minutes for older versions).
- Copy the files in the testnet data folder to a backup medium, and delete all files in the testnet folder except ioncoin.conf and wallet.dat.
- Run the installer (on Windows) or just copy over /Applications/ION-Qt (on Mac) or iond/ion-qt (on Linux).

### Compatibility

ION Core is extensively tested on multiple operating systems using the Linux kernel, macOS 10.8+, and Windows Vista and later.

Microsoft ended support for Windows XP on April 8th, 2014, No attempt is made to prevent installing or running the software on Windows XP, you can still do so at your own risk but be aware that there are known instabilities and issues. Please do not report issues about Windows XP to the issue tracker.

ION Core should also work on most other Unix-like systems but is not frequently tested on them.
Database space usage improvements
--------------------------------
Version 0.13.0.0 introduced a new database (evodb) which is found in the datadir of Ion Core. It turned
out that this database grows quite fast when a lot of changes inside the deterministic masternode list happen,
which is for example the case when a lot PoSe punishing/banning is happening. Such a situation happened
immediately after the activation LLMQ DKGs, causing the database to grow a lot. This release introduces
a new format in which information in "evodb" is stored, which causes it grow substantially slower.  

Version 0.14.0.0 also introduced a new database (llmq) which is also found in the datadir of Ion Core.
This database stores all LLMQ signatures for 7 days. After 7 days, a cleanup task removes old signatures.
The idea was that the "llmq" database would grow in the beginning and then stay at an approximately constant
size. The recent stress test on mainnet has however shown that the database grows too much and causes a risk
of out-of-space situations. This release will from now also remove signatures when the corresponding InstantSend
lock is fully confirmed on-chain (superseded by a ChainLock). This should remove >95% of all signatures from
the database. After the upgrade, no space saving will be observed however as this logic is only applied to new
signatures, which means that it will take 7 days until the whole "llmq" database gets to its minimum size.

DKG and LLMQ signing failures fixed
-----------------------------------
Recent stress tests have shown that masternodes start to ban each other under high load and specific situations.
This release fixes this and thus makes it a highly recommended upgrade for masternodes.

MacOS: macOS: disable AppNap during sync and mixing
---------------------------------------------------
AppNap is disabled now when Ion Core is syncing/reindexing or mixing.

Signed binaries for Windows
---------------------------
This release is the first one to include signed binaries for Windows.

New RPC command: quorum memberof <proTxHash>
--------------------------------------------
This RPC allows you to verify which quorums a masternode is supposed to be a member of. It will also show
if the masternode succesfully participated in the DKG process.

More information about number of InstantSend locks
--------------------------------------------------
The debug console will now show how many InstantSend locks Ion Core knows about. Please note that this number
does not necessarily equal the number of mempool transactions.

The "getmempoolinfo" RPC also has a new field now which shows the same information.

0.14.0.3 Change log
===================

See detailed [set of changes](https://bitbucket.org/ioncoin/ion/compare/v0.14.0.2...ionpay:v0.14.0.3).

- [`f2443709b`](https://bitbucket.org/ioncoin/ion/commit/f2443709b) Update release-notes.md for 0.14.0.3 (#3054)
- [`17ba23871`](https://bitbucket.org/ioncoin/ion/commit/17ba23871) Re-verify invalid IS sigs when the active quorum set rotated (#3052)
- [`8c49d9b54`](https://bitbucket.org/ioncoin/ion/commit/8c49d9b54) Remove recovered sigs from the LLMQ db when corresponding IS locks get confirmed (#3048)
- [`2e0cf8a30`](https://bitbucket.org/ioncoin/ion/commit/2e0cf8a30) Add "instantsendlocks" to getmempoolinfo RPC (#3047)
- [`a8fb8252e`](https://bitbucket.org/ioncoin/ion/commit/a8fb8252e) Use fEnablePrivateSend instead of fPrivateSendRunning
- [`a198a04e0`](https://bitbucket.org/ioncoin/ion/commit/a198a04e0) Show number of InstantSend locks in Debug Console (#2919)
- [`013169d63`](https://bitbucket.org/ioncoin/ion/commit/013169d63) Optimize on-disk deterministic masternode storage to reduce size of evodb (#3017)
- [`9ac7a998b`](https://bitbucket.org/ioncoin/ion/commit/9ac7a998b) Add "isValidMember" and "memberIndex" to "quorum memberof" and allow to specify quorum scan count (#3009)
- [`99824a879`](https://bitbucket.org/ioncoin/ion/commit/99824a879) Implement "quorum memberof" (#3004)
- [`7ea319fd2`](https://bitbucket.org/ioncoin/ion/commit/7ea319fd2) Bail out properly on Evo DB consistency check failures in ConnectBlock/DisconnectBlock (#3044)
- [`b1ffedb2d`](https://bitbucket.org/ioncoin/ion/commit/b1ffedb2d) Do not count 0-fee txes for fee estimation (#3037)
- [`974055a9b`](https://bitbucket.org/ioncoin/ion/commit/974055a9b) Fix broken link in PrivateSend info dialog (#3031)
- [`781b16579`](https://bitbucket.org/ioncoin/ion/commit/781b16579) Merge pull request #3028 from PastaPastaPasta/backport-12588
- [`5af6ce91d`](https://bitbucket.org/ioncoin/ion/commit/5af6ce91d) Add Ion Core Group codesign certificate (#3027)
- [`873ab896c`](https://bitbucket.org/ioncoin/ion/commit/873ab896c) Fix osslsigncode compile issue in gitian-build (#3026)
- [`ea8569e97`](https://bitbucket.org/ioncoin/ion/commit/ea8569e97) Backport #12783: macOS: disable AppNap during sync (and mixing) (#3024)
- [`4286dde49`](https://bitbucket.org/ioncoin/ion/commit/4286dde49) Remove support for InstantSend locked gobject collaterals (#3019)
- [`788d42dbc`](https://bitbucket.org/ioncoin/ion/commit/788d42dbc) Bump version to 0.14.0.3 and copy release notes (#3053)

Credits
=======

Thanks to everyone who directly contributed to this release:

- UdjinM6

As well as everyone that submitted issues and reviewed pull requests.

Older releases
==============

### Mandatory Update
___  

ION Core v5.0.99 is a mandatory update for all users. This release contains new consensus rules and improvements that are not backwards compatible with older versions. Users will have a grace period of up to two week to update their clients before enforcement of this update is enabled - a grace period that will end at block 1320000 the latest.

### How to Upgrade
___
If you are running an older version, shut it down. Wait until it has completely shut down (which might take a few minutes for older versions), then run the installer (on Windows) or just copy over /Applications/ION-Qt (on Mac) or iond/ion-qt (on Linux).

### Compatibility
ION Core is extensively tested on multiple operating systems using the Linux kernel, macOS 10.8+, and Windows Vista and later.

Microsoft ended support for Windows XP on April 8th, 2014, No attempt is made to prevent installing or running the software on Windows XP, you can still do so at your own risk but be aware that there are known instabilities and issues. Please do not report issues about Windows XP to the issue tracker.

ION Core should also work on most other Unix-like systems but is not frequently tested on them.

### Noteable Changes

- Move to bitbucket
  - The ION Core project is now available through the [ION Coin Bitbucket repository](https://bitbucket.org/ioncoin/ion/)
  - The ION Core repository at github will for now remain available, but will no longer be updated.
- Implementation of [IIP0006](https://github.com/ionomy/iips/blob/master/iip_0006.md)
  - Changes are adopted on mainnet at block <FORKBLOCK> and on testnet at block 155400.
  - Block rewards are reduced to 0.5 ION per block
    - 70% of block rewards are awarded to masternodes, 30% to stakers
  - Fee policy
    - The new fee policy, proposed and adopted in IIP0006, is implemented. As a result, this client will only relay and mine transactions with
      a fee rate of 0.01 ION per KB.
    - The fee calculation approach for token transactions has been made more accurate.
    - 50% of fees are awarded to masternodes, 25% to staking nodes and 25% is burned.
- Switch from Proof-of-Stake to Hybrid Proof-of-Work/Proof-of-Stake
  - During the Hybrid phase, miners can generate POW blocks and staking nodes can generate POS blocks.
  - Both the POW difficulty algorithm and the POS difficulty algorithm aim at producing 1 block every 2 minutes.
    - Combined, the ION chain will keep producing 1 block every 1 minute.
  - Miners do not receive ION rewards. Instead, they receive rewards in the Electron token (ELEC).
  - Mining uses Dash' X11 algorithm.
  - The block version number includes a bit to specify if it is a POW or a POS block.
- Core: the core code has been rebased from PivX to Dash.
  - The staking functionality has been ported to the new code base.
  - The token implementation (ATP) has been ported to the new code base.
  - The zerocoin functionality (xION) has been partially ported to the new code base.
  - Fixes to regtest and testnet
    - A previously unsolved issue related to running regtest scripts no longer occurs in the new code base
    - The regtest mining and staking functionality allows generating POS, POW and Hybrid blocks
    - The new regtest genesis is inline with the updated source
    - Testnet is running the new code base, including deterministic masternodes
  - Updated dependencies
    - boost 1.70
    - QT 5.9.8
    - expat 2.2.9
    - libevent 2.1.11
    - [zeromq latest master](https://github.com/zeromq/libzmq/tree/eb54966cb9393bfd990849231ea7d10e34f6319e)
    - dbus 1.13.12
    - miniupnpc 2.0.20180203
    - native_ds_store 1.1.2
    - native_biplist 1.0.3
    - native_mac_alias 2.0.7
    - X11
      - libSM 1.2.3
      - libX11 1.6.9
      - libICE 1.0.10
      - libXau 1.0.9
      - libxcb 1.13
      - libXext 1.3.4
      - xcb_proto 1.13
      - xproto 7.0.31
      - xtrans 1.4.0
  - More supported architectures
    - mips
    - mipsel
    - s390x
    - powerppc64
    - powerppc64le
  - New gui and artworks
    - spinner
  - Updated and refactored build process
    - Gitian build script extended
      - can be used with latest debian or ubuntu
      - added upload to a server over SSH
      - added hashing of resulted archives
    - fix latest dependencies
- Developers tools
  - VSCode
    - added spellchecker exclusion list
    - build, debug and scripts for vscode debugger
- All sources spellchecked
- BLS
  - [BLS Signature Scheme](https://github.com/dashpay/dips/blob/master/dip-0006/bls_signature_scheme.md)
  - [BLS M-of-N Threshold Scheme and Distributed Key Generation](https://github.com/dashpay/dips/blob/master/dip-0006/bls_m-of-n_threshold_scheme_and_dkg.md#bls-m-of-n-threshold-scheme-and-distributed-key-generation)
- New version keeps all languages which our old sources had. For more info please check [translation process](translation_process.md).
- New hardcoded seeds

#### Migrate Travis as pipeline

#### Hybrid Proof-of-Work / Proof-of-Stake

Combining POW with POS overcomes limitations in both. ION now supports mixed POW and POS blocks, where each algorithm
manages its own difficulty targets, but provide and use entropy from both type of blocks.

In POW blocks:
- Miners receive 0.5 ELEC per block
- Masternodes receive 70% of 0.5 ION = 0.35 ION rewards
- Masternodes receive 50% of the mined transaction fees
- 50% of the mined transaction fees are burned

In POS blocks:
- Stakers receive 2 x 30% of 0.5 ION = 0.30 ION rewards
- Masternodes receive 70% of 0.5 ION = 0.35 ION rewards
- Stakers receive 30% of the mined transaction fees
- Masternodes receive 50% of the mined transaction fees
- 20% of the mined transaction fees are burned

Since 50% of the blocks will be POS blocks, stakers will receive an average of 30% of the block rewards.

#### Zerocoin

- The core part of the zerocoin functionality has been ported to the new code base. 
  - This includes all functionality related to verification of zerocoin transactions.
  - This excludes functionality to create new zerocoin transactions.

#### Masternodes

Testnet masternodes requires using the `protx` command:
- Ensure a 20k collateral is available (`masternode outputs`).
- Generate the keys:
  - Generate the 'owner private key' (`getnewaddress`).
  - Generate the 'voting key address' (`getnewaddress`).
  - Generate the 'masternode payout address' (`getnewaddress`).
  - Optionally, generate the 'fee source address' (`getnewaddress`).
  - Generate the 'operator public and private keys' (`bls generate`).
- Configure the remote wallet:
  - Add `masternode=1` to the remote ioncoin.conf
  - Add `masternodeblsprivkey=<BLS_PRIVKEY>` to the remote ioncoin.conf (the private part of the 'operator public and private keys')
  - Add `externalip=<EXTERNAL_IP>` to the remote ioncoin.conf (the external IP address of the server hosting the remote masternode)
- Configure the controller wallet:
  - Run the following command in the debug console: `protx register_prepare <collateralHash> <collateralIndex> <ipAndPort> <ownerKeyAddr>
  <operatorPubKey> <votingKeyAddr> <operatorReward> <payoutAddress> (<feeSourceAddress>)`
  - The above command returns:
    - A binary transaction ("tx")
    - A collateral address ("collateralAddress")
    - A message to sign ("signMessage")
  - Run the following command in the debug console: `signmessage <collateralAddress> <signMessage>`
  - The above command returns a signature
  - Finally, submit the registration transaction with the following command: `protx register_submit <tx> <signature>`
- When the remote node is started, it will automatically activate.
- Run the sentinel on the remote server to let other masternodes know about yours
  - Edit `sentinel.conf` to set the network to 'testnet'.

#### Token implementation

- The Atomic Token Protocol (ATP) has been fully ported to the new code base.
  - Various improvements have been added to the verification code and the database code for tokens.

  **Introduction:**  

  As part of the integration of game development functionality and blockchain technology, the ION community chose to adopt a token system as part of its blockchain core. The community approved proposal IIP 0002 was put to vote in July 2018, after which development started. Instead of developing a solution from scratch, the team assessed a number of proposals and implementations that were currently being worked on for other Bitcoin family coins. Selection criteria were:

  * Fully open, with active development
  * Emphasis on permissionless transactions
  * Efficient in terms of resource consumption
  * Simple and elegant underlying principles 

  The ATP system implemented is based on the Group Tokenization proposal by Andrew Stone / BU.

  **References:**

  [GROUP Tokenization specification by Andrew Stone](https://docs.google.com/document/d/1X-yrqBJNj6oGPku49krZqTMGNNEWnUJBRFjX7fJXvTs/edit#heading=h.sn65kz74jmuf)  
  [GROUP Tokenization reference implementation for Bitcoin Cash](https://github.com/gandrewstone/BitcoinUnlimited/commits/tokengroups)  

  For the technical principles underlying ION Group Tokenization, the above documentation is used as our standard.

  ION developers fine tuned, extended and customized the Group Tokenization implementation. This documentation aims to support the ION community in:

  * Using the ION group token system
  * Creating additional tests as part of the development process
  * Finding new use cases that need development support

### New RPC Commands

#### Masternodes

masternode "command" ...
masternodelist ( "mode" "filter" )

`masternode count`        - Get information about number of masternodes (DEPRECATED options: 'total', 'ps', 'enabled', 'qualify', 'all')
`masternode current`      - Print info on current masternode winner to be paid the next block (calculated locally)
`masternode outputs`      - Print masternode compatible outputs
`masternode status`       - Print masternode status information
`masternode list`         - Print list of all known masternodes (see masternodelist for more info)
`masternode winner`       - Print info on next masternode winner to vote for
`masternode winners`      - Print list of masternode winners

### Deprecated RPC Commands

#### Masternodes

`createmasternodekey `  
`getmasternodeoutputs `  
`getmasternodecount`  
`getmasternodeoutputs`  
`getmasternodescores ( blocks )`  
`getmasternodestatus`  
`getmasternodewinners ( blocks "filter" )`  
`startmasternode "local|all|many|missing|disabled|alias" lockwallet ( "alias" )`
`listmasternodeconf ( "filter" )`  
`listmasternodes ( "filter" )`



*version* Change log
==============

Detailed release notes follow. This overview includes changes that affect behavior, not code moves, refactors and string updates. For convenience in locating the code changes and accompanying discussion, both the pull request and git merge commit are mentioned.

### Core Features

### Build System
 
### P2P Protocol and Network Code

### GUI
 
### RPC/REST

### Wallet
 
### Miscellaneous
 
 
## Credits

Thanks to everyone who directly contributed to this release:


#### Linux
```
81c6c81ea4cf67f11d40b6215fdf3b7914577368b238eb4da2f7ff887f3d7b3b  ion-3.0.2-arm-linux-gnueabihf-debug.tar.gz
f3bf55bc9282a882410dabe793772501e85124322f8f30a1b0ae4f4684be0837  ion-3.0.2-arm-linux-gnueabihf.tar.gz
3b780fd5ba985847251a93a97c3aedad282e8cecb641c8873aa1591e3d84b420  ion-3.0.2-i686-pc-linux-gnu-debug.tar.gz
9483e8af1af7175efaf92c3efc99ec6ddee868e629be5269a9985abdbeade91c  ion-3.0.2-i686-pc-linux-gnu.tar.gz
a57d962fd75f97a4b73d9fb65f43611acf15c7d9139e840d455c421bf7253170  ion-3.0.2-x86_64-linux-gnu-debug.tar.gz
13ea2506bdf620d77d8316672adab68a0a5107ae9b1590a6ed5486f3b0509f63  ion-3.0.2-x86_64-linux-gnu.tar.gz
30e0390dc5d5f3bfbd91f340884ab75ca30056ff49c5831edc49fcae8c9413af  src/ion-3.0.2.tar.gz
76fa4b28ae291521be3ee12d22fa7d9b6085cafa0cc14c1c60b6b69f498d5a57  ion-linux-3.1-res.yml
```
#### windows
```
Generating report
e7e3a92a32a8dc924e64d19a276af11cfe74193cc5aad26bc7335e2e2e835a57  ion-3.0.2-win-unsigned.tar.gz
2a9b1199f03068c6ea18e5e25cd6736c15c7ddf9247ede67079bcd544c1426ad  ion-3.0.2-win32-debug.zip
624c4d36d9145efcaac2a9ed00368f2aac61631a9b75ebfc546926009e395db0  ion-3.0.2-win32-setup-unsigned.exe
49659943ab915444ab926bd392a34d9fb3397cf997f34e3deaa31f43bab185a7  ion-3.0.2-win32.zip
1a2ba3aac86fb44355b7b879f6c269d5556b1e302f3a54602c51101eedc3efe6  ion-3.0.2-win64-debug.zip
073f490607616680a5a5def73a1231885860aa047e526e7bf4694a4b88b11a56  ion-3.0.2-win64-setup-unsigned.exe
6f88c1bb4eec9b077c2e1ee60e9371b184d460a920e6170610f67dbc786e9faf  ion-3.0.2-win64.zip
679ac050e5555e097785b835f666de3d2e6b0e8af0dcefc7430ef3bfec386180  src/ion-3.0.2.tar.gz
eec21196d7d4abab4fb8c32882fed5157271161ecba4693a67d6ca407503e030  ion-win-3.1-res.yml
```
#### macos
```
7bc9149778661d03f5b26a97ff7dbbf8c9113d5198fe433e4e9b69cbb02f80f4  ion-3.0.2-osx-unsigned.dmg
2ae951d05b053790e0916db1e519fbf3f37258741ae67ffa15a8ddf455881b9b  ion-3.0.2-osx-unsigned.tar.gz
1b2f4dbcc02c423924a2dbbf2b813eca094fd796e1424a7478a14651d82524ae  ion-3.0.2-osx64.tar.gz
679ac050e5555e097785b835f666de3d2e6b0e8af0dcefc7430ef3bfec386180  src/ion-3.0.2.tar.gz
0be4184f68aaf67653b704e0bed30a83ede84fcea9c53ca39aa50c804acff614  ion-osx-3.1-res.yml
```
=======
=======
==============

Minimum Supported MacOS Version
------

The minimum supported version of MacOS (OSX) has been moved from 10.8 Mountain Lion to 10.10 Yosemite. Users still running a MacOS version prior to Yosemite will need to upgrade their OS if they wish to continue using the latest version(s) of the ION Core wallet.

Major New Features
------

### BIP65 (CHECKLOCKTIMEVERIFY) Soft-Fork

ION Core v3.2.0 introduces new consensus rules for scripting pathways to support the [BIP65](https://github.com/bitcoin/bips/blob/master/bip-0065.mediawiki) standard. This is being carried out as a soft-fork in order to provide ample time for stakers to update their wallet version.

### Automint Addresses

A new "Automint Addresses" feature has been added to the wallet that allows for the creation of new addresses who's purpose is to automatically convert any ION funds received by such addresses to xION. The feature as a whole can be enabled/disabled either at runtime using the `-enableautoconvertaddress` option, via RPC/Console with the `enableautomintaddress` command, or via the GUI's options dialog, with the default being enabled.

Creation of these automint addresses is currently only available via the RPC/Console `createautomintaddress` command, which takes no additional arguments. The command returns a new ION address each time, but addresses created by this command can be re-used if desired.

### In-wallet Proposal Voting

A new UI wallet tab has been introduced that allows users to view the current budget proposals, their vote counts, and vote on proposals if the wallet is acting as a masternode controller. The visual design is to be considered temporary, and will be undergoing further design and display improvements in the future.

### Zerocoin Lite Node Protocol

Support for the ZLN Protocol has been added, which allows for a node to opt-in to providing extended network services for the protocol. By default, this functionality is disabled, but can be enabled by using the `-peerbloomfilterszc` runtime option.

A full technical writeup of the protocol can be found [Here](https://ioncoin.xyz/wp-content/uploads/2018/11/Zerocoin_Light_Node_Protocol.pdf).

GUI Changes
------

### Console Security Warning

Due to an increase in social engineering attacks/scams that rely on users relaying information from console commands, a new warning message has been added to the Console window's initial welcome message.

### Optional Hiding of Orphan Stakes

The options dialog now contains a checkbox option to hide the display of orphan stakes from both the overview and transaction history sections. Further, a right-click context menu option has been introduced in the transaction history tab to achieve the same effect.

**Note:** This option only affects the visual display of orphan stakes, and will not prevent them nor remove them from the underlying wallet database.

### Transaction Type Recoloring

The color of various transaction types has been reworked to provide better visual feedback. Staking and masternode rewards are now purple, orphan stakes are now light gray, other rejected transactions are in red, and normal receive/send transactions are black.

### Receive Tab Changes

The address to be used when creating a new payment request is now automatically displayed in the form. This field is not user-editable, and will be updated as needed by the wallet.

A new button has been added below the payment request form, "Receiving Addresses", which allows for quicker access to all the known receiving addresses. This one-click button is the same as using the `File->Receiving Addresses...` menu command, and will open up the Receiving Addresses UI dialog.

Historical payment requests now also display the address used for the request in the history table. While this information was already available when clicking the "Show" button, it was an extra step that shouldn't have been necessary.

### Privacy Tab Changes

The entire right half of the privacy tab can now be toggled (shown/hidden) via a new UI button. This was done to reduce "clutter" for users that may not wish to see the detailed information regarding individual denomination counts.

RPC Changes
------

### Backupwallet Sanity

The `backupwallet` RPC command no longer allows for overwriting the currently in use wallet.dat file. This was done to avoid potential file corruption caused by multiple conflicting file access operations.

Build System Changes
------

### Completely Disallow Qt4

Compiling the ION Core wallet against Qt4 hasn't been supported for quite some time now, but the build system still recognized Qt4 as a valid option if Qt5 couldn't be found. This has now been remedied and Qt4 will no longer be considered valid during the `configure` pre-compilation phase.

### Further OpenSSL Deprecation

Up until now, the zerocoin library relied exclusively on OpenSSL for it's bignum implementation. This has now been changed with the introduction of GMP as an arithmetic operator and the bignum implementation has now been redesigned around GMP. Users can still opt to use OpenSSL for bignum by passing `--with-zerocoin-bignum=openssl` to the `configure` script, however such configuration is now deprecated.

**Note:** This change introduces a new dependency on GMP (libgmp) by default.

### RISC-V Support

Support for the new RISC-V 64bit processors has been added, though still experimental. Pre-compiled binaries for this CPU architecture are available for linux, and users can self-compile using gitian, depends, or an appropriate host system natively.

*version* Change log
==============

Detailed release notes follow. This overview includes changes that affect behavior, not code moves, refactors and string updates. For convenience in locating the code changes and accompanying discussion, both the pull request and git merge commit are mentioned.

### Core Features

### Build System
 
### P2P Protocol and Network Code

### GUI
 
### RPC/REST

### Wallet
 
### Miscellaneous
 
 
## Credits

Thanks to everyone who directly contributed to this release:
=======

#### Mac OSX High Sierra  
Currently there are issues with the 4.x gitian release on MacOS version 10.13 (High Sierra), no reports of issues on older versions of MacOS.
### Atomic Token Protocol (ATP)
_____

**Introduction:**  

As part of the integration of game development functionality and blockchain technology, the ION community chose to adopt a token system as part of its blockchain core. The community approved proposal IIP 0002 was put to vote in July 2018, after which development started. Instead of developing a solution from scratch, the team assessed a number of proposals and implementations that were currently being worked on for other Bitcoin family coins. Selection criteria were:

* Fully open, with active development
* Emphasis on permissionless transactions
* Efficient in terms of resource consumption
* Simple and elegant underlying principles 

The ATP system implemented is based on the Group Tokenization proposal by Andrew Stone / BU.

**References:**

[GROUP Tokenization specification by Andrew Stone](https://docs.google.com/document/d/1X-yrqBJNj6oGPku49krZqTMGNNEWnUJBRFjX7fJXvTs/edit#heading=h.sn65kz74jmuf)  
[GROUP Tokenization reference implementation for Bitcoin Cash](https://github.com/gandrewstone/BitcoinUnlimited/commits/tokengroups)  

For the technical principles underlying ION Group Tokenization, the above documentation is used as our standard.

ION developers fine tuned, extended and customized the Group Tokenization implementation. This documentation aims to support the ION community in:

* Using the ION group token system
* Creating additional tests as part of the development process
* Finding new use cases that need development support

### Noteable Changes
______
- core: switch core from pivx to ion
- updated dependencies
- QT 5.9.8
- expat 2.2.9
- libevent 2.1.11
- zeromq 4.3.2
- dbus 1.13.12
- miniupnpc 2.0.20180203
- native_ds_storke 1.1.2
- native_biplist 1.0.3
- native_mac_alias 2.0.7

##### Zerocoin
- Reimplement zerocoin to new source

##### Protocol change
- 

### New RPC Commands
__________

#### Tokens

`configuremanagementtoken "ticker" "name" decimalpos "description_url" description_hash ( confirm_send )  `  
`configuretoken "ticker" "name" ( decimalpos "description_url" description_hash ) ( confirm_send )  `  
`createtokenauthorities "groupid" "ionaddress" authoritylist  `  
`droptokenauthorities "groupid" "transactionid" outputnr [ authority1 ( authority2 ... ) ] `   
`getsubgroupid "groupid" "data"  `  
`gettokenbalance ( "groupid" )  `  
`listtokenauthorities "groupid"`    
`listtokenssinceblock "groupid" ( "blockhash" target-confirmations includeWatchonly ) `   
`listtokentransactions "groupid" ( count from includeWatchonly ) `   
`melttoken "groupid" quantity  `  
`minttoken "groupid" "ionaddress" quantity  `  
`sendtoken "groupid" "address" amount  `  
`tokeninfo [list, all, stats, groupid, ticker, name] ( "specifier " )  `  
`scantokens <action> ( <scanobjects> ) `

#### Masternodes
`createmasternodekey `  
`getmasternodeoutputs `  
`getmasternodecount`  
`getmasternodeoutputs`  
`getmasternodescores ( blocks )`  
`getmasternodestatus`  
`getmasternodewinners ( blocks "filter" )`  
`startmasternode "local|all|many|missing|disabled|alias" lockwallet ( "alias" )`
`listmasternodeconf ( "filter" )`  
`listmasternodes ( "filter" )`


### Deprecated RPC Commands
___
#### Masternodes
`masternode count`  
`masternode current`  
`masternode debug`  
`masternode genkey`  
`masternode outputs`  
`masternode start`  
`masternode start-alias`  
`masternode start-<mode>`  
`masternode status`  
`masternode list`  
`masternode list-conf`  
`masternode winners`  


### 5.0.99 Change log
=======
### 5.0.99 Change log  
=======
### 5.0.01 Change log  
ckti <ckti@i2pmail.org> (1):

- `3ad7b4d` CircleCI is now being used for continuous integration