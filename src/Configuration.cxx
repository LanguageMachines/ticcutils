/*
  Copyright (c) 2006 - 2021
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

#include "ticcutils/Configuration.h"

#include <string>
#include <map>
#include <set>
#include <fstream>
#include <stdexcept>
#include <iostream>
#include "ticcutils/StringOps.h"

using namespace std;

namespace TiCC {

  Configuration::Configuration(){
    /// set up a Configuration structure
    myMap["global"] = ssMap();
  }

  string fixControl( const string& s, char c ){
    /// replace special characters
    /*!
      \param s the inputstring
      \param c special 'symbol' to replace
      \return a new string where all occurences of "\\c" aee replaced  by "\c"
    */
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
    /// replace all "\\n", "\\r", "\\t" sequences by "\n", "\r" and "\t"
    string result = s;
    result = fixControl( result, 'n' );
    result = fixControl( result, 'r' );
    result = fixControl( result, 't' );
    return result;
  }

  bool Configuration::get_att_val( const string& line, const string& section ){
    /// parse a string into an attribute/value pair and insert in a section
    /*!
      \param line the line to parse
      \param section the section to insert in.
      \return true if we have a result
     */
    string::size_type pos = line.find("=");
    if ( pos != string::npos ){
      string att = line.substr(0,pos);
      att = TiCC::trim(att);
      string val = line.substr(pos+1);
      val = TiCC::trim(val);
      if ( val[0] == '"' && val[val.length()-1] == '"' ){
	val = val.substr(1, val.length()-2);
      }
      val = fixControls( val );
      myMap[section][att] = val;
      return true;
    }
    return false;
  }

  bool Configuration::fill( const string& fileName ){
    /// fill a Configuration structure from a file
    /*!
      \param fileName the name of the input file
      \return true on succes
    */
    ifstream is( fileName );
    if ( !is ){
      cerr << "unable to read configuration from " << fileName << endl;
      return false;
    }
    string cdir = dirname( fileName );
    if ( cdir == "." ){
      cdir = "";
    }
    else {
      cdir += "/";
    }
    //  cerr << "dirname= " << cdir << endl;
    myMap["global"]["configDir"] = cdir; // can be overidden below
    string inLine;
    string section = "global";
    while ( getline( is, inLine ) ){
      string line = TiCC::trim(inLine);
      if ( line.empty() || line[0] == '#' ){
	continue;
      }
      if ( match_front( line, "[[" ) ){
	if ( line[line.length()-1] == ']' &&
	     line[line.length()-2] == ']' ){
	  section = line.substr(2,line.length()-4);
	  //	cerr << "GOT section = " << section << endl;
	}
	else {
	  cerr << "invalid section: in line '" << line << "'" << endl;
	  return false;
	}
      }
      else if ( !get_att_val ( line, section ) ){
	cerr << "invalid attribute value pair in line '"
	     << line << "'" << endl;
	return false;
      }
    }
    return true;
  }

  bool Configuration::fill( const string& fileName, const string& insect ){
    /// fill a Configuration structure from a file for a certain section
    /*!
      \param fileName the name of the input file
      \param insect the section to find and fill
      \return true on succes
    */
    ifstream is( fileName );
    if ( !is ){
      cerr << "unable to read configuration from " << fileName << endl;
      return false;
    }
    bool found = false;
    string inLine;
    string localsection;
    string section = TiCC::trim(insect);
    //  cerr << "looking for section = " << insection << endl;
    while ( getline( is, inLine ) ){
      string line = TiCC::trim(inLine);
      if ( line.empty() || line[0] == '#' ){
	continue;
      }
      if ( match_front( line, "[[" ) ){
	if ( line[line.length()-1] == ']' &&
	     line[line.length()-2] == ']' ){
	  localsection = line.substr(2,line.length()-4);
	  localsection = TiCC::trim(localsection);
	  //	cerr << "GOT section = " << localsection << endl;
	}
	else {
	  cerr << "invalid section: in line '" << line << "'" << endl;
	  return false;
	}
      }
      else if ( localsection == section ){
	found = true;
	if ( !get_att_val( line, section ) ){
	  cerr << "invalid attribute value pair in line '"
	       << line << "'" << endl;
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

  string encode_ctrl( const string& in ){
    /// encdode "\n", "\r" and "\t" into "\\n", "\\r" and "\\t"
    /*!
      \param in the input string
      \return an encodes string
    */
    string out;
    for ( const auto& c : in ){
      switch ( c ){
      case '\n': out += "\\n";
	break;
      case '\r': out += "\\r";
	break;
      case '\t': out += "\\t";
	break;
      default:
	out += c;
      }
    }
    return out;
  }

  void Configuration::dump( ostream& os ) const {
    /// dump a Configuration to a stream
    auto it1 = myMap.find("global");
    if ( it1 == myMap.end() ){
      os << "empty" << endl;
      return;
    }
    os << "[[global]]" << endl;
    auto it2 = it1->second.begin();
    while ( it2 != it1->second.end() ){
      os << it2->first << "=" << it2->second << endl;
      ++it2;
    }
    it1 = myMap.begin();
    while ( it1 != myMap.end() ){
      if ( it1->first != "global" ){
	os << endl << "[[" << it1->first << "]]" << endl;
	it2 = it1->second.begin();
	while ( it2 != it1->second.end() ){
	  os << it2->first << "=" << encode_ctrl(it2->second) << endl;
	  ++it2;
	}
      }
      ++it1;
    }
  }

  void Configuration::create_configfile( const string& name ) const {
    /// create a named configfile
    /*!
      \param name the name of the output file
    */
    ofstream os( name );
    if ( !os ){
      throw runtime_error( "unable to create outputfile: " + name );
    }
    dump( os );
  }

  string Configuration::setatt( const string& inatt,
				const string& inval,
				const string& insect ){
    /// set an attribute/value pair in a section
    /*!
      \param inatt the attribute to add
      \param inval the value to insert
      \param insect the section to insert in. When empty, use the "global" one.
      \return the old value if \e inatt was already set.
     */
    string oldVal;
    string att = TiCC::trim(inatt);
    string val = TiCC::trim(inval);
    string sect = TiCC::trim(insect);
    if ( sect.empty() ){
      sect = "global";
    }
    auto it1 = myMap.find( sect );
    if ( it1 != myMap.end() ){
      auto const& it2 = it1->second.find( att );
      if ( it2 != it1->second.end() ){
	oldVal = it2->second;
      }
      it1->second[att] = val;
    }
    else {
      myMap[sect].insert( make_pair( att, val ) );
    }
    return oldVal;
  }

  string Configuration::clearatt( const string& inatt,
				  const string& insect ){
    /// remove an attribute (and its value) from a section
    /*!
      \param inatt the attribute to remove
      \param insect the section to search,  When empty, use the "global" one.
      \return the value of the removed attribute
    */
    //    cerr << "clear att: '" << inatt << "' in [" << insect << "]" << endl;
    string oldVal;
    string sect = TiCC::trim(insect);
    string att = TiCC::trim(inatt);
    if ( sect.empty() ){
      sect = "global";
    }
    //    cerr << "clear sect [" << sect << "]" << endl;
    auto it1 = myMap.find( sect );
    if ( it1 != myMap.end() ){
      //      cerr << "found sect [" << sect << "]" << endl;
      auto const& it2 = it1->second.find( att );
      if ( it2 != it1->second.end() ){
	//	cerr << "found att [" << att << "]" << endl;
	oldVal = it2->second;
      }
      else {
	//	cerr << "missed att [" << att << "]" << endl;
      }
      it1->second.erase( att );
    }
    else {
      //      cerr << "MISSED sect [" << sect << "]" << endl;
    }
    return oldVal;
  }

  string Configuration::getatt( const string& inatt,
				const string& insect ) const {
    /// return the value for an attribute
    /*!
      \param inatt the attribute to remove
      \param insect the section to search,  When empty, use the "global" one.
      \return the value of the attribute
      \note when the attribute is NOT FOUND in the given section, we try to
      retrieve it from the 'global' one. Which serves as a fallback then
    */
    string sect = TiCC::trim(insect);
    string att = TiCC::trim(inatt);
    string key = sect;
    if ( key.empty() ){
      key = "global";
    }
    auto const& it1 = myMap.find( key );
    if ( it1 == myMap.end() ){
      return "";
    }
    else {
      auto const& it2 = it1->second.find( att );
      if ( it2 == it1->second.end() ){
	if ( sect.empty() || sect == "global" ){
	  return "";
	}
	else {
	  return getatt( att, "global" );
	}
      }
      else {
	return it2->second;
      }
    }
  }

  map<string,string> Configuration::lookUpAll( const string& insect ) const {
    /// return all attribute/value values in a given section
    /*!
      \param insect the section to search. When empty use 'global'
      \return a map of all attribute/value pairs
    */
    map<string,string> result;
    string sect = TiCC::trim(insect);
    if ( sect.empty() ){
      sect = "global";
    }
    auto const& it1 = myMap.find( sect );
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
    /// return the names of all sections
    set<string> result;
    result.insert("global");
    auto it = myMap.begin();
    while ( it != myMap.end() ){
      result.insert( it->first );
      ++it;
    }
    return result;
  }

  bool Configuration::hasSection( const string& insect ) const {
    /// check the presence of a section
    string sect = TiCC::trim( insect );
    if ( !sect.empty() ){
      auto const it = myMap.find( sect );
      if ( it != myMap.end() ){
	return true;
      }
    }
    return false;
  }

  void Configuration::merge( const Configuration& in, bool override ) {
    /// merge two Configuration objects
    /*!
      \param in the Configuration to add
      \param override when true, override allready present attributes
     */
    // get al sections from in;
    set<string> sections = in.lookUpSections();
    for ( const auto& s : sections ){
      // for every section, get all at-val pairs
      auto avs = in.lookUpAll( s );
      for ( const auto& av : avs ){
	// merge every at-val in the wanted section
	if ( !override
	     &&  myMap[s].find( av.first ) != myMap[s].end() ){
	  continue;
	}
	setatt( av.first, av.second, s );
      }
    }
  }

}
