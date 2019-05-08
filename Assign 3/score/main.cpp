// $Id: main.cpp,v 1.11 2018-01-25 14:19:29-08 - - $

#include <cstdlib>
#include <exception>
#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>
#include <regex>

using namespace std;

#include "listmap.h"
#include "xpair.h"
#include "util.h"

using str_str_map = listmap<string,string>;
using str_str_pair = str_str_map::value_type;

void scan_options (int argc, char** argv) {
   opterr = 0;
   for (;;) {
      int option = getopt (argc, argv, "@:");
      if (option == EOF) break;
      switch (option) {
         case '@':
            debugflags::setflags (optarg);
            break;
         default:
            complain() << "-" << char (optopt) << ": invalid option"
                       << endl;
            break;
      }
   }
}
void process(str_str_map& list, string line );

int main (int argc, char** argv) {
   sys_info::execname (argv[0]);
   scan_options (argc, argv);

   str_str_map list; // create a list.
   int line_number;
   string filename;
   if (argc > 1 ) {
      for( int i = 1; i <= argc - 1; ++i ) {
         filename = argv[i];
         //infile.open( filename, ios_base::in); //open for reading
         ifstream infile {filename};
         if( !infile ) {
            cerr << "Cannot open file " << filename <<"\n";
         } 
         else {
            string line;
            line_number = 0;
            while ( getline( infile, line ) && !infile.eof() ) {
               ++line_number;
               cout << "-" << filename << ": " << line_number
                    << ": " << line << "\n";
                   process( list, line );
            }
         }
      }
      list.~listmap();
   }
   else {
      string line;
      while (getline( cin, line) ) {
         ++ line_number;
         cout << "-:" << line_number <<": " << line <<"\n";
         process( list, line );
      }
   }
}
void process(str_str_map& list, string line) {
   regex comment {R"(^\s*#\s*(.*)\s*?$)"};
   regex blank {R"(^\s*?$)"};
   regex key_equal_value {R"(^\s*(.\w*)\s*=\s*(.*)\s*?$)"};
   regex key {R"(^\s*(.*)\s*?$)"};
   regex key_equals{R"(^\s*(.\w*|[a-z]|[A-Z])\s*[=]\s*?$)"};
   regex equals {R"(^\s*[=]\s*$)"};
   regex equals_value{R"(^\s*[=]\s*(.*)\s*?$)"};
   regex _exit{R"(^\s*exit\s*?$)"};
   smatch result;

   if( regex_search( line, result, _exit ) ) {
      list.~listmap();
      exit(0);
   } 
   if( regex_search( line, result, key ) ) {
      smatch res;
      string temp = result[1];
     
      if( regex_search( temp, res, key_equal_value )) {
         // key = value
         if( res[2] != "" ) {
            str_str_pair pair( res[1], res[2] );
            str_str_map::iterator itor = list.insert( pair );
            cout << itor->first << " = " << itor->second <<"\n";
         }
         else if( res[2] == "" ) {// key =
            str_str_map::iterator itor = list.find( res[1] );
            if( itor != list.end() ) {
               //found
               list.erase( itor );
               itor.erase();
               itor = list.begin(); //prevent invalidate itor 
            }
            else {
               cout << "Key \" " << res[1] <<" \" "
                   << "not found.\n";
            }
         }
      }
      else if( regex_search( temp, res, equals_value ) ) {
         str_str_map::iterator itor;
         for( itor = list.begin(); 
            itor != list.end(); ++itor ){
            if( itor->second == res[1] ) {
               cout << itor->first << " = "
                    << itor->second << "\n";
            }
         }
      }
      else if ( regex_search( temp, res, comment )) {
         cout << res[1] <<"\n";
      }
      else if( regex_search( temp, res, blank )) {
         cout << "";
      }
      else {
         str_str_map::iterator itor = list.find( result[1] );
         if( itor != list.end() ){
            cout << itor->first << " = " 
                 << itor->second << "\n";
         }
         else {
            cout << "Key \" " << result[1] 
                 << " \" not found.\n";
         }
      } 
   }
   if( regex_search( line, result, equals ) ) {
      str_str_map::iterator itor;
      for( itor = list.begin(); 
         itor != list.end(); ++itor ){
         cout << itor->first << " = "
              << itor->second <<"\n";
      }
   }
}
