/*
  Copyright (c) 2006 - 2021
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

#include "ticcutils/TreeHash.h"

#include <iostream>
#include <cstdlib>
#include <cstring>

using namespace std;

namespace Hash {

  using namespace Tries;

  HashInfo::HashInfo( const string& value, unsigned int index ):
    name(value), ID(index){
    /// create a HashInfo record
    /*!
      \param value the value to store
      \param index the index to store
    */
  }

  HashInfo::~HashInfo(){
    /// destroy a HashInfo record
  }

  ostream& operator<<( ostream& os, const HashInfo& tok ){
    /// output a HashInfo record
    os << tok.ID << " " << tok.name;
    return os;
  }

  StringHash::StringHash():
    NumOfTokens(0) {
    /// initialize a new StringHash
  }

  StringHash::~StringHash(){
    /// destroy a StringHash
  }

  unsigned int StringHash::Hash( const string& value ){
    /// lookup or create a hash for the string parameter
    /*!
      \param value the string to hash
      \return the hash value

      when a new hash is inserted, the reverse index is also updated
    */
    unsigned int idx = 0;
    HashInfo *info = StringTree.Retrieve( value );
    if ( !info ){
      info = new HashInfo( value, ++NumOfTokens );
      info = reinterpret_cast<HashInfo *>(StringTree.Store( value, info ));
    }
    idx = info->Index();
    if ( idx >= rev_index.size() ){
      rev_index.resize( rev_index.size() + 1000 );
    }
    rev_index[idx] = info;
    return idx;
  }

  unsigned int StringHash::Lookup( const string& value ) const {
    /// lookup the hash for a string in the StringHash
    /*!
      \param value the string to lookup
      \return the hash value, or 0 when not found
    */
    HashInfo *info = StringTree.Retrieve( value );
    if ( info ){
      return info->Index();
    }
    return 0;
  }

  const string& StringHash::ReverseLookup( unsigned int index ) const {
    /// lookup the string value for a certain index
    /*!
      \param index the index we search
      \return the string value

      \note this assumes the index is valid, which isn't checked!
    */
    return rev_index[index]->Name();
  }

  ostream& operator << ( ostream& os, const StringHash& S ){
    /// output the content of a whole StringHash structure (Debugging only)
    return os << &S.StringTree;
  }

}
