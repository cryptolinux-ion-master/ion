### Usage

To build dependencies for the current arch+OS:

    make

To build for another arch/OS:

    make HOST=host-platform-triplet

For example:

    make HOST=x86_64-w64-mingw32 -j4

A prefix will be generated that's suitable for plugging into Ion's
configure. In the above example, a dir named x86_64-w64-mingw32 will be
created. To use it for Ion:

    ./configure --prefix=`pwd`/depends/x86_64-w64-mingw32

Common `host-platform-triplets` for cross compilation are:

- `i686-pc-linux-gnu` for Linux PC 32-Bit
- `x86_64-pc-linux-gnu` for Linux PC 64-Bit
- `x86_64-w64-mingw32` for Win64
- `i686-w64-mingw32` for Win32
- `x86_64-apple-darwin16` for macOS
- `arm-linux-gnueabihf` for Linux ARM 32 bit
- `aarch64-linux-gnu` for Linux ARM 64 bit
- `mipsel-linux-gnu`for Linux MIPSEL 32 bit
- `mips-linux-gnu`for Linux MIPS 32 bit
- `powerpc-linux-gnu` for Linux PPC 32 bit
- `powerpc64-linux-gnu` for Linux PPC64 64 bit
- `powerpc64el-linux-gnu` for Linux PPC64EL 64 bit
- `riscv32-linux-gnu` for Linux RISC-V 32 bit
- `riscv64-linux-gnu` for Linux RISC-V 64 bit
- `s390x-linux-gnu` for Linux S390X
- `armv7a-linux-android` for Android ARM 32 bit
- `aarch64-linux-android` for Android ARM 64 bit
- `i686-linux-android` for Android x86 32 bit
- `x86_64-linux-android` for Android x86 64 bit

No other options are needed, the paths are automatically configured.

Dependency Options:
The following can be set when running make: make FOO=bar

    SOURCES_PATH: downloaded sources will be placed here
    BASE_CACHE: built packages will be placed here
    SDK_PATH: Path where sdk's can be found (used by OSX)
    FALLBACK_DOWNLOAD_PATH: If a source file can't be fetched, try here before giving up
    NO_QT: Don't download/build/cache qt and its dependencies
    NO_WALLET: Don't download/build/cache libs needed to enable the wallet
    NO_UPNP: Don't download/build/cache packages needed for enabling upnp
    DEBUG: disable some optimizations and enable more runtime checking
    HOST_ID_SALT: Optional salt to use when generating host package ids
    BUILD_ID_SALT: Optional salt to use when generating build package ids

If some packages are not built, for example `make NO_WALLET=1`, the appropriate
options will be passed to Ion Core's configure. In this case, `--disable-wallet`.

Additional targets:

    download: run 'make download' to fetch all sources without building them
    download-osx: run 'make download-osx' to fetch all sources needed for osx builds
    download-win: run 'make download-win' to fetch all sources needed for win builds
    download-linux: run 'make download-linux' to fetch all sources needed for linux builds

### Other documentation

- [description.md](description.md): General description of the depends system
- [packages.md](packages.md): Steps for adding packages

