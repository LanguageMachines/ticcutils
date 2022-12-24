/*
  Copyright (c) 2006 - 2023
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

#include <cassert>
#include <string>
#include <cstdlib>
#include "config.h"
#include <iostream>
#include <unistd.h>
#include <stdexcept>

#include "ticcutils/StringOps.h"
#include "ticcutils/LogStream.h"

using namespace std;
using namespace TiCC;

class Sub1 {
public:
  explicit Sub1( LogStream& log ){
    ls = new LogStream( log, "-SUB1" );
    *Log(ls) << "created a sub1 " << endl;
  }
  ~Sub1(){ delete ls; };
  void exec( int i ){
    int sleeps = rand()%(i+1) + 1;
    sleep(sleeps);
    *Log(ls) << i << " x" << endl;
  }
  LogStream *ls;
private:
  Sub1( const Sub1& );
};

class Sub2 {
public:
  explicit Sub2( LogStream* log ){
    ls = log;
    *Log(*ls) << "created a sub2 " << endl;
  }
  ~Sub2(){};
  void exec( int i ){
    int sleeps = rand()%(i+1) + 1;
    sleep(sleeps);
    *Log(ls) << i << " y" << endl;
  }
  LogStream *ls;
};

class Sub3 {
public:
  explicit Sub3( Sub2& s ){
    ls = new LogStream( s.ls, "-SUB3", StampMessage );
    *Log(ls) << "created a sub3 " << endl;
  }
  ~Sub3(){ delete ls; };
  void exec( int i ){
    int sleeps = rand()%(i+1) + 1;
    sleep(sleeps);
    *Log(ls) << i << " z" << endl;
  }
  LogStream *ls;
private:
  Sub3( const Sub3& );
};

int main(){
  LogStream the_log( "main-log" );
  Sub1 sub1( the_log );
  Sub2 sub2( &the_log );
  assert( IsActive( the_log ) );
  sub1.exec(1);
#pragma omp parallel for schedule(dynamic)
  for ( int i = 0; i < 5; ++i ){
    sub1.exec(i+2);
    sub2.exec(i+2);
    Sub3 sub3( sub2 );
    sub3.exec(i+2);
  }
    sub2.exec(1);
}
