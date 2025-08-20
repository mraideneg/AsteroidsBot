#ifndef CYGWIN_INTEROP_HPP
#define CYGWIN_INTEROP_HPP

#include <string>

enum KeyCode {
    KEY_NONE,
    KEY_UP,
    KEY_LEFT,
    KEY_RIGHT
};

bool kbhit();
void makeMove(int move, int sock_fd);

#endif
