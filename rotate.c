/*
 * Copyright © 2008 Rui Miguel Silva Seabra <rms@1407.org>
 * Copyright © 2008 Fabian Henze <hoacha@quantentunnel.de>
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

#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>

void packet_loop();
int read_packet(int from, int *x, int *y, int *z);
int get_position(int x, int y, int z);
void rotate(int pos);
void catch_alarm(int var);
void set_options(char **args);

/* we're reading this kind of events:
struct input_event {
    struct timeval time;
    __u16 code;
    __u16 type;
    __s32 value;
};
*/

#define UPRIGHT 0
#define RIGHT 1
#define UPSIDEDOWN 2
#define LEFT 3
#define EVENT_PATH "/dev/input/event3"

int current_x, current_y, current_z;
int current_pos = -1;
int file = -1;

static Display *display;
static Window rootWindow;
ushort debug = 0;
ushort change_brightness = 0;


#define BIG_DIFFERENCE 400
/* this ----v idea doesn't seem to be working right, for later investigation */
#define LONG_TIME 0

#define EVENT_PATH "/dev/input/event3"
#define GET_BRIGHTNESS_PATH "/sys/class/backlight/pcf50633-bl/actual_brightness"
#define SET_BRIGHTNESS_PATH "/sys/class/backlight/pcf50633-bl/brightness"



int set_brightness_file = -1;
int get_brightness_file = -1;

void *packet = NULL;


int main(int argc, char **argv)
{
    int x, y, z;

    file = open(EVENT_PATH, O_RDONLY);
    if (file < 0) {
	fprintf(stderr, "Can't open '%s': %s\n", EVENT_PATH,
		strerror(errno));
	exit(1);
    }

    set_options(argv);

    if (change_brightness) {
	set_brightness_file = open(SET_BRIGHTNESS_PATH, O_RDWR);
	if (set_brightness_file < 0) {
	    fprintf(stderr, "Can't open '%s': %s\n", SET_BRIGHTNESS_PATH,
		    strerror(errno));
	    fprintf(stderr, "No brightness control enabled\n");
	}
    }


    /* initialize current position */
    while (!read_packet(file, &x, &y, &z));
    current_pos = get_position(x, y, z);

    display = XOpenDisplay(":0");
    if (display == NULL) {
	fprintf(stderr, "Can't open display %s\n", XDisplayName(":0"));
	exit(1);
    }

    packet_loop();
}

void set_options(char **args)
{
    char *arg = (char *) *args;
    int i = 1;

    printf("OPTIONS:\n");
    while (arg != NULL) {
	if (0 == strncmp(arg, "debug", 5)) {
	    printf("RUNNING IN DEBUG MODE\n");
	    debug = 1;
	} else if (0 == strncmp(arg, "brightness", 10)) {
	    if (debug)
		printf("USING BRIGHT CONTROL\n");
	    change_brightness = 1;
	}
	arg = (char *) *(args + i++);
    }
}

void packet_loop()
{
    int x, y, z;
    int pos, last_pos = -1;
    char *orientation, *current_orientation;

    while (1) {
	if (!screen_locked() && !screen_dimmed()) {
	    while (!read_packet(file, &x, &y, &z));
	    pos = get_position(x, y, z);
	    if (debug) {
		switch (current_pos) {
		case UPRIGHT:
		    current_orientation = "UPRIGHT";
		    break;
		case UPSIDEDOWN:
		    current_orientation = "UPSIDEDOWN";
		    break;
		case LEFT:
		    current_orientation = "LEFT";
		    break;
		case RIGHT:
		    current_orientation = "RIGHT";
		    break;
		default:
		    current_orientation = "INIT";
		    break;
		}
	    }

	    if (pos >= 0 && pos != current_pos) {
		current_pos = pos;
		rotate(pos);
		sleep(2);
	    } else
		usleep(150000);

	    if (debug) {
		switch (pos) {
		case UPRIGHT:
		    orientation = "UPRIGHT";
		    break;
		case UPSIDEDOWN:
		    orientation = "UPSIDEDOWN";
		    break;
		case LEFT:
		    orientation = "LEFT";
		    break;
		case RIGHT:
		    orientation = "RIGHT";
		    break;
		}
		printf("read accel(x,y,z) = (%d,%d,%d) => from %s to %s\n",
		       x, y, z, current_orientation, orientation);
	    }
	} else {
	    if (debug)
		printf("Screen locked or dimmed, not rotating...\n");
	    sleep(5);
	}
    }
}

int read_packet(int from, int *x, int *y, int *z)
{
    static struct input_event event_x, event_y, event_z, event_syn;
    void *packet_memcpy_result = NULL;
    int packet_size = sizeof(struct input_event);
    int size_of_packet = 4 * packet_size;
    int bytes_read = 0;

    signal(SIGALRM, catch_alarm);
    alarm(5);

    packet = malloc(size_of_packet);

    if (!packet) {
	fprintf(stderr, "malloc failed\n");
	exit(1);
    }

    bytes_read = read(from, packet, size_of_packet);

    if (bytes_read < packet_size) {
	fprintf(stderr, "fread failed\n");
	exit(1);
    }

    /* obtain the full packet */
    packet_memcpy_result = memcpy(&event_x, packet, packet_size);
    packet_memcpy_result =
	memcpy(&event_y, packet + packet_size, packet_size);
    packet_memcpy_result =
	memcpy(&event_z, packet + 2 * packet_size, packet_size);
    packet_memcpy_result =
	memcpy(&event_syn, packet + 3 * packet_size, packet_size);

    *x = event_x.value;
    *y = event_y.value;
    *z = event_z.value;

    free(packet);

    packet = NULL;
    signal(SIGALRM, SIG_DFL);
    alarm(0);

    if (event_syn.type == EV_SYN)
	return (1);
    else
	return (0);
}

ushort neighbour(int value, int target, int neighbour)
{
    return (target - abs(neighbour) < value
	    && value <= target + abs(neighbour));
}

int get_position(int x, int y, int z)
{

    const int total_accel = x ^ 2 + y ^ 2 + z ^ 2;

    if ((900 <= abs(z)) && (abs(z) < 1100)) {
	if (debug)
	    printf(" low angle\n");
	return (-1);
    }

    /* if ((900000 > total_accel) || (total_accel > 1200000)) {
       if(debug) printf(" shaking around\n");
       return(-2);
       } */

    /* if (900000 > total_accel) {
       if(debug) printf(" too little shaking\n");
       return(-2);
       } */

    if (total_accel > 1200000) {
	if (debug)
	    printf(" too much shaking\n");
	return (-2);
    }

    switch (current_pos) {
    case UPRIGHT:
	if (y < 0 && abs(x) < -2 * y)
	    return (UPRIGHT);
	else if (y > 0 && abs(x) < y)
	    return (UPSIDEDOWN);
	else if (x > 0)
	    return (RIGHT);
	else if (x < 0)
	    return (LEFT);
	break;
    case RIGHT:
	if (x > 0 && abs(y) < 2 * x)
	    return (RIGHT);
	else if (x < 0 && abs(y) < -x)
	    return (LEFT);
	else if (y > 0)
	    return (UPSIDEDOWN);
	else if (y < 0)
	    return (UPRIGHT);
	break;
    case LEFT:
	if (x < 0 && abs(y) < -2 * x)
	    return (LEFT);
	else if (x > 0 && abs(y) < x)
	    return (RIGHT);
	else if (y > 0)
	    return (UPSIDEDOWN);
	else if (y < 0)
	    return (UPRIGHT);
	break;
    case UPSIDEDOWN:
	if (y > 0 && abs(x) < 2 * y)
	    return (UPSIDEDOWN);
	else if (y < 0 && abs(x) < -y)
	    return (UPRIGHT);
	else if (x > 0)
	    return (RIGHT);
	else if (x < 0)
	    return (LEFT);
	break;
    default:
	if (y < 0 && abs(x) < -y)
	    return (UPRIGHT);
	else if (y > 0 && abs(x) < y)
	    return (UPSIDEDOWN);
	else if (x > 0)
	    return (RIGHT);
	else if (x < 0)
	    return (LEFT);
	break;
    }

    return (-1);
}

void rotate(int pos)
{
    ushort return_val = -1;

    Rotation r_to, r;
    XRRScreenConfiguration *config;
    int current_size = 0;
    int screen = -1;
    char current_brightness[3] = "63\n";
    char brightness_off[2] = "0\n";
    ushort do_rotate = 0;

    screen = DefaultScreen(display);
    rootWindow = RootWindow(display, screen);
    XRRRotations(display, screen, &r);

    switch (pos) {
    case UPRIGHT:
	r_to = RR_Rotate_0;
	do_rotate = 1;
	break;
    case RIGHT:
	r_to = RR_Rotate_90;
	do_rotate = 1;
	break;
    case UPSIDEDOWN:
	r_to = RR_Rotate_180;
	do_rotate = 1;
	break;
    case LEFT:
	r_to = RR_Rotate_270;
	do_rotate = 1;
	break;
    default:
	break;
    }

    if (do_rotate) {
	if (change_brightness && set_brightness_file >= 0) {
	    lseek(get_brightness_file, 0, SEEK_SET);
	    read(get_brightness_file, &current_brightness, 2);
	    lseek(set_brightness_file, 0, SEEK_SET);
	    write(set_brightness_file, &brightness_off, 2);
	    usleep(500000);
	}

	if (debug)
	    printf("ROTATING!\n");
	config = XRRGetScreenInfo(display, rootWindow);
	current_size = XRRConfigCurrentConfiguration(config, &r);
	XRRSetScreenConfig(display, config, rootWindow, current_size, r_to,
			   CurrentTime);
	usleep(500000);

	if (change_brightness && set_brightness_file >= 0) {
	    lseek(set_brightness_file, 0, SEEK_SET);
	    write(set_brightness_file, &current_brightness, 3);
	}
	current_pos = pos;
    }
}

int screen_dimmed()
{
    char current_brightness[3] = "63\n";
    short currently_bright = 0;

    if (!change_brightness)
	return (0);

    get_brightness_file = open(GET_BRIGHTNESS_PATH, O_RDONLY);
    if (get_brightness_file < 0) {
	fprintf(stderr, "Can't open '%s': %s\n", GET_BRIGHTNESS_PATH,
		strerror(errno));
	fprintf(stderr, "No brightness check enabled\n");
	return (0);
    } else {
	/* read the current brightness, and dim only if it's bright */
	lseek(get_brightness_file, 0, SEEK_SET);
	read(get_brightness_file, &current_brightness, 2);
	currently_bright = atoi(current_brightness);

	close(get_brightness_file);

	if (debug)
	    printf("Dimmed level: %d\n", currently_bright);

	if (currently_bright > 0)
	    return (0);
	else
	    return (1);

    }
}

int screen_locked()
{
    /* shouldn't rotate if screen is locked, waste of energy */
    return (0);
}

void catch_alarm(int var)
{
    if (packet) {
	free(packet);
	packet = NULL;
    }
    packet_loop();
}
