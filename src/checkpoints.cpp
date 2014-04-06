// Copyright (c) 2009-2012 The Bitcoin developers
// Copyright (c) 2013-2014 PandaCoin Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/assign/list_of.hpp> // for 'map_list_of()'
#include <boost/foreach.hpp>

#include "checkpoints.h"

#include "main.h"
#include "uint256.h"

namespace Checkpoints
{
    typedef std::map<int, uint256> MapCheckpoints;

    // How many times we expect transactions after the last checkpoint to
    // be slower. This number is a compromise, as it can't be accurate for
    // every system. When reindexing from a fast disk with a slow CPU, it
    // can be up to 20, while when downloading from a slow network with a
    // fast multicore CPU, it won't be much higher than 1.
    static const double fSigcheckVerificationFactor = 5.0;

    struct CCheckpointData {
        const MapCheckpoints *mapCheckpoints;
        int64 nTimeLastCheckpoint;
        int64 nTransactionsLastCheckpoint;
        double fTransactionsPerDay;
    };

    // What makes a good checkpoint block?
    // + Is surrounded by blocks with reasonable timestamps
    //   (no blocks before with a timestamp after, none after with
    //    timestamp before)
    // + Contains no strange transactions
    static MapCheckpoints mapCheckpoints =
        boost::assign::map_list_of
        (     0, uint256("0x68fad98bd07315eef904fa3bf4344a38cb4f05549f659272bad7b4e88961d4c5"))
        (   100, uint256("0x8a6c9dbf73f6cd42df5047c8e453957515a352ef496a035d28471fd58efa289a"))
        (   500, uint256("0x4df715da8df0c28fe67c06c1fbf4bcfec3d3753238c9a90633948167293b1d51"))
        (  1000, uint256("0x95069fa83e4e5ce590ec6ce4d904ef4441b2c271b37724328dbb51dfbaae4d97"))
        (  2000, uint256("0xfbe01f6aafd2744636c9fbd93914f3748c9c9f8a02b728c31b7872729a918e36"))
        (  3000, uint256("0x78ff58ee873c556c1252e7e0ef5c2f8834404f6c6822e228229503ad1641cab2"))
        (  4000, uint256("0x011f4d8c682d8e733041598a2d2fe4e912aa7341e7eb1ed15f3a0a19f0ed3dc9"))
        (  5000, uint256("0x46e8863a1239f72cf96d45c1618b06e5076a4323fe3c49212b0f912455a087d8"))
        (  6000, uint256("0xda3748d0b3889ed1486542db95f6ecdedb1abe8a43f3115688954092a00c08ea"))
        (  6500, uint256("0x5b329acdcb3525e11f185277ca7fe53ecfa2e7841136701092233adca14f2bf5"))
        (  7000, uint256("0xf82e33c9345d85441d7351772500094cf4f57b338503623aa9b981fb6ac04864"))
        ( 63505, uint256("0xfc3fa63af056c025221a4a8cd137cf392b4c16c763b0c7939e0d6daf49a4f210"))
        ( 66965, uint256("0xe5e27953427def08705caa5195f273d9e92893359537a19cd73413da78b83210"))
        ( 71000, uint256("0x836b8b1a328528ef5d837186f46d2522603c2188f6d41ca4127261791a90963d"))
        ;
    static const CCheckpointData data = {
        &mapCheckpoints,
        1388890893, // * UNIX timestamp of last checkpoint block
        2982687,    // * total number of transactions between genesis and last checkpoint
                    //   (the tx=... number in the SetBestChain debug.log lines)
        8000.0     // * estimated number of transactions per day after checkpoint
    };

    static MapCheckpoints mapCheckpointsTestnet =
        boost::assign::map_list_of
        (     0, uint256("0x"))
        ;
    static const CCheckpointData dataTestnet = {
        &mapCheckpointsTestnet,
        1369685559,
        37581,
        300
    };

    const CCheckpointData &Checkpoints() {
        if (fTestNet)
            return dataTestnet;
        else
            return data;
    }

    bool CheckBlock(int nHeight, const uint256& hash)
    {
        if (fTestNet) return true; // Testnet has no checkpoints
        if (!GetBoolArg("-checkpoints", true))
            return true;

        const MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;

        MapCheckpoints::const_iterator i = checkpoints.find(nHeight);
        if (i == checkpoints.end()) return true;
        return hash == i->second;
    }

    // Guess how far we are in the verification process at the given block index
    double GuessVerificationProgress(CBlockIndex *pindex) {
        if (pindex==NULL)
            return 0.0;

        int64 nNow = time(NULL);

        double fWorkBefore = 0.0; // Amount of work done before pindex
        double fWorkAfter = 0.0;  // Amount of work left after pindex (estimated)
        // Work is defined as: 1.0 per transaction before the last checkoint, and
        // fSigcheckVerificationFactor per transaction after.

        const CCheckpointData &data = Checkpoints();

        if (pindex->nChainTx <= data.nTransactionsLastCheckpoint) {
            double nCheapBefore = pindex->nChainTx;
            double nCheapAfter = data.nTransactionsLastCheckpoint - pindex->nChainTx;
            double nExpensiveAfter = (nNow - data.nTimeLastCheckpoint)/86400.0*data.fTransactionsPerDay;
            fWorkBefore = nCheapBefore;
            fWorkAfter = nCheapAfter + nExpensiveAfter*fSigcheckVerificationFactor;
        } else {
            double nCheapBefore = data.nTransactionsLastCheckpoint;
            double nExpensiveBefore = pindex->nChainTx - data.nTransactionsLastCheckpoint;
            double nExpensiveAfter = (nNow - pindex->nTime)/86400.0*data.fTransactionsPerDay;
            fWorkBefore = nCheapBefore + nExpensiveBefore*fSigcheckVerificationFactor;
            fWorkAfter = nExpensiveAfter*fSigcheckVerificationFactor;
        }

        return fWorkBefore / (fWorkBefore + fWorkAfter);
    }

    int GetTotalBlocksEstimate()
    {
        if (fTestNet) return 0; // Testnet has no checkpoints
        if (!GetBoolArg("-checkpoints", true))
            return 0;

        const MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;

        return checkpoints.rbegin()->first;
    }

    CBlockIndex* GetLastCheckpoint(const std::map<uint256, CBlockIndex*>& mapBlockIndex)
    {
        if (fTestNet) return NULL; // Testnet has no checkpoints
        if (!GetBoolArg("-checkpoints", true))
            return NULL;

        const MapCheckpoints& checkpoints = *Checkpoints().mapCheckpoints;

        BOOST_REVERSE_FOREACH(const MapCheckpoints::value_type& i, checkpoints)
        {
            const uint256& hash = i.second;
            std::map<uint256, CBlockIndex*>::const_iterator t = mapBlockIndex.find(hash);
            if (t != mapBlockIndex.end())
                return t->second;
        }
        return NULL;
    }
}
