#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <cstring>

using namespace std;

bool is_port_open(const string& ip, int port, int timeout_ms = 500) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) return false;

    // Set socket non-blocking
    int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);

    int result = connect(sockfd, (struct sockaddr*)&addr, sizeof(addr));
    if (result < 0) {
        if (errno != EINPROGRESS) {
            close(sockfd);
            return false;
        }
    }

    fd_set wait_set;
    FD_ZERO(&wait_set);
    FD_SET(sockfd, &wait_set);

    timeval timeout;
    timeout.tv_sec = timeout_ms / 1000;
    timeout.tv_usec = (timeout_ms % 1000) * 1000;

    result = select(sockfd + 1, nullptr, &wait_set, nullptr, &timeout);
    if (result <= 0) {  // timeout or error
        close(sockfd);
        return false;
    }

    int so_error;
    socklen_t len = sizeof(so_error);
    getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &so_error, &len);

    close(sockfd);
    return so_error == 0;
}

void print_help(const char* prog_name) {
    cout << "Simple Port Scanner\n";
    cout << "Usage:\n";
    cout << "  " << prog_name << " <target_ip> <start_port> <end_port>\n";
    cout << "Options:\n";
    cout << "  -h, --help     Show this help message\n";
    cout << "\nExample:\n";
    cout << "  " << prog_name << " 192.168.1.1 20 100\n";
}

int main(int argc, char* argv[]) {
    if (argc != 4 || strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
        print_help(argv[0]);
        return 0;
    }

    string target_ip = argv[1];
    int start_port = stoi(argv[2]);
    int end_port = stoi(argv[3]);

    if (start_port < 1 || end_port > 65535 || start_port > end_port) {
        cerr << "Invalid port range. Ports must be between 1 and 65535 and start_port <= end_port.\n";
        return 1;
    }

    cout << "\nScanning ports " << start_port << " to " << end_port << " on " << target_ip << "...\n\n";

    for (int port = start_port; port <= end_port; ++port) {
        if (is_port_open(target_ip, port)) {
            cout << "[+] Port " << port << " is OPEN\n";
        }
    }

    return 0;
}
