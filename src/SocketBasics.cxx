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

#include "ticcutils/SocketBasics.h"

#include <string>
#include <stdexcept>
#include <iostream>

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <climits>
#include <fcntl.h>

#include "config.h"
#include "ticcutils/StringOps.h"
#include "ticcutils/Timer.h"

using namespace std;

namespace Sockets {

  Socket::~Socket() {
    /// destroy a Socket object
    if ( sock >= 0 ) {
      ::close(sock);
    }
  }

  // #define KEEP // experiment with keep-alive
  // #define DEBUG

  bool Socket::read( string& line ) {
    /// read a string from the Socket
    /*!
      \param line the result
      \return true when at least one byte is read. false otherwise

      a line is terminated by a newline (\\n) or a return (\\r)
    */
    if ( !isValid() ){
      mess = "read: socket invalid";
#ifdef DEBUG
      cerr << mess << endl;
#endif
      return false;
    }
    line = "";
    long int total_count = 0;
    char last_read = 0;
    long int bytes_read = -1;
#ifdef KEEP
    val = 1;
    setsockopt( sock, SOL_SOCKET, SO_KEEPALIVE, (void *)&val, sizeof(val) );
    val = 20;
    setsockopt( sock, SOL_TCP, TCP_KEEPIDLE, (void *)&val, sizeof(val) );
#endif
    while ( last_read != 10 ) { // read 1 character at a time upto \n
      bytes_read = ::read( sock, &last_read, 1 );
      if ( bytes_read <= 0 ){
#ifdef DEBUG
	cerr << "read res = " << bytes_read  << " ( " << strerror(bytes_read) << ")" << endl;
#endif
	// The other side may have closed unexpectedly
	::close(sock);
	sock = -1;
	break;
      }
      if ( ( last_read != 10 ) && ( last_read != 13 ) ) {
	line += last_read;
	total_count++;
      }
    }
    if ( bytes_read < 0 ) {
      mess = string("connection closed ") + strerror( bytes_read );
#ifdef DEBUG
      cerr << mess << endl;
#endif

      return false;
    }
    else if ( bytes_read == 0 ) {
      return false;
    }
    else {
      return true;
    }
  }

  bool Socket::read( string& result, unsigned int timeout ) {
    /// read a line from a nonblocking Socket
    /*!
      \param result the read line
      \param timeout seconds to use for retry
      \return true is at least one byte is read, false on error

      a line is terminated by a newline (\\n) or a return (\\r)
    */
    result = "";
    if ( !nonBlocking ){
      mess = "attempted a read with timeout on a blocking socket";
      return false;
    }
    else {
      char buf[5];
      int count = 0;
      while ( timeout > 0 ){
	ssize_t res = ::read( sock, buf, 1 );
#ifdef DEBUG
	cerr << "read res = " << res  << " ( " << strerror(res) << ")" << endl;
#endif
	if ( res == 1 ){
	  char c = buf[0];
#ifdef DEBUG
	  cerr << "-'" << c << "'-" << endl;
#endif
	  if ( c == '\n' ){
	    return true;
	  }
	  if ( c == '\r' ){
	    continue;
	  }
	  result += c;
	}
	else if ( res == -1 || res == EAGAIN || res == EWOULDBLOCK ){
	  TiCC::Timer::milli_wait(100);
	  if ( ++count == 10 ){
	    --timeout;
	    count = 0;
	  }
	}
	else {
	  mess = strerror( res );
	  ::close(sock);
	  sock = -1;
	  return false;
	}
      }
      mess = "timed out";
    }
    return false;
  }


  bool Socket::write( const string& line ){
    /// write a line to a socket
    /*!
      \param line the line to write
      \return true on succes, false on error
    */
    if ( !isValid() ){
      mess = "write: socket invalid";
      return false;
    }
    if ( !line.empty() ){
      size_t bytes_sent = 0;
      long int this_write;
      size_t count = line.length();
      const char *str = line.c_str();
      while ( bytes_sent < count ){
	do {
	  this_write = ::write(sock, str, count - bytes_sent);
#ifdef DEBUG
	  cerr << "write res = " << this_write  << " ( " << strerror(this_write) << ")" << endl;
#endif
	} while ( (this_write < 0) && (errno == EINTR) );
	if (this_write <= 0){
	  break;
	}
	bytes_sent += this_write;
	str += this_write;
      }
      if ( bytes_sent < count ) {
	mess = "write: failed to sent " + TiCC::toString(count - bytes_sent) +
	  " bytes out of " + TiCC::toString(count);
	::close(sock);
	sock = -1;
	return false;
      }
    }
    return true;
  }

  bool Socket::write( const string& line, unsigned int timeout ){
    /// write a line to a non-blocking socket
    /*!
      \param line the line to write
      \param timeout the number of seconds to use for retrying
      \return true on succes, false on error
    */
    if ( !isValid() ){
      mess = "write: socket invalid";
      return false;
    }
    if ( !line.empty() ){
      size_t bytes_sent = 0;
      size_t count = line.length();
      const char *str = line.c_str();
      while ( timeout > 0 && bytes_sent < count ){
	int res = ::write(sock, str, 1 );
	if ( res == 1 ){
	  ++bytes_sent;
	  ++str;
	}
	else if ( res == EAGAIN || res == EWOULDBLOCK ){
	  TiCC::Timer::milli_wait(100);
	  if ( ++count == 10 ){
	    --timeout;
	    count = 0;
	  }
	}
	else {
	  mess = strerror( res );
	  ::close(sock);
	  sock = -1;
	  return false;
	}
      }
      if ( bytes_sent < count ) {
	mess = "write: failed to sent " + TiCC::toString(count - bytes_sent) +
	  " bytes out of " + TiCC::toString(count);
	::close(sock);
	sock = -1;
	return false;
      }
    }
    return true;
  }

  string Socket::getMessage() const{
    /// return an error message, which might be set in lower layers
    string m;
    if ( isValid() ){
      m = "socket " + TiCC::toString(sock);
    }
    else {
      m = "invalid socket ";
    }
    if ( !mess.empty() ){
      m += ": " + mess;
    }
    return m;
  }

  bool Socket::setBlocking( ) {
    /// set the socket to blocking mode (which is the default for normal
    /// IO-streams)
    /*!
      \return false on failure, true otherwise
    */
    int opts = fcntl( sock, F_GETFL );
#ifdef DEBUG
    cerr << "socket opts = " << opts << endl;
#endif
    if ( opts < 0 ) {
      mess = "fctl failed";
#ifdef DEBUG
    cerr << "fctl: " << mess << endl;
#endif
      return false;
    }
    else {
      opts = ( opts & ~O_NONBLOCK );
#ifdef DEBUG
      cerr << "try to set socket opts = " << opts << endl;
#endif
      if ( fcntl( sock, F_SETFL, opts ) < 0 ){
	mess = "fctl failed";
#ifdef DEBUG
	cerr << "fctl: " << mess << endl;
#endif
	return false;
      }
    }
    nonBlocking = false;
#ifdef DEBUG
    cerr << "setBlocking done" << endl;
#endif
    return true;
  }

  bool Socket::setNonBlocking() {
    /// set the socket to non-blocking mode
    /*!
      \return false on failure, true otherwise
    */
    int opts = fcntl( sock, F_GETFL );
#ifdef DEBUG
    cerr << "socket opts = " << opts << endl;
#endif
    if ( opts < 0 ) {
      mess = "fctl failed";
#ifdef DEBUG
    cerr << "fctl: " << mess << endl;
#endif
      return false;
    }
    else {
      opts = ( opts | O_NONBLOCK );
#ifdef DEBUG
      cerr << "try to set socket opts = " << opts << endl;
#endif
      if ( fcntl( sock, F_SETFL, opts ) < 0 ){
	mess = "fctl failed";
#ifdef DEBUG
	cerr << "fctl: " << mess << endl;
#endif
	return false;
      }
    }
    nonBlocking = true;
#ifdef DEBUG
    cerr << "setNonBlocking done" << endl;
#endif
    return true;
  }

#ifdef HAVE_GETADDRINFO

  bool ClientSocket::connect( const string& hostString,
			      const string& portString ){
    /// connect a Client to an external server
    /*!
      \param hostString the name of the server to use
      \param portString the number of the port of the server
      \return true on success, false otherwise
    */
    struct addrinfo *res, *aip;
    struct addrinfo hints;
    memset( &hints, 0, sizeof(hints) );
    hints.ai_flags = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    int eno;
    sock = -1;
    if ( (eno=getaddrinfo( hostString.c_str(), portString.c_str(),
			   &hints, &res ) ) != 0 ){
      mess = "ClientSocket connect: invalid hostname '" +
	hostString + "' (" + gai_strerror(eno) + ")";
    }
    else {
      aip = res;
      while ( aip ){
	sock = socket( aip->ai_family, aip->ai_socktype, aip->ai_protocol);
	if ( sock > 0 ){
	  break;
	}
	sock = -1;
	aip = aip->ai_next;
      }
      if ( sock < 0 || aip == 0 ){
	mess = string( "ClientSocket: Socket could not be created: (" )
	  + strerror(errno) + ")";
      }
      else {
	// connect the socket
	int val = 1;
	setsockopt( sock, SOL_SOCKET, SO_REUSEADDR, (void *)&val, sizeof(val) );
	val = 1;
	setsockopt( sock, IPPROTO_TCP, TCP_NODELAY, (void *)&val, sizeof(val) );
	if ( ::connect( sock, aip->ai_addr, aip->ai_addrlen ) < 0 ){
	  mess = string( "ClientSocket: Connection on ") + hostString + ":"
	    + TiCC::toString(sock) + " failed (" + strerror(errno) + ")";
	  close( sock );
	  sock = -1;
	}
      }
      freeaddrinfo( res ); // and delete all addr_info stuff
    }
    return isValid();
  }

  bool ServerSocket::connect( const string& port ){
    /// connect the Server to an port
    /*!
      \param port the number of the port of the server
      \return true on success, false otherwise
    */
    sock = -1;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_flags    = AI_PASSIVE;
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo *res;
    int status = getaddrinfo( 0, port.c_str(), &hints, &res);
    if ( status != 0) {
      mess = string("getaddrinfo error:: [") + gai_strerror(status) + "]";
    }
    else {
      struct addrinfo *resSave = res;
      // try to start up server
      //
      while ( res ){
	sock = socket( res->ai_family, res->ai_socktype, res->ai_protocol );
	if ( sock >= 0 ){
	  int val = 1;
	  if ( setsockopt( sock, SOL_SOCKET, SO_REUSEADDR,
			   (void *)&val, sizeof(val) ) == 0 ){
	    val = 1;
	    if ( setsockopt( sock, IPPROTO_TCP, TCP_NODELAY,
			     (void *)&val, sizeof(val) ) == 0 ){
	      if ( ::bind( sock, res->ai_addr, res->ai_addrlen ) == 0 ){
		break;
	      }
	    }
	  }
	  mess = strerror( errno );
	  sock = -1;
	}
	res = res->ai_next;
      }
      freeaddrinfo( resSave );
    }
    return isValid();
  }

  bool ServerSocket::accept( ClientSocket& newSocket ){
    /// accept a connection on a socket
    /*!
      \param newSocket the socket to connect to
      \return true on success, false otherwise
    */
    newSocket.sock = -1;
    struct sockaddr_storage cli_addr;
    socklen_t clilen = sizeof(cli_addr);
    int newsock = ::accept( sock, (struct sockaddr *)&cli_addr, &clilen );
    if ( newsock < 0 ){
      if ( errno == EINTR ){
	mess = string("server-accept interrupted." );
      }
      else {
	mess = string("server-accept failed: (") + strerror(errno) + ")";
      }
      return false;
    }
    else {
      char host_name[NI_MAXHOST];
      int err = getnameinfo( (struct sockaddr *)&cli_addr,
			     clilen,
			     host_name, sizeof(host_name),
			     0, 0,
			     0 );
      string name;
      if ( err != 0 ){
	name = string(" failed: getnameinfo ") + strerror(errno);
      }
      else {
	name = host_name;
      }
      err = getnameinfo( (struct sockaddr *)&cli_addr,
			 clilen,
			 host_name, sizeof(host_name),
			 0, 0,
			 NI_NUMERICHOST );
      if ( err == 0 ){
	name += string(" [") + host_name + "]";
      }
      newSocket.sock = newsock;
      newSocket.clientName = name;
    }
    return newSocket.isValid();
  }

#else

  /// Converts ascii text to in_addr struct.
  /// NULL is returned if the address can not be found.
  struct in_addr *atoaddr( const string& address ){
    struct hostent *host;
    static struct in_addr saddr;

    /* First try it as aaa.bbb.ccc.ddd. */
    const char *add = address.c_str();
    saddr.s_addr = inet_addr(add);
    if (saddr.s_addr != (unsigned int)-1) {
      return &saddr;
    }
    host = gethostbyname(add);
    if (host != NULL) {
      return (struct in_addr *) *host->h_addr_list;
    }
    return NULL;
  }

  bool ClientSocket::connect( const string& hostString,
			      const string& portString ){
    /// connect a Client to an external server
    /*!
      \param hostString the name of the server to use
      \param portString the number of the port of the server
      \return true on success, false otherwise
    */
    int port = TiCC::stringTo<int>( portString );
    if (port == -1) {
      mess = "ClientSocket connect: invalid port number";
      return false;
    }
    struct in_addr *addr = atoaddr( hostString );
    if (addr == NULL) {
      mess = "ClientSocket connect:  Invalid host.";
      return false;
    }

    struct sockaddr_in address;
    memset((char *) &address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = addr->s_addr;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if ( sock < 0 ){
      mess = "ClientSocket connect: socket failed";
    }
    else {
      int val = 1;
      setsockopt( sock, SOL_SOCKET, SO_REUSEADDR, (void *)&val, sizeof(val) );
      val = 1;
      setsockopt( sock, IPPROTO_TCP, TCP_NODELAY, (void *)&val, sizeof(val) );
      int connected = ::connect( sock, (struct sockaddr *) &address,
				 sizeof(address));
      if (connected < 0) {
	mess = string( "ClientSocket connect: ") + host + ":" + portNum +
	  " failed (" + strerror( errno ) + ")";
      }
    }
    return isValid();
  }

  bool ServerSocket::connect( const string& port ){
    /// connect the Server to an port
    /*!
      \param port the number of the port of the server
      \return true on success, false otherwise
    */
    sock = -1;
    sock = socket( AF_INET, SOCK_STREAM, 0 );
    if ( sock < 0 ){
      mess = string("ServerSocket connect: socket failed (" )
	+ strerror( errno ) + ")";
    }
    else {
      int val = 1;
      setsockopt( sock, SOL_SOCKET, SO_REUSEADDR,
		  (void *)&val, sizeof(val) );
      val = 1;
      setsockopt( sock, IPPROTO_TCP, TCP_NODELAY,
		  (void *)&val, sizeof(val) );
      struct sockaddr_in serv_addr;
      memset((char *) &serv_addr, 0, sizeof(serv_addr));
      serv_addr.sin_family = AF_INET;
      serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
      int TCP_PORT = TiCC::stringTo<int>(port);
      serv_addr.sin_port = htons(TCP_PORT);
      if ( bind( sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr) ) < 0 ){
	mess = string( "ServerSocket connect: bind failed (" )
	  + strerror( errno ) + ")";
      }
    }
    return isValid();
  }

  bool ServerSocket::accept( ServerSocket& newSocket ){
    /// accept a connection on a socket
    /*!
      \param newSocket the socket to connect to
      \return true on success, false otherwise
    */
    newSocket.sock = -1;
    struct sockaddr_storage cli_addr;
    socklen_t clilen = sizeof(cli_addr);
    int newsock = ::accept( sock, (struct sockaddr *)&cli_addr, &clilen );
    if ( newsock < 0 ){
      if ( errno == EINTR ){
	mess = string("server-accept interrupted." );
      }
      else {
	mess = string("server-accept failed: (") + strerror(errno) + ")";
      }
      return false;
    }
    else {
      string clientname;
      struct sockaddr_in rem;
      socklen_t remlen = sizeof(rem);
      if ( getpeername( newsock, (struct sockaddr *)&rem, &remlen ) >= 0 ){
	struct hostent *host = gethostbyaddr( (char *)&rem.sin_addr,
					      sizeof rem.sin_addr,
					      AF_INET );
	if ( host ){
	  clientname = host->h_name;
	  char **p;
	  for (p = host->h_addr_list; *p != 0; p++) {
	    struct in_addr in;
	    (void) memcpy(&in.s_addr, *p, sizeof (in.s_addr));
	    clientname += string(" [") + inet_ntoa(in) + "]";
	  }
	}
      }
      newSocket.clientName = clientname;
      newSocket.sock = newsock;
    }
    return newSocket.isValid();
  }

#endif

  bool ServerSocket::listen( unsigned int num ){
    /// start listening on a socket
    /*!
      \param num the maximum connections we accept
      \return true on succes, false otherwise
    */
    if ( ::listen( sock, num) < 0 ) {
      // maximum of num pending requests
      mess = string("server-listen failed: (") + strerror(errno) + ")";
      return false;
    }
    else
      return true;
  }

}
