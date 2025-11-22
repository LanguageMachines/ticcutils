/*
  Copyright (c) 2006 - 2025
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

#include "ticcutils/ServerBase.h"

#ifndef HAVE_DAEMON
#include <fcntl.h> // for implementing daemon
#endif
#include <pthread.h>
#include <unistd.h>
#include <stdint.h>
#include <cstdlib>
#include <cerrno>
#include <csignal>
#include <cstring>
#include <string>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include "ticcutils/Configuration.h"
#include "ticcutils/CommandLine.h"
#include "ticcutils/Timer.h"
#include "ticcutils/StringOps.h"
#include "config.h"
#include "ticcutils/FdStream.h"

using namespace std;
using namespace TiCC;

#define LOG *Log(_my_log)

namespace TiCCServer {

  const string serv_short_opts = "S:C:c:";
  const string serv_long_opts = ",pidfile:,logfile:,daemonize:,debug:,config:,protocol:";
    // leave the , in front of this line!. This string is appended later.

  string Version() { return VERSION; }
  string VersionName() { return PACKAGE_STRING; }

  childArgs::childArgs( ServerBase *server, Sockets::ClientSocket *sock ):
    _mother(server),_socket(sock){
    /// create a childArgs structure
    /*!
      \param server our Server object
      \param sock the Socket object

      This fuction opens an input and an output stream connected to the socket.
    */
    _id = _socket->getSockId();
    _is.open(_id);
    _os.open(_id);
  }

  childArgs::~childArgs( ){
    /// destroy the childArgs object
    _os.flush();
    delete _socket;
  }

  string ServerBase::VersionInfo( bool full ){
    string result;
    ostringstream oss;
    oss << VERSION;
    if ( full ){
      oss << ", compiled on " << __DATE__ << ", " << __TIME__;
    }
    result = oss.str();
    return result;
  }

  ServerBase::ServerBase( const Configuration *config,
			  void *callback_data ):
    _my_log(),
    _do_daemon( true ),
    _debug( false ),
    _max_conn( 25 ),
    _server_port( 7000 ),
    _callback_data( callback_data ),
    _protocol( "tcp" ),
    _config(config)
  {
    /// create a Basic Server
    /*!
      \param config the configuration informatio to use
      \param callback_data a structure with data to use in every call
    */
    string value = _config->lookUp( "port" );
    if ( !value.empty() ){
      if ( !stringTo( value, _server_port ) ){
	string mess = "ServerBase: invalid value '" + value + "' for port";
	throw runtime_error( mess );
      }
    }
    else {
      string mess = "ServerBase:missing 'port' in config ";
      throw runtime_error( mess );
    }
    value = _config->lookUp( "maxconn" );
    if ( !value.empty() ){
      if ( !stringTo( value, _max_conn ) ){
	string mess = "ServerBase: invalid value '" + value + "' for maxconn";
	throw runtime_error( mess );
      }
    }
    value = _config->lookUp( "protocol" );
    if ( !value.empty() ){
      _protocol = value;
    }
    value = _config->lookUp( "daemonize" );
    if ( !value. empty() ){
      if ( value == "no" ){
	_do_daemon = false;
      }
      else if ( value == "yes" ){
	_do_daemon = true;
      }
      else {
	string mess = "ServerBase: invalid value '" + value
	  + "' for --daemonize";
	throw runtime_error( mess );
      }
    }
    value = _config->lookUp( "logfile" );
    if ( !value.empty() ){
      _log_file = value;
    }
    value = _config->lookUp( "pidfile" );
    if ( !value.empty() ){
      _pid_file = value;
    }
    value = _config->lookUp( "name" );
    if ( !value.empty() ){
      _name = value;
    }
    else {
      _name = _protocol + "-server";
    }
    _my_log.set_message( _name );
    value = _config->lookUp( "debug" );
    if ( !value.empty() ){
      if ( value == "no" ){
	_debug = false;
      }
      else if ( value == "yes" ){
	_debug = true;
      }
      else {
	string mess = "ServerBase: invalid value '" + value
	  + "' for --debug; use 'yes' or 'no'";
	throw runtime_error( mess );
      }
    }
    _socket = 0;
  }

  void ServerBase::server_usage(void) {
    /// display helpful information
    cerr << "Server options" << endl;
    cerr << "--config=<f> or -c <f> : read server settings from file <f>" << endl;
    cerr << "--pidfile=<f> : store pid in file <f>" << endl;
    cerr << "--logfile=<f> : log server activity in file <f>" << endl;
    cerr << "--daemonize=[yes|no] (default yes)" << endl;
    cerr << "--protocol=[tcp|http|json] (default tcp)" << endl << endl;
    cerr << "OR, without config file:" << endl;
    cerr << "-S <port> : run as a server on <port>" << endl;
    cerr << "-C <num>  : accept a maximum of 'num' parallel connections (default 10)" << endl;
  }

  Configuration *initServerConfig( TiCC::CL_Options& opts ){
    /// initialize a Configuration from command-line options
    /*!
      \param opts The command line options
      \return the created Configuration
    */
    Configuration *config = new Configuration();
    bool old = false;
    string value;
    if ( !opts.extract( "config", value )
	 && !opts.extract( 'c', value ) ){
      if ( opts.extract( 'S', value ) ){
	config->setatt( "port", value );
	old = true;
	if ( opts.extract( 'C', value ) ){
	  config->setatt( "maxconn", value );
	}
      }
      if ( !old ){
	cerr << "missing '--config' or '-c' option" << endl;
	delete config;
	return 0;
      }
    }
    else if ( !config->fill( value ) ){
      cerr << "unable to read a configuration from " << value << endl;
      delete config;
      return 0;
    }
    if ( opts.extract( "pidfile", value ) ){
      config->setatt( "pidfile", value );
    }
    if ( opts.extract( "logfile", value ) ){
      config->setatt( "logfile", value );
    }
    if ( opts.extract( "daemonize", value ) ){
      if ( value.empty() ){
	value = "true";
      }
      config->setatt( "daemonize", value );
    }
    if ( opts.extract( "debug", value ) ){
      config->setatt( "debug", value );
    }
    if ( opts.extract( "protocol", value ) ){
      config->setatt( "protocol", value );
    }
    if ( old ){
      string rest = opts.toString();
      config->setatt( "default", rest );
    }
    return config;
  }

  void *ServerBase::callChild( void *a ) {
    /// generic callback function
    /// pass the argument as a childArgs struct to the Server
    childArgs* ca = reinterpret_cast<childArgs*>(a);
    ca->mother()->socketChild( ca );
    return 0;
  }

  volatile static bool keepGoing = true;

  void KillServerFun( int Signal ){
    /// function to handle SIGTERM signals
    if ( Signal == SIGTERM ){
      cerr << "KillServerFun caught a signal SIGTERM" << endl;
      keepGoing = false; // so stop accepting new connections
      // need a better plan here.
      sleep(10); // give children some spare time...
    }
  }

  void AfterDaemonFun( int Signal ){
    /// function to handle SIGCHILD signals
    cerr << "AfterDaemonFun caught a signal " << Signal << endl;
    if ( Signal == SIGCHLD ){
      exit(1);
    }
  }

  void BrokenPipeChildFun( int Signal ){
    /// function to handle SIGPIPE signals
    cerr << "BrokenPipeChildFun caught a signal " << Signal << endl;
    if ( Signal == SIGPIPE ){
      signal( SIGPIPE, BrokenPipeChildFun );
    }
  }

  /// run the Server as a daemon
  /*!
    \param noCD if ≠ 0, don't cd to the root dir
    \param noClose if ≠ 0 don't close stdin and stdout
    \return
  */
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
    /// send message that we are too busy
    os << "Maximum connections exceeded." << endl;
    os << "try again later..." << endl;
  }

  void ServerBase::socketChild( childArgs *args ){
    /// run a child in a new thread
    /*!
      \param args the arguments to use.

      this function calls callback() and will run until that function
      returns.
    */
    signal( SIGPIPE, BrokenPipeChildFun );
    LOG << "Thread " << (uintptr_t)pthread_self() << " on socket "
	<< args->id() << ", started at: "
	<< Timer::now() << endl;
    static int service_count=0;
    static pthread_mutex_t my_lock = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_lock(&my_lock);
    // use a mutex to update the global service counter
    if ( service_count >= maxConn() ){
      sendReject( args->os() );
      pthread_mutex_unlock( &my_lock );
      LOG << "Thread " << (uintptr_t)pthread_self()
	  << " refused " << endl;
    }
    else {
      ++service_count;
      pthread_mutex_unlock( &my_lock );
      callback( args );
      pthread_mutex_lock(&my_lock);
      // use a mutex to update and display the global service counter
      LOG << "Socket total = " << --service_count << endl;
      pthread_mutex_unlock(&my_lock);
    }
    // close the socket and exit this thread
    LOG << "Thread " << (uintptr_t)pthread_self()
	<< ", terminated at: " << Timer::now() << endl;
    delete args;
  }

  void HttpServerBase::sendReject( ostream& os ) const {
    /// send HTTP message that we are too busy
    os << "Status:503 Maximum number of connections exceeded.\n" << endl;
  }

  // ***** This is the routine that is executed from a new HTTP thread *******
  void HttpServerBase::socketChild( childArgs *args ){
    /// run a HTTP child in a new thread
    /*!
      \param args the arguments to use.

      this function sets the connected streams to Non-Blocking and then calls
      serverBase::socketChild
    */
    args->socket()->setNonBlocking();
    ServerBase::socketChild( args );
  }

  int ServerBase::Run(){
    /// run a Server. Must be configured before.
    LOG << "Starting a " << _protocol
	<< " server on port " << _server_port << endl;
    if ( !_pid_file.empty() ){
      // check validity of pidfile
      if ( _do_daemon && _pid_file[0] != '/' ) {
	// make sure the path is absolute
	_pid_file = '/' + _pid_file;
      }
      unlink( _pid_file.c_str() );
      ofstream pid_file( _pid_file );
      if ( !pid_file ){
	LOG<< "unable to create pidfile:"<< _pid_file << endl;
	LOG<< "not Started" << endl;
	return EXIT_FAILURE;
      }
    }
    ostream *logS = 0;
    if ( !_log_file.empty() ){
      if ( _do_daemon && _log_file[0] != '/' ) {
	// make sure the path is absolute
	_log_file = '/' + _log_file;
      }
      logS = new ofstream( _log_file );
      if ( logS && logS->good() ){
	LOG << "switching logging to file " << _log_file << endl;
	_my_log.associate( *logS );
	LOG  << "Started logging " << endl;
	LOG  << "debugging is " << (doDebug()?"on":"off") << endl;
      }
      else {
	delete logS;
	LOG << "unable to create logfile: " << _log_file << endl;
	LOG << "not started" << endl;
	return EXIT_FAILURE;
      }
    }

    int start = 1;
    if ( _do_daemon ){
      LOG << "running as a dæmon" << endl;
      signal( SIGCHLD, AfterDaemonFun );
      start = daemonize( 0, _log_file.empty() );
    }
    if ( start < 0 ){
      cerr << "failed to daemonize error= " << strerror(errno) << endl;
      return EXIT_FAILURE;
    };
    if ( !_pid_file.empty() ){
      // we have a liftoff!
      // signal it to the world
      ofstream pid_file( _pid_file ) ;
      if ( !pid_file ){
	LOG << "unable to create pidfile:"<< _pid_file << endl;
	LOG << "server NOT Started" << endl;
	return EXIT_FAILURE;
      }
      else {
	pid_t pid = getpid();
	pid_file << pid << endl;
	LOG << "wrote PID=" << pid << " to " << _pid_file << endl;
      }
    }
    // set the attributes
    pthread_attr_t attr;
    if ( pthread_attr_init(&attr) ||
	 pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_DETACHED ) ){
      LOG << "Threads: couldn't set attributes" << endl;
      return EXIT_FAILURE;
    }
    LOG << "Now running a " << _protocol
	<< " server on port " << _server_port << endl;

    pthread_t chld_thr;

    Sockets::ServerSocket server;
    string portString = toString<int>(_server_port);
    if ( !server.connect( portString ) ){
      LOG << "failed to start Server: " << server.getMessage() << endl;
      return EXIT_FAILURE;
    }

    if ( !server.listen( 5 ) ) {
      // maximum of 5 pending requests
      LOG << server.getMessage() << endl;
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
      Sockets::ClientSocket *newSocket = new Sockets::ClientSocket();
      if ( !server.accept( *newSocket ) ){
	cerr << "accept failed: " + server.getMessage() << endl;
	delete newSocket;
	LOG << server.getMessage() << endl;
	if ( ++failcount > 20 ){
	  LOG << "accept failcount > 20 " << endl;
	  LOG << "server stopped." << endl;
	  return EXIT_FAILURE;
	}
	else {
	  continue;
	}
      }
      else {
	if ( !keepGoing ) break;
	failcount = 0;
	LOG << "Accepting Connection #"
	    << newSocket->getSockId()
	    << " from remote host: "
	    << newSocket->getClientName() << endl;
	// create a new thread to process the incoming request
	// (The thread will terminate itself when done processing
	// and release its socket handle)
	//
	childArgs *args = new childArgs( this, newSocket );
	pthread_create( &chld_thr, &attr, callChild, static_cast<void *>(args) );
      }
      // the server is now free to accept another socket request
    }
    // some cleanup
    pthread_attr_destroy(&attr);
    delete logS;
    return EXIT_SUCCESS;
  }

}
