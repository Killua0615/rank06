#ifndef MINI_DB_HPP
  #define MINI_DB_HPP

#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <fstream>
#include <unistd.h>
#include <signal.h>
#include <cstdlib>
#include <map>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>

extern std::string g_filename;
extern std::map<std::string, std::string> g_db;
extern int g_sockfd;

void save_db_and_exit(int sig);
void send_res(int fd, const std::string &msg);
void load_db(void);
int setup_server(int port);
void handle_line(int fd, const std::string &line);

#endif