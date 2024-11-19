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

#ifndef TICC_XML_TOOLS_H
#define TICC_XML_TOOLS_H

#include <cassert>
#include <string>
#include <map>
#include <list>
#include <iostream>
#include "libxml/xmlstring.h"
#include "libxml/globals.h"
#include "libxml/parser.h"

namespace TiCC {

  xmlDoc *create_xmlDocument( const std::string& );
  xmlNode *getRoot( xmlDoc *);
  bool isNCName( const std::string& );
  std::string create_NCName( const std::string& );

  inline const xmlChar *to_xmlChar( const std::string& in ){
    return reinterpret_cast<const xmlChar *>(in.c_str());
  }

  inline const std::string to_string( const xmlChar *in ){
    return reinterpret_cast<const char *>(in);
  }

  inline const std::string to_string( const xmlChar *in, size_t len ){
    return std::string( reinterpret_cast<const char *>(in), len );
  }

  inline xmlNode *XmlNewNode( const std::string& elem ){
    return xmlNewNode( 0, to_xmlChar(elem) );
  }

  inline xmlNode *XmlNewNode( xmlNs *ns, const std::string& elem ){
    return xmlNewNode( ns, to_xmlChar(elem) );
  }

  inline xmlNode *XmlNewComment( const std::string& elem ){
    return xmlNewComment( to_xmlChar(elem) );
  }

  inline xmlNode *XmlNewChild( xmlNode *node,
			       const std::string& elem ){
    xmlNode *chld = xmlNewNode( 0, to_xmlChar(elem) );
    return xmlAddChild( node, chld );
  }

  inline xmlNode *XmlNewChild( xmlNode *node,
			       xmlNs *ns,
			       const std::string& elem ){
    xmlNode *chld = xmlNewNode( ns, to_xmlChar(elem) );
    return xmlAddChild( node, chld );
  }

  inline xmlNode *XmlNewTextChild( xmlNode *node,
				   const std::string& elem,
				   const std::string& val ){
    if ( val.empty() )
      return xmlNewTextChild( node,
			      0,
			      to_xmlChar(elem),
			      0 );
    else
      return xmlNewTextChild( node,
			      0,
			      to_xmlChar(elem),
			      to_xmlChar(val) );
  }

  inline xmlNode *XmlNewTextChild( xmlNode *node,
				   xmlNs *ns,
				   const std::string& elem,
				   const std::string& val ){
    if ( val.empty() )
      return xmlNewTextChild( node,
			      ns,
			      to_xmlChar(elem)
			      ,0 );
    else
      return xmlNewTextChild( node,
			      ns,
			      to_xmlChar(elem),
			      to_xmlChar(val) );
  }

  inline void XmlAddContent( xmlNode *node, const std::string& cont ){
    xmlNodeAddContent( node, to_xmlChar(cont) );
  }

  inline xmlAttr *XmlSetAttribute( xmlNode *node,
				   const std::string& att,
				   const std::string& val ){
    return xmlSetProp( node,
		       to_xmlChar(att),
		       to_xmlChar(val) );
  }

  inline std::string Name( const xmlNode *node ){
    std::string result;
    if ( node ){
      result = to_string(node->name);
    }
    return result;
  }

  inline std::string TextValue( const xmlNode *node ){
    /// extract the string content of an xmlNode
    /*!
      \param node The xmlNode to extract from
      \return the string value of node
    */
    std::string result;
    if ( node ){
      xmlChar *tmp = xmlNodeGetContent( node );
      if ( tmp ){
	result = to_string(tmp );
	xmlFree( tmp );
      }
    }
    return result;
  }

  inline std::string getAttribute( const xmlNode *node,
				   const std::string& att ){
    if ( node ){
      const xmlAttr *a = node->properties;
      while ( a ){
	if ( att == to_string(a->name) ){
	  return TextValue(a->children);
	}
	a = a->next;
      }
    }
    return "";
  }

  inline std::map<std::string,std::string> getAttributes( const xmlNode *node ){
    std::map<std::string,std::string> result;
    if ( node ){
      const xmlAttr *a = node->properties;
      while ( a ){
	result[to_string(a->name)] = TextValue(a->children);
	a = a->next;
      }
    }
    return result;
  }

  std::string getNS( const xmlNode *, std::string& );
  inline std::string getNS( const xmlNode *n ) {
    std::string s;
    return getNS( n, s);
  }

  std::map<std::string,std::string> getNSvalues( const xmlNode * );
  std::map<std::string,std::string> getDefinedNS( const xmlNode * );

  const std::string serialize( const xmlNode& );
  const std::string serialize( const xmlNode* );
  const std::string serialize( const xmlDoc& );
  const std::string serialize( const xmlDoc* );

  inline std::ostream& operator << ( std::ostream& os, const xmlDoc& doc ){
    os << serialize(doc);
    return os;
  }

  inline std::ostream& operator << ( std::ostream& os, const xmlDoc* doc ){
    if ( doc ){
      os << serialize(*doc);
    }
    else {
      os << "No xmlDoc";
    }
    return os;
  }

  inline std::ostream& operator << ( std::ostream& os, const xmlNode& node ){
    os << serialize( node );
    return os;
  }

  inline std::ostream& operator << ( std::ostream& os, const xmlNode *node ){
    if ( node ){
      os << serialize( *node );
    }
    else {
      os << "No xmlNode";
    }
    return os;
  }

  std::list<xmlNode*> FindNodes( const xmlNode *, const std::string& );
  xmlNode *xPath( const xmlNode *, const std::string& );
  std::list<xmlNode*> FindNodes( xmlDoc *, const std::string& );
  xmlNode *xPath( xmlDoc *, const std::string& );

} // namespace TiCC

#endif
