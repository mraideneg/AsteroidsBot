#include "cygwin_interop.hpp"

#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <locale>
#include <codecvt>
#include <string>
#include <sys/socket.h>

bool kbhit() {
    // Checks if a key has been pressed
    termios oldt, newt;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    int ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF) {
        ungetc(ch, stdin);
        return true;
    }

    return false;
}

void makeMove(int move, int sock_fd) {
    // Sends the best move over TCP back to the game
    char key_char = '\0';

    switch (move) {
        case 0: key_char = 'w'; break; // thrust up
        case 1: key_char = 'a'; break; // rotate left
        case 2: key_char = 'd'; break; // rotate right
    }

    if (key_char != '\0') {
        send(sock_fd, &key_char, 1, 0); // send a single character
    }
}