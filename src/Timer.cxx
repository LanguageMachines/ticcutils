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

#include <ticcutils/Timer.h>

#include <iostream>
#include <sstream>
#include <string>

using namespace std;

namespace TiCC {

  void Timer::milli_wait( int m_secs ){
    /// sleep for some milli-seconds
    /*!
      \param m_secs the milliseconds to sleep
    */
    struct timespec tv;
    ldiv_t div = ldiv( m_secs, 1000 );
    tv.tv_sec = div.quot;               // seconds
    tv.tv_nsec = div.rem * 1000000;     // nanoseconds
    while ( nanosleep( &tv, &tv ) < 0 ){
      // continue when interrupted
    }
  }

  void Timer::stop(){
    /// stop the current timer and add the current lap
    timeval now_time;
    gettimeofday( &now_time, 0 );
    long usecs = (myTime.tv_sec + now_time.tv_sec - startTime.tv_sec) * 1000000
      + myTime.tv_usec + now_time.tv_usec - startTime.tv_usec;
    ldiv_t div = ldiv( usecs, 1000000 );
    myTime.tv_sec = div.quot;
    myTime.tv_usec = div.rem;
  }

  string Timer::now(){
    /// return the current system time as a string
    time_t now_time;
    time( &now_time );
    char buffer[256];
    strftime( buffer, 100, "%c", localtime( &now_time ) );
    return buffer;
  }

  ostream& operator << ( ostream& os, const Timer& T ){
    /// pretty print the Timers value to a stream
    /*!
      \param os a stream
      \param T the Timer to display
    */
    ldiv_t div = ldiv( T.myTime.tv_usec, 1000 );
    os << T.myTime.tv_sec << " seconds, " << div.quot << " milliseconds and "
       << div.rem << " microseconds";
    return os;
  }

  Timer& Timer::operator+=( const Timer& rhs ){
    /// add the value of a Timer to this one
    /*!
      \param rhs the timer to add
      \return the incremented Timer
    */
    timeradd( &(this->myTime), &(rhs.myTime), &(this->myTime) );
    return *this;
  }

  Timer operator+( Timer lhs, const Timer& rhs){
    /// add the value of 2 Timers
    /*!
      \param lhs the timer to add to
      \param rhs the timer to add
      \return the incremented Timer
    */
    lhs += rhs;
    return lhs;
  }

  string Timer::toString(){
    /// pretty print the Timer value to a string
    stringstream os;
    os << *this;
    return os.str();
  }

}
