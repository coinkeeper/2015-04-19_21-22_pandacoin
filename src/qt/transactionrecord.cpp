#include "transactionrecord.h"

#include "wallet.h"
#include "base58.h"
#include <map>

/* Return positive answer if transaction should be shown in list.
 */
bool TransactionRecord::showTransaction(const CWalletTx &wtx)
{
    if (wtx.IsCoinBase())
    {
        // Ensures we show generated coins / mined transactions at depth 1
        if (!wtx.IsInMainChain())
        {
            return false;
        }
    }
    return true;
}

/*
 * Decompose CWallet transaction to model transaction records.
 */
QList<TransactionRecord> TransactionRecord::decomposeTransaction(const CWallet *wallet, const CWalletTx &wtx)
{
    QList<TransactionRecord> parts;
    int64_t nTime = wtx.GetTxTime();
    int64_t nCredit = wtx.GetCredit(true);
    int64_t nDebit = wtx.GetDebit();
    int64_t nNet = nCredit - nDebit;
    uint256 hash = wtx.GetHash(), hashPrev = 0;
    std::map<std::string, std::string> mapValue = wtx.mapValue;

    if (nNet > 0 || wtx.IsCoinBase() || wtx.IsCoinStake())
    {
        //
        // Credit
        //
        BOOST_FOREACH(const CTxOut& txout, wtx.vout)
        {
            if(wallet->IsMine(txout))
            {
                TransactionRecord sub(hash, nTime);
                sub.fromAddress = "";

                CTxDestination address;
                sub.idx = parts.size(); // sequence number
                sub.credit = txout.nValue;
                if (ExtractDestination(txout.scriptPubKey, address) && IsMine(*wallet, address))
                {
                    // Received by Bitcoin Address
                    sub.type = TransactionRecord::RecvWithAddress;
                    sub.address = CBitcoinAddress(address).ToString();
                }
                else
                {
                    // Received by IP connection (deprecated features), or a multisignature or other non-simple transaction
                    sub.type = TransactionRecord::RecvFromOther;
                    sub.address = mapValue["from"];
                }
                if (wtx.IsCoinBase())
                {
                    // Generated (proof-of-work)
                    sub.type = TransactionRecord::Generated;
                }
                if (wtx.IsCoinStake())
                {
                    // Generated (proof-of-stake)

                    if (hashPrev == hash)
                        continue; // last coinstake output

                    sub.type = TransactionRecord::Generated;
                    sub.credit = nNet > 0 ? nNet : wtx.GetValueOut() - nDebit;
                    hashPrev = hash;
                }

                parts.append(sub);
            }
        }
    }
    else
    {
        bool fAllFromMe = true;
        BOOST_FOREACH(const CTxIn& txin, wtx.vin)
            fAllFromMe = fAllFromMe && wallet->IsMine(txin);

        bool fAllToMe = true;
        BOOST_FOREACH(const CTxOut& txout, wtx.vout)
            fAllToMe = fAllToMe && wallet->IsMine(txout);

        TransactionRecord sub(hash, nTime);

        if (fAllFromMe && fAllToMe)
        {
            sub.address = "";

            // Sender
            CTxDestination toAddress;
            ExtractDestination(wtx.vout[0].scriptPubKey, toAddress);
            CTxDestination fromAddress=toAddress;
            CTxIn txin=wtx.vin[0];
            CWalletTx prev = wallet->mapWallet.find(txin.prevout.hash)->second;
            ExtractDestination(prev.vout[txin.prevout.n].scriptPubKey, fromAddress);
            int count=0;
            while((!wallet->mapAddressBook.count(fromAddress)||!wallet->IsMine(txin))&&count++<20)
            {
                txin=prev.vin[0];
                prev=wallet->mapWallet.find(txin.prevout.hash)->second;
                ExtractDestination(prev.vout[txin.prevout.n].scriptPubKey, fromAddress);
            }

            // Maps
            std::map<std::string, int64_t> transactionMap;

            // Store all change for addresses - for multi transactions we need to subtract this from amounts as we go.
            // For non-multi transactions we just ignore this as it is taken care of automatically.
            std::map<std::string, int64_t> addressChangeMap;
            BOOST_FOREACH(const CTxOut& txout, wtx.vout)
            {
                if(wallet->IsChange(txout))
                {
                    CTxDestination changeAddress;
                    CTxIn txin;
                    CWalletTx prev = wtx;
                    int count=0;
                    while((!wallet->mapAddressBook.count(changeAddress)||!wallet->IsMine(txin))&&count++<20)
                    {
                        txin=prev.vin[0];
                        prev=wallet->mapWallet.find(txin.prevout.hash)->second;
                        ExtractDestination(prev.vout[txin.prevout.n].scriptPubKey, changeAddress);
                    }
                    addressChangeMap[CBitcoinAddress(changeAddress).ToString()] += txout.nValue;
                }
            }

            std::string stest = CBitcoinAddress(toAddress).ToString();
            // Credit
            BOOST_FOREACH(const CTxOut& txout, wtx.vout)
            {
                if(wallet->IsChange(txout))
                    continue;

                CTxDestination creditAddress=toAddress;
                ExtractDestination(txout.scriptPubKey, creditAddress);
                transactionMap[CBitcoinAddress(creditAddress).ToString()] += txout.nValue;
            }

            // Debit
            if(wtx.vin.size()<=2)
            {
                transactionMap[CBitcoinAddress(fromAddress).ToString()] += -nDebit;
            }
            else
            {
                BOOST_FOREACH(CTxIn partin, wtx.vin)
                {
                    CTxDestination partFromAddress=fromAddress;
                    CWalletTx partPrev = wallet->mapWallet.find(partin.prevout.hash)->second;
                    int64_t d = -partPrev.vout[partin.prevout.n].nValue;
                    ExtractDestination(partPrev.vout[partin.prevout.n].scriptPubKey, partFromAddress);
                    int count=0;
                    while((!wallet->mapAddressBook.count(partFromAddress)||!wallet->IsMine(partin))&&count++<20)
                    {
                        partin=partPrev.vin[0];
                        partPrev=wallet->mapWallet.find(partin.prevout.hash)->second;
                        ExtractDestination(partPrev.vout[partin.prevout.n].scriptPubKey, partFromAddress);
                    }
                    if(count == 20)
                    {
                        transactionMap[CBitcoinAddress(fromAddress).ToString()] += d;
                    }
                    else
                    {
                        transactionMap[CBitcoinAddress(partFromAddress).ToString()] += d;
                    }
                }
            }

            for(std::map<std::string, int64_t>::iterator iter = transactionMap.begin(); iter != transactionMap.end(); iter++)
            {
                if(addressChangeMap.find(iter->first) != addressChangeMap.end())
                {
                    iter->second += addressChangeMap[iter->first];
                    addressChangeMap[iter->first] = 0;
                    // Change has cancelled the transaction out entirely.
                    if(iter->second == 0)
                    {
                        continue;
                    }
                }
                // fixme: Try match the inputs and outputs.
                if(iter->second > 0)
                {
                    sub.debit=0;
                    sub.credit = iter->second;
                    sub.type = TransactionRecord::InternalReceive;
                    sub.fromAddress="";
                    sub.address=iter->first;
                }
                else
                {
                    sub.credit=0;
                    sub.debit = iter->second;
                    sub.type = TransactionRecord::InternalSend;
                    sub.fromAddress=iter->first;
                }
                parts.append(sub);
            }
            transactionMap.clear();
        }
        else if (fAllFromMe)
        {
            //
            // Debit
            //
            int64_t nTxFee = nDebit - wtx.GetValueOut();

            // Store all change for addresses - for multi transactions we need to subtract this from amounts as we go.
            // For non-multi transactions we just ignore this as it is taken care of automatically.
            std::map<std::string, int64_t> addressChangeMap;
            BOOST_FOREACH(const CTxOut& txout, wtx.vout)
            {
                if(wallet->IsChange(txout))
                {
                    CTxDestination changeAddress;
                    CTxIn txin;
                    CWalletTx prev = wtx;
                    int count=0;
                    while((!wallet->mapAddressBook.count(changeAddress)||!wallet->IsMine(txin))&&count++<20)
                    {
                        txin=prev.vin[0];
                        prev=wallet->mapWallet.find(txin.prevout.hash)->second;
                        ExtractDestination(prev.vout[txin.prevout.n].scriptPubKey, changeAddress);
                    }
                    addressChangeMap[CBitcoinAddress(changeAddress).ToString()] += txout.nValue;
                }
            }
            std::vector<TransactionRecord> debitArray;

            for (unsigned int nOut = 0; nOut < wtx.vout.size(); nOut++)
            {
                const CTxOut& txout = wtx.vout[nOut];
                sub.idx = parts.size();

                if(wallet->IsChange(txout))
                {
                    continue;
                }

                CTxDestination address;
                if (ExtractDestination(txout.scriptPubKey, address))
                {
                    // Sent to Bitcoin Address
                    sub.type = TransactionRecord::SendToAddress;
                    sub.address = CBitcoinAddress(address).ToString();

                    // Sender
                    CTxDestination toAddress;
                    ExtractDestination(wtx.vout[0].scriptPubKey, toAddress);

                    // Single output (or two outputs where one is change) - fixme: explicitely check that one is change or is this assumption safe?
                    if(wtx.vin.size() <= 2)
                    {
                        CTxDestination fromAddress=toAddress;
                        CTxIn txin=wtx.vin[0];
                        CWalletTx prev = wallet->mapWallet.find(txin.prevout.hash)->second;
                        ExtractDestination(prev.vout[txin.prevout.n].scriptPubKey, fromAddress);
                        int count=0;
                        while((!wallet->mapAddressBook.count(fromAddress)||!wallet->IsMine(txin))&&count++<20)
                        {
                            txin=prev.vin[0];
                            prev=wallet->mapWallet.find(txin.prevout.hash)->second;
                            ExtractDestination(prev.vout[txin.prevout.n].scriptPubKey, fromAddress);
                        }
                        sub.fromAddress = CBitcoinAddress(fromAddress).ToString();
                        sub.debit = -nDebit;

                        debitArray.push_back(sub);
                    }
                    else //Multi input, have to handle this a bit differently..
                    {
                        std::map<std::string, int64_t> addressAmountMap;
                        BOOST_FOREACH(CTxIn txin, wtx.vin)
                        {
                            CTxDestination fromAddress=toAddress;
                            CWalletTx prev = wallet->mapWallet.find(txin.prevout.hash)->second;
                            ExtractDestination(prev.vout[txin.prevout.n].scriptPubKey, fromAddress);

                            int64_t nValue = prev.vout[txin.prevout.n].nValue;
                            int count=0;
                            while((!wallet->mapAddressBook.count(fromAddress)||!wallet->IsMine(txin))&&count++<20)
                            {
                                txin=prev.vin[0];
                                prev=wallet->mapWallet.find(txin.prevout.hash)->second;
                                ExtractDestination(prev.vout[txin.prevout.n].scriptPubKey, fromAddress);
                            }
                            addressAmountMap[CBitcoinAddress(fromAddress).ToString()] += nValue;
                        }
                        std::map<std::string, int64_t>::iterator iter = addressAmountMap.begin();
                        for(; iter != addressAmountMap.end(); iter++)
                        {
                            sub.fromAddress = iter->first;
                            sub.debit = -iter->second;
                            debitArray.push_back(sub);
                        }
                    }
                    for(unsigned int i = 0; i < debitArray.size(); i++)
                    {
                        TransactionRecord debit = debitArray[i];
                        if(addressChangeMap.find(debit.fromAddress) != addressChangeMap.end())
                        {
                            debit.debit += addressChangeMap[debit.fromAddress];
                            addressChangeMap[debit.fromAddress] = 0;
                            // Change has cancelled the transaction out entirely.
                            if(debit.debit == 0)
                            {
                                continue;
                            }
                            // Change has cancelled the transaction out entirely (and we still have more change left).
                            if(debit.debit > 0)
                            {
                                addressChangeMap[debit.fromAddress] = debit.debit;
                                continue;
                            }
                        }
                        parts.append(debit);
                    }
                    debitArray.clear();
                }
                else
                {
                    // Sent to IP, or other non-address transaction like OP_EVAL
                    sub.type = TransactionRecord::SendToOther;
                    sub.address = mapValue["to"];

                    int64_t nValue = txout.nValue;
                    /* Add fee to first output */
                    if (nTxFee > 0)
                    {
                        nValue += nTxFee;
                        nTxFee = 0;
                    }
                    sub.debit = -nValue;

                    parts.append(sub);
                }

            }
        }
        else
        {
            //
            // Mixed debit transaction, can't break down payees
            //
            parts.append(TransactionRecord(hash, nTime, TransactionRecord::Other, "", nNet, 0));
        }
    }

    return parts;
}

void TransactionRecord::updateStatus(const CWalletTx &wtx)
{
    //checkme: Not sure if this is strictly necessary or overkill?
    balanceNeedsRecalc = true;

    // Determine transaction status

    // Find the block the tx is in
    CBlockIndex* pindex = NULL;
    std::map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.find(wtx.hashBlock);
    if (mi != mapBlockIndex.end())
        pindex = (*mi).second;

    // Sort order, unrecorded transactions sort to the top
    status.sortKey = strprintf("%010d-%01d-%010u-%03d",
        (pindex ? pindex->nHeight : std::numeric_limits<int>::max()),
        (wtx.IsCoinBase() ? 1 : 0),
        wtx.nTimeReceived,
        idx);
    status.countsForBalance = wtx.IsTrusted() && !(wtx.GetBlocksToMaturity() > 0);
    status.depth = wtx.GetDepthInMainChain();
    status.cur_num_blocks = nBestHeight;

    if (!wtx.IsFinal())
    {
        if (wtx.nLockTime < LOCKTIME_THRESHOLD)
        {
            status.status = TransactionStatus::OpenUntilBlock;
            status.open_for = nBestHeight - wtx.nLockTime;
        }
        else
        {
            status.status = TransactionStatus::OpenUntilDate;
            status.open_for = wtx.nLockTime;
        }
    }

    // For generated transactions, determine maturity
    else if(type == TransactionRecord::Generated)
    {
        if (wtx.GetBlocksToMaturity() > 0)
        {
            status.status = TransactionStatus::Immature;

            if (wtx.IsInMainChain())
            {
                status.matures_in = wtx.GetBlocksToMaturity();

                // Check if the block was requested by anyone
                if (GetAdjustedTime() - wtx.nTimeReceived > 2 * 60 && wtx.GetRequestCount() == 0)
                    status.status = TransactionStatus::MaturesWarning;
            }
            else
            {
                status.status = TransactionStatus::NotAccepted;
            }
        }
        else
        {
            status.status = TransactionStatus::Confirmed;
        }
    }
    else
    {
        if (status.depth < 0)
        {
            status.status = TransactionStatus::Conflicted;
        }
        else if (GetAdjustedTime() - wtx.nTimeReceived > 2 * 60 && wtx.GetRequestCount() == 0)
        {
            status.status = TransactionStatus::Offline;
        }
        else if (status.depth == 0)
        {
            status.status = TransactionStatus::Unconfirmed;
        }
        else if (status.depth < RecommendedNumConfirmations)
        {
            status.status = TransactionStatus::Confirming;
        }
        else
        {
            status.status = TransactionStatus::Confirmed;
        }
    }
}

bool TransactionRecord::statusUpdateNeeded()
{
    return status.cur_num_blocks != nBestHeight;
}

std::string TransactionRecord::getTxID()
{
    return hash.ToString(); // + strprintf("-%03d", idx);
}

