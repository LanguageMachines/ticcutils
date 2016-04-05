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

#include <string>
#include <map>
#include <set>
#include <fstream>
#include <stdexcept>
#include <iostream>
#include "ticcutils/StringOps.h"
#include "ticcutils/Configuration.h"

using namespace std;

namespace TiCC {
  Configuration::Configuration(){
    myMap["global"] = ssMap();
  }

  string fixControl( const string& s, char c ){
    string sString;
    string rString;
    switch ( c ){
    case 't':
      sString = "\\t";
      rString = "\t";
      break;
    case 'r':
      sString = "\\r";
      rString = "\r";
      break;
    case 'n':
      sString = "\\n";
      rString = "\n";
      break;
    default:
      throw logic_error("invalid char for fixControl" );
    }
    string::size_type pos1 = s.find( sString );
    if ( pos1 == string::npos ){
      return s;
    }
    else {
      string result = s.substr( 0, pos1 );
      result += rString;
      string::size_type pos2 = s.find( sString, pos1+1 );
      while ( pos2 != string::npos ){
	result += s.substr( pos1+2, pos2-pos1-2 );
	result += rString;
	pos1 = pos2;
	pos2 = s.find( sString, pos1+1 );
      }
      result += s.substr( pos1+2 );
      return result;
    }
  }

  string fixControls( const string& s ){
    string result = s;
    result = fixControl( result, 'n' );
    result = fixControl( result, 'r' );
    result = fixControl( result, 't' );
    return result;
  }

  bool Configuration::fill( const string& fileName ){
    cdir = dirname( fileName ) + "/";
    //  cerr << "dirname= " << cdir << endl;
    ifstream is( fileName );
    if ( !is ){
      cerr << "unable to read configuration from " << fileName << endl;
      return false;
    }
    string inLine;
    string section = "global";
    while ( getline( is, inLine ) ){
      string line = TiCC::trim(inLine);
      if ( line.empty() )
	continue;
      if ( line[0] == '#' )
	continue;
      if ( line.find( "[[") == 0  )
	if ( line[line.length()-1] == ']' &&
	     line[line.length()-2] == ']' ){
	  section = line.substr(2,line.length()-4);
	  //	cerr << "GOT section = " << section << endl;
	}
	else {
	  cerr << "invalid section: in line '" << line << "'" << endl;
	  return false;
	}
      else {
	string::size_type pos = line.find("=");
	if ( pos != string::npos ){
	  string att = line.substr(0,pos);
	  string val = line.substr(pos+1);
	  if ( val[0] == '"' && val[val.length()-1] == '"' )
	    val = val.substr(1, val.length()-2);
	  val = fixControls( val );
	  myMap[section][att] = val;
	  if ( section == "global"
	       && att == "configDir" )
	    cdir = val;
	}
	else {
	  cerr << "invalid attribute value pair in line '" << line << "'" << endl;
	  return false;
	}
      }
    }
    return true;
  }

  bool Configuration::fill( const string& fileName, const string& section ){
    ifstream is( fileName );
    if ( !is ){
      cerr << "unable to read configuration from " << fileName << endl;
      return false;
    }
    bool found = false;
    string inLine;
    string localsection;
    //  cerr << "looking for section = " << section << endl;
    while ( getline( is, inLine ) ){
      string line = TiCC::trim(inLine);
      if ( line.empty() )
	continue;
      if ( line[0] == '#' )
	continue;
      if ( line.find( "[[") == 0  )
	if ( line[line.length()-1] == ']' &&
	     line[line.length()-2] == ']' ){
	  localsection = line.substr(2,line.length()-4);
	  //	cerr << "GOT section = " << localsection << endl;
	}
	else {
	  cerr << "invalid section: in line '" << line << "'" << endl;
	  return false;
	}
      else if ( localsection == section ){
	found = true;
	string::size_type pos = line.find("=");
	if ( pos != string::npos ){
	  string att = line.substr(0,pos);
	  string val = line.substr(pos+1);
	  if ( val[0] == '"' && val[val.length()-1] == '"' )
	    val = val.substr(1, val.length()-2);
	  myMap[section][att] = val;
	}
	else {
	  cerr << "invalid attribute value pair in line '" << line << "'" << endl;
	  return false;
	}
      }
    }
    if ( !found ){
      cerr << "unable to find a section [[" << section << "]] in file: "
	   << fileName << endl;
      return false;
    }
    return true;
  }

  void Configuration::dump( ostream& os ) const {
    auto it1 = myMap.find("global");
    if ( it1 == myMap.end() ){
      os << "empty" << endl;
      return;
    }
    os << "[[global]]" << endl;
    if ( !cdir.empty() ){
      os << "configDir=" << cdir << endl;
    }
    auto it2 = it1->second.begin();
    while ( it2 != it1->second.end() ){
      string out = it2->second;
      os << it2->first << "=" << it2->second << endl;
      ++it2;
    }
    it1 = myMap.begin();
    while ( it1 != myMap.end() ){
      if ( it1->first != "global" ){
	os << "[[" << it1->first << "]]" << endl;
	it2 = it1->second.begin();
	while ( it2 != it1->second.end() ){
	  os << it2->first << "=" << it2->second << endl;
	  ++it2;
	}
      }
      ++it1;
    }
  }

  void Configuration::create_configfile( const string& name ) const {
    ofstream os( name );
    if ( !os ){
      throw runtime_error( "unable to create outputfile: " + name );
    }
    dump( os );
  }

  string Configuration::setatt( const string& att,
				const string& val,
				const string& sect ){
    string oldVal;
    string section = sect;
    if ( section.empty() )
      section = "global";
    auto it1 = myMap.find( section );
    if ( it1 != myMap.end() ){
      auto const& it2 = it1->second.find( att );
      if ( it2 != it1->second.end() ){
	oldVal = it2->second;
      }
      it1->second[att] = val;
    }
    else {
      myMap[section].insert( make_pair( att, val ) );
    }
    return oldVal;
  }

  string Configuration::lookUp( const string& att, const string& section ) const {
    string key = section;
    if ( key.empty() )
      key = "global";
    auto const& it1 = myMap.find( key );
    if ( it1 == myMap.end() ){
      return "";
    }
    else {
      auto const& it2 = it1->second.find( att );
      if ( it2 == it1->second.end() ){
	if ( section.empty() || section == "global" )
	  return "";
	else
	  return lookUp( att, "global" );
      }
      else
	return it2->second;
    }
  }

  map<string,string> Configuration::lookUpAll( const string& section ) const {
    map<string,string> result;
    string key = section;
    if ( key.empty() )
      key = "global";
    auto const& it1 = myMap.find( key );
    if ( it1 != myMap.end() ){
      auto it2 = it1->second.begin();
      while ( it2 != it1->second.end() ){
	result[it2->first] = it2->second;
	++it2;
      }
    }
    return result;
  }

  set<string> Configuration::lookUpSections() const {
    set<string> result;
    result.insert("global");
    auto it = myMap.begin();
    while ( it != myMap.end() ){
      result.insert( it->first );
      ++it;
    }
    return result;
  }

  bool Configuration::hasSection( const string& section ) const {
    if ( !section.empty() ){
      auto const it = myMap.find( section );
      if ( it != myMap.end() )
	return true;
    }
    return false;
  }

}
