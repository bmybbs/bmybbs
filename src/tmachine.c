#ifdef CAN_EXEC
#include "bbs.h"

#ifdef PTY_EXEC
extern int pty;
#endif

char term_type[64];
int term_cols;
int term_lines;

int max_timeout;

unsigned char options[256];
unsigned int mstate;
static void ask_option(unsigned char o, unsigned char p);
static int tmachine_server_recv_and_check_option(int net);
	
static void
ask_option(unsigned char o, unsigned char p)
{
	queue_write_char(&qneto, (char) IAC);
	queue_write_char(&qneto, (char) o);
	queue_write_char(&qneto, (char) p);
	options[p] = o;
}

static int
tmachine_server_recv_and_check_option(int net)
{
	char buf[256];
	int count, i;
	unsigned char ch;

	static unsigned char sb_buf[64];
	static unsigned int sb_ptr;

	count = read(net, buf, sizeof (buf));
	if (count <= 0)
		return -1;

	for (i = 0; i < count; i++) {
		ch = (unsigned char) buf[i];
		switch (mstate) {
		case 0:
			if (ch == IAC) {
				mstate = IAC;
				break;
			}
			queue_write(&qneti, (char *) &ch, 1);
			break;

		case IAC:
			if (ch == IAC) {
				mstate = 0;
				queue_write(&qneti, (char *) &ch, 1);
				break;
			}
			mstate = ch;
			break;

		case DONT:
			mstate = 0;
			if ((options[ch] == WILL) || (options[ch] == WONT)) {
				options[ch] = DONT;
				break;
			}
			ask_option(WONT, ch);
			break;

		case DO:
			mstate = 0;
			if ((options[ch] == WILL) || (options[ch] == WONT)) {
				options[ch] = DO;
				break;
			}
			ask_option(WONT, ch);
			break;

		case WILL:
			mstate = 0;
			if ((options[ch] == DO) || (options[ch] == DONT)) {
				if (ch == TELOPT_TTYPE) {
					unsigned char ttype[] =
					    { IAC, SB, TELOPT_TTYPE, 1, IAC,
						SE
					};
					queue_write(&qneto, (char *) ttype,
						    sizeof (ttype));
				}
				options[ch] = WILL;
				break;
			}
			ask_option(DONT, ch);
			break;

		case WONT:
			mstate = 0;
			if ((options[ch] == DO) || (options[ch] == DONT)) {
				if (ch == TELOPT_TTYPE)
					strcpy(term_type, "vt220");
				if (ch == TELOPT_NAWS) {
					term_cols = 80;
					term_lines = 25;
				}
				options[ch] = WONT;
				break;
			}
			ask_option(DONT, ch);
			break;

		case SB:
			sb_ptr = 0;
			if (ch == TELOPT_TTYPE)
				mstate = 1000;
			else if (ch == TELOPT_NAWS)
				mstate = 1001;
			else
				mstate = 1002;
			break;

		case 1000:	// ttype
		case 1001:
		case 1002:
			if (ch == IAC) {
				mstate += 8000;
				break;
			}
			if (sb_ptr < sizeof (sb_buf))
				sb_buf[sb_ptr++] = ch;
			break;

		case 9000:
		case 9001:
		case 9002:
			if (ch == IAC) {
				mstate -= 8000;
				if (sb_ptr < sizeof (sb_buf))
					sb_buf[sb_ptr++] = ch;
				break;
			}
			if (ch == SE) {
				if (mstate == 9000) {
					sb_buf[sb_ptr] = 0;
					strcpy(term_type, (char *) &sb_buf[1]);
				}
				if (mstate == 9001) {
					struct winsize ws;
					term_cols =
					    htons(*(short *) &sb_buf[0]);
					term_lines =
					    htons(*(short *) &sb_buf[2]);
#ifdef PTY_EXEC
					if (pty) {
						ws.ws_col = term_cols;
						ws.ws_row = term_lines;
						ioctl(pty, TIOCSWINSZ,
						      (char *) &ws);
					}
#endif
				}
			}
			mstate = 0;
			break;

		default:
			mstate = 0;
			break;
		}
	}
	return count;
}

int
tmachine_init(int net)
{
	memset(options, 0, sizeof (options));
	mstate = 0;

#ifdef PTY_EXEC
	pty = 0;
#endif

	ask_option(DO, TELOPT_ECHO);
	ask_option(WONT, TELOPT_BINARY);
	ask_option(WILL, TELOPT_SGA);
	ask_option(DO, TELOPT_LFLOW);
	queue_send(&qneto, net);

	do {
		if (tmachine_server_recv_and_check_option(net) < 0)
			return 0;
		queue_send(&qneto, net);
	} while (options[TELOPT_LFLOW] == DO);

	ask_option(DO, TELOPT_TTYPE);
	ask_option(DO, TELOPT_NAWS);
	queue_send(&qneto, net);
	do {
		if (tmachine_server_recv_and_check_option(net) < 0)
			return 0;
		queue_send(&qneto, net);
	} while (options[TELOPT_NAWS] == DO || options[TELOPT_TTYPE] == DO);

	while (options[TELOPT_TTYPE] == WILL && !term_type[0]) {
		if (tmachine_server_recv_and_check_option(net) < 0)
			return 0;
	}

	// Finally, to clean things up, we turn on our echo.  This
	// will break stupid 4.2 telnets out of local terminal echo.
	if (options[TELOPT_ECHO] == WONT) {
		ask_option(WILL, TELOPT_ECHO);
		queue_send(&qneto, net);
	}
	return 1;
}
#endif
