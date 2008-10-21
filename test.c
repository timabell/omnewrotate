/*
 * Copyright © 2008 Rui Miguel Silva Seabra <rms@1407.org>
 *
 * Inspired upon Chris Ball's rotate, this is a totally new rewrite.
 *
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>

static Display  *display;
static Window   rootWindow;
ushort debug = 0;

/* we're reading this kind of events:
struct input_event {
    struct timeval time;
    __u16 code;
    __u16 type;
    __s32 value;
};

*/

#define BIG_DIFFERENCE 400
/* this ----v idea doesn't seem to be working right, for later investigation */
#define LONG_TIME 0

#define EVENT_PATH "/dev/input/event3"

struct input_event current_a, current_b, current_c;

int read_packet(int from, struct input_event *x, struct input_event *y, struct input_event *z, struct input_event *syn)  {
	void* packet = NULL;
	void* packet_memcpy_result = NULL;
	int packet_size=sizeof(struct input_event);
	int size_of_packet=4*packet_size;
	int bytes_read = 0;

	packet = malloc(size_of_packet);

	if(!packet) {
		fprintf(stderr, "malloc failed\n");
		exit(1);
	}

	bytes_read = read (from, packet, size_of_packet);

	if (bytes_read < packet_size) {
		fprintf(stderr, "fread failed\n");
		exit(1);
	}

	/* obtain the full packet */
	packet_memcpy_result = memcpy(x, packet, packet_size);
	packet_memcpy_result = memcpy(y, packet+packet_size, packet_size);
	packet_memcpy_result = memcpy(z, packet+2*packet_size, packet_size);
	packet_memcpy_result = memcpy(syn, packet+3*packet_size, packet_size);
	free(packet);
	if(syn->type == EV_SYN)
		return(1);
	else
		return(0);
}

int very_different_than_previously(struct input_event event_a, struct input_event event_b, struct input_event event_c) {
/*
	if(	(abs(current_x.time.tv_sec - event_x.time.tv_sec) > LONG_TIME) &&
*/
	if(
		(
		 (abs(current_a.value - event_a.value) >= BIG_DIFFERENCE) ||
		 (abs(current_b.value - event_b.value) >= BIG_DIFFERENCE) ||
		 (abs(current_c.value - event_c.value) >= BIG_DIFFERENCE)
		)
	  ) return(1);
	else
		return(0);
}
void reset_current_position(struct input_event event_a, struct input_event event_b, struct input_event event_c) {
	current_a.value=event_a.time.tv_sec = event_a.time.tv_sec;

	current_a.value=event_a.value;
	current_b.value=event_b.value;
	current_c.value=event_c.value;
}

ushort neighbour(int value, int target, int neighbour) {
	return ( target-abs(neighbour) < value && value <= target+abs(neighbour) );
}

void swap_orientation(struct input_event event_a, struct input_event event_b, struct input_event event_c) {
	__s32 a = event_a.value;
	__s32 b = event_b.value;
	__s32 c = event_c.value;

	//if(very_different_than_previously(event_a, event_b, event_c)) {
		//printf("(a,b,c) = (%d,%d,%d) == ", a, b, c);

		printf("Types: a(%d), b(%d), c(%d)\n", event_a.type, event_b.type, event_c.type);
		printf("Codes: a(%d), b(%d), c(%d)\n", event_a.code, event_b.code, event_c.code);
		printf("Value: a(%d), b(%d), c(%d)\n", event_a.value, event_b.value, event_c.value);

		if(c > a && c > b && neighbour(a, 0, 20) && neighbour(c,1000,200)) {
			printf("horizontal");
		} else if(b < a && b < c && neighbour(b,-1000,200)) {
			printf("vertical straight");
		} else if(b > a && b > c && neighbour(b,1000,200)) {
			printf("vertical upside down");
		} else if(a > b && a > c && neighbour(a,1000,500)) {
			printf("right");
		} else if(a < b && a < c && neighbour(a,-1000,500)) {
			printf("left");
		}

	printf("\n");
	//}

}

int screen_locked() {
	/* shouldn't rotate if screen is locked, waste of energy */
	return(0);
}

int main (int argc, char ** argv) {
	int file = -1;
	char * time=(char*)malloc(20);
	struct input_event syn, a, b, c;

	file = open(EVENT_PATH, O_RDONLY);
	if (file < 0) {
		fprintf(stderr, "Can't open '%s': %s\n",EVENT_PATH,strerror(errno));
		exit(1);
	}

	if(argc > 1) debug=1;

	/* initialize current position */
	read_packet(file, &current_a, &current_b, &current_c, &syn);

	display = XOpenDisplay(":0");
	if (display == NULL) {
		fprintf (stderr, "Can't open display %s\n", XDisplayName(":0"));
		exit(1);
	}

	
	while(1) {
		//printf("Reading packet...");
		if(read_packet(file, &a, &b, &c, &syn)) {
			swap_orientation(a,b,c);
		} else printf("Skipped a packet\n");
	}

}
