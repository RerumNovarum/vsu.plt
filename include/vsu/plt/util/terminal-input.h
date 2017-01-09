#ifndef _VSUPLT_TERMINAL_INPUT_H_
#define _VSUPLT_TERMINAL_INPUT_H_

#include <fcntl.h>
#include <unistd.h>
#include <termios.h> /* to turn off canonical mode */

struct vsuplt_term_input;
typedef struct vsuplt_term_input * vsuplt_term_input_ptr;

typedef void (*vsuplt_term_key_handler)(
    vsuplt_term_input_ptr in_cfg,
    char c);

struct vsuplt_term_input {
    void *self;
	struct termios saved_stdin_tattrs;
    vsuplt_term_key_handler handlers[256];
};

void
vsuplt_term_input(vsuplt_term_input_ptr cfg);

#endif /* ifndef  _VSUPLT_TERMINAL_INPUT_H_ */
