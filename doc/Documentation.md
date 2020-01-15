
# SwabianCoin Technical Documentation

![Logo](swabiancoin_logo.png "SwabianCoin")

## Table of Contents

 - [Mining](#mining)
 - [Transactions](#transactions)
 - [Blockchain](#blockchain)
 - [P2P Network](#p2p-network)

## Mining

Similar to raw materials like gold, the idea of SwabianCoin is to have a limited resource of coins ("nuggets") which may be mined by everyone. 
The investment is the (computational) effort and the return is the amount of found coins.
As time goes by, more and more coins are mined and it gets harder and harder to find new coins.

The idea behind the SwabianCoin mining process is to search for simple strings which contain 
 
 - the creators public key (to ensure that coins cannot be stolen by any other identity)
 - the highest hash of the previous epoch (to ensure that coins cannot be precalculated)
 - and arbitrary characters.
  
The SHA-256 hash value of this string must lie inside a certain hash region to be a valid coin.
This region changes after every 1000 mined coins (which is called one epoch).

The hash region is calculated in the following way:

hash_upper_limit = (2<sup>32</sup> - 1) &divide; &alpha;<sup>epoch</sup>

hash_lower_limit = min((hash_upper_limit &divide; &alpha;) + 1, hash_upper_limit)

with &alpha; = 1.001776032062287 and epoch = the current epoch starting from zero

The constant &alpha; has been chosen to limit the number of epochs to around 100 000:

2<sup>32</sup>=&alpha;<sup>100 000</sup>

&rarr; &alpha; &asymp; 1.001776032062287

Hence, the success probabilities for some example epochs are:

| Epoch    | Success probability per trial |
|---------:|:--------:|
|        0 | 1.773e-03
|     1000 | 3.006e-04
|    10000 | 3.486e-11
|    90000 | 7.787e-73
|   100000 | 8.636e-78

Note: The hash area of epoch 92537 consists of 1000 hash values. Since a new epoch is only entered when 1000 unique minings have been found within the hash area of an epoch, the highest reachable epoch is 92538.

## Transactions

Transactions in SwabianCoin consist of two parts:

- The public key of the receiver.
- The amount of coins to transfer to the receiver. The minimum possible amount to transfer is <sup>1</sup>&frasl;<sub>1000000</sub> coin.

## Blockchain

The blockchain consists of BaselineBlocks and CollectionBlocks. 

A BaselineBlock contains all wallets and information about the current mining epoch. It can be seen as a status quo of the complete currency including all participants. 
If a BaselineBlock is added to the Blockchain, everything that happened before that block is not necessary anymore to proceed with the blockchain. Thus, the blocks before a BaselineBlock are always thrown away. 

A CollectionBlock adds transactions and created coins to the blockchain. 
It can be seen as an extension to the last BaselineBlock. 
The current status quo of the complete currency including all participants is always a combination of the last BaselineBlock and all following CollectionBlocks.

### Block Descriptions

#### Header

| Variable             | Description |
|----------------------|-------------|
| block_hash           | The SHA-256 hash value of this block. The input of the hash function is the serialized block object. 
| previous_block_hash  | The SHA-256 hash value of the predecessor of the current block.
| block_type           | An identifier of the block type (either CollectionBlock or BaselineBlock) 
| block_uid            | The unique id of this block - starting from 1 in ascending order 

#### CollectionBlock

| Variable             | Description |
|----------------------|-------------|
| header               | Header as described [here](#header)
| transactions         | A vector containing TransactionSubBlocks sorted by the sub block hash value. The size is restricted to 10000 transactions per block.
| creations            | A vector containing CreationSubBlocks sorted by the sub block hash value. The size is restricted to 1000 creations per block and epoch.

#### BaselineBlock

| Variable             | Description |
|----------------------|-------------|
| header               | Header as described [here](#header)
| mining_state         | The current mining state containing<ul><li>the current epoch</li><li>the highest data value hash of the last epoch</li><li>the highest data value hash of the current epoch</li><li>the number of mined coins in the current epoch</li></ul>
| data_value_hashes    | A vector of vectors containing the hashes of all data values created. The vector of vectors is sorted by epoch and hash value. 
| wallets              | A map containing all existing wallets sorted by the owner's public key. Each wallet contains the owners current balance.

#### SubBlock Header

| Variable             | Description |
|----------------------|-------------|
| block_hash           | The SHA-256 hash value of this sub block. The input of the hash function is the serialized sub block object. 
| previous_block_hash  | The SHA-256 hash value of the predecessor of the current block.
| block_type           | An identifier of the block type (either TransactionSubBlock or CreationSubBlock) 

#### TransactionSubBlock
        
| Variable             | Description |
|----------------------|-------------|
| header               | SubBlock Header as described [here](#subblock-header)
| fraction             | The amount of coins to transfer from pre_owner to post_owner. The minimum amount is <sup>1</sup>&frasl;<sub>1000000</sub> coin. 
| pre_owner            | The public key of the sender of the transaction sum
| post_owner           | The public key of the receiver of the transaction sum
| signature            | The ECDSA signature of this sub block signed with the private key of pre_owner

#### CreationSubBlock

| Variable             | Description |
|----------------------|-------------|
| header               | SubBlock Header as described [here](#subblock-header)
| coin_value           | A string which contains some information about the creator, the history and arbitrary characters. See [Mining](#mining). The maximum length of the coin_value string is 1024 characters. 
| creator              | The public key of the creator/miner of this coin
| signature            | The ECDSA signature of this sub block signed with the private key of creator

## P2P Network

### Communication Layer

The communication layer of SwabianCoin is based on the proven-in-use p2p network torrent. The peers find each other using a trackerless torrent.

The lookup procedure when connecting to the network is done in two stages:

- The peer tries to fetch an active peers list from the swabiancoin.com domain using a https request.
- If the first step does not succeed, the peer looks for active peers using a hard coded fallback list. 

The communication between the peers is done using the extension protocol defined in the Bittorrent Enhancement Proposal 10 ([BEP-10](https://www.bittorrent.org/beps/bep_0010.html)).

### Block Negotiation

A new block is negotiated between all peers in a two minute cycle. 

In phase 1 (the first 30 seconds of this cycle), there is no communication between the peers. 
The peers use this time to prepare the upcoming block with their own transactions and generated coins.

In phase 2 (the remaining 90 seconds of this cycle), the block negotiation itself takes place. 
At the beginning of this phase, the peers propagate the prepared block to all directly connected peers. Every incoming block is checked for validity and merged with the own one (which means that valid transactions and coins of the received block are added to our own one). Every four seconds, the updated block is propagated to all directly connected peers.

Peers are only allowed to propagate new transactions and coins with the initial block propagation in phase 2. 
This makes sure that the block negotiation converges over time to one common block which is added to the blockchain at the end of phase 2. After that, the next cycle starts.

Every 720 cycles (which means every 24 hours), an empty cycle is inserted. The peers use this empty cycle to generate a new baseline.

### Blockchain synchronization on startup

On startup, the peer asks the directly connected peers sequentially for all blocks of the current blockchain. As soon as the peer is synchronized with its connected peers, it takes part in the block negotiation.