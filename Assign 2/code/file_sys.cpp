// $Id: file_sys.cpp,v 1.7 2019-04-25 11:45:09-07 - - $1

#include <iostream>
#include <stdexcept>
#include <unordered_map>
#include <typeinfo>
using namespace std;

#include "debug.h"
#include "file_sys.h"
#include "commands.h"

int inode::next_inode_nr {1};
inode::inode(file_type type): inode_nr (next_inode_nr++) {
   switch (type) {
      case file_type::PLAIN_TYPE:
           contents = make_shared<plain_file>();
           break;
      case file_type::DIRECTORY_TYPE:
           contents = make_shared<directory>();
           break;
     //default: assert( false );
   }
   DEBUGF ('i', "inode " << inode_nr << ", type = " << type);
}

shared_ptr< base_file > inode::get_contents() {
   return contents;
}
void inode::set_name( const string& dirname ) {
   name = dirname;
}

string inode::get_name() {
   return name;
}
void inode::set_type( const string& type_name ) {
   _type = type_name;
}
string inode::get_type() {
   return _type;
}
int inode::get_inode_nr() const {
   DEBUGF ('i', "inode = " << inode_nr);
   return inode_nr;
}


file_error::file_error (const string& what):
            runtime_error (what) {
}

struct file_type_hash {
   size_t operator() (file_type type) const {
      return static_cast<size_t> (type);
   }
};

ostream& operator<< (ostream& out, file_type type) {
   static unordered_map<file_type,string,file_type_hash> hash {
      {file_type::PLAIN_TYPE, "PLAIN_TYPE"},
      {file_type::DIRECTORY_TYPE, "DIRECTORY_TYPE"},
   };
   return out << hash[type];
}

inode_state::inode_state() {
   DEBUGF ('i', "root = " << root << ", cwd = " << cwd
          << ", prompt = \"" << prompt() << "\"");
   //create root inode as directory contains dot and dotdot
   root = make_shared< inode >(file_type::DIRECTORY_TYPE);
   root->set_name( "/" );
   
   root->get_contents()->set_dirents(".", root);
   root->get_contents()->set_dirents("..", root);

   map<string, inode_ptr> temp = root->get_contents()->get_dirents();
   cwd = root;   
}

inode_ptr inode_state::get_root() {
   return root;
}

inode_ptr inode_state::get_cwd() {
   return cwd;
}

inode_ptr inode_state::get_dest( const string& dest ) {
   inode_ptr working_dir;
   map<string, inode_ptr>::iterator it;
   bool flag = true;
   if( dest.size() == 0 ) {
      working_dir = cwd;
   }
   else if (dest.size() >= 1 ) {
      if( dest.at(0) =='/' ) {
         working_dir = root;
      }
      else {
         working_dir = cwd;
      }
   
      wordvec path = split( dest, "/" );
      map<string, inode_ptr> current_dirents;
      if( path.size() > 0 ) {
         for( long unsigned i = 0; i <= path.size() -1; ++i ) {
            current_dirents = 
               working_dir->get_contents()->get_dirents();
            it = current_dirents.find( path.at(i) );
            if( it != current_dirents.end() 
                && it->second->get_type() == "DIRECTORY" ) {
                working_dir = it->second;
            }
            else {
               cout << "There is no such directory '" 
                    << path.at(i) << "'\n";
               flag = false;
               break;
            }
        }
     }
   }
   if( flag ) {
      return working_dir;
   }
   else {
      return nullptr;
   }
}
void inode_state::set_cwd( inode_ptr current ) {
   cwd = current;
}
const string& inode_state::prompt() const { return prompt_; }
void inode_state::set_prompt( const string& str ) {
   prompt_ = str;
}
ostream& operator<< (ostream& out, const inode_state& state) {
   out << "inode_state: root = " << state.root
       << ", cwd = " << state.cwd;
   return out;
}

size_t plain_file::size() const {
   size_t size {0};
   DEBUGF ('i', "size = " << size);
   string str = "";
   for( long unsigned i = 0; i <= data.size() -1; ++i) {
      str = str + data.at(i) + " ";
   }
   size = str.size()-1;
   return size;
}

const wordvec& plain_file::readfile() const {
   DEBUGF ('i', data);
   return data;
}

void plain_file::writefile (const wordvec& words) {
   DEBUGF ('i', words);
   data = words;
}

void plain_file::remove (const string&) {
   throw file_error ("is a plain file");

}

inode_ptr plain_file::mkdir (const string&, inode_ptr) {
   throw file_error ("is a plain file");
}

inode_ptr plain_file::mkfile (const string&, const wordvec&) {
   
   throw file_error ("file cannot create itself");
}

void plain_file::set_dirents( const string& , inode_ptr ) {
   throw file_error( "is a plain file");
}

map<string, inode_ptr>& plain_file::get_dirents() {
   throw file_error( "is a plain file");
}
directory::directory() {
   inode_ptr ptr = NULL;
   dirents.insert( make_pair(".", ptr) ); 
   dirents.insert( make_pair( "..", ptr ) );
}
size_t directory::size() const {
   size_t size {0};
   DEBUGF ('i', "size = " << size);
   size = dirents.size();
   return size;
}

const wordvec& directory::readfile() const {
   throw file_error ("is a directory");
}

void directory::writefile (const wordvec&) {
   throw file_error ("is a directory");
}

void directory::remove (const string& filename) {
   DEBUGF ('i', filename);
   map<string, inode_ptr>::iterator it;
   it = dirents.find( filename );
   if( it != dirents.end() ) {
      //set pointer pointed file's object to nullptr
     it->second = nullptr;
      // delete its position in the map.
      it = dirents.erase( it );
   }
}

inode_ptr directory::mkdir (const string& dirname, 
                              inode_ptr parents ) {
   DEBUGF ('i', dirname);
   inode_ptr new_dir = nullptr;
   //check for existing directory
   map<string, inode_ptr>::iterator it;
   it = dirents.find( dirname );
   if( it != dirents.end() ) {
      throw command_error("The directory '"
         + dirname + "' has already existed.");
   }
   else {
      new_dir = 
        make_shared< inode >(file_type::DIRECTORY_TYPE);
      inode_ptr dotdot = parents;
      inode_ptr dot = new_dir;
   
      new_dir->set_name( dirname );
      new_dir->set_type( "DIRECTORY" );     

      new_dir->get_contents()->set_dirents( ".", dot);
      new_dir->get_contents()->set_dirents( "..", dotdot );
      //add child directory to current directory
      dirents.insert( make_pair( dirname, new_dir ) );
   }
   return new_dir;
}


inode_ptr directory::mkfile (const string& filename, 
                             const wordvec& content) {
   DEBUGF ('i', filename);
   //check for existing file
   map<string, inode_ptr>::iterator it;
   it = dirents.find( filename );
   if( it != dirents.end() ) {
      inode_ptr temp = it->second;
      temp->get_contents()->writefile( content );
      return temp;
   }
   else {
      inode_ptr new_file = 
          make_shared< inode > (file_type::PLAIN_TYPE);
      new_file->set_name( filename );
      new_file->set_type( "FILE" );
      
      new_file->get_contents()->writefile( content );
      dirents.insert( make_pair(filename, new_file) );
      return new_file;
   }
}
void directory::set_dirents( 
          const string& str, inode_ptr ptr ) {
    map<string, inode_ptr>::iterator it;
    it = dirents.find( str );
    if (it != dirents.end() ) {
        it->second = ptr;
    } 
    else {
        dirents.insert( make_pair(str, ptr) );
    }
}

map<string, inode_ptr>& directory::get_dirents() {
   return dirents;
}
