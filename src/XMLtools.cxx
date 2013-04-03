/*
  $Id$
  $URL$

  Copyright (c) 1998 - 2013
  ILK   - Tilburg University
  CLiPS - University of Antwerp
 
  This file is part of ticcutils

  timbl is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.

  timbl is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, see <http://www.gnu.org/licenses/>.

  For questions and suggestions, see:
      http://ilk.uvt.nl/software.html
  or send mail to:
      timbl@uvt.nl
*/

#include <iostream>
#include <string>
#include <stdexcept>
#include "libxml/xpath.h"
#include "ticcutils/XMLtools.h"

using namespace std;

namespace TiCC {

  XmlDoc::XmlDoc( const std::string& elem ){
    the_doc = xmlNewDoc( (const xmlChar*)"""1.0""" );
    MakeRoot( elem );
  }
  
  const string XmlDoc::toString() const {
    xmlChar *buf; 
    int size;
    xmlDocDumpMemory( the_doc, &buf, &size );
    const string result = string( (const char *)buf, size );
    xmlFree( buf );
    return result;
  }
  
  xmlNode *XmlDoc::getRoot() const {
    if ( the_doc )
      return xmlDocGetRootElement(the_doc);
    else
      return 0;
  }
  
  void XmlDoc::setRoot( xmlNode *node ){
    if ( the_doc )
      xmlDocSetRootElement(the_doc, node );
  }
  
  xmlNode *XmlDoc::MakeRoot( const string& elem ){
    xmlNode *root;
    root = xmlNewDocNode( the_doc, 0, (const xmlChar*)elem.c_str(), 0 );
    xmlDocSetRootElement( the_doc, root );
    return root;
  }

  string getNS( const xmlNode *node, string& prefix ){
    string result;
    prefix = "";
    xmlNs *p = node->ns;
    if ( p ){
      if ( p->prefix ){
	prefix = (char *)p->prefix;
      }
      result = (char *)p->href;
    }
    return result;
  }

  map<string,string> getNSvalues( const xmlNode *node ){
    map<string,string> result;
    xmlNs *p = node->ns;
    while ( p ){
      string pre;
      string val;
      if ( p->prefix ){
	pre = (char *)p->prefix;
      }
      val = (char *)p->href;
      result[pre] = val;
      p = p->next;
    }
    return result;
  }

  list<xmlNode*> FindLocal( xmlXPathContext* ctxt, 
			    const string& xpath ){
    list<xmlNode*> nodes;
    xmlXPathObject* result = xmlXPathEval((xmlChar*)xpath.c_str(), ctxt);
    if ( result ){
      if (result->type != XPATH_NODESET) {
	xmlXPathFreeObject(result);
	throw runtime_error( "sorry, only nodeset types supported for now." );
	return nodes;
      }
      xmlNodeSet* nodeset = result->nodesetval;
      if ( nodeset ){
	for (int i = 0; i != nodeset->nodeNr; ++i)
	  nodes.push_back(nodeset->nodeTab[i]);
      }
      else {
	throw runtime_error( "FindLocal: Missing nodeset" );
      }
      xmlXPathFreeObject(result);
    }
    else {
      throw runtime_error( "Invalid Xpath: '" + xpath + "'" );
    }
    return nodes;
  }
  
  list<xmlNode*> FindNodes( xmlNode* node,
			    const string& xpath ){
    xmlXPathContext* ctxt = xmlXPathNewContext( node->doc );
    ctxt->node = node;
    ctxt->namespaces = xmlGetNsList( node->doc, ctxt->node );
    ctxt->nsNr = 0;
    if (ctxt->namespaces != 0 ) {
      while (ctxt->namespaces[ctxt->nsNr] != 0 ){
	ctxt->nsNr++;
      }
    }
    list<xmlNode*> nodes = FindLocal( ctxt, xpath );
    if (ctxt->namespaces != NULL)
      xmlFree(ctxt->namespaces);
    xmlXPathFreeContext(ctxt);
    return nodes;
  }

  xmlNode *xPath( xmlNode *node, const string& xpath ){
    // try to find a path, but it may not be there...
    // if there are more, just return the first

    list<xmlNode*> srch = FindNodes( node, xpath );
    xmlNode *result = 0;
    if ( srch.size() != 0 ){
      result = srch.front();
    }
    return result;
  }
      
  string serialize( const xmlNode& node ){
    // serialize to a string (XML fragment)
    xmlBuffer *buf = xmlBufferCreate();
    xmlNodeDump( buf, 0, const_cast<xmlNode*>(&node), 0, 0 );
    string result = (const char*)xmlBufferContent( buf );
    xmlBufferFree( buf );
    return result;
  }
  
}
