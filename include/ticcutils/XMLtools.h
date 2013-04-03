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

#ifndef TICC_XML_TOOLS_H
#define TICC_XML_TOOLS_H

#include <string>
#include <map>
#include <list>
#include "libxml/tree.h"

namespace TiCC {

  inline xmlNode *XmlNewNode( const std::string& elem ){
    return xmlNewNode( 0, (const xmlChar*)elem.c_str() );
  }
  
  inline xmlNode *XmlNewNode( xmlNs *ns, const std::string& elem ){
    return xmlNewNode( ns, (const xmlChar*)elem.c_str() );
  }
  
  inline xmlNode *XmlNewComment( const std::string& elem ){
    return xmlNewComment( (const xmlChar*)elem.c_str() );
  }
  
  inline xmlNode *XmlNewChild( xmlNode *node, 
			       const std::string& elem ){
    xmlNode *chld = xmlNewNode( 0, (const xmlChar*)elem.c_str() );
    return xmlAddChild( node, chld );
  }

  inline xmlNode *XmlNewChild( xmlNode *node, 
			       xmlNs *ns,
			       const std::string& elem ){
    xmlNode *chld = xmlNewNode( ns, (const xmlChar*)elem.c_str() );
    return xmlAddChild( node, chld );
  }

  inline xmlNode *XmlNewTextChild( xmlNode *node, 
				   const std::string& elem, 
				   const std::string& val ){
    if ( val.empty() )
      return xmlNewTextChild( node, 0, (xmlChar*)elem.c_str(), 0 );
    else 
      return xmlNewTextChild( node, 0, 
			      (const xmlChar*)elem.c_str(),
			      (const xmlChar*)val.c_str() );
  }

  inline xmlNode *XmlNewTextChild( xmlNode *node, 
				   xmlNs *ns,
				   const std::string& elem, 
				   const std::string& val ){
    if ( val.empty() )
      return xmlNewTextChild( node, ns,
			      (xmlChar*)elem.c_str(), 0 );
    else 
      return xmlNewTextChild( node, ns,
			      (const xmlChar*)elem.c_str(),
			      (const xmlChar*)val.c_str() );
  }

  inline void XmlAddContent( xmlNode *node, const std::string& cont ){
    xmlNodeAddContent( node, (const xmlChar*)cont.c_str() );
  }
  
  inline xmlAttr *XmlSetAttribute( xmlNode *node, 
				   const std::string& att,
				   const std::string& val ){
    return xmlSetProp( node, 
		       (const xmlChar*)att.c_str(), 
		       (const xmlChar*)val.c_str() );
  }
  
  inline std::string getAttribute( const xmlNode *node, 
				   const std::string& att ){
    if ( node ){
      xmlAttr *a = node->properties;
      while ( a ){
	if ( att == (char*)a->name )
	  return (char *)a->children->content;
	a = a->next;
      }
    }
    return "";
  }

  std::string getNS( const xmlNode *, std::string& );
  inline std::string getNS( const xmlNode *n ) {
    std::string s;
    return getNS( n, s);
  }

  std::map<std::string,std::string> getNSvalues( const xmlNode * );

  std::string serialize( const xmlNode& node );

  inline std::string Name( const xmlNode *node ){
    std::string result;
    if ( node ){
      result = (char *)node->name;
    }
    return result;
  }

  inline std::string XmlContent( const xmlNode *node ){
    std::string result;
    if ( node ){
      xmlChar *tmp = xmlNodeListGetString( node->doc, node->children, 1 );
      if ( tmp ){
	result = std::string( (char *)tmp );
	xmlFree( tmp );
      }
    }
    return result;
  }

  class XmlDoc {
    friend std::ostream& operator << ( std::ostream& , const XmlDoc& );
  public:
    XmlDoc( const std::string& );
    ~XmlDoc(){
      xmlFreeDoc( the_doc );
    }
    void setRoot( xmlNode* );
    xmlNode *getRoot() const;
    xmlNode *MakeRoot( const std::string& );
    const std::string toString() const;
  private:
    xmlDoc *the_doc;
  };

  inline std::ostream& operator << ( std::ostream& os, const XmlDoc& doc ){
    os << doc.toString();
    return os;
  }

  inline std::ostream& operator << ( std::ostream& os, const xmlNode& node ){
    os << serialize( node );
    return os;
  }

  inline std::ostream& operator << ( std::ostream& os, const xmlNode *node ){
    os << serialize( *node );
    return os;
  }

  std::list<xmlNode*> FindNodes( xmlNode *, const std::string& );
  xmlNode *xPath( xmlNode *, const std::string& );

} // namespace TiCC

#endif
