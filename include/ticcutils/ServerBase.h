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

#ifndef SERVERBASE_H
#define SERVERBASE_H

#include "ticcutils/LogStream.h"
#include "ticcutils/Configuration.h"
#include "ticcutils/CommandLine.h"
#include "ticcutils/SocketBasics.h"
#include "ticcutils/FdStream.h"

namespace TimblServer {

  extern const std::string serv_short_opts;
  extern const std::string serv_long_opts;

  class childArgs;

  class ServerBase {
  private:
    ServerBase( const ServerBase& ); // no copies allowed
    ServerBase& operator=( const ServerBase& );  // no copies allowed
  public:
    explicit ServerBase( const TiCC::Configuration *, void * );
    virtual ~ServerBase(){ delete _socket; delete _config; };
    bool doDebug() {
      /*!
	\return true of debugging is on
      */
      return _debug;
    };
    static void server_usage();
    static std::string VersionInfo( bool );
    static int daemonize( int , int );
    int maxConn() const {
      /*!
	\return the maximum simultaneous connections allowed
      */
      return _max_conn;
    };
    void setDebug( bool d ){ _debug = d; };
    Sockets::ServerSocket *TcpSocket() const {
      /*!
	\return the ServerSocket
      */
      return _socket;
    };
    static void *callChild( void * );
    void *callback_data() const {
      /*!
	\return the callback_data structure
      */
      return _callback_data;
    };
    int Run();
    TiCC::LogStream& logstream() {
      /*!
	\return the current LogStream
      */
      return _my_log;
    }
    const TiCC::Configuration *config() const {
      /*!
	\return the current Configuration
      */
      return _config;
    };
    virtual void socketChild( childArgs * );
    virtual void callback( childArgs* ) = 0;
    virtual void sendReject( std::ostream& ) const;

  protected:
    TiCC::LogStream _my_log;
    std::string _log_file;
    std::string _pid_file;
    std::string _name;
    bool _do_daemon;
    bool _debug;
    int _max_conn;
    int _server_port;
    void *_callback_data;
    Sockets::ServerSocket *_socket;
    std::string _protocol;
    std::string _config_file;
    const TiCC::Configuration *_config;
  };

  class childArgs {
  public:
    childArgs( ServerBase *, Sockets::ServerSocket * );
    ~childArgs();
    int id() const {
      /*!
	\return the id of these data
      */
      return _id;
    };
    std::ostream& os() {
      /*!
	\return the connected output stream
      */
      return _os;
    };
    std::istream& is() {
      /*!
	\return the connected input stream
      */
      return _is;
    };
    ServerBase *mother() const {
      /*!
	\return the ServerBase object we belong to
      */
      return _mother;
    };
    TiCC::LogStream& logstream() {
      /*!
	\return the LogStream of the Serverbase
      */
      return _mother->logstream();
    }
    Sockets::ServerSocket *socket() const {
      /*!
	\return the ServerSocket we are connected to
      */
      return _socket;
    };
    bool debug() const {
      /*!
	\return the debug status of our ServerBase
      */
      return _mother->doDebug();
    };
  private:
    ServerBase *_mother;
    Sockets::ServerSocket *_socket;
    int _id;
    fdistream _is;
    fdostream _os;
    childArgs( const childArgs& ); // no copies allowed
    childArgs& operator=( const childArgs& ); // no copies allowed
  };

  class TcpServerBase : public ServerBase {
  public:
    explicit TcpServerBase( const TiCC::Configuration *c,
			    void *cb ):ServerBase( c, cb ){};
  };

  class HttpServerBase : public ServerBase {
  public:
    void socketChild( childArgs * );
    virtual void sendReject( std::ostream& os ) const;
    explicit HttpServerBase( const TiCC::Configuration *c,
			     void *cb ): ServerBase( c, cb ){};
  };

  std::string Version();
  std::string VersionName();

  inline int daemonize( int noCD , int noClose ){
    return ServerBase::daemonize( noCD, noClose);
  }

  TiCC::Configuration *initServerConfig( TiCC::CL_Options& );

}
#endif // SERVERBASE_H
