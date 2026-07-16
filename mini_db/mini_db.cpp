#include "mini_db.hpp"

std::string g_filename;
std::map<std::string, std::string> g_db;
int g_sockfd = -1;

/* ---------- SIGINT: 保存して終了 ---------- */
void save_db_and_exit(int sig) {
	(void)sig;
	std::ofstream file(g_filename.c_str());
	if (file.is_open()) {
		std::map<std::string, std::string>::iterator it;
		for (it = g_db.begin(); it != g_db.end(); ++it)
			file << it->first << " " << it->second << "\n";
		file.close();
	}
	if (g_sockfd != -1)
		close(g_sockfd);
	exit(0);
}

/* ---------- 送信 ---------- */
void send_res(int fd, const std::string &msg) {
	send(fd, msg.c_str(), msg.length(), 0);
}

/* ---------- 起動時ロード ---------- */
void load_db(void) {
	std::ifstream infile(g_filename.c_str());
	if (!infile.is_open())
		return;
	std::string key, val;
	while (infile >> key >> val)
		g_db[key] = val;
	infile.close();
}

/* ---------- socket / bind / listen ---------- */
int setup_server(int port) {
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd == -1)
		return -1;
	struct sockaddr_in servaddr;
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);  // 127.0.0.1
	servaddr.sin_port = htons(port);
	if (bind(fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0
		|| listen(fd, 100) != 0) {
		close(fd);
		return -1;
	}
	return fd;
}

/* ---------- 1行 = 1コマンド ---------- */
void handle_line(int fd, const std::string &line) {
	std::stringstream ss(line);
	std::string cmd, key, val;
	ss >> cmd >> key >> val;

	if (cmd == "POST" && !key.empty() && !val.empty()) {
		g_db[key] = val;
		send_res(fd, "0\n");
	} else if (cmd == "GET" && !key.empty() && val.empty()) {
		if (g_db.count(key))
			send_res(fd, "0 " + g_db[key] + "\n");
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

/* ---------- main ---------- */
int main(int ac, char **av) {
	if (ac != 3)
		return 1;
	g_filename = av[2];
	load_db();

	g_sockfd = setup_server(atoi(av[1]));
	if (g_sockfd == -1)
		return 1;
	signal(SIGINT, save_db_and_exit);
	std::cout << "ready" << std::endl;

	fd_set current_sockets, ready_sockets;
	FD_ZERO(&current_sockets);
	FD_SET(g_sockfd, &current_sockets);
	int max_fd = g_sockfd;
	std::map<int, std::string> buffers;

	while (true) {
		ready_sockets = current_sockets;               // 毎回コピー
		if (select(max_fd + 1, &ready_sockets, NULL, NULL, NULL) < 0)
			continue;
		for (int i = 0; i <= max_fd; i++) {
			if (!FD_ISSET(i, &ready_sockets))
				continue;

			if (i == g_sockfd) {                        // 新規接続
				int client_fd = accept(g_sockfd, NULL, NULL);
				if (client_fd >= 0) {
					FD_SET(client_fd, &current_sockets);
					if (client_fd > max_fd)
						max_fd = client_fd;
					buffers[client_fd] = "";
				}
			} else {                                    // 受信
				char buf[1025];
				int bytes = recv(i, buf, 1024, 0);
				if (bytes <= 0) {                       // 切断3点セット
					close(i);
					FD_CLR(i, &current_sockets);
					buffers.erase(i);
				} else {
					buf[bytes] = 0;
					buffers[i] += buf;                  // 溜める
					size_t pos;
					while ((pos = buffers[i].find('\n')) != std::string::npos) {
						std::string line = buffers[i].substr(0, pos);
						buffers[i].erase(0, pos + 1);
						if (!line.empty() && line[line.length() - 1] == '\r')
							line.erase(line.length() - 1);
						handle_line(i, line);
					}
				}
			}
		}
	}
	return 0;
}