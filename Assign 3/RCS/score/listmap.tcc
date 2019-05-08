// $Id: listmap.tcc,v 1.12 2019-02-21 17:28:55-08 - - $

#include "listmap.h"
#include "debug.h"

//
/////////////////////////////////////////////////////////////////
// Operations on listmap::node.
/////////////////////////////////////////////////////////////////
//

//
// listmap::node::node (link*, link*, const value_type&)
//
template <typename Key, typename Value, class Less>
listmap<Key,Value,Less>::node::node (node* next_, node* prev_,
                                     const value_type& value_):
            link (next_, prev_), value (value_) {
}

//
/////////////////////////////////////////////////////////////////
// Operations on listmap.
/////////////////////////////////////////////////////////////////
//

//
// listmap::~listmap()
//
template <typename Key, typename Value, class Less>
listmap<Key,Value,Less>::~listmap() {
   DEBUGF ('l', reinterpret_cast<const void*> (this));
   iterator itor = begin();
   while (itor != end() ) {
      node* curr = itor.get_where();
      ++itor;
      anchor()->next = itor.get_where();
      curr->next->prev = anchor();
      curr->next = nullptr;
      curr->prev = nullptr;
      delete curr;
   } 
}


//
// iterator listmap::insert (const value_type&)
//
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::iterator
listmap<Key,Value,Less>::insert (const value_type& pair) {
   DEBUGF ('l', &pair << "->" << pair);
   //find in current listmap. if exist then replace it.
   iterator front = begin();
   iterator itor = begin();
   iterator back = end();
   if( front == back ) { //list is empty
      node* new_node = new node( anchor(), anchor(), pair );
      anchor()->next =  new_node;
      anchor()->prev = new_node;
      ++itor;
      return itor;
   }
   else {
      itor = find( pair.first ); 
      if( itor != end() ) {
         //found
         itor->second = pair.second;
         return itor;
      }
      else {
         itor = begin();
         while ( itor != end() && less(itor->first, pair.first) ) {
            ++itor;
         }
         if ( itor == front ) {
            //insert at front
            node* new_node = new node( anchor()->next, anchor(), pair );
            anchor()->next->prev = new_node;
            anchor()->next = new_node;
            --itor;
         }
         else if ( itor == end() ) {
         // insert at the end
           node* new_node = new node( anchor(), anchor()->prev, pair );
           anchor()->prev->next = new_node;
           anchor()->prev = new_node;
           --itor;
         }
         else { //insert in the middle
            node* curr = itor.get_where();
            node* new_node = new node( curr, curr->prev, pair );
            curr->prev->next = new_node;
            curr->prev = new_node;
            --itor;
         }
      return itor;
      }
   }
}

//
// listmap::find(const key_type&)
//
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::iterator
listmap<Key,Value,Less>::find (const key_type& that) {
   DEBUGF ('l', that);
   iterator itor = begin();
   while( itor != end() && itor->first != that ) {
      ++itor;
   }
   return itor;
}

//
// iterator listmap::erase (iterator position)
//
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::iterator
listmap<Key,Value,Less>::erase (iterator position) {
   DEBUGF ('l', &*position);
   node* curr = position.get_where();
   curr->prev->next = curr->next;
   curr->next->prev = curr->prev;
   return position;
}


//
/////////////////////////////////////////////////////////////////
// Operations on listmap::iterator.
/////////////////////////////////////////////////////////////////
//

//
// listmap::value_type& listmap::iterator::operator*()
//
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::value_type&
listmap<Key,Value,Less>::iterator::operator*() {
   DEBUGF ('l', where);
   return where->value;
}
//
// void listmap::iterator::erase()
//
template <typename Key, typename Value, class Less>
void listmap<Key,Value,Less>::iterator::erase() {
   node* curr = get_where();
   curr->next = nullptr;
   curr->prev = nullptr;
   delete curr;
}
//
// listmap::value_type* listmap::iterator::operator->()
//
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::value_type*
listmap<Key,Value,Less>::iterator::operator->() {
   DEBUGF ('l', where);
   return &(where->value);
}

//
// listmap::iterator& listmap::iterator::operator++()
//
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::iterator&
listmap<Key,Value,Less>::iterator::operator++() {
   DEBUGF ('l', where);
   where = where->next;
   return *this;
}

//
// listmap::iterator& listmap::iterator::operator--()
//
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::iterator&
listmap<Key,Value,Less>::iterator::operator--() {
   DEBUGF ('l', where);
   where = where->prev;
   return *this;
}
template <typename Key, typename Value, class Less>
typename listmap<Key,Value,Less>::node*
listmap<Key,Value,Less>::iterator::get_where() {
   return where;
}

//
// bool listmap::iterator::operator== (const iterator&)
//
template <typename Key, typename Value, class Less>
inline bool listmap<Key,Value,Less>::iterator::operator==
            (const iterator& that) const {
   return this->where == that.where;
}

//
// bool listmap::iterator::operator!= (const iterator&)
//
template <typename Key, typename Value, class Less>
inline bool listmap<Key,Value,Less>::iterator::operator!=
            (const iterator& that) const {
   return this->where != that.where;
}

