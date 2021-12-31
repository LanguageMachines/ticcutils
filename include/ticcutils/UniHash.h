/*
  Copyright (c) 2006 - 2022
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

#ifndef TICC_UNITREE_H
#define TICC_UNITREE_H

#include <vector>
#include <ostream>
#include "ticcutils/UniTrie.h"
#include "ticcutils/Unicode.h"

namespace Hash {

  /// \brief UniInfo is a structure used by UnicodeHash to store a
  /// UnicodeString and an unique ID
  class UniInfo {
    friend std::ostream& operator<< ( std::ostream&, const UniInfo& );
  public:
    UniInfo( const icu::UnicodeString&, const unsigned int );
    ~UniInfo();
    const icu::UnicodeString& value() const {
      /*!
	\return the value in the UniInfo
      */
      return _value;
    };
    unsigned int index() const {
      /*!
	\return the index in the UniInfo
      */
      return _ID;
    };
  private:
    const icu::UnicodeString _value;
    unsigned int _ID;
    UniInfo( const UniInfo& );
    UniInfo& operator=( const UniInfo& );
  };

  /// \brief The UnicodeHash class is used to enumerate Unicode strings.
  ///
  /// Every string gets an UNIQUE id assigned.
  ///
  /// It also keeps a reverse index from the id back to the string.
  ///
  /// Internally it uses a UniTrie for fast inserting en retrieving
  class UnicodeHash {
    friend std::ostream& operator << ( std::ostream&, const UnicodeHash& );
  public:
    UnicodeHash();
    ~UnicodeHash();
    unsigned int num_of_entries() const {
      /*!
	\return the number of entries in the StringHash
      */
      return _num_of_tokens;
    };
    unsigned int hash( const icu::UnicodeString& );
    unsigned int lookup( const icu::UnicodeString& ) const;
    const icu::UnicodeString& reverse_lookup( unsigned int ) const;
  private:
    unsigned int _num_of_tokens;
    std::vector<UniInfo*> _rev_index;
    Tries::UniTrie<UniInfo> _tree;
    UnicodeHash( const UnicodeHash& );
    UnicodeHash& operator=( const UnicodeHash& );
  };

}
#endif
