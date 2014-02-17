# PandaCoin [PND, Ᵽ] Integration/Staging Tree
http://thepandacoin.co

![PandaCoin](http://i.imgur.com/qbrwsMm.png)

## What is PandaCoin? - Such coin
PandaCoin is like Bitcoin, but based on Litecoin, and also much more wow.
http://thepandacoin.co

## License - Much license
PandaCoin is released under the terms of the MIT license. See [COPYING](COPYING)
for more information or see http://opensource.org/licenses/MIT.

## Development and contributions - omg developers
Developers work in their own trees, then submit pull requests when they think
their feature or bug fix is ready.

## Very Much Frequently Asked Questions

### How much panda can exist?
Early 2015 (approximately a year and a half after release) there will be approximately 100,000,000,000 coins.
Each subsequent block will grant 10,000 coins to encourage miners to continue to secure the network and make up for lost wallets on hard drives/phones/lost encryption passwords/etc.
For the block schedule, see http://en.wikipedia.org/wiki/PandaCoin#Block_schedule

### How get panda?
Scrypt Proof of Work

1 Minute Block Targets, 4 Hour Diff Readjustments

Special reward system: Random block rewards

Block 1-50,000: 0-1,000,000 PandaCoin Reward

Block 50,001 — 100,000: 0-500,000 PandaCoin Reward

Block 100,001 — 200,000: 0-250,000 PandaCoin Reward

Block 200,001 — 300,000: 0-125,000 PandaCoin Reward

Block 300,001 — 400,000: 0-62,500 PandaCoin Reward

Block 400,001 - 500,000: 0-31,250 PandaCoin Reward

Block 500,000+: 5,000 PandaCoin Reward

### Wow plz make pandacoind

    sudo apt-get install build-essential \
                         libssl-dev \
                         libdb5.1++-dev \
                         libboost-all-dev \
                         libqrencode-dev \
                         libminiupnpc-dev

    cd src/
    make -f makefile.unix USE_UPNP=1 USE_IPV6=1 USE_QRCODE=1

### Such ports
RPC 22444
P2P 22445
