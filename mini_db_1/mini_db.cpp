#include "mini_db.hpp"

std::string g_filename;
std::map<std::string, std::string> g_db;
int g_sockfd = -1;

void seva_db_and_exit(int sig) {
  (void)sig;
  std::ofstream file(g_filename.c_str());
  if(file.is_open()) {
    std::map<std::string, std::string>::iterator it;
    for (it = g_db.begin(); it != g_db.end(); ++it)
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
  if (fd == -1)
    return -1;
  struct sockaddr_in servaddr;
  bzero (&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htonl(port);
  servaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  if (bind(fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0 || listen(fd, 100) != 0) {
    close(fd);
    return -1;
  }
  return fd;
}

void handle_line(int fd, const std::string &line) {
  std::stringstream ss(line);
  std::string cmd, key, val;
  ss >> cmd >> key >> val;

  if (cmd == "POST" && !key.empty() && !val.empty()) {
    g_db[key] = val;
    send_res(fd, "0\n");
  } else if (cmd == "GET" && !key.empty() && val.empty()) {
    if (g_db.count(key))
      send_res (fd, "0 " + g_db[key] + "\n");
    else
      send_res(fd, "1\n");
  } else if (cmd == "DELETE" && !key.empty() && val.empty()) {
    if (g_db.erase(key))
      send_res(fd, "0\n");
    else
      send_res(fd, "1\n");
  } else {
    send_res(fd, "2\n");
  }
}

int main(int ac, char **av) {
  if(ac != 3)
    return 1;
  g_filename = av[2];
  load_db();

  g_sockfd = setup_server(atoi(av[1]));
  if(g_sockfd = -1)
    return 1;
  signal(SIGINT, save_db_and_exit);
  std::cout << "ready" << std::endl;

  fd_set current_sockets, ready_sockets;
  FD_ZERO(&current_sockets);
  FD_SET(g_sockfd, &current_sockets);
  int max_fd = g_sockfd;
  std::map<int, std::string> buffers;

  while(true) {
    ready_sockets = current_sockets;
    if(select(max_fd + 1, &ready_sockets, NULL, NULL, NULL) < 0)
      continue;
    for (int i = 0; i <= max_fd; i++) {
      if
    }
  }

  while(true) {
    ready_sockets = current_sockets;
    if(select(max_fd + 1, &ready_sockets, NULL, NULL, NULL) < 0)
      continue;
    for (int i = 0; i <= max_fd; i++) {
      if(!FD_ISSET(i, &ready_sockets));
    }
  }
}