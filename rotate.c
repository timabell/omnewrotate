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

struct input_event current_x, current_y, current_z;

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

int very_different_than_previously(struct input_event event_x, struct input_event event_y, struct input_event event_z) {
/*
	if(	(abs(current_x.time.tv_sec - event_x.time.tv_sec) > LONG_TIME) &&
*/
	if(
		(
		 (abs(current_x.value - event_x.value) >= BIG_DIFFERENCE) ||
		 (abs(current_y.value - event_y.value) >= BIG_DIFFERENCE) ||
		 (abs(current_z.value - event_z.value) >= BIG_DIFFERENCE)
		)
	  ) return(1);
	else
		return(0);
}
void reset_current_position(struct input_event event_x, struct input_event event_y, struct input_event event_z) {
	current_x.value=event_x.time.tv_sec = event_x.time.tv_sec;

	current_x.value=event_x.value;
	current_y.value=event_y.value;
	current_z.value=event_z.value;
}

ushort neighbour(int value, int target, int neighbour) {
        return ( target-abs(neighbour) < value && value <= target+abs(neighbour) );
}

void swap_orientation(struct input_event event_x, struct input_event event_y, struct input_event event_z) {
	__s32 x = event_x.value;
	__s32 y = event_y.value;
	__s32 z = event_z.value;

	Rotation r_to,r;
	XRRScreenConfiguration * config;
	int current_size=0;
	int screen = -1;
	ushort do_rotate=0;

	screen = DefaultScreen(display);
	rootWindow = RootWindow(display, screen);
	XRRRotations(display, screen, &r);

	if(very_different_than_previously(event_x, event_y, event_z)) {
		if(!debug) printf("Postion (%d,%d,%d): ",x,y,z);

		if( z > x && z > y && neighbour(x, 0, 20) && neighbour(z,1000,200) ) {
			printf(" horizontal");
			r_to=RR_Rotate_0;
			do_rotate=1;
		}
		if( y < x && y < z && neighbour(y,-1000,200) ) {
			printf(" vertical");
			r_to=RR_Rotate_0;
			do_rotate=1;
		}
		if( x > y && x > z && neighbour(x,1000,500) ) {
			printf(" right");
			r_to=RR_Rotate_90;
			do_rotate=1;
		}
		if( y > x && y > z && neighbour(y,1000,200) ) {
			printf(" upsideDown");
			r_to=RR_Rotate_180;
			do_rotate=1;
		}
		if( x < y && x < z && neighbour(x,-1000,500) ) {
			printf(" left");
			r_to=RR_Rotate_270;
			do_rotate=1;
		}

		if( z < 0 ) {
			printf(" turnedDown");
			/* objective: make the phone silent (for meetings, for example) */
		}
		if( 0 <= z ) {
			printf(" turnedUp");
			/* objective: if the phone is silent because of us, return to the previous profile */
		}

		printf("\n");

		if(do_rotate) {
			printf("ROTATING!\n");
			config = XRRGetScreenInfo(display, rootWindow);
			current_size = XRRConfigCurrentConfiguration (config, &r);
			XRRSetScreenConfig(display, config, rootWindow, current_size, r_to, CurrentTime);
			sleep(1);
		}
	}
}

int screen_locked() {
	/* shouldn't rotate if screen is locked, waste of energy */
	return(0);
}

int main (int argc, char ** argv) {
	int file = -1;
	char * time=(char*)malloc(20);
	struct input_event syn, x, y, z;

	file = open(EVENT_PATH, O_RDONLY);
	if (file < 0) {
		fprintf(stderr, "Can't open '%s': %s\n",EVENT_PATH,strerror(errno));
		exit(1);
	}

	if(argc > 1) debug=1;

	/* initialize current position */
	read_packet(file, &current_x, &current_y, &current_z, &syn);

	display = XOpenDisplay(":0");
	if (display == NULL) {
		fprintf (stderr, "Can't open display %s\n", XDisplayName(":0"));
		exit(1);
	}

	

	while(1) {
		printf("Reading packet...");
		if(read_packet(file, &x, &y, &z, &syn)) {
			printf(" read.\n");
			if(!screen_locked()) {
				swap_orientation(x, y, z);
	
				/* reset current position */
				reset_current_position(x,y,z);
			}
	
			if(debug) {
				/*
				printf("Data:\tTime\t\t\t\tType\tCode\tValue\n");
				strftime(time,20,"%Y-%m-%d %H:%M:%S",localtime(&x.time.tv_sec));
				printf("\t%s\t\t%d\t%d\t%d\n", time, x.type, x.code, x.value);
				strftime(time,20,"%Y-%m-%d %H:%M:%S",localtime(&y.time.tv_sec));
				printf("\t%s\t\t%d\t%d\t%d\n", time, y.type, y.code, y.value);
				strftime(time,20,"%Y-%m-%d %H:%M:%S",localtime(&z.time.tv_sec));
				printf("\t%s\t\t%d\t%d\t%d\n", time, z.type, z.code, z.value);
				strftime(time,20,"%Y-%m-%d %H:%M:%S",localtime(&syn.time.tv_sec));
				printf("\t%s\t\t%d\t%d\t%d\n", time, syn.type, syn.code, syn.value);
				*/
				printf("(x,y,z) = (%d,%d,%d)\n", x.value, y.value, z.value);
			}
		}
	}

}
