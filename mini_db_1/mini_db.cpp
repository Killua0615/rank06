#include "mini_db.hpp"

std::string g_filename;
std::map<std::string, std::string> g_db;
int g_sockfd = -1;

void save_db_and_exit(int sig) {
  (void)sig;
  std::ofstream file(g_filename.c_str());
  if(file.is_open()) {
    std::map<std::string, std::string>::iterator it;
    for(it = g_db.begin(); it != g_db.end(); ++it)
      file << it->first << " " << it->second << "\n";
    file.close();
  }
  if(g_sockfd != -1)
    close(g_sockfd);
  exit(0);
}

void send_res(int fd, const std::string &msg) {
  send(fd, msg.c_str(), msg.length(), 0);
}

void load_db(void) {
  std::ifstream infile(g_filename.c_str());
  if(!infile.is_open())
    return;
  std::string key, val;
  while(infile >> key >> val)
    g_db[key] = val;
  infile.close();
}

int setup_server(int port) {
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if(fd = -1)
    return -1;
  struct sockaddr_in servaddr;
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htonl(port);
  servaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  if(bind(fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0 || listen(fd, 100) != 0) {
    close(fd);
    return -1;
  }
  return fd;
}

