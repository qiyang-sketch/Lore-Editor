/*
 * A text editor for fun
 * source: https://viewsourcecode.org/snaptoken/kilo/02.enteringRawMode.html
 * @author Qiyang Zhong
 */
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>

#define CTRL_KEY(k) ((k) & 0x1f)
//macro: bit operation

/*
 * @die terminates text editor
 */
void die(const char *s) {
    perror(s);
    exit(1);
}

/*
 * @disableRawMode() so program can process every input as it was typed in
 */
struct termios orig_termios;
//termios is a type from termios.h
void disableRawMode() {
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
        //tcsetattr applies argument when program exits
        //return -1 on fails
        //orig_termios to which restoring global terminal conditions go
        //assigned to @raw
        //TCSAFLUSH discard unread input
        die("tcsetattr");
}

/*
 * enableRawMode() showing every byte as we type
 */
void enableRawMode() {
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1)
        die("tcgetattr");
    ////return -1 on fails
    atexit(disableRawMode);
    // disableRowMode() called automatically when the program exits
    struct termios raw = orig_termios;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    //input flag
    //~IXON disable interrupt functions of ctrl-s ctrl-q
    raw.c_oflag &= ~(OPOST);
    //output flag
    // ~OPOST modifies newline \n
    raw.c_cflag |= (CS8);
    //control flag
    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
    //local flag ~(ECHO) disables input from displaying
    // &= bitwise-and
    //~ICANON enable reading a byte instead of a line
    //~ISIG disable terminate functions of ctrl-c ctrl-z
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;
    //setting time-out to read() uses
    //VMIN VTIME flag
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
        die("tcsetattr");
}

char editorReadKey() {
    int nread;
    char c;
    while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
        if (nread == -1 && errno != EAGAIN) die("read");
    }
    return c;
}

void editorDrawRows() {
    int y;
    for (y = 0; y < 24; y++) {
        write(STDOUT_FILENO, "~\r\n", 3);
    }
}

void editorRefreshScreen() {
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);
    editorDrawRows();
    write(STDOUT_FILENO, "\x1b[H", 3);
}



void editorProcessKeypress() {
    char c = editorReadKey();
    switch (c) {
        case CTRL_KEY('q'):
            exit(0);
            break;
    }
}

/*
 * @main() control input and flow
 */
int main() {
    //showing every byte as we type
    enableRawMode();

    while(1){
    //condition always true, infinite loop

        editorRefreshScreen();
        editorProcessKeypress();
        char c = '\0';
        if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN)
            die("read");
        //read() return number of bytes that it read
        //return -1 on fails
        if (iscntrl(c)) {
            //iscntrl() tests whether a character is a control character
            printf("%d\r\n", c);
            //use \r\n to start newline
        } else {
            printf("%d ('%c')\r\n", c, c);
        }
        if (c == CTRL_KEY('q'))
        //macro map ctrl-q to quit
            break;
        /*if(c == 'q'){
            printf("Do you want to quit?");
        }*/
    }
    return 0;
}
