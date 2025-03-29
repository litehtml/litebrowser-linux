# litebrowser-linux

A simple browser based on the [litehtml](https://github.com/litehtml/litehtml) engine for Linux and Mac. See [litebrowser](https://github.com/litehtml/litebrowser) for the Windows version.

## Pre-requisites on Linux

Install dependencies:

### Fedora
 ```
 dnf install gtkmm4.0-devel libcurl-devel cairo-devel pango-devel
 ```

## Pre-requisites on Mac

Install dependencies using [Homebrew](https://brew.sh/):

 * [gtkmm3](https://formulae.brew.sh/formula/gtkmm4)
 * [gtk+3](https://formulae.brew.sh/formula/gtk4)

```
brew install gtkmm4 gtk4
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
