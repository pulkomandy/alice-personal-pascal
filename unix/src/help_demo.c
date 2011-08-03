
#include <stdio.h>
#include <io.h>
#inlcude <dev.h>

#define HELP_PORT	23
#define CTRL_C		0x03

main()
{
	unsigned tid;
	unsigned num_signals;
	unsigned old_options;
	int	c;

	/*
	 * Attach to HELP port if possible
	 */
	tid = attach(HELP_PORT);

	if (tid != 0) {
		tfprintf(stderr, "Help port in use by task: %04x\n", tid);
		exit(-1);
	}

	/*
	 * Turn off keyboard ECHO and EDIT options.
	 */
	old_options = get_option(stdin);
	set_option(stdin, old_options & ~(EDIT|ECHO));

	tfprintf(stderr, "Keyboard echo program with HELP key demo.\n");
	for (;;) {
		/*
		 * Test for HELP key presses
		 */
		num_signals = read_port(HELP_PORT);

		if (num_signals != 0) {
			tfprintf(stderr, "\nHELP key press detected.\n");
			tfprintf(stderr, "Number of HELP signals: %d\n\n",
				num_signals);
		}

		/*
		 * Read keyboard char...
		 */
		c = getchar();

		if (c == CTRL_C || c == EOF)
			break;

		putchar(c);
		fflush(stdout);
	}

	set_option(stdin, old_options);
}
