#include <vsu/plt/util/terminal-input.h>
#include <err.h>

static inline void set_stdin_mode(vsuplt_term_input_ptr in)
{
	struct termios t;
	/* retrieve current terminal attribute s */
	if (tcgetattr(STDIN_FILENO, &t) < 0)
		err(1, "tcgetattr() failed;");
	in->saved_stdin_tattrs = t;
	/* disable canonical mode and echoing */
	t.c_lflag &= ~(ECHO | ICANON);
	/* set min != 0 and time=0,
	 * so that read() waits until there are at least min bytes in the queue
	 */ 
	t.c_cc[VMIN] = 1;
	t.c_cc[VTIME] = 0;

	/* commit changes */
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &t) < 0)
		err(1, "tcsetattr() failed;");
}

static inline void reset_stdin_mode(vsuplt_term_input_ptr cfg)
{
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &cfg->saved_stdin_tattrs) < 0)
		err(1, "tcsetattr(): reset stdin");
}

void
vsuplt_term_input(vsuplt_term_input_ptr cfg)
{
    char c;
	set_stdin_mode(cfg);
	while (read(STDIN_FILENO, &c, 1) > 0)
        if (cfg->handlers[(int)c] != NULL)
            (*(cfg->handlers[(int)c]))(cfg, c);
	reset_stdin_mode(cfg);
}
