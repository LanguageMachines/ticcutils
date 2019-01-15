/*
  Copyright (c) 2006 - 2019
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

  HashInfo::HashInfo( const string& Tname, unsigned int Indx ):
    name(Tname), ID(Indx){}

  HashInfo::~HashInfo(){
  }

  ostream& operator<<( ostream& os, const HashInfo& tok ){
    os << tok.ID << " " << tok.name;
    return os;
  }

  StringHash::StringHash():
    NumOfTokens(0)
  {}

  StringHash::~StringHash( ){
  }

  unsigned int StringHash::Hash( const string& name ){
    unsigned int idx = 0;
    HashInfo *info = StringTree.Retrieve( name );
    if ( !info ){
      info = new HashInfo( name, ++NumOfTokens );
      info = reinterpret_cast<HashInfo *>(StringTree.Store( name, info ));
    }
    idx = info->Index();
    if ( idx >= rev_index.size() ){
      rev_index.resize( rev_index.size() + 1000 );
    }
    rev_index[idx] = info;
    return idx;
  }

  unsigned int StringHash::Lookup( const string& name ) const {
    HashInfo *info = StringTree.Retrieve( name );
    if ( info )
      return info->Index();
    else
      return 0;
  }

  const string& StringHash::ReverseLookup( unsigned int index ) const {
    return rev_index[index]->Name();
  }

  ostream& operator << ( ostream& os, const StringHash& S){
    return os << &S.StringTree; }

  LexInfo::LexInfo( const string& Tname, const string& Tran ):
    name(Tname),trans(Tran){}

  LexInfo::~LexInfo(){}

  ostream& operator<<( ostream& os, const LexInfo& LI ){
    os << " " << LI.name << " - " << LI.trans;
    return os;
  }

  Lexicon::Lexicon(){}

  Lexicon::~Lexicon(){}

  LexInfo *Lexicon::Lookup( const string& name ) const {
    return reinterpret_cast<LexInfo *>(LexTree.Retrieve( name ));
  }

  LexInfo *Lexicon::Store( const string& name, const string& translation ){
    LexInfo *info = 0;
    info = LexTree.Retrieve( name );
    if ( !info ){
      info = new LexInfo( name, translation );
      info = LexTree.Store( name, info );
    }
    return info;
  }

  ostream& operator<<( ostream& os, const Lexicon& L )
  { return os << &L.LexTree; }

}
