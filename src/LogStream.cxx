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

#include "ticcutils/LogStream.h"

#include <ctime>

#include <string>
#include <typeinfo>
#include <pthread.h>

#if defined __GNUC__
#define DARE_TO_OPTIMIZE
#endif

//#define LSDEBUG

using std::ostream;
using std::streambuf;
using std::cerr;
using std::endl;
using std::bad_cast;
using std::string;

namespace TiCC {
  LogStream::LogStream( int ) :
    ostream( static_cast<streambuf *>(0) ),
    buf( cerr ),
    single_threaded_mode(false){
    /// create a LogStream  with an empty streambuf connected to cerr
  }

  LogStream null_stream( 0 ); /// fallback LogStream to cerr

  LogStream::LogStream() :
    ostream( &buf ),
    buf( cerr, "", StampBoth ),
    single_threaded_mode(false) {
    /// create a LogStream connected to cerr
  }

  LogStream::LogStream( const string& message, LogFlag stamp ) :
    ostream( &buf ),
    buf( cerr, message, stamp ),
    single_threaded_mode(false) {
    /// create a LogStream connected to cerr
    /*!
      \param message the prefix message
      \param stamp the stamping flag to use
    */
  }

  LogStream::LogStream( ostream& as,
			const string& message,
			LogFlag stamp ) :
    ostream( &buf ),
    buf( as, message, stamp ),
    single_threaded_mode(false){
    /// create a LogStream connected to an output stream
    /*!
      \param as a stream to connect to
      \param message the prefix message
      \param stamp the stamping flag to use
    */
  }

  LogStream::LogStream( const LogStream& ls,
			const string& message,
			LogFlag stamp ):
    ostream( &buf ),
    buf( ls.buf.AssocStream(),
	 ls.buf.Message(),
	 stamp ),
    single_threaded_mode( ls.single_threaded_mode ){
    /// create a LogStream connected to a LogStream
    /*!
      \param ls a LogStream to connect to
      \param message the prefix message to append to the parents message
      \param stamp the stamping flag to use

      the new Stream takes all properties from the parent, except for the
      message and the stamp flag
    */
    buf.Level( ls.buf.Level() );
    buf.Threshold( ls.buf.Threshold() );
    add_message( message );
  }

  LogStream::LogStream( const LogStream& ls,
			const string& message ):
    ostream( &buf ),
    buf( ls.buf.AssocStream(),
	 ls.buf.Message(),
	 ls.buf.StampFlag() ),
    single_threaded_mode( ls.single_threaded_mode ){
    /// create a LogStream connected to a LogStream
    /*!
      \param ls a LogStream to connect to
      \param message the prefix message to append to the parents message

      the new Stream takes all properties from the parent, except for the
      message
    */
    buf.Level( ls.buf.Level() );
    buf.Threshold( ls.buf.Threshold() );
    add_message( message );
  }

  LogStream::LogStream( const LogStream *ls ):
    ostream( &buf ),
    buf( ls->buf.AssocStream(),
	 ls->buf.Message(),
	 ls->buf.StampFlag() ),
    single_threaded_mode( ls->single_threaded_mode ){
    /// create a LogStream connected to a LogStream
    /*!
      \param ls a LogStream to connect to
      the new Stream takes all properties from the parent
    */
    buf.Level( ls->buf.Level() );
    buf.Threshold( ls->buf.Threshold() );
  }

  void LogStream::add_message( const string& s ){
    /// append a string to the current messsage
    if ( !s.empty() ){
      string tmp = buf.Message();
      tmp += s;
      buf.Message( tmp );
    }
  }

  void LogStream::add_message( const int i ){
    /// append a number to the current messsage
    string m = "-" + std::to_string(i);
    add_message( m );
  }

  static bool static_init = false;

  bool LogStream::set_single_threaded_mode( ){
    /// set the LogStream to single threaded mode
    /*!
      \return true on succes
      only possible before any LogStream actions have been performed
    */
    if ( !static_init ){
      single_threaded_mode = true;
      return true;
    }
    else
      return false;
  }

  pthread_mutex_t global_logging_mutex = PTHREAD_MUTEX_INITIALIZER;
  pthread_mutex_t global_lock_mutex = PTHREAD_MUTEX_INITIALIZER;

  /// @cond HIDDEN
  /// a structure to store locking information per thread
  struct lock_s { pthread_t id; int cnt; time_t tim; };

#define MAX_LOCKS 500

  lock_s locks[MAX_LOCKS];
  /// @endcond

  bool LogStream::Problems(){
    /// perform a sanity check on the mutex locks
#ifdef LSDEBUG
    cerr << "test for problems" << endl;
#endif
    bool result = false;
    time_t lTime;
    time(&lTime);
    pthread_mutex_lock( &global_lock_mutex );
    for ( int i=0; i < MAX_LOCKS; i++ ){
      if ( locks[i].id != 0 &&
	   lTime - locks[i].tim > 30 ){
	result = true;
	cerr << "ALERT" << endl;
	cerr << "ALERT" << endl;
	cerr << "Thread " << locks[i].id
	     << "is blocking our LogStreams since " << lTime - locks[i].tim
	     << " seconds!" << endl;
	cerr << "ALERT" << endl;
	cerr << "ALERT" << endl;
      }
    }
    pthread_mutex_unlock( &global_lock_mutex );
    return result;
  }

  inline int get_lock( pthread_t ID ){
    /// get a thread specific lock
    /*!
      \param ID the thread Id
      \return the number of the lock. Will throw when no lock is available
    */
    time_t lTime;
    time(&lTime);
    pthread_mutex_lock( &global_lock_mutex );
    int free_lock = -1;
    for ( int i=0; i < MAX_LOCKS; i++ ){
      if ( pthread_equal( locks[i].id, ID ) ){
	pthread_mutex_unlock( &global_lock_mutex );
	return i;
      }
      else if ( free_lock < 0 && locks[i].id == 0 ){
        free_lock = i;
      }
    }
    if ( free_lock < 0 ){
      throw( "LogStreams FATAL error: get_lock() failed " );
    }
    locks[free_lock].id = ID;
    locks[free_lock].cnt = 0;
    locks[free_lock].tim = lTime;
    pthread_mutex_unlock( &global_lock_mutex );
    return free_lock;
  }

  inline bool init_mutex(){
    /// acquire the logging mutex
    if ( !static_init ){
      for (int i=0; i < MAX_LOCKS; i++ ) {
	locks[i].id = 0;
	locks[i].cnt = 0;
      }
      static_init = true;
#ifdef LSDEBUG
      cerr << "MUTEX system initialized!" << endl;
#endif
    }
#ifdef LSDEBUG
    cerr << "voor Lock door thread " << pthread_self() << endl;
#endif
    int pos = get_lock( pthread_self() );
    if ( locks[pos].cnt == 0 ){
      pthread_mutex_lock( &global_logging_mutex );
#ifdef LSDEBUG
      cerr << "Thread " << pthread_self()  << " locked [" << pos
	   << "]" << endl;
#endif
    }
    ++locks[pos].cnt;
#ifdef LSDEBUG
    if ( locks[pos].cnt > 1 ){
      cerr << "Thread " << pthread_self()  << " regained [" << pos
	   << "] cnt = " << locks[pos].cnt << endl;
    }
#endif
    return static_init;
  }

  inline void mutex_release(){
    /// release the logging mutex
#ifdef LSDEBUG
    cerr << "voor UnLock door thread " << pthread_self() << endl;
#endif
    int pos = get_lock( pthread_self() );
    locks[pos].cnt--;
    if ( locks[pos].cnt < 0 ){
      throw( "LogStreams FATAL error: mutex_release() failed" );
    }
#ifdef LSDEBUG
    if ( locks[pos].cnt > 0 ){
      cerr << "Thread " << pthread_self()  << " still owns [" << pos
	   << "] cnt = "<< locks[pos].cnt << endl;
    }
#endif
    if ( locks[pos].cnt == 0 ){
      pthread_mutex_lock( &global_lock_mutex );
      locks[pos].id = 0;
      pthread_mutex_unlock( &global_lock_mutex );
#ifdef LSDEBUG
      cerr << "Thread " << pthread_self()  << " unlocked [" << pos << "]" << endl;
#endif
      pthread_mutex_unlock( &global_logging_mutex );
    }
  }

  bool LogStream::IsBlocking(){
    /// is the current level below the threshold?
    if ( !bad() ){
#ifdef LSDEBUG
      cerr << "IsBlocking( " << get_level() << "," << get_threshold() << ")"
	   << " ==> " << ( get_level() < get_threshold() ) << endl;
#endif
      return get_level() < get_threshold();
    }
    else {
      return true;
    }
  }

  bool IsActive( LogStream &ls ){
    /// is the current level above the threshold, meaning this stream is active
    return !ls.IsBlocking();
  }

  bool IsActive( LogStream *ls ){
    /// is the current level above the threshold, meaning this stream is active
    return ls && !ls->IsBlocking();
  }


  Log::Log( LogStream *os ): my_stream(0), my_level(LogSilent){
    /// create a Log object on the LogStream with Normal threshold
    if ( !os ){
      throw( "LogStreams FATAL error: No Stream supplied! " );
    }
    if ( os->single_threaded() || init_mutex() ){
      my_level = os->get_threshold();
      my_stream = os;
      os->set_threshold( LogNormal );
    }
  }

  Log::Log( LogStream& os ):  my_stream(0), my_level(LogSilent){
    /// create a Log object on the LogStream with Normal threshold
    if ( os.single_threaded() || init_mutex() ){
      my_stream = &os;
      my_level = os.get_threshold();
      os.set_threshold( LogNormal );
    }
  }

  Log::~Log(){
    /// destroy the Log object
    my_stream->flush();
    my_stream->set_threshold( my_level );
    if ( !my_stream->single_threaded() ){
      mutex_release();
    }
  }

  LogStream& Log::operator *(){
    /// return the LogStream from the Log object
#ifdef DARE_TO_OPTIMIZE
    if ( my_stream->get_level() >= my_stream->get_threshold() ){
      return *my_stream;
    }
    else {
      return null_stream;
    }
#else
    return *my_stream;
#endif
  }

  Dbg::Dbg( LogStream *os ): my_stream(0), my_level(LogSilent){
    /// create a Dbg object on the LogStream with Debug threshold
    if ( !os ){
      throw( "LogStreams FATAL error: No Stream supplied! " );
    }
    if ( os->single_threaded() || init_mutex() ){
      my_stream = os;
      my_level = os->get_threshold();
      os->set_threshold( LogDebug );
    }
  }

  Dbg::Dbg( LogStream& os ):  my_stream(0), my_level(LogSilent){
    /// create a Dbg object on the LogStream with Debug threshold
    if ( os.single_threaded() || init_mutex() ){
      my_stream = &os;
      my_level = os.get_threshold();
      os.set_threshold( LogDebug );
    }
  }

  Dbg::~Dbg(){
    /// destroy the Dbg object
    my_stream->flush();
    my_stream->set_threshold( my_level );
    if ( !my_stream->single_threaded() ){
      mutex_release();
    }
  }

  LogStream& Dbg::operator *() {
    /// return the LogStream from the Dbg object
#ifdef DARE_TO_OPTIMIZE
    if ( my_stream->get_level() >= my_stream->get_threshold() ){
      return *my_stream;
    }
    else {
      return null_stream;
    }
#else
    return *my_stream;
#endif
  }

  xDbg::xDbg( LogStream *os ): my_stream(0), my_level(LogSilent){
    /// create a xDbg object on the LogStream with Heavy threshold
    if ( !os ){
      throw( "LogStreams FATAL error: No Stream supplied! " );
    }
    if ( os->single_threaded() || init_mutex() ){
      my_stream = os;
      my_level = os->get_threshold();
      os->set_threshold( LogHeavy );
    }
  }

  xDbg::xDbg( LogStream& os ):  my_stream(0), my_level(LogSilent){
    /// create a xDbg object on the LogStream with Heavy threshold
    if ( os.single_threaded() || init_mutex() ){
      my_stream = &os;
      my_level = os.get_threshold();
      os.set_threshold( LogHeavy );
    }
  }

  xDbg::~xDbg(){
    /// destroy the xDbg object
    my_stream->flush();
    my_stream->set_threshold( my_level );
    if ( !my_stream->single_threaded() ){
      mutex_release();
    }
  }

  LogStream& xDbg::operator *(){
    /// return the LogStream from the xDbg object
#ifdef DARE_TO_OPTIMIZE
    if ( my_stream->get_level() >= my_stream->get_threshold() ){
      return *my_stream;
    }
    else {
      return null_stream;
    }
#else
    return *my_stream;
#endif
  }

  xxDbg::xxDbg( LogStream *os ): my_stream(0), my_level(LogSilent){
    /// create a xxDbg object on the LogStream with Extreme threshold
    if ( !os ){
      throw( "LogStreams FATAL error: No Stream supplied! " );
    }
    if ( os->single_threaded() || init_mutex() ){
      my_stream = os;
      my_level = os->get_threshold();
      os->set_threshold( LogExtreme );
    }
  }

  xxDbg::xxDbg( LogStream& os ):  my_stream(0), my_level(LogSilent){
    /// create a xxDbg object on the LogStream with Extreme threshold
    if ( os.single_threaded() || init_mutex() ){
      my_stream = &os;
      my_level = os.get_threshold();
      os.set_threshold( LogExtreme );
    }
  }

  xxDbg::~xxDbg(){
    /// destroy the xxDbg object
    my_stream->flush();
    my_stream->set_threshold( my_level );
    if ( !my_stream->single_threaded() ){
      mutex_release();
    }
  }

  LogStream& xxDbg::operator *(){
    /// return the LogStream from the xxDbg object
#ifdef DARE_TO_OPTIMIZE
    if ( my_stream->get_level() >= my_stream->get_threshold() ){
      return *my_stream;
    }
    else {
      return null_stream;
    }
#else
    return *my_stream;
#endif
  }

}
