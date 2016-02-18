/*
  Copyright (c) 2006 - 2016
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
    bool doDebug() { return debug; };
    virtual ~ServerBase(){};
    static std::string VersionInfo( bool );
    static int daemonize( int , int );
    int maxConn() const { return _maxConn; };
    ServerBase( const TiCC::Configuration * );
    void setDebug( bool d ){ debug = d; };
    Sockets::ServerSocket *TcpSocket() const { return tcp_socket; };
    static void *callChild( void * );
    int Run();
    virtual void socketChild( childArgs * );
    virtual void callback( childArgs* ) = 0;
    virtual void sendReject( std::ostream& os ) const;

    TiCC::LogStream myLog;
    std::string logFile;
    std::string pidFile;
    std::string name;
    bool doDaemon;
    bool debug;
    int _maxConn;
    int serverPort;
    void *callback_data;
    Sockets::ServerSocket *tcp_socket;
    std::string serverProtocol;
    std::string serverConfigFile;
    const TiCC::Configuration *config;
    std::map<std::string, std::string> serverConfig;
  };

  class childArgs {
  public:
    childArgs( ServerBase *, Sockets::ServerSocket * );
    ~childArgs();
    int id() const { return _id; };
    std::ostream& os() { return _os; };
    std::istream& is() { return _is; };
    ServerBase *mother() const { return _mother; };
    TiCC::LogStream& logstream() { return _mother->myLog; }
    Sockets::ServerSocket *socket() const { return _socket; };
    bool debug() const { return _mother->doDebug(); };
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
  TcpServerBase( const TiCC::Configuration *c ):ServerBase( c ){};
  };

  class HttpServerBase : public ServerBase {
  public:
    void socketChild( childArgs * );
    virtual void sendReject( std::ostream& os ) const;
  HttpServerBase( const TiCC::Configuration *c ): ServerBase( c ){};
  };

  std::string Version();
  std::string VersionName();

  inline int daemonize( int noCD , int noClose ){
    return ServerBase::daemonize( noCD, noClose);
  }

  TiCC::Configuration *initServerConfig( TiCC::CL_Options& );

}
#endif // SERVERBASE_H
