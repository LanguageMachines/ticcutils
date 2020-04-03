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

#ifndef SOCKET_BASICS_H
#define SOCKET_BASICS_H

#include <string>

#ifdef _WIN32
#include <winsock.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <arpa/inet.h>
#endif

namespace Sockets {

  class Socket {
  public:
  Socket(): nonBlocking(false),sock(-1){
      /// create a new Socket. Not connected yet!
    };
    virtual ~Socket();
    bool isValid() const {
      /*!
	\return true if the Socket object is connected to a working socket
      */
      return sock != -1 ;
    };
    std::string getMessage() const;
    int getSockId() const {
      /*!
	\return the socket's id
      */
      return sock;
    }
    bool read( std::string& );
    bool read( std::string&, unsigned int );
    bool write( const std::string& );
    bool write( const std::string&, unsigned int );
    bool setNonBlocking();
    bool setBlocking();
  protected:
    bool nonBlocking; //!< (non-)blocking status. default is false
    int sock;         //!< the id of the internal socket
    std::string mess; //!< a buffer to store error messages
  };

  class ClientSocket: public Socket {
  public:
    bool connect( const std::string&, const std::string& );
  };

  class ServerSocket: public Socket {
  public:
    bool connect( const std::string& );
    bool listen( unsigned int = 5 );
    bool accept( ServerSocket& );
    std::string getClientName() const {
      /*!
	\return the given name of the Client
      */
      return clientName;
    };
  private:
    std::string clientName; //!< store the client's name here
  };
}

#endif
