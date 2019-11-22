// Quang Dang
// qvdang
// $Id: cix.cpp,v 1.9 2019-04-05 15:04:28-07 - - $

#include <iostream>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
using namespace std;

#include <libgen.h>
#include <sys/types.h>
#include <unistd.h>

#include "protocol.h"
#include "logstream.h"
#include "sockets.h"

logstream outlog (cout);
struct cix_exit: public exception {};

unordered_map<string,cix_command> command_map {
   {"error", cix_command::ERROR},
   {"get", cix_command::GET},
   {"rm", cix_command::RM},
   {"put", cix_command::PUT},
   {"exit", cix_command::EXIT},
   {"help", cix_command::HELP},
   {"ls"  , cix_command::LS},
};

static const char help[] = R"||(
exit         - Exit the program.  Equivalent to EOF.
get filename - Copy remote file to local host.
help         - Print help summary.
ls           - List names of files on remote server.
put filename - Copy local file to remote host.
rm filename  - Remove file from remote server.
)||";

void cix_help() {
   cout << help;
}

void cix_error (cix_header& header) {
   unordered_map<string, cix_command>::iterator it =
                                        command_map.begin();
   while (it != command_map.end() && it->second != header.command ){
      ++it;
   }
   if (it == command_map.end() ) {
      outlog << "Invalid header\n.";
   }
}

void cix_get (client_socket &server, string filename) {
   cix_header header;
   if ( filename.size() > 58 ) {
      outlog << "File name longer than 58 characters is prohibited.\n";
      throw cix_exit();
   }
   header.command = cix_command::GET;
   for (size_t i = 0; i <= filename.size() - 1; ++ i) {
      header.filename[i] = filename[i];
   }
   outlog << "sending header " << header << endl;
   send_packet (server, &header, sizeof header);
   recv_packet (server, &header, sizeof header);
   outlog << "received header " << header << endl;
   if (header.command != cix_command::FILEOUT) {
      outlog << "sent GET, server did not return FILEOUT" << endl;
      outlog << "server returned " << header << endl;
   }
   else {
      auto buffer = make_unique<char[]> (header.nbytes + 1);
      recv_packet (server, buffer.get(), header.nbytes);
      outlog << "received " << header.nbytes << "bytes " << endl;
      ofstream file_received (header.filename, ios::binary);
      file_received.seekp(0);
      file_received.write ( buffer.get(), header.nbytes);
  }
}
void cix_ls (client_socket& server) {
   cix_header header;
   header.command = cix_command::LS;
   outlog << "sending header " << header << endl;
   send_packet (server, &header, sizeof header);
   recv_packet (server, &header, sizeof header);
   outlog << "received header " << header << endl;
   if (header.command != cix_command::LSOUT) {
      outlog << "sent LS, server did not return LSOUT" << endl;
      outlog << "server returned " << header << endl;
   }else {
      auto buffer = make_unique<char[]> (header.nbytes + 1);
      recv_packet (server, buffer.get(), header.nbytes);
      outlog << "received " << header.nbytes << " bytes" << endl;
      buffer[header.nbytes] = '\0';
      cout << buffer.get();
   }
}

void cix_put (client_socket& server, string filename ) {
   cix_header header;
   if ( filename.size() > 58 ) {
     outlog << "File name longer than 58 characters is prohibited.\n";
      throw cix_exit();
   }
   header.command = cix_command::PUT;
   for (size_t i = 0; i <= filename.size() - 1; ++ i) {
      header.filename[i] = filename[i];
   }
   ifstream file_send (header.filename, ios::binary|ios::ate);
   if ( !file_send.is_open() ) {
      outlog << "file open fail. " << strerror (errno) << endl;
      header.nbytes = errno;
      return;
   }  
   else {
      int size = file_send.tellg();
      char *buffer = new char [size];
      file_send.read( buffer, size );
      header.nbytes = size;
      send_packet (server, &header, sizeof header);
      send_packet (server, &buffer, size);
      recv_packet (server, &header, sizeof header);
      file_send.close();
      delete[] buffer;
   }
   if (header.command == cix_command::NAK) {
      outlog << "sent PUT, server did not return ACK." << endl;
      outlog << "server returned: " << header << endl;
   }
   else if (header.command == cix_command::ACK ) {
      outlog << "Put file successed." << endl;
   }
}

void cix_rm (client_socket& server, string filename) {
   cix_header header;
   if ( filename.size() > 58 ) {
     outlog << "File name longer than 58 characters is prohibited.\n";
      throw cix_exit();
   }
   for (size_t i = 0; i <=filename.size() - 1; ++i ) {
      header.filename[i] = filename[i];
   }
   header.command = cix_command::RM;
   header.nbytes = 0;
   send_packet (server, &header, sizeof header);
   recv_packet (server, &header, sizeof header);
   if (header.command == cix_command::NAK) {
      outlog << "sent RM, server did not return ACK." << endl;
      outlog << "server returned: " << header << endl;
   }
   else if (header.command == cix_command::ACK) {
      outlog << "rm file successed. " << endl;
   }
}
void usage() {
   cerr << "Usage: " << outlog.execname() << " [host] [port]" << endl;
   throw cix_exit();
}

int main (int argc, char** argv) {
   outlog.execname (basename (argv[0]));
   outlog << "starting" << endl;
   vector<string> args (&argv[1], &argv[argc]);
   if (args.size() > 2) usage();
   string host = get_cix_server_host (args, 0);
   in_port_t port = get_cix_server_port (args, 1);
   outlog << to_string (hostinfo()) << endl;
   try {
      outlog << "connecting to " << host << " port " << port << endl;
      client_socket server (host, port);
      outlog << "connected to " << to_string (server) << endl;
      for (;;) {
         string line;
         string token;
         vector <string> command;
         getline (cin, line);
         stringstream _line (line);
         while (getline (_line, token, ' ')) {
            command.push_back (token);
         } 
         if (cin.eof()) throw cix_exit();
         outlog << "command " << line << endl;
         const auto& itor = command_map.find (command[0]);
         cix_command cmd = itor == command_map.end()
                         ? cix_command::ERROR : itor->second;
         switch (cmd) {
            case cix_command::EXIT:
               throw cix_exit();
               break;
            case cix_command::HELP:
               cix_help();
               break;
            case cix_command::LS:
               cix_ls (server);
               break;
            case cix_command::GET:
               cix_get (server, command[1]);
               break;
            case cix_command::PUT:
               cix_put (server, command[1]);
               break;
            case cix_command::RM:
               cix_rm (server, command[1]);
               break;
            default:
               outlog << line << ": invalid command" << endl;
               break;
         }
      }
   }catch (socket_error& error) {
      outlog << error.what() << endl;
   }catch (cix_exit& error) {
      outlog << "caught cix_exit" << endl;
   }
   outlog << "finishing" << endl;
   return 0;
}

