
/*
  Copyright (c) 2006 - 2024
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

#include "ticcutils/UniHash.h"
#include "ticcutils/Unicode.h"
// normalizer2.h is included via Unicode.h

using namespace std;
using namespace icu;

namespace Hash {

  UniInfo::UniInfo( const UnicodeString& value,
		    const unsigned int index ):
    _value(value),_ID(index){
    /// create a UniInfo record
    /*!
      \param value the value to store, assumed to be NFC normalized!
      \param index the index to store
    */
  }

  UniInfo::~UniInfo(){
    /// destroy a UniInfo record
  }

  ostream& operator<<( ostream& os,
		       const UniInfo& tok ){
    /// output a UniInfo record
    os << tok._ID << " " << tok._value;
    return os;
  }

  UnicodeHash::UnicodeHash():
    _num_of_tokens(0) {
    /// initialize a new UnicodeHash
  }

  UnicodeHash::~UnicodeHash(){
    /// destroy a UnicodeHash
    // We must explicitly delete the pointers stored in the map
    for ( auto& pair : _map ) {
        delete pair.second;
    }
  }

  unsigned int UnicodeHash::hash( const UnicodeString& value ){
    /// lookup or create a hash for the string parameter
    /*!
      \param value the string to hash
      \return the hash value
      when a new hash is inserted, the reverse index is also updated
      the UnicodeString will be NFC normalized first.
    */
    UErrorCode status = U_ZERO_ERROR;
    const Normalizer2 *n2 = Normalizer2::getNFCInstance(status);
    const UnicodeString *pVal = &value;
    UnicodeString norm_storage;

    // Optimization: Only normalize if strictly necessary
    if ( U_SUCCESS(status) && !n2->isNormalized(value, status) ){
      n2->normalize(value, norm_storage, status);
      pVal = &norm_storage;
    }

    // Map lookup
    auto it = _map.find( *pVal );
    UniInfo *info = nullptr;

    if ( it == _map.end() ){
      // Not found, insert new
      info = new UniInfo( *pVal, ++_num_of_tokens );
      _map.insert({ *pVal, info });
    } else {
      info = it->second;
    }

    unsigned int idx = info->index();
    if ( idx >= _rev_index.size() ){
      // Ensure capacity before resizing
      if ( idx >= _rev_index.capacity() ) {
           _rev_index.reserve( idx + 10000 );
      }
      _rev_index.resize( idx + 1000, nullptr );
    }
    _rev_index[idx] = info;
    return idx;
  }

  unsigned int UnicodeHash::lookup( const UnicodeString& value ) const {
    /// lookup the hash for a string in the UnicodeHash
    /*!
      \param value the string to lookup
      \return the hash value, or 0 when not found
    */
    UErrorCode status = U_ZERO_ERROR;
    const Normalizer2 *n2 = Normalizer2::getNFCInstance(status);
    const UnicodeString *pVal = &value;
    UnicodeString norm_storage;

    // Optimization: Only normalize if strictly necessary
    if ( U_SUCCESS(status) && !n2->isNormalized(value, status) ){
      n2->normalize(value, norm_storage, status);
      pVal = &norm_storage;
    }

    auto it = _map.find( *pVal );
    if ( it != _map.end() ){
      return it->second->index();
    }
    return 0;
  }

  const UnicodeString& UnicodeHash::reverse_lookup( unsigned int index ) const {
    /// lookup the string value for a certain index
    /*!
      \param index the index we search
      \return the string value

      \note this assumes the index is valid, which isn't checked!
    */
    return _rev_index[index]->value();
  }

  ostream& operator << ( ostream& os, const UnicodeHash& S ){
    /// output the content of a whole UnicodeHash structure (Debugging only)
    return os << "UnicodeHash map size: " << S._map.size();
  }

}
