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

  XmlDoc::XmlDoc( const string& elem ){
    /// create an XmlDoc with a root node
    /*!
      \param elem the tag of the root node
    */
    the_doc = xmlNewDoc( to_xmlChar("1.0") );
    MakeRoot( elem );
  }

  const string XmlDoc::toString() const {
    /// serialize a complete XmlDoc to an UTF-8 string
    xmlChar *buf;
    int size;
    xmlDocDumpFormatMemoryEnc( the_doc, &buf, &size, "UTF-8", 1 );
    const string result = string( to_char(buf), size );
    xmlFree( buf );
    return result;
  }

  xmlNode *XmlDoc::getRoot() const {
    /// return the root node
    if ( the_doc ){
      return xmlDocGetRootElement(the_doc);
    }
    return 0;
  }

  void XmlDoc::setRoot( xmlNode *node ){
    /// set the root node
    if ( the_doc ){
      xmlDocSetRootElement(the_doc, node );
    }
  }

  xmlNode *XmlDoc::MakeRoot( const string& elem ){
    /// create a root node with tag \e elem
    /*!
      \param elem the tag of the new root node
      \return the newly created node
    */
    xmlNode *root;
    root = xmlNewDocNode( the_doc,
			  0,
			  to_xmlChar(elem.c_str()),
			  0 );
    xmlDocSetRootElement( the_doc, root );
    return root;
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
	prefix = to_char(p->prefix);
      }
      result = to_char(p->href);
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
	pre = to_char(p->prefix);
      }
      val = to_char(p->href);
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
	pre = to_char(p->prefix);
      }
      val = to_char(p->href);
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
    xmlXPathObject* result = xmlXPathEval( to_xmlChar(xpath.c_str()), ctxt);
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
			    to_xmlChar(defaultP.c_str()),
			    to_xmlChar(value.c_str()) );
      }
      else {
	xmlXPathRegisterNs( ctxt,
			    to_xmlChar(key.c_str()),
			    to_xmlChar(value.c_str()) );
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

  string serialize( const xmlNode& node ){
    /// serialize an xmlNode to a string (XML fragment)
    xmlBuffer *buf = xmlBufferCreate();
    xmlNodeDump( buf, 0, const_cast<xmlNode*>(&node), 0, 0 );
    string result = to_char(xmlBufferContent( buf ));
    xmlBufferFree( buf );
    return result;
  }

}
