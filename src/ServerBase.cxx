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

#ifndef HAVE_DAEMON
#include <fcntl.h> // for implementing daemon
#endif
#include <string>
#include <cerrno>
#include <csignal>
#include <fstream>
#include "ticcutils/Configuration.h"
#include "ticcutils/Timer.h"
#include "ticcutils/StringOps.h"
#include "config.h"
#include "ticcutils/FdStream.h"
#include "ticcutils/ServerBase.h"

using namespace std;
using namespace TiCC;

namespace TimblServer {

  const string serv_short_opts = "S:C:";
  const string serv_long_opts =
    // leave the , below!. This string is appended later
    ",pidfile:,logfile:,daemonize::,debug:,config:";

  string Version() { return VERSION; }
  string VersionName() { return PACKAGE_STRING; }

  childArgs::childArgs( ServerBase *server, Sockets::ServerSocket *sock ):
    _mother(server),_socket(sock){
    _id = _socket->getSockId();
    _is.open(_id);
    _os.open(_id);
  }

  childArgs::~childArgs( ){
    _os.flush();
    delete _socket;
  }

  string ServerBase::VersionInfo( bool full ){
    string result;
    ostringstream oss;
    oss << VERSION;
    if ( full )
      oss << ", compiled on " << __DATE__ << ", " << __TIME__;
    result = oss.str();
    return result;
  }

  ServerBase::ServerBase( const Configuration *c ):
    myLog("BasicServer"),
    doDaemon( true ),
    debug( false ),
    _maxConn( 25 ),
    serverPort( 7000 ),
    callback_data( 0 ),
    config(c)
  {
    string value = config->lookUp( "port" );
    if ( !value.empty() ){
      if ( !stringTo( value, serverPort ) ){
	cerr << "config:invalid value '" << value << "' for port" << endl;
	exit(1);
      }
    }
    else {
      cerr << "missing 'port' in config " << endl;
      exit(1);
    }
    value = config->lookUp( "maxconn" );
    if ( !value.empty() ){
      if ( !stringTo( value, _maxConn ) ){
	cerr << "config: invalid value '" << value << "' for maxconn" << endl;
	exit(1);
      }
    }
    value = config->lookUp( "protocol" );
    if ( !value.empty() ){
      serverProtocol = value;
    }
    else {
      serverProtocol = "tcp";
    }
    value = config->lookUp( "daemonize" );
    if ( !value. empty() ){
      if ( value == "no" )
	doDaemon = false;
      else if ( value == "yes" )
	doDaemon = true;
      else {
	cerr << "config: invalid value '" << value << "' for --daemonize" << endl;
	exit(1);
      }
    }
    value = config->lookUp( "logfile" );
    if ( !value.empty() )
      logFile = value;
    value = config->lookUp( "pidfile" );
    if ( !value.empty() )
      pidFile = value;
    value = config->lookUp( "name" );
    if ( !value.empty() )
      name = value;
    else
      name = serverProtocol + "-server";
    myLog.message( name );
    value = config->lookUp( "debug" );
    if ( !value.empty() ){
      if ( value == "no" )
	debug = false;
      else if ( value == "yes" )
	debug = true;
      else {
	cerr << "config: invalid value '" << value << "' for --debug" << endl;
	exit(1);
      }
    }
    tcp_socket = 0;
  }

  Configuration *initServerConfig( TiCC::CL_Options& opts ){
    Configuration *config = new Configuration();
    bool old = false;
    string value;
    if ( !opts.extract( "config", value ) ){
      if ( opts.extract( 'S', value ) ){
	config->setatt( "port", value );
	old = true;
	if ( opts.extract( 'C', value ) ){
	  config->setatt( "maxconn", value );
	}
      }
      if ( !old ){
	cerr << "missing --config option" << endl;
	return 0;
      }
    }
    else if ( !config->fill( value ) ){
      cerr << "unable to read a configuration from " << value << endl;
      return 0;
    }
    if ( opts.extract( "pidfile", value ) ){
      config->setatt( "pidfile", value );
    }
    if ( opts.extract( "logfile", value ) ){
      config->setatt( "logfile", value );
    }
    if ( opts.extract( "daemonize", value ) ){
      if ( value.empty() )
	value = "true";
      config->setatt( "daemonize", value );
    }
    if ( opts.extract( "debug", value ) ){
      config->setatt( "debug", value );
    }
    if ( old ){
      string rest = opts.toString();
      config->setatt( "default", rest );
    }
    return config;
  }

  string getProtocol( const string& serverConfigFile ){
    string result = "tcp";
    ifstream is( serverConfigFile );
    if ( !is ){
      cerr << "problem reading " << serverConfigFile << endl;
      return result;
    }
    else {
      string line;
      while ( getline( is, line ) ){
	if ( line.empty() || line[0] == '#' )
	  continue;
	string::size_type ispos = line.find('=');
	if ( ispos == string::npos ){
	  cerr << "invalid entry in: " << serverConfigFile
	       << "offending line: '" << line << "'" << endl;
	  return result;
	}
	else {
	  string base = line.substr(0,ispos);
	  string rest = line.substr( ispos+1 );
	  if ( !rest.empty() ){
	    string tmp = base;
	    lowercase(tmp);
	    if ( tmp == "protocol" ){
	      string protocol = rest;
	      lowercase( protocol );
	      if ( protocol != "http" && protocol != "tcp" ){
		cerr << "invalid protocol: " << protocol << endl;
		return result;
	      }
	      return protocol;
	    }
	  }
	}
      }
    }
    return result;
  }

  void *ServerBase::callChild( void *a ) {
    childArgs* ca = (childArgs*)a;
    ca->mother()->socketChild( ca );
    return 0;
  }

  static bool keepGoing = true;

  void KillServerFun( int Signal ){
    if ( Signal == SIGTERM ){
      cerr << "KillServerFun caught a signal SIGTERM" << endl;
      keepGoing = false; // so stop accepting new connections
      // need a better plan here.
      sleep(10); // give children some spare time...
    }
  }

  void AfterDaemonFun( int Signal ){
    cerr << "AfterDaemonFun caught a signal " << Signal << endl;
    if ( Signal == SIGCHLD ){
      exit(1);
    }
  }

  void BrokenPipeChildFun( int Signal ){
    cerr << "BrokenPipeChildFun caught a signal " << Signal << endl;
    if ( Signal == SIGPIPE ){
      signal( SIGPIPE, BrokenPipeChildFun );
    }
  }


#ifdef HAVE_DAEMON
  int ServerBase::daemonize( int noCD , int noClose ){
    return daemon( noCD, noClose );
  }
#else

  int ServerBase::daemonize( int noCD , int noClose ){
    switch (fork()) {
    case -1:
      /* error */
      cerr << "daemon fork failed: " << strerror(errno) << endl;
      return -1;
      break;
    case 0:
      /* child */
      break;
    default:
      /* parent */
      exit(0);
    }
    //
    // a good child
    //
    if (setsid() == -1) {
      cerr << "daemon setting session id for daemon failed: "
	   << strerror(errno) << endl;
      return -1;
    }

    if ( !noCD ){
      if ( chdir("/") < 0 ){
	cerr << "daemon cd failed: " << strerror(errno) << endl;
	return -1;
      }
    }
    if ( !noClose ){
      close (0);
      close (1);
      close (2);
      //
      // Set up the standard file descriptors.
      //
      (void) open ("/dev/null", O_RDWR);
      (void) dup2 (0, 1);
      (void) dup2 (0, 2);
    }
    return 0;
  }
#endif // HAVE_DAEMON

  void ServerBase::sendReject( ostream& os ) const {
    os << "Maximum connections exceeded." << endl;
    os << "try again later..." << endl;
  }

  // ***** This is the routine that is executed from a new TCP thread *******
  void ServerBase::socketChild( childArgs *args ){
    signal( SIGPIPE, BrokenPipeChildFun );
    *Log(myLog) << "Thread " << (uintptr_t)pthread_self() << " on socket "
		<< args->id() << ", started at: " << Timer::now();
    static int service_count=0;
    static pthread_mutex_t my_lock = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_lock(&my_lock);
    // use a mutex to update the global service counter
    if ( service_count >= maxConn() ){
      sendReject( args->os() );
      pthread_mutex_unlock( &my_lock );
      *Log(myLog) << "Thread " << (uintptr_t)pthread_self()
		  << " refused " << endl;
    }
    else {
      ++service_count;
      pthread_mutex_unlock( &my_lock );
      callback( args );
      pthread_mutex_lock(&my_lock);
      // use a mutex to update and display the global service counter
      *Log(myLog) << "Socket total = " << --service_count << endl;
      pthread_mutex_unlock(&my_lock);
    }
    // close the socket and exit this thread
    *Log(myLog) << "Thread " << (uintptr_t)pthread_self()
		<< ", terminated at: " << Timer::now();
    delete args;
  }

  void HttpServerBase::sendReject( ostream& os ) const {
    os << "Status:503 Maximum number of connections exceeded.\n" << endl;
  }

  // ***** This is the routine that is executed from a new HTTP thread *******
  void HttpServerBase::socketChild( childArgs *args ){
    args->socket()->setNonBlocking();
    ServerBase::socketChild( args );
  }

  int ServerBase::Run(){
    *Log(myLog) << "Starting a " << serverProtocol
		<< " server on port " << serverPort << endl;
    if ( !pidFile.empty() ){
      // check validity of pidfile
      if ( doDaemon && pidFile[0] != '/' ) // make sure the path is absolute
	pidFile = '/' + pidFile;
      unlink( pidFile.c_str() ) ;
      ofstream pid_file( pidFile ) ;
      if ( !pid_file ){
	*Log(myLog)<< "unable to create pidfile:"<< pidFile << endl;
	*Log(myLog)<< "not Started" << endl;
	return EXIT_FAILURE;
      }
    }
    ostream *logS = 0;
    if ( !logFile.empty() ){
      if ( doDaemon && logFile[0] != '/' ) // make sure the path is absolute
	logFile = '/' + logFile;
      logS = new ofstream( logFile );
      if ( logS && logS->good() ){
	*Log(myLog) << "switching logging to file " << logFile << endl;
	myLog.associate( *logS );
	*Log(myLog)  << "Started logging " << endl;
	*Log(myLog)  << "debugging is " << (doDebug()?"on":"off") << endl;
      }
      else {
	delete logS;
	*Log(myLog) << "unable to create logfile: " << logFile << endl;
	*Log(myLog) << "not started" << endl;
	return EXIT_FAILURE;
      }
    }

    int start = 1;
    if ( doDaemon ){
      *Log(myLog) << "running as a dæmon" << endl;
      signal( SIGCHLD, AfterDaemonFun );
      start = daemonize( 0, logFile.empty() );
    }
    if ( start < 0 ){
      cerr << "failed to daemonize error= " << strerror(errno) << endl;
      return EXIT_FAILURE;
    };
    if ( !pidFile.empty() ){
      // we have a liftoff!
      // signal it to the world
      ofstream pid_file( pidFile ) ;
      if ( !pid_file ){
	*Log(myLog) << "unable to create pidfile:"<< pidFile << endl;
	*Log(myLog) << "server NOT Started" << endl;
	return EXIT_FAILURE;
      }
      else {
	pid_t pid = getpid();
	pid_file << pid << endl;
	*Log(myLog) << "wrote PID=" << pid << " to " << pidFile << endl;
      }
    }
    // set the attributes
    pthread_attr_t attr;
    if ( pthread_attr_init(&attr) ||
	 pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_DETACHED ) ){
      *Log(myLog) << "Threads: couldn't set attributes" << endl;
      return EXIT_FAILURE;
    }
    *Log(myLog) << "Starting Server on port:" << serverPort << endl;

    pthread_t chld_thr;

    Sockets::ServerSocket server;
    string portString = toString<int>(serverPort);
    if ( !server.connect( portString ) ){
      *Log(myLog) << "failed to start Server: " << server.getMessage() << endl;
      return EXIT_FAILURE;
    }

    if ( !server.listen( 5 ) ) {
      // maximum of 5 pending requests
      *Log(myLog) << server.getMessage() << endl;
      return EXIT_FAILURE;
    }

    int failcount = 0;
    struct sigaction act;
    sigaction( SIGTERM, NULL, &act ); // get current action
    act.sa_handler = KillServerFun;
    act.sa_flags &= ~SA_RESTART;      // do not continue after SIGTERM
    sigaction( SIGTERM, &act, NULL );
    while( keepGoing ){ // waiting for connections loop
      signal( SIGPIPE, SIG_IGN );
      Sockets::ServerSocket *newSocket = new Sockets::ServerSocket();
      if ( !server.accept( *newSocket ) ){
	cerr << "accept failed: " + server.getMessage() << endl;
	delete newSocket;
	*Log(myLog) << server.getMessage() << endl;
	if ( ++failcount > 20 ){
	  *Log(myLog) << "accept failcount > 20 " << endl;
	  *Log(myLog) << "server stopped." << endl;
	  return EXIT_FAILURE;
	}
	else {
	  continue;
	}
      }
      else {
	if ( !keepGoing ) break;
	failcount = 0;
	*Log(myLog) << "Accepting Connection #"
		    << newSocket->getSockId()
		    << " from remote host: "
		    << newSocket->getClientName() << endl;
	// create a new thread to process the incoming request
	// (The thread will terminate itself when done processing
	// and release its socket handle)
	//
	childArgs *args = new childArgs( this, newSocket );
	pthread_create( &chld_thr, &attr, callChild, (void *)args );
      }
      // the server is now free to accept another socket request
    }
    // some cleanup
    pthread_attr_destroy(&attr);
    delete logS;
    return EXIT_SUCCESS;
  }

}
