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

#ifndef SERVERBASE_H
#define SERVERBASE_H

#include <iosfwd>
#include <string>
#include "ticcutils/LogStream.h"
#include "ticcutils/Configuration.h"
#include "ticcutils/SocketBasics.h"
#include "ticcutils/FdStream.h"

namespace TiCC { class CL_Options; }
namespace TiCCServer {

  extern const std::string serv_short_opts;
  extern const std::string serv_long_opts;

  class childArgs;

  /// \brief ServerBase provides functions to setup a Server in a generic
  /// way
  ///
  /// ServerBase serves as a baseclass for more specialized servers for TCP
  /// HTTP connections
  class ServerBase {
  private:
    ServerBase( const ServerBase& ) = delete; // no copies allowed
    ServerBase& operator=( const ServerBase& ) = delete;  // no copies allowed
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

  /// \brief childArgs carries important data for Server connections
  ///
  /// this is passed using a callback function to every new Socket connection
  /// the Server creates
  class childArgs {
  public:
    childArgs( ServerBase *, Sockets::ClientSocket * );
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
    Sockets::ClientSocket *socket() const {
      /*!
	\return the ClientSocket we belong to
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
    Sockets::ClientSocket *_socket;
    int _id;
    fdistream _is;
    fdostream _os;
    childArgs( const childArgs& ) = delete; // no copies allowed
    childArgs& operator=( const childArgs& ) = delete; // no copies allowed
  };

  /// \brief TcpServerBase is a baseclass for TCP connections
  class TcpServerBase : public ServerBase {
  public:
    explicit TcpServerBase( const TiCC::Configuration *c,
			    void *cb ):ServerBase( c, cb ){};
  };

  /// \brief HttpServerBase is a baseclass for Http connections
  class HttpServerBase : public ServerBase {
  public:
    void socketChild( childArgs * ) override;
    virtual void sendReject( std::ostream& os ) const override;
    explicit HttpServerBase( const TiCC::Configuration *c,
			     void *cb ): ServerBase( c, cb ){};
  };

  std::string Version();
  std::string VersionName();

  inline int daemonize( int noCD , int noClose ){
    /// wrapper to expose the daemonize function to std::
    return ServerBase::daemonize( noCD, noClose);
  }

  TiCC::Configuration *initServerConfig( TiCC::CL_Options& );

}
#endif // SERVERBASE_H
