/*
  $Id: StringOps.cxx 15564 2013-01-07 14:25:32Z sloot $
  $URL: https://ilk.uvt.nl/svn/sources/libticcutils/trunk/src/StringOps.cxx $

  Copyright (c) 1998 - 2013
  ILK   - Tilburg University
  CLiPS - University of Antwerp
 
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
      http://ilk.uvt.nl/software.html
  or send mail to:
      timbl@uvt.nl
*/

#include <iostream>
#include <sstream>
#include <string>
#include <ticcutils/Timer.h>

using namespace std;

namespace TiCC {

  void Timer::stop(){
    timeval now;
    gettimeofday( &now, 0 );
    long usecs = (myTime.tv_sec + now.tv_sec - startTime.tv_sec) * 1000000 
      + myTime.tv_usec + now.tv_usec - startTime.tv_usec;
    ldiv_t div = ldiv( usecs, 1000000 );
    myTime.tv_sec = div.quot;
    myTime.tv_usec = div.rem;
  }

  string Timer::now() {
    time_t now;
    time( &now );
    return asctime( localtime( &now ) );
  }

  ostream& operator << ( ostream& os, const Timer& T ){
    ldiv_t div = ldiv( T.myTime.tv_usec, 1000 );
    os << T.myTime.tv_sec << " seconds, " << div.quot << " milliseconds and "
       << div.rem << " microseconds";
    return os;
  }
  
  string Timer::toString(){
    stringstream os;
    os << *this;
    return os.str();
  }

}
