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

#ifndef SOCKET_BASICS_H
#define SOCKET_BASICS_H

#include <string>

#ifdef _WIN32
#include <winsock.h>
#else
#endif

namespace Sockets {

  /// \brief The Socket class is a wrapper around the low-level Unix socket routines.
  ///
  /// It provides functions to create connections with associated C++
  /// io-streams for reading and writing and also low-level functions to read
  /// and write directly from/to the sockets
  ///
  /// There is also a non-blocking variant, which makes some asynchronous
  /// IO possible, using a timeout value
  class Socket {
  public:
  Socket(): nonBlocking(false),sock(-1){
      /// create a new Socket. Not connected yet!
    };
    virtual ~Socket();
    bool isValid() const {
      /*!
	\return true if the Socket object is connected to a working Unix socket
      */
      return sock != -1 ;
    };
    std::string getMessage() const;
    int getSockId() const {
      /*!
	\return the internal socket id
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

  /// \brief The ClientSocket implements a connect function to connect a Socket
  /// to a host::port
  class ClientSocket: public Socket {
    friend class ServerSocket;
  public:
    bool connect( const std::string&, const std::string& );
    const std::string& getClientName() const {
      /*!
	\return the name of the Client (auto-generated on creation)
      */
      return clientName;
    };
  private:
    std::string clientName; //!< store the client's name here
  };

  /// The ServerSocket implements functions to set up a Server on a port
  class ServerSocket: public Socket {
  public:
    bool connect( const std::string& );
    bool listen( unsigned int = 5 );
    bool accept( ClientSocket& newSocket );
  };
}

#endif
