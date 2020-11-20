# Melvix

<p align="center">
    <i>„A lightweight unix-unlike operating system“</i>
    <br><br>
    <a href="https://github.com/marvinborner/Melvix/actions?query=workflow%3A%22Project+build%22" target="_blank">
        <img src="https://img.shields.io/github/workflow/status/marvinborner/Melvix/Project%20build?style=for-the-badge" />
    </a>
    <a href="https://app.codacy.com/manual/marvin-borner/Melvix/dashboard" target="_blank">
        <img src="https://img.shields.io/codacy/grade/4ae29e218d7c439eaa549ea828ffcaac?style=for-the-badge" />
    </a>
    <a href="https://www.buymeacoffee.com/marvinborner" target="_blank">
        <img src="https://img.shields.io/static/v1?label=Support&message=buymeacoffee&color=brightgreen&style=for-the-badge" />
    </a>
</p>

## Disclaimer

This project is somewhat of a coding playground for me. It doesn't have any useful functionality (yet?). Be aware that the installation on real hardware is not recommended and may break your computer.

## Features

-   From scratch (no POSIX/UNIX compatibility at all)
-   No external libraries
-   Efficient Multitasking
-   EXT2 filesystem
-   Minimal GUI
-   Fast boot time (< 1s)
-   TCP/IP stack and rtl8139 driver
-   Small size (< 100KiB)
-   Compiles with `-Wall -Wextra -pedantic-errors -std=c99 -Ofast`

## Screenshot

![Melvix screenshot](screenshot.png?raw=true "Screenshot")

## Test

-   Install the qemu i386 emulator
-   Download the `disk-img` artifact from the newest [GitHub Workflow build](https://github.com/marvinborner/Melvix/actions)
-   Unzip `disk-img.zip`
-   Run `qemu-system-i386 -vga std -drive file=path/to/disk.img,format=raw,index=1,media=disk -netdev user,id=net0 -device rtl8139,netdev=net0`
-   Enjoy, or try building it yourself!

## Build & Test

-   Use any system running GNU/Linux or OpenBSD

-   Install build dependencies (package names may vary depending on your operating system)

    -   Ubuntu/Debian _"instructions"_ can be found here: [GitHub Workflow](https://raw.githubusercontent.com/marvinborner/Melvix/main/.github/workflows/build.yml)
    -   OpenBSD: `pkg_add git ccache gcc g++ gmake bison gmp libmpc mpfr texinfo curl nasm qemu e2fsprogs`
    -   git
    -   binutils
    -   ccache
    -   gcc
    -   make
    -   bison
    -   flex
    -   gmp
    -   mpc
    -   mpfr
    -   texinfo
    -   curl
    -   nasm
    -   qemu

-   Clone this repository using `git clone --recurse-submodules https://github.com/marvinborner/Melvix.git`

-   Load fonts and images into the disk image via `./run disk`

-   Run `./run`

-   If you need help: `./run to help`

## Licenses

Melvix is released under the MIT License and uses parts of the following 3rd party projects:

Knowledge:

-   [OSDev wiki](https://wiki.osdev.org) - Very helpful!
-   [James Molloy's tutorials](http://jamesmolloy.co.uk/tutorial_html/)
-   [virtix - tasking inspiration](https://github.com/16Bitt/virtix/) - [MIT License](https://github.com/16Bitt/virtix/blob/85a3c58f3d3b8932354e85a996a79c377139c201/LICENSE)
-   [studix - FS inspiration](https://github.com/orodley/studix) - [MIT License](https://github.com/orodley/studix/blob/d1b1d006010120551df58ff3faaf97484dfa9806/LICENSE)
-   [ToAruOS - PCI and network driver inspiration](https://github.com/klange/toaruos) - [NCSA License](https://github.com/klange/toaruos/blob/351d5d38f22b570459931475d36468bf4e37f45a/LICENSE)

Resources:

-   [Spleen font](https://github.com/fcambus/spleen) - [MIT License](https://github.com/fcambus/spleen/blob/5759e9abb130b89ba192edc5324b12ef07b7dad3/LICENSE)
-   [ext2util](https://github.com/lazear/ext2util) - [MIT License](https://github.com/lazear/ext2util/blob/9bbcc81be46416491f8d46f25f7f775cfb7f2261/LICENSE)

Libraries:
-   [upng (heavily modified lodepng)](https://github.com/elanthis/upng) - [ZLIB License](https://github.com/lvandeve/lodepng/blob/7fdcc96a5e5864eee72911c3ca79b1d9f0d12292/LICENSE)
