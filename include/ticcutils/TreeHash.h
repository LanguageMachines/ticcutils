/*
  Copyright (c) 2006 - 2020
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

#ifndef TICC_TREE_H
#define TICC_TREE_H

#include <vector>
#include <ostream>
#include "ticcutils/Trie.h"

namespace Hash {
  using namespace Tries;

  /// HashInfo is a structure used by StringHash to store a string and
  /// an unique ID
  class HashInfo {
    friend std::ostream& operator<< ( std::ostream&, const HashInfo& );
  public:
    HashInfo( const std::string&, const unsigned int );
    ~HashInfo();
    const std::string& Name() const {
      /*!
	\return the value in the HashInfo
      */
      return name;
    };
    unsigned int Index() const {
      /*!
	\return the index in the HashInfo
      */
      return ID;
    };
  private:
    const std::string name;
    unsigned int ID;
    HashInfo( const HashInfo& );
    HashInfo& operator=( const HashInfo& );
  };

  /// The Stringhash class is used to enumerate strings.
  ///
  /// Every string gets an UNIQUE id assigned.
  ///
  /// It also keeps a reverse index from the id back to the string.
  ///
  /// Internally it uses a Trie for fast inserting en retrieving
  class StringHash {
    friend std::ostream& operator << ( std::ostream&, const StringHash& );
  public:
    StringHash();
    ~StringHash();
    unsigned int NumOfEntries() const {
      /*!
	\return the number of entries in the StringHash
      */
      return NumOfTokens;
    };
    unsigned int Hash( const std::string& );
    unsigned int Lookup( const std::string& ) const;
    const std::string& ReverseLookup( unsigned int ) const;
  private:
    unsigned int NumOfTokens;
    std::vector<HashInfo*> rev_index;
    Trie<HashInfo> StringTree;
    StringHash( const StringHash& );
    StringHash& operator=( const StringHash& );
  };

}
#endif
