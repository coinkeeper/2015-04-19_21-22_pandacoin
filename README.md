# PandaCoin [PND, Ᵽ] Integration/Staging Tree
http://thepandacoin.net

![PandaCoin](http://i.imgur.com/IoZ3PAD.png)

## What is PandaCoin?
PandaCoin is like Bitcoin, but based on Dogecoin.
http://thepandacoin.net

## License
PandaCoin is released under the terms of the MIT license. See [COPYING](COPYING)
for more information or see http://opensource.org/licenses/MIT.

## Development and contributions
Developers work in their own trees, then submit pull requests when they think
their feature or bug fix is ready.

## Frequently Asked Questions

### How much panda can exist?
Early 2015 (approximately a year and a half after release) there will be approximately 100,000,000,000 coins.
Each subsequent block will grant 10,000 coins to encourage miners to continue to secure the network and make up for lost wallets on hard drives/phones/lost encryption passwords/etc.

### How to get panda?
Scrypt Proof of Work

1 Minute Block Targets

Reward system:

50,000 per block until further notice on either proper reward structure or PoS.

### Compile pandacoind

    sudo apt-get install build-essential \
                         libssl-dev \
                         libdb5.1++-dev \
                         libboost-all-dev \
                         libqrencode-dev \
                         libminiupnpc-dev

    cd src/
    make -f makefile.unix USE_UPNP=1 USE_IPV6=1 USE_QRCODE=1

### Ports
RPC 22444
P2P 22445
