/*
  Copyright (c) 2006 - 2025
  CLST  - Radboud University
  ILK   - Tilburg University

  This file is part of ticcutils

  ticcutils is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.

  ticcutils is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, see <http://www.gnu.org/licenses/>.

  For questions and suggestions, see:
      https://github.com/LanguageMachines/ticcutils/issues
  or send mail to:
      lamasoftware (at ) science.ru.nl
*/

#ifndef TICC_ENUM_FLAGS_H
#define TICC_ENUM_FLAGS_H

#include <type_traits>

namespace TiCC {

#define DEFINE_ENUM_FLAG_OPERATORS(ENUMTYPE)	\
  inline ENUMTYPE operator | (ENUMTYPE a, ENUMTYPE b) { return ENUMTYPE(((std::underlying_type<ENUMTYPE>::type)a) | ((std::underlying_type<ENUMTYPE>::type)b)); } \
  inline ENUMTYPE &operator |= (ENUMTYPE &a, ENUMTYPE b) { return (ENUMTYPE &)(((std::underlying_type<ENUMTYPE>::type &)a) |= ((std::underlying_type<ENUMTYPE>::type)b)); } \
  inline ENUMTYPE operator & (ENUMTYPE a, ENUMTYPE b) { return ENUMTYPE(((std::underlying_type<ENUMTYPE>::type)a) & ((std::underlying_type<ENUMTYPE>::type)b)); } \
  inline ENUMTYPE &operator &= (ENUMTYPE &a, ENUMTYPE b) { return (ENUMTYPE &)(((std::underlying_type<ENUMTYPE>::type &)a) &= ((std::underlying_type<ENUMTYPE>::type)b)); } \
  inline ENUMTYPE operator ~ (ENUMTYPE a) { return ENUMTYPE(~((std::underlying_type<ENUMTYPE>::type)a)); } \
  inline ENUMTYPE operator ^ (ENUMTYPE a, ENUMTYPE b) { return ENUMTYPE(((std::underlying_type<ENUMTYPE>::type)a) ^ ((std::underlying_type<ENUMTYPE>::type)b)); } \
  inline ENUMTYPE &operator ^= (ENUMTYPE &a, ENUMTYPE b) { return (ENUMTYPE &)(((std::underlying_type<ENUMTYPE>::type &)a) ^= ((std::underlying_type<ENUMTYPE>::type)b)); } \
  inline bool operator % ( const ENUMTYPE &a, ENUMTYPE b) { return (a & b) == b; }
} //namespace TICC
#endif
