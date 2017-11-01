/*
  Copyright (c) 2006 - 2017
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
#ifndef TICC_TIMER_H
#define TICC_TIMER_H

#include <cstdlib>
#include <sys/time.h>
#include <iostream>

namespace TiCC {
  class Timer {
  public:
    friend std::ostream& operator << ( std::ostream& os, const Timer& T );
    Timer(){ reset(); };
    void reset(){ myTime.tv_sec=0; myTime.tv_usec=0; };
    void start(){
      gettimeofday( &startTime, 0 );
    };
    void stop();
    unsigned int secs() { stop(); return myTime.tv_sec; };
    std::string toString();
    static std::string now();
  private:
    timeval startTime;
    timeval myTime;
  };

}

#endif // TICC_TIMER_H
