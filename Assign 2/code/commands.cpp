// $Id: commands.cpp,v 1.19 2019-04-25 11:52:22-07 - - $

#include "commands.h"
#include "debug.h"

command_hash cmd_hash {
   {"cat"   , fn_cat   },
   {"cd"    , fn_cd    },
   {"echo"  , fn_echo  },
   {"exit"  , fn_exit  },
   {"ls"    , fn_ls    },
   {"lsr"   , fn_lsr   },
   {"make"  , fn_make  },
   {"mkdir" , fn_mkdir },
   {"prompt", fn_prompt},
   {"pwd"   , fn_pwd   },
   {"rm"    , fn_rm    },
   {"rmr"   , fn_rmr   },
};

command_fn find_command_fn (const string& cmd) {
   // Note: value_type is pair<const key_type, mapped_type>
   // So: iterator->first is key_type (string)
   // So: iterator->second is mapped_type (command_fn)
   DEBUGF ('c', "[" << cmd << "]");
   const auto result = cmd_hash.find (cmd);
   if (result == cmd_hash.end()) {
      execname( "yshell" );
      throw command_error (cmd + ": no such function");
   }
   return result->second;
}

command_error::command_error (const string& what):
            runtime_error (what) {
}

int exit_status_message() {
   int exit_status = exit_status::get();
   cout << execname() << ": exit(" << exit_status << ")" << endl;
   return exit_status;
}

void fn_cat (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   inode_ptr working_dir;
   map<string, inode_ptr>::iterator it;
   string filename;
   if (words.size() <= 1 ) {
      execname( "cat" );
      throw command_error ("Syntax error!");
   } 
   else {
      for( long unsigned i = 1; i <= words.size()- 1; ++i ) {
         if( words.at(i).size() == 1 ) {
            working_dir = state.get_cwd();
            filename = words.at(i);
         }
         else {
            string dest = "";
            if( words.at(i)[0] == '/' ) {
               dest = dest + "/";
            }
            wordvec path = split( words.at(i), "/" );
 
            if (path.size() == 1) {
               filename = path.at( path.size() - 1 );
            }
            else if( path.size() > 1 ){
               filename = path.at( path.size() - 1 );
               for( long unsigned j = 0; j <= path.size() - 2; ++j ){
                  dest = dest + path.at(j) + "/";
               }
            }
            working_dir = state.get_dest( dest );
            if( working_dir == nullptr ) {
               return;
            }
         }
         //find file in working_dir;
         map<string, inode_ptr> current_dirents;
         current_dirents = working_dir->get_contents()->get_dirents();
         it = current_dirents.find( filename );
         if( it != current_dirents.end() && 
              it->second->get_type() == "FILE" ){
             wordvec contents = it->second->get_contents()->readfile();
             if( contents.size() > 0 ) {
                for (long unsigned k = 0; 
                      k <= contents.size() - 1; ++k ) {
                   cout << contents.at(k) << " ";
                }
                cout << "\n";              
             }
          }
          else {
              execname( "cat" );
              throw command_error(filename + 
                  ": No such file or directory");
          }
      }
   } 
}

void fn_cd (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   inode_ptr working_dir;
   string path;
   bool flag = true;
   if( words.size() == 1 ) {
      state.set_cwd( state.get_root() );
   }
   else {
      path = words[1];
      if( path[0] == '/' ) {
         working_dir = state.get_root();
      }
      else {
         working_dir = state.get_cwd();
      }
      wordvec directories = split(path, "/");
      map<string, inode_ptr> current_dirents;
      map<string, inode_ptr>::iterator it ;
      if( directories.size() > 0 ) {
         for( long unsigned i = 0; i <= directories.size() - 1; ++i ) {
            current_dirents = 
               working_dir->get_contents()->get_dirents();
            it = current_dirents.find( directories.at(i) );
            if ( it != current_dirents.end() ) {
               working_dir = it->second;
            } 
            else {
               flag = false;
               execname( "cd" );
               throw command_error( directories.at(i) + ":" +
                                    " No such file or directory");
            }       
         }
      }
      if( flag ) {
         state.set_cwd( working_dir );
      }
   }
}
void fn_echo (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   cout << word_range (words.cbegin() + 1, words.cend()) << endl;
}


void fn_exit (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   //set the exit status;
   int status;
   if( words.size() > 1 ) {
      status = atoi( words.at(1).c_str() );
   }
   else {
      status = 0;
   }
   exit_status::set(status);
   exit_status_message();
   exit(status);
}

void fn_ls (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   inode_ptr  working_dir;
   string dest= "";
   bool flag = true;
   if( words.size() == 1 ) {
      working_dir = state.get_cwd();
      dest = working_dir->get_name();
      flag = true;
   }
   else {
      string path = words[1];
      dest = path;
      if( path[0] =='/' ) {
         working_dir = state.get_root();
      }
      else {
         working_dir = state.get_cwd();
      }
      if( path.size() > 1 ) {
         wordvec dirs = split(path, "/");
         map<string, inode_ptr>::iterator it;
         for( long unsigned i = 0; i <= dirs.size()-1; ++i) {
            map<string, inode_ptr> temp;
            temp = working_dir->get_contents()->get_dirents();
            it = temp.find( dirs.at(i));
            if( it == temp.end()) {
               throw command_error("There is no such directory.");
               flag = false;
            }
            else {
               working_dir = it->second;
            } 
        }
     } 
   }
   if( flag ) {
      map<string, inode_ptr> print_map = 
            working_dir->get_contents()->get_dirents();
      map<string, inode_ptr>::iterator it;
      cout << dest << ":\n";
      for (it = print_map.begin(); it != print_map.end(); ++it){
         cout << "      "<< it->second->get_inode_nr() 
              << "         "
              << it->second->get_contents()->size()
              << "    "  << it->first;
         if( it->second->get_type() == "DIRECTORY" && 
              ( it->first != "." && it->first != ".." ) ) {
            cout << "/";
         }
         cout << "\n";
      }
   }
}

void fn_lsr (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   inode_ptr working_dir;
   string dest = "";
   map<string, inode_ptr>::iterator it;
   map<string, inode_ptr> current_dirents;
   inode_ptr subdir;
   wordvec recur_call;
   recur_call.push_back("lsr");
   
   for( long unsigned i = 1; i <= words.size() - 1; ++i ) {
      dest = words[i];
      working_dir = state.get_dest( dest );
   //call function ls
   //to ls the current directory
      recur_call.push_back( dest );
      fn_ls( state, recur_call );
      current_dirents = working_dir->get_contents()->get_dirents();
      //go deeper if there are subdirectories 
      for( it = current_dirents.begin();
         it != current_dirents.end(); ++it ) {
         if( (it->second->get_type() == "DIRECTORY")
             && (it->first != ".") && (it->first != "..") ) {
             if( dest.size() ==1 && dest.at(0) == '/' ){
                dest = dest + it->first;
             }
             else {
                dest = dest + "/" + it->first;
             }
          //update the dest and make a recursive call
             recur_call.pop_back();
             recur_call.push_back( dest );
             fn_lsr( state, recur_call );
         }
      }
   }
}
void fn_make (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   inode_ptr working_dir;
   bool flag = true;
   map<string, inode_ptr>::iterator it;
   map<string, inode_ptr> current_dirents;
   if( words.size() == 1 ) {
      execname( "make" );
      throw command_error("Syntax error!");
   }
   else {
      if( words[1].at(0) == '/') {
         working_dir = state.get_root();
      }
      else {
         working_dir = state.get_cwd();
      }
      wordvec path = split( words[1], "/" );
      string filename;
      if( path.size() == 1 ) {
         filename = path.at(0);
      }
      else if( path.size() > 1 ) {
         filename = path.at( path.size()-1 );
         for( long unsigned i = 0; i <= path.size() - 2; ++i ){
            current_dirents = 
                working_dir->get_contents()->get_dirents();
            it = current_dirents.find( path.at(i) );
           
            if( it != current_dirents.end() && 
                (it->second->get_type() == "DIRECTORY") ) {
               working_dir = it->second;
            }
            else {
               flag = false;
               execname( "make" );
               throw command_error(path.at(i) + ": " +
                     "No such file or directory.");
            }
         } 
      }
      wordvec contents;
      if( words.size() > 2 ) {
         for( long unsigned i = 2; i <= words.size() -1 ; ++i) {
            contents.push_back( words.at(i) );   
         }
      }
      //create a file
      if( flag ) {
         //check if there is exit the directory which has 
         //the same name
         current_dirents = working_dir->get_contents()->get_dirents();
         it = current_dirents.find( filename );
         if( it != current_dirents.end() ) { // found
            if( it->second->get_type() == "DIRECTORY" ) {
               execname( "make" );
               throw command_error( filename + ": " +
                       " is a directory.");
            }
             else {
                working_dir
                   ->get_contents()->mkfile( filename, contents ); 
             }
          }
          else { //not found  
             working_dir->get_contents()->mkfile( filename, contents ); 
          }
      }
   }
}
void fn_mkdir (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   string path = words[1];
   bool flag = true;
   inode_ptr working_dir; 
   //start from root
   if( path[0] == '/' ) {
      working_dir = state.get_root();
   }
   else {
      working_dir = state.get_cwd();
   }
   wordvec multi_dir = split( path, "/" );
   if (multi_dir.size() > 1 ) {
         shared_ptr< base_file > temp;
         map<string, inode_ptr>::iterator it;
      for (long unsigned i = 0; i <= multi_dir.size() - 2; ++i) {
         temp = working_dir->get_contents();
         map<string, inode_ptr> my_dirents = temp->get_dirents();
         it = my_dirents.find(multi_dir.at(i));
         if( it != my_dirents.end() ) {
            working_dir = it->second;
         }  
         else {
            throw command_error(multi_dir[i] +":" 
                 + " No such directory or file.");
            flag = false;
         }
      }
   }
   else if ( multi_dir.size() == 0 ) {
      flag = false;
      execname("mkdir");
      throw command_error( "Syntax error." );
   }
   //create a directory
   if( flag ) {
      shared_ptr<base_file> new_dir;
      new_dir = working_dir->get_contents();
      new_dir->mkdir( multi_dir[ multi_dir.size() -1 ], working_dir );
   }
}

void fn_prompt (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   string str = "";
   if( words.size() > 1 ) {
      for (unsigned long i = 1;  i <= words.size() - 1; ++i){
         str = str + words.at(i)+ " "; 
      }
      state.set_prompt( str );
   }
}

void fn_pwd (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   
   inode_ptr working_dir = state.get_cwd();
   inode_ptr parents;
   string current_name = working_dir->get_name();
 
   wordvec path;
   path.push_back( current_name );

   map<string, inode_ptr>::iterator it;

   map<string, inode_ptr> current_dirents;
   current_dirents = working_dir->get_contents()->get_dirents();
    
   it = current_dirents.find("..");
   parents = it->second;
   while (parents != state.get_root() ) {
      working_dir = parents;
      current_name = working_dir->get_name();
      path.push_back( current_name );
 
      current_dirents = working_dir->get_contents()->get_dirents();
      it = current_dirents.find("..");
      parents = it->second;
   }
   //add root to vector name
   if( parents != working_dir ) {
      current_name = parents->get_name();
      path.push_back( current_name );
   }
   for( int i = path.size() - 1; i >= 0; i--) {
      cout<< path.at(i);
      int j = path.size() - 1;
      if( path.at(i) != "/" && (i < j - 1) ) {
         cout <<"/";
      }
   }
   cout<<"\n";
}

void fn_rm (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   map<string, inode_ptr>::iterator it;
   map<string, inode_ptr> current_dirents;
   inode_ptr working_dir;
   wordvec path;
   string dest = "";
   string target;

   if( words.size() == 1 ) {
      execname( "rm" );
      throw command_error("Syntax error!");
   }
   else{
      path = split( words.at(1), "/" );
      if( words.at(1).at(0) == '/' ) {
         dest = dest + "/";
      }
      if( path.size() ==1 ) {
         target = path.at(0);
      }
      else if( path.size() > 1 ){
         target = path.at( path.size() -1 );
         for( long unsigned i = 0; i <= path.size() - 2; ++i) {
            dest = dest + path.at(i) + "/";
         }
      }
      working_dir = state.get_dest( dest );
      current_dirents = 
           working_dir->get_contents()->get_dirents();
      for( it = current_dirents.begin(); 
           it != current_dirents.end(); ++it) {
         it = current_dirents.find( target );
         if( it != current_dirents.end() ) {
            if( it->second->get_type() == "DIRECTORY" ) {
               execname( "rm" );
               throw command_error("Cannot remove a directory."); 
            }
            else if( it->second->get_type() == "FILE" ) {
               it = current_dirents.end();
               working_dir->get_contents()->remove( target );
            }
         }
         else {
            execname("rm");
            throw command_error( "No file '" + target +
                    "' has found.");
         }
      }
   }
}
void fn_rmr (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   map< string, inode_ptr >::iterator it;
   map< string, inode_ptr >current_dirents;
   map<string, inode_ptr> target_dirents;
   string target;
   string dest = "";
   wordvec new_dest;
   new_dest.push_back( "rmr" );
   wordvec path;
   inode_ptr working_dir;   
   inode_ptr _target;
   
   if( words.size() == 1 ) {
       execname( "rmr" );
       throw command_error( "Syntax error!" );
   }
   else {
      if( words.at(1).at(0) =='/' ) {
         dest = dest + "/";
      }
      path = split( words.at(1), "/");
      if( path.size() == 1 ) {
         target = path.at(0);
      }
      else if( path.size() > 1 ) {
         target = path.at( path.size() - 1 );
         for( unsigned long i = 0; i <= path.size() - 2; ++i ) {
            dest = dest + path.at(i) + "/";
         }
      }
      working_dir = state.get_dest( dest );
      current_dirents = working_dir->get_contents()->get_dirents();
      it = current_dirents.find( target );
      if( it != current_dirents.end() ) {
         if( it->second->get_type() == "FILE" ) {
            working_dir->get_contents()->remove( target );
         }
         else if( it->second->get_type() =="DIRECTORY" ) {
            //Check whether subdir is empty or not
            _target = it->second;
            target_dirents = _target->get_contents()->get_dirents();
            if( target_dirents.size() == 2 ) { // it is empty
               for( it = target_dirents.begin(); 
                    it != target_dirents.end(); ++it ){
                  it->second = nullptr;//free pointers dot and dotdot
               }
               //delete subdir from the map.
                working_dir->get_contents()->remove( target );
            }
            else { //it is not empty
               //remove everything inside first
               for( it = target_dirents.begin();  
                    it != target_dirents.end(); ++it) {
                  if( it->first == ".." || it->first == "." ){
                     it->second = nullptr;
                  }
                  if( it->first != ".." || it->first != "." ){
                     new_dest.push_back( dest + "/" + it->first );
                    // recursive call
                     fn_rmr( state, new_dest );
                  }
               }
             // remove current directory
             it = current_dirents.find( target );
             if( it != current_dirents.end() ) {
                for( it = current_dirents.begin();
                   it != current_dirents.end(); ++it ){
                   it->second = nullptr;        
                }
              working_dir->get_contents()->remove( target );
              ++it;
              }
            }
         }
      }
   }
}

