/*
  Copyright (c) 2006 - 2022
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

#ifndef TICC_UNI_TRIE_H
#define TICC_UNI_TRIE_H

#include <iostream>
#include "unicode/schriter.h"
#include "unicode/ustream.h"

namespace Tries {
  // A node in the generic trie.
  template <class Info> class UniTrieNode;
  template <class Info> std::ostream& operator<<( std::ostream&,
						  const UniTrieNode<Info> * );

  /// \brief A genric class for the nodes we store in the UniTrie class
  template <class Info> class UniTrieNode {
    template <class U>
      friend std::ostream& operator<<( std::ostream&,
				       const UniTrieNode<U> * );
  public:
    explicit UniTrieNode( UChar32 );
    ~UniTrieNode();
    Info *add_to_tree( Info *, const icu::UnicodeString& );
    Info *scan_tree( const icu::UnicodeString& ) const;
    void Iterate( void(*)( Info *, void * ), void * );
    void Iterate( void(*)( Info * ) );
  private:
    UChar32 label;                //!< the label.
    Info *the_info;               //!< The information at this pnt.
    UniTrieNode *next_node;       //!< Pointer to the next node.
    UniTrieNode *sub_node;        //!< Pointer to the sub node.
    UniTrieNode( const UniTrieNode& );
    UniTrieNode& operator=( const UniTrieNode& );
    Info *add_to_tree( Info *, icu::StringCharacterIterator& );
    Info *scan_tree( icu::StringCharacterIterator& ) const;
  };

  template <class Info>
    inline Info *UniTrieNode<Info>::scan_tree( icu::StringCharacterIterator& sit ) const {
    /// search a matching field in the UniTrie
    /*!
      \param name the string to match in the UniTrie
      \return an Info node where it is found in the tree or NULL.

      This function searches the first character of \e name in the UniTrie

      if NOT found it returns NULL. Otherwise it recursively searches for
      the next character in \e name
    */
    UniTrieNode *subtree = sub_node; // top node has empty label!
    while ( subtree ) {
      if ( subtree->label == sit.current32() ){
	sit.next32(); // move to next character in the word
	if ( sit.current32() == icu::CharacterIterator::DONE ){
	  // OK, we are at the end
	  return subtree->the_info;
	}
	else {
	  // more to mact, so move deeper in the tree
	  subtree = subtree->sub_node;
	}
      }
      else if ( subtree->label > sit.current32() ){
	// no match. Bail out
	return NULL;
      }
      else {
	// match might be on a next node in the tree. move there
	subtree = subtree->next_node;
      }
    }
    return NULL;
  }

  template <class Info>
    inline Info *UniTrieNode<Info>::scan_tree( const icu::UnicodeString& name ) const {
    /// search a matching field in the UniTrie
    /*!
      \param name the stringto match in the UniTrie
      \return an Info node where it is found in the tree or NULL.

      This function searches the first character of \e name in the UniTrie

      if NOT found it returns NULL. Otherwise it recursively searches for
      the next character in \e name
    */
    icu::StringCharacterIterator sit( name );
    return scan_tree( sit );
  }

  template <class Info>
    inline std::ostream& operator << ( std::ostream& os,
				       const UniTrieNode<Info> *tree ){
    /// recursively print a UniTrie to a stream
    if ( tree ){
      os << tree->sub_node;
      if ( tree->the_info ){
	os << tree->the_info << std::endl;
      }
      os << tree->next_node;
    }
    return os;
  }

  template <class Info>
    inline void UniTrieNode<Info>::Iterate( void (*F)( Info * ) ){
    /// execute the function F on each entry in the UniTrie
    if ( the_info ){
#pragma omp critical(uni_trie_mod)
      {
	F( the_info );
      }
    }
    if ( sub_node ){
      sub_node->Iterate( F );
    }
    if ( next_node ){
      next_node->Iterate( F );
    }
  }

  template <class Info>
    inline void UniTrieNode<Info>::Iterate( void (*F)( Info *, void * ),
					    void *arg ){
    /// execute the function F on each entry in the UniTrie
    if ( the_info ){
#pragma omp critical(uni_trie_mod)
      {
	F( the_info, arg );
      }
    }
    if ( sub_node ){
      sub_node->Iterate( F, arg );
    }
    if ( next_node ){
      next_node->Iterate( F, arg );
    }
  }

  template <class Info>
    inline UniTrieNode<Info>::UniTrieNode( UChar32 lab ):
    label(lab),
    the_info(NULL),
    next_node(NULL),
    sub_node(NULL)
    {
      /// create an empty UniTrieNode for the character \e lab
    }

  template <class Info>
    inline UniTrieNode<Info>::~UniTrieNode(){
    /// destroy a UniTrieNode
    delete the_info;
    delete sub_node;
    delete next_node;
  }

  template <class Info>
    inline Info *UniTrieNode<Info>::add_to_tree( Info *info,
						 icu::StringCharacterIterator& uit ){
    /// add an Info record to the UniTrie
    /*!
      \param info The information to store
      \param uit a Unicode Character iterator
      If the \e uit iterator is done, we are at the bottom, and we can store the
      info. If there is already Info, we discard the new \e info
    */
    if ( uit.current32() == icu::CharacterIterator::DONE ){
      // we reached the end of 'uit'
      if ( !the_info ){
	the_info = info;
      }
      else {
	delete( info );
      }
      return the_info;
    }
    else {
      // Search all the nodes in the node_list for a
      // fitting sub node. If found, continue with it.
      //
      UniTrieNode<Info> **subNodePtr = &sub_node; // First one.
      while (*subNodePtr != NULL) {
	if ( (*subNodePtr)->label == uit.current32() ) {
	  // it matches, insert more here
	  uit.next32(); // move to next character
	  return (*subNodePtr)->add_to_tree( info, uit );
	}
	else if ( (*subNodePtr)->label > uit.current32() ) {
	  // we have to insert a new UniTrieNode to store info
	  UniTrieNode<Info> *tmp = *subNodePtr;
	  *subNodePtr = new UniTrieNode<Info>( uit.current32() );
	  (*subNodePtr)->next_node = tmp; // and use this als starting point
	  // for further insertions
	  uit.next32(); // move to next character
	  return (*subNodePtr)->add_to_tree( info, uit );
	}
	// we have to move to the next node
	subNodePtr = &((*subNodePtr)->next_node);
      }
      // no match, so we create a new one at the end, and continue inserting.
      //
      *subNodePtr = new UniTrieNode<Info>( uit.current32() );
      uit.next32();
      return (*subNodePtr)->add_to_tree( info, uit );
    }
  }

  template <class Info>
    inline Info *UniTrieNode<Info>::add_to_tree( Info *info,
						 const icu::UnicodeString& value ){
    /// add an Info record to the UniTrie, using the label \e value
    /*!
      \param info The information to store
      \param value a character array with a label
    */
    icu::StringCharacterIterator sit( value );
    return add_to_tree( info, sit );
  }

  // a generic UniTrie.
  template <class Info> class UniTrie;
  template <class Info> std::ostream &operator<<( std::ostream &,
						  const UniTrie<Info> * );

  /// \brief A class to store opaque data in a UniTrie
  template <class Info> class UniTrie {
    template <class U>
    friend std::ostream &operator << ( std::ostream &,
				       const UniTrie<U> * );
  public:
    UniTrie():
      Tree( new UniTrieNode<Info>( '\0' ) )
      {
      };
    ~UniTrie() {
      delete Tree;
    };
    Info *Store( const icu::UnicodeString& str, Info *info ) {
      return Tree->add_to_tree( info, str );
    };
    Info *Retrieve( const icu::UnicodeString& str ) const{
      return Tree->scan_tree( str ); };
    void ForEachDo( void F( Info *, void * ), void *arg ){
      if ( Tree ) {
	Tree->Iterate( F, arg );
      }
    };
    void ForEachDo( void F( Info * ) ) {
      if ( Tree ){
	Tree->Iterate( F );
      }
    };
  protected:
    UniTrieNode<Info> *Tree;
    UniTrie( const UniTrie& );
    UniTrie& operator=( const UniTrie& );
  };

  template <class Info>
    inline std::ostream &operator << ( std::ostream &os,
				       const UniTrie<Info> *T ){
    if ( T ){
      os << T->Tree;
    }
    return os;
  }

}
#endif
