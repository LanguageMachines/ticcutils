# osx_pkg.m4 - Macros to add OSX brew locations to pkg-config. -*- Autoconf -*-
# serial 2 (pkg-config-0.24)
#
# Copyright © 2024 Ko van der Sloot <K.vanderSloot@let.ru.nl>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#
# As a special exception to the GNU General Public License, if you
# distribute this file as part of a program that contains a
# configuration script generated by Autoconf, you may include it under
# the same distribution terms that you use for the rest of that program.

# AC_OSX_PKG_ALL()
# add all /opt/{package} directories to the PKG_CONFIG search path
# ----------------------------------
AC_DEFUN([AC_OSX_PKG_ALL],
[
case ${host_os} in
 linux*)
 # linux is wellbehaved
    ;;
 darwin*)
 # darwin isn't
    for i in `ls /usr/local/opt/`
    do
      if test -d "/usr/local/opt/$i/lib/pkgconfig"
      then
        export PKG_CONFIG_PATH="$PKG_CONFIG_PATH:/usr/local/opt/$i/lib/pkgconfig"
      fi
    done
    for i in `ls /opt/homewbrew/opt`
    do
      if test -d "/opt/homebrew/opt/$i/lib/pkgconfig"
      then
        export PKG_CONFIG_PATH="$PKG_CONFIG_PATH:/opt/homebrew/opt/$i/lib/pkgconfig"
      fi
    done
    ;;
esac
])

# AC_OSX_PKG_ALL([LIST_OF_PACKAGES])
# fore every packake in LIST_OF_PACKAGES, add the /opt/{package} directory
# to the PKG_CONFIG search path
# ----------------------------------
AC_DEFUN([AC_OSX_PKG],
[
case ${host_os} in
 linux*)
 # linux is wellbehaved
    ;;
 darwin*)
 # darwin/mac0s isn't
    for i in $*
    do
      if test -d "/usr/local/opt/$i/lib/pkgconfig"
      then
        export PKG_CONFIG_PATH="$PKG_CONFIG_PATH:/usr/local/opt/$i/lib/pkgconfig"
      else
        if test -d "/opt/homebrew/opt//$i/lib/pkgconfig"
        then
          export PKG_CONFIG_PATH="$PKG_CONFIG_PATH:/opt/homebrew/opt/$i/lib/pkgconfig"
        fi
      fi
    done
    ;;
esac
])
