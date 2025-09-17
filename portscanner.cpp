#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <cstring>

using namespace std;

/**
 * Attempts to connect to the specified IP and port with a timeout.
 * Uses non-blocking socket and select() to implement the timeout for connect().
 *
 * @param ip          Target IP address as string
 * @param port        Target port number
 * @param timeout_ms  Timeout in milliseconds for connection attempt (default 500ms)
 * @return            true if port is open (connection succeeded), false otherwise
 */
bool is_port_open(const string& ip, int port, int timeout_ms = 500) {
    // Create a TCP socket (IPv4)
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        return false; // Failed to create socket

    // Set socket to non-blocking mode
    int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

    // Setup target address structure
    sockaddr_in addr;
    addr.sin_family = AF_INET;                // IPv4
    addr.sin_port = htons(port);              // Convert port to network byte order
    inet_pton(AF_INET, ip.c_str(), &addr.sin_addr); // Convert IP string to binary form

    // Start connection attempt
    int result = connect(sockfd, (struct sockaddr*)&addr, sizeof(addr));
    if (result < 0) {
        // If connection in progress (expected in non-blocking mode), continue
        if (errno != EINPROGRESS) {
            close(sockfd);
            return false; // Connection failed immediately
        }
    }

    // Prepare for select(): wait for socket to become writable or timeout
    fd_set wait_set;
    FD_ZERO(&wait_set);
    FD_SET(sockfd, &wait_set);

    // Set timeout value for select()
    timeval timeout;
    timeout.tv_sec = timeout_ms / 1000;              // seconds
    timeout.tv_usec = (timeout_ms % 1000) * 1000;    // microseconds

    // Wait until socket is ready for writing (connect finished) or timeout
    result = select(sockfd + 1, nullptr, &wait_set, nullptr, &timeout);
    if (result <= 0) {
        // select() timed out or error occurred, consider port closed/unreachable
        close(sockfd);
        return false;
    }

    // Check the socket for connection errors
    int so_error;
    socklen_t len = sizeof(so_error);
    getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &so_error, &len);

    close(sockfd); // Clean up socket resource

    // If no error, connection succeeded → port is open
    return so_error == 0;
}

/**
 * Prints usage information to the console.
 *
 * @param prog_name  Name of the executable/program
 */
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
    // Show help if arguments are incorrect or help flag is passed
    if (argc != 4 || strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
        print_help(argv[0]);
        return 0;
    }

    // Parse input arguments
    string target_ip = argv[1];
    int start_port = stoi(argv[2]);
    int end_port = stoi(argv[3]);

    // Validate port range
    if (start_port < 1 || end_port > 65535 || start_port > end_port) {
        cerr << "Invalid port range. Ports must be between 1 and 65535 and start_port <= end_port.\n";
        return 1;
    }

    cout << "\nScanning ports " << start_port << " to " << end_port << " on " << target_ip << "...\n\n";

    // Iterate over each port in the range
    for (int port = start_port; port <= end_port; ++port) {
        // Check if port is open
        if (is_port_open(target_ip, port)) {
            cout << "[+] Port " << port << " is OPEN\n";
        }
        // Closed ports are silently ignored
    }

    return 0;
}
