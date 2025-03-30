# litebrowser-linux

A simple browser based on the [litehtml](https://github.com/litehtml/litehtml) engine for Linux and Mac. See [litebrowser](https://github.com/litehtml/litebrowser) for the Windows version.

## Pre-requisites on Linux

### Install required dependencies:

#### Fedora
 ```
 dnf install gtkmm4.0-devel libcurl-devel cairo-devel pango-devel
 ```

### Optional dependencies:

The optional library ```libadwaita``` will add support for dark themes, High Contrast mode and some other GNOME related features.

#### Fedora
 ```
 dnf install libadwaita-devel
 ```

## Pre-requisites on Mac

Install dependencies using [Homebrew](https://brew.sh/):

 * [gtkmm4](https://formulae.brew.sh/formula/gtkmm4)
 * [gtk4](https://formulae.brew.sh/formula/gtk4)

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
