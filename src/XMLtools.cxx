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

#include "ticcutils/XMLtools.h"

#include <string>
#include <cstddef>
#include <stdexcept>
#include <utility>
#include "libxml/xpath.h"
#include "libxml/xpathInternals.h"
#include "libxml/parser.h"

using namespace std;

namespace TiCC {

  xmlDoc *create_xmlDocument( const std::string& root_name ){
    xmlDoc *result=  xmlNewDoc( TiCC::to_xmlChar("1.0") );
    xmlNode *root = xmlNewDocNode( result,
				   0,
				   TiCC::to_xmlChar(root_name),
				   0 );
    xmlDocSetRootElement( result, root );
    return result;
  }

  xmlNode *getRoot( xmlDoc *doc ){
    return xmlDocGetRootElement(doc);
  }

  const string serialize( const xmlDoc& doc ) {
    /// serialize a complete xmlDoc to an UTF-8 string
    xmlChar *buf;
    int size;
    xmlDocDumpFormatMemoryEnc( const_cast<xmlDoc*>(&doc), &buf, &size, "UTF-8", 1 );
    const string result = to_string( buf, size );
    xmlFree( buf );
    return result;
  }

  bool isNCName( const string& s ){
    /// test if a string is a valid NCName value
    /*!
      \param s the inputstring
      \return true if \e s may be used as an NCName (e.g. for xml:id)
    */
    int test = xmlValidateNCName( to_xmlChar(s), 0 );
    if ( test != 0 ){
      return false;
    }
    return true;
  }

  string create_NCName( const string& s ){
    /// create a valid NCName
    /*!
      \param s a string to be used as template
      \return a string that is a valid NCname

      Make sure these prerequisites are met:
      An xsd:NCName value must start with either a letter or underscore ( _ )
      and may contain only letters, digits, underscores ( _ ), hyphens ( - ),
      and periods ( . ).
    */
    if ( isNCName( s ) ){
      return s;
    }
    else {
      string result = s;
      while ( !result.empty()
	      && ( result.front() == '.'
		   || result.front() == '-'
		   || !isalpha(result.front() ) ) ){
	if ( result.front() == '_' ){
	  break;
	}
	result.erase(result.begin());
      }
      if ( result.empty() ){
	throw runtime_error( "unable to create a valid NCName from '"
			     + s + "', would be empty" );
      }
      if ( isNCName( result ) ){
	return result;
      }
      else {
	auto it = result.begin();
	while ( it != result.end() ){
	  if ( *it == ' ' ){
	    // replace spaces by '_'
	    *it = '_';
	    ++it;
	  }
	  else if ( *it == '-'
		    || *it == '_'
		    || *it == '.' ){
	    // not alphanumeric, but allowed
	    ++it;
	  }
	  else if ( !isalnum(*it) ){
	    it = result.erase(it);
	  }
	  else {
	    ++it;
	  }
	}
	if ( result.empty() ){
	  throw runtime_error( "unable to create a valid NCName from '"
			       + s + "', (empty result)" );
	}
	else if ( !isNCName( result ) ){
	  throw runtime_error( "unable to create a valid NCName from '"
			       + s + "'" );
	}
	return result;
      }
    }
  }

  string getNS( const xmlNode *node, string& prefix ){
    /// get the NameSpace of a node
    /*!
      \param node the node to examine
      \param prefix the prefix of the namespace
      \return the href of the namespace
    */
    string result;
    prefix = "";
    const xmlNs *p = node->ns;
    if ( p ){
      if ( p->prefix ){
	prefix = to_string(p->prefix);
      }
      result = to_string(p->href);
    }
    return result;
  }

  map<string,string> getNSvalues( const xmlNode *node ){
    /// get the NameSpaces related to a node
    /*!
      \param node the node to examine
      \return a map of prefix to href
    */
    map<string,string> result;
    const xmlNs *p = node->ns;
    while ( p ){
      string pre;
      string val;
      if ( p->prefix ){
	pre = to_string(p->prefix);
      }
      val = to_string(p->href);
      result[pre] = val;
      p = p->next;
    }
    return result;
  }

  map<string,string> getDefinedNS( const xmlNode *node ){
    /// get the NameSpaces defined on a node
    /*!
      \param node the node to examine
      \return a map of prefix to href
    */
    map<string,string> result;
    const xmlNs *p = node->nsDef;
    while ( p ){
      string pre;
      string val;
      if ( p->prefix ){
	pre = to_string(p->prefix);
      }
      val = to_string(p->href);
      result[pre] = val;
      p = p->next;
    }
    return result;
  }

  //#define DEBUG_XPATH

  list<xmlNode*> FindLocal( xmlXPathContext* ctxt,
			    const string& xpath ){
    /// extract all nodes matching a XPath
    /*!
      \param ctxt the XPathContext to search through
      \param xpath an XPath expression
      \return a list of all matching nodes
    */
    list<xmlNode*> nodes;
    xmlXPathObject* result = xmlXPathEval( to_xmlChar(xpath), ctxt);
    if ( result ){
      if (result->type != XPATH_NODESET) {
	xmlXPathFreeObject(result);
	throw runtime_error( "sorry, only nodeset types supported for now." );
      }
      xmlNodeSet* nodeset = result->nodesetval;
      if ( nodeset ){
	for ( int i = 0; i != nodeset->nodeNr; ++i ){
	  nodes.push_back(nodeset->nodeTab[i]);
	}
      }
      xmlXPathFreeObject(result);
    }
    else {
      throw runtime_error( "Invalid Xpath: '" + xpath + "'" );
    }
    return nodes;
  }

  const string defaultP = "default";

  void register_namespaces( xmlXPathContext* ctxt ){
    /// register all the namespaces in a XPathContext
    /*!
      \param ctxt The XPathContext we wil register to

      Special care is taken to register a default prefix for the anonymous
      prefix
    */
    map<string,string> m = getNSvalues( ctxt->node );
#ifdef DEBUG_XPATH
    {
      cerr << m.size() << " namespaces for node " << Name( ctxt->node ) << endl;
      for ( auto const& [key,val] : m ){
	cerr << key << ":" << val << endl;
      }
    }
#endif
    for ( auto const& [key,value] : m ){
      if ( key.empty() ){
	// the anonymous namespace
	xmlXPathRegisterNs( ctxt,
			    to_xmlChar(defaultP),
			    to_xmlChar(value) );
      }
      else {
	xmlXPathRegisterNs( ctxt,
			    to_xmlChar(key),
			    to_xmlChar(value) );
      }
    }
  }

  string replaceStarNS( const string& xPath ){
    /// helper function to enable search with a wilcard prefix
    // this function replace "*:" prefixes by a default prefix
    /*!
      \param xPath an XPath expression
      \return a variant with substituted "*:" prefixes
    */
    string result;
    string::size_type pos = xPath.find( "*:" );
    if ( pos == string::npos ){
      result = xPath;
    }
    else {
      result = xPath.substr( 0, pos ) + defaultP + ":"
	+ replaceStarNS(xPath.substr( pos+2 ) );
    }
    return result;
  }

  list<xmlNode*> FindNodes( const xmlNode* node,
			    const string& xPath ){
    /// extract all nodes matching a XPath
    /*!
      \param node the node to start searching at
      \param xPath an XPath expression
      \return a list of all matching nodes
    */
    string xpath = replaceStarNS( xPath );
#ifdef DEBUG_XPATH
    cerr << "replaced " << xPath << " by " << xpath << endl;
#endif
    xmlXPathContext* ctxt = xmlXPathNewContext( node->doc );
    ctxt->node = const_cast<xmlNode *>(node);
    register_namespaces( ctxt );
    list<xmlNode*> nodes = FindLocal( ctxt, xpath );
#ifdef DEBUG_XPATH
    if ( nodes.empty() ){
      cerr << "no " << xPath << " nodes found in " << Name(node) << endl;
    }
    else {
      cerr << "Found " << nodes.size() << " nodes in " << Name(node) << endl;
    }
#endif
    if ( ctxt->namespaces != NULL ){
      xmlFree(ctxt->namespaces);
    }
    xmlXPathFreeContext(ctxt);
    return nodes;
  }

  list<xmlNode*> FindNodes( xmlDoc* doc,
			    const string& xpath ){
    /// extract all nodes matching a XPath
    /*!
      \param doc the xmlDoc to search in
      \param xpath an XPath expression
      \return a list of all matching nodes
    */
    const xmlNode *root = xmlDocGetRootElement( doc );
    return FindNodes( root, xpath );
  }

  xmlNode *xPath( const xmlNode *node, const string& xpath ){
    /// search a node using an XPath expression
    /*!
      \param node the node to search in
      \param xpath the XPath expression to use
      \return 0 when nothing is found, or the first match if 1 or more matches
      are found
    */
    list<xmlNode*> srch = FindNodes( node, xpath );
    xmlNode *result = 0;
    if ( !srch.empty() ){
      result = srch.front();
    }
    return result;
  }

  xmlNode *xPath( xmlDoc *doc, const string& xpath ){
    /// search a node using an XPath expression
    /*!
      \param doc the xmlDoc to search in
      \param xpath the XPath expression to use
      \return 0 when nothing is found, or the first match if 1 or more matches
      are found
    */
    const xmlNode *root = xmlDocGetRootElement( doc );
    return xPath( root, xpath );
  }

  const string serialize( const xmlNode& node ){
    /// serialize an xmlNode to a string (XML fragment)
    xmlBuffer *buf = xmlBufferCreate();
    xmlNodeDump( buf, 0, const_cast<xmlNode*>(&node), 0, 0 );
    const string result = to_string(xmlBufferContent( buf ));
    xmlBufferFree( buf );
    return result;
  }

}
