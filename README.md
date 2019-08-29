[![Build Status](https://travis-ci.org/LanguageMachines/ticcutils.svg?branch=master)](https://travis-ci.org/LanguageMachines/ticcutils) [![Language Machines Badge](http://applejack.science.ru.nl/lamabadge.php/ticcutils)](http://applejack.science.ru.nl/languagemachines/) [![DOI](https://zenodo.org/badge/9028755.svg)](https://zenodo.org/badge/latestdoi/9028755)

TiCC utils
==============

    TiCC utils 0.22 (c) ILK/CLST 1998 - 2019
    by Ko van der Sloot

    Tilburg centre for Cognition and Communication, Tilburg University.
    Centre for Language and Speach Technology, Radboud University

This module contains useful functions for general use in the TiCC software
stack and beyond.

TiCC utils is distributed under the GNU Public Licence (see the file ``COPYING``).

This software has been tested on:
- Intel/AMD platforms running several versions of Linux, including Ubuntu,
  Debian and Fedora.
- 64 bits
- MAC platform running OS X 10.13

Compilers:
- GCC (4.9 - 7.0). It is highly recommended to upgrade to at least GCC 5.0
- CLANG. Preferrable a recent version which supports OpenMP

Contents of this distribution:
- Sources
- Licensing information ( ``COPYING`` )
- Installation instructions ( ``INSTALL`` )
- Build system based on Gnu Autotools
- Documentation ( in the ``docs/`` directory )

Dependencies:
To be able to succesfully build libticcl from the tarball, you need the
following pakages:
- ``autotools``
- ``autoconf-archive``
- ``pkg-config``
- ``libxml2-dev``
- ``icu-dev``
- ``libboost-dev`` and ``libboost-regex-dev``
- ``libtar-dev``
- ``libbz2-dev``
- ``zlib1g-dev``

Installation Instructions
--------------------------------

To install ticcutils, first consult whether your distribution's package manager
has an up-to-date package.  If not, for easy installation of ticcutils, it is
included as part of our software distribution LaMachine:
https://proycon.github.io/LaMachine .

To compile from source instead:
1. ``sh bootstrap.sh``
2. ``configure``
3. ``make``
4. ``make install``
5. *optional:* ``make check``
