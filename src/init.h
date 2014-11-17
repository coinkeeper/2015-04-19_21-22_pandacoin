// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef BITCOIN_INIT_H
#define BITCOIN_INIT_H

#include "wallet.h"

#ifndef HEADLESS
#include "qt/optionsmodel.h"
#endif

extern CWallet* pwalletMain;
extern std::string strWalletFileName;
void StartShutdown();
void Shutdown(void* parg);

#ifdef HEADLESS
bool AppInit2();
#else
bool AppInit2(OptionsModel& optionsModel);
#endif

std::string HelpMessage();

#endif
