# litebrowser-linux

A simple browser based on the [litehtml](https://github.com/litehtml/litehtml) engine for Linux and Mac. See [litebrowser](https://github.com/litehtml/litebrowser) for the Windows version.

## Pre-requisites on Linux

Install dependencies:

 * vim-core for xxd

## Pre-requisites on Mac

Install dependencies using [Homebrew](https://brew.sh/):

 * [gtkmm3](https://formulae.brew.sh/formula/gtkmm3)
 * [gtk+3](https://formulae.brew.sh/formula/gtk+3)

```
brew install gtkmm3 gtk+3
```

## Common Build instructions

Clone this repository and build the `litebrowser` executable:

```
git clone --recursive https://github.com/litehtml/litebrowser-linux.git
cd litebrowser-linux
mkdir build
cd build
cmake ..
make
```
