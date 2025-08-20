// main.cpp
#include <iostream>
#include <vector>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "json.hpp"

#include "ship.hpp"
#include "asteroid.hpp"
#include "ai.hpp"
#include "constants.hpp"
#include "cygwin_interop.hpp"

using json = nlohmann::json;

int main() {

    ship_trajectory ship_traj(layers);

    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("socket failed");
        return 1;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(5000);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    if (connect(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect failed");
        close(sock_fd);
        return 1;
    }

    char buffer[4096];
    std::string recv_buffer;
    bool running = true;

    while (running) {
        if (kbhit()) {
            char c = getchar();
            if (c == 'q') break;
        }

        ssize_t bytes_read = recv(sock_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_read <= 0) {
            std::cout << (bytes_read == 0 ? "Server closed connection.\n" : "recv failed\n");
            break;
        }

        buffer[bytes_read] = '\0';
        recv_buffer += buffer;

        size_t last_newline_pos = recv_buffer.find_last_of('\n');
        if (last_newline_pos != std::string::npos) {
            std::string json_state = recv_buffer.substr(0, last_newline_pos);
            size_t prev_newline = json_state.find_last_of('\n');
            if (prev_newline != std::string::npos) {
                json_state = json_state.substr(prev_newline + 1);
            }
            recv_buffer.erase(0, last_newline_pos + 1);

            json j = json::parse(json_state);

            int num_asteroids = j["asteroids"].size();
            std::vector<asteroid_state> asteroids(num_asteroids);

            for (int i = 0; i < num_asteroids; ++i) {
                asteroids[i].pos[0] = j["asteroids"][i]["pos"][0];
                asteroids[i].pos[1] = j["asteroids"][i]["pos"][1];
                asteroids[i].vel[0] = j["asteroids"][i]["vel"][0];
                asteroids[i].vel[1] = j["asteroids"][i]["vel"][1];
                asteroids[i].rad = j["asteroids"][i]["radius"];
            }

            asteroid_trajectory ast_traj(asteroids, num_asteroids, layers);
            int move = ship_traj.best_move(
                j["ship"]["pos"][0], j["ship"]["pos"][1],
                j["ship"]["vel"][0], j["ship"]["vel"][1],
                j["ship"]["angle"], ast_traj
            );

            makeMove(move, sock_fd);
        }
    }

    close(sock_fd);
    return 0;
}
