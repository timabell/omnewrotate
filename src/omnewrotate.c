/* vim:tabstop=4
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

#include <config.h>

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
#include <pthread.h>

#ifdef HAVE_LIBFRAMEWORD_GLIB

#include <dbus/dbus.h>

#endif

void *packet_reading_thread(void *);
void define_position(void);
int read_packet(void);
int neighbour(int value, int target, int neighbour);
void do_rotation(void);
void display_version(void);
void display_help(void);


/* we're reading this kind of events:
struct input_event {
	struct timeval time;
	__u16 code;
	__u16 type;
	__s32 value;
};
*/

#define UP 0
#define RIGHT 1
#define DOWN 2
#define LEFT 3

int x = 0;
int y = 0;
int z = 0;

/* Position flags */
int face_up = 1;
int vertical = 0;
int left = 0;
int right = 0;
int up = 0;
int down = 0;

int current_pos = -1;
int event3 = -1;
int read_sleep = 250000;
int accel_threshold = 50; //how close to 1000 in x/y/z accelerometer needs to read before rotating (command line option "-t")
int kernel_version = 2629; // 2629 == 2.6.29 (default), 2632 = 2.6.32

static Display *display;
static Window rootWindow;
ushort debug = 0;
ushort skip_zero = 1;
ushort change_brightness = 0;
#ifdef HAVE_LIBFRAMEWORD_GLIB
ushort use_dbus = 1;
#else
ushort use_dbus = 0;
#endif


#define BIG_DIFFERENCE 400
/* this ----v idea doesn't seem to be working right, for later investigation */
#define LONG_TIME 0

#define EVENT_PATH "/dev/input/event3"
#define GET_BRIGHTNESS_PATH_2629 "/sys/class/backlight/gta02-bl/actual_brightness"
#define SET_BRIGHTNESS_PATH_2629 "/sys/class/backlight/gta02-bl/brightness"
#define GET_BRIGHTNESS_PATH_2632 "/sys/class/backlight/pcf50633-bl/actual_brightness"
#define SET_BRIGHTNESS_PATH_2632 "/sys/class/backlight/pcf50633-bl/brightness"

#define NUM_THREADS 1


int set_brightness_file = -1;
int get_brightness_file = -1;
char current_brightness[4];

void set_linux_type(int kernel_version, char *set_brightness_path, char *get_brightness_path)
{
	struct stat buf;

	switch(kernel_version)
	{
		case 0:	// unspecified guessing
		{
			if(! (set_brightness_path && get_brightness_path) )
			{
				fprintf(stderr, "With unspecified Linux version, you need to define set_brightness and get_brightness paths.\n");
				exit(1);
			}
		}
		case 2632:	// Linux 2.6.32
		{
			if(set_brightness_path)
				free(set_brightness_path);
			if(!get_brightness_path)
				free(get_brightness_path);
			if((stat(SET_BRIGHTNESS_PATH_2632, &buf) == 0 && stat(GET_BRIGHTNESS_PATH_2632, &buf) == 0))
			{
				set_brightness_path = SET_BRIGHTNESS_PATH_2632;
				get_brightness_path = GET_BRIGHTNESS_PATH_2632;
				snprintf(current_brightness, 5, "63\n");
			}
			else
			{
				if((stat(SET_BRIGHTNESS_PATH_2629, &buf) == 0 && stat(GET_BRIGHTNESS_PATH_2629, &buf) == 0))
				{
					fprintf(stderr, "Expected Linux 2.6.32 style but found Linux 2.6.29 style paths for brightness\n");
					set_brightness_path = SET_BRIGHTNESS_PATH_2629;
					get_brightness_path = GET_BRIGHTNESS_PATH_2629;
					snprintf(current_brightness, 5, "255\n");
				}
				else
				{
					fprintf(stderr, "Unsupported Linux version\n");
					exit(2);
				}
			}
			break;
		}
		default:	// Linux 2.6.29
		{
			if(set_brightness_path)
				free(set_brightness_path);
			if(get_brightness_path)
				free(get_brightness_path);
			if((stat(SET_BRIGHTNESS_PATH_2629, &buf) == 0 && stat(GET_BRIGHTNESS_PATH_2629, &buf) == 0))
			{
				set_brightness_path = SET_BRIGHTNESS_PATH_2629;
				get_brightness_path = GET_BRIGHTNESS_PATH_2629;
				snprintf(current_brightness, 5, "255\n");
			}
			else
			{
				if((stat(SET_BRIGHTNESS_PATH_2632, &buf) == 0 && stat(GET_BRIGHTNESS_PATH_2632, &buf) == 0))
				{
					fprintf(stderr, "Expected Linux 2.6.29 style but found Linux 2.6.32 style paths for brightness\n");
					set_brightness_path = SET_BRIGHTNESS_PATH_2632;
					get_brightness_path = GET_BRIGHTNESS_PATH_2632;
					snprintf(current_brightness, 4, "63\n");
				}
				else
				{
					fprintf(stderr, "Unsupported Linux version\n");
					exit(2);
				}
			}
			break;
		}
	}
}

int main(int argc, char **argv)
{
	struct stat buf;
	pthread_t p_thread[1];
	int i;
	int pos=0;
	static char options[] = "pbd0hva:g:s:t:k:";
	int option;
	char * accelerometer = (char*)NULL;
	char * get_brightness_path = (char*)NULL;
	char * set_brightness_path = (char*)NULL;

#ifdef HAVE_LIBFRAMEWORD_GLIB
	DBusError err;
	DBusConnection* conn;
#endif
	int ret=0;

	while((option = getopt(argc,argv,options)) != -1)
	{
		switch(option)
		{
			case '0':
			{
				skip_zero=0;
				break;
			}
			case 'a':
			{
				accelerometer = strndup(optarg, 1024);
				if(stat(accelerometer, &buf) < 0)
				{
					fprintf(stderr, "Problem accessing accelerometer at %s: %s\n", accelerometer, strerror(errno));
					free(accelerometer);
					exit(1);
				}
				break;
			}
			case 'b':
			{
				change_brightness=1;
				break;
			}
			case 'd':
			{
				debug=1;
				break;
			}
			case 'g':
			{
				get_brightness_path = strndup(optarg, 1024);
				if(stat(get_brightness_path, &buf) < 0)
				{
					free(get_brightness_path);
					get_brightness_path=NULL;
				}
				break;
			}
			case 'h':
			{
				display_help();
				exit(0);
			}
			case 'p':
			{
				read_sleep = 1000000;
				break;
			}
			case 's':
			{
				set_brightness_path = strndup(optarg, 1024);
				if(stat(set_brightness_path, &buf) < 0)
				{
					free(set_brightness_path);
					set_brightness_path=NULL;
				}
				break;
			}
			case 'k':
			{
				kernel_version = atoi(optarg);
				break;
			}
			case 't':
			{
				accel_threshold = atoi(optarg);
				break;
			}
			case 'v':
			{
				display_version();
				exit(0);
			}
			default:
			{
				exit(1);
			}
		}
	}

	set_linux_type(kernel_version, set_brightness_path, set_brightness_path);

	display = XOpenDisplay(":0");
	if (display == NULL)
	{
		fprintf(stderr, "Can't open display %s\n", XDisplayName(":0"));
		exit(1);
	}

#ifdef HAVE_LIBFRAMEWORD_GLIB
	dbus_error_init(&err);
	conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
	if (dbus_error_is_set(&err))
	{
		fprintf(stderr, "Connection Error (%s), so not using dbus\n", err.message);
		dbus_error_free(&err);
	}
	if (NULL == conn) use_dbus=0;
#endif

	if (change_brightness && !use_dbus) {
		set_brightness_file = open(set_brightness_path, O_WRONLY);
		get_brightness_file = open(get_brightness_path, O_WRONLY);

		if (set_brightness_file < 0 || get_brightness_file < 0)
		{
			fprintf(stderr, "Can't open '%s': %s\n", set_brightness_path, strerror(errno));
			fprintf(stderr, "No brightness control enabled\n");
			change_brightness = 0;
		}
	}

	if(accelerometer == NULL) event3 = open(EVENT_PATH, O_RDONLY);
	else event3 = open(accelerometer, O_RDONLY);

	if (event3 < 0)
	{
		fprintf(stderr, "Can't open '%s': %s\n", EVENT_PATH,
		strerror(errno));
		exit(1);
	}

	/* Create a the packet reading thread */
	for(i = 0; i < NUM_THREADS; i++)
	{
		if(pthread_create(&p_thread[i], NULL, packet_reading_thread, (void *)(long)i) != 0)
		{
			fprintf(stderr, "Error creating the packet reading thread");
			exit(1);
		}
	}

	if (debug) printf("Begin loop.\n");
	while(1)
	{
		usleep(read_sleep);
		pos=current_pos;
		define_position();
		if(current_pos != pos) do_rotation();
	}
	return(ret);
}

void display_version(void)
{
	printf("omnewrotate version %s is licensed under the GNU GPLv3 or later.\n", VERSION);
}

void display_help(void)
{
	display_version();
	printf("\nUsage:\n"\
		"	-0	Don't skip packets if any coordinate is '0'\n"
		"	-a	Accelerometer path, by default '" EVENT_PATH "'\n"
		"	-b	Use brightness (dimming and back) effects\n"
		"	-d	Debug mode (extra yummy output)\n"
		"	-k	Set the kernel version. Use 2629 (default, Linux 2.6.29)\n"
		"       or 2632 to change to Linux 2.6.32 or greater\n"
		"       use 0 to be able to arbitrarily change brightness paths\n"
		"	-g	Get current brightness path,\n"
		"		by default '" GET_BRIGHTNESS_PATH_2629 "'\n"
		"	-h	Help (what you're reading right now)\n"
		"	-p	Powersaving features (like sleeping longer, etc...)\n"
		"	-s	Set current brightness path,\n"
		"		by default '" SET_BRIGHTNESS_PATH_2629 "'\n"
		"	-t	Threshold for rotating. Default +/-50 (approx 20deg) \n"
		"		against target of 1000. 300 gives approx 45deg\n"
		"	-v	Show version and license\n"
		"\n"
		);
}

void do_rotation(void)
{
	Rotation r_to = RR_Rotate_0;
	Rotation r = RR_Rotate_0;
	XRRScreenConfiguration *config;
	int screen = -1;
	int current_size = 0;

	screen = DefaultScreen(display);
	rootWindow = RootWindow(display, screen);
	XRRRotations(display, screen, &r);

	char brightness_off[2] = "0\n";

	switch(current_pos)
	{
		case(UP):
			{
				if(debug) printf("Rotating to UP\n");
				r_to = RR_Rotate_0;
				break;
			}
		case(DOWN):
			{
				if(debug) printf("Rotating to DOWN\n");
				r_to = RR_Rotate_180;
				break;
			}
		case(RIGHT):
			{
				if(debug) printf("Rotating to RIGHT\n");
				r_to = RR_Rotate_90;
				break;
			}
		case(LEFT):
			{
				if(debug) printf("Rotating to LEFT\n");
				r_to = RR_Rotate_270;
				break;
			}
		default: break;
	}

	if (change_brightness)
	{
		if(debug) printf("Dimming screen for nifty effect\n");

		if(use_dbus)
		{
		}
		else
		{
			lseek(get_brightness_file, 0, SEEK_SET);
			read(get_brightness_file, &current_brightness, 3);
			lseek(set_brightness_file, 0, SEEK_SET);
			write(set_brightness_file, &brightness_off, 2);
		}
		usleep(500000);
	}

	config = XRRGetScreenInfo(display, rootWindow);
	current_size = XRRConfigCurrentConfiguration(config, &r);
	XRRSetScreenConfig(display, config, rootWindow, current_size, r_to, CurrentTime);

	if (change_brightness)
	{
		if(debug) printf("Recovering screen brightness for nifty effect\n");
		usleep(500000);
		lseek(set_brightness_file, 0, SEEK_SET);
		write(set_brightness_file, &current_brightness, 4);
	}

}

void define_position(void)
{

	/* Conclude all facts about current position */
	if(neighbour(z,0,500))
		vertical=1;
	else
		vertical=0;

	if(z>=0)
		face_up=1;
	else
		face_up=0;

	if(neighbour(x,-1000,accel_threshold))
		left=1;
	else
		left=0;

	if(neighbour(x,1000,accel_threshold))
		right=1;
	else
		right=0;

	if(neighbour(y,1000,accel_threshold))
		down=1;
	else
		down=0;

	if(neighbour(y,-1000,accel_threshold))
		up=1;
	else
		up=0;

	if(down) current_pos=DOWN;
	else if(up) current_pos=UP;
	else if(left) current_pos=LEFT;
	else if(right) current_pos=RIGHT;

	if(debug)
	{
		printf("v(x,y,z)=(% 5d,% 5d,% 5d) == ",x,y,z);

		printf(" face %s", face_up?"up":"down");
		printf(" %s-ish", vertical?"vertical":"horizontal");
		if(left) printf(" left");
		if(right) printf(" right");
		if(up) printf(" up");
		if(down) printf(" down");

		printf("\n");
	}
}

/**
  * neighbour(): test proximity of one value to another based on a threshold.
  * returns true if "value" is nearer to "target" than threshold "value"
  * value: the varying value to test
  * target: the number to test proximity of to
  * threshold: the threshold for how close "value" has to be to "target" to succeed
  * eg: for target=100 and threshold=10, values of 91 to 110 inclusive would return true
  */
int neighbour(int value, int target, int threshold)
{
	return (value > (target - abs(threshold)) && (value <= (target + abs(threshold))));
}

void *packet_reading_thread(void *arg)
{
	while(1)
	{
		while(!read_packet());
		usleep(250000);
	}
}

int read_packet()
{
	static struct input_event event_x, event_y, event_z, event_syn;
	void *packet_memcpy_result = NULL;
	int packet_size = sizeof(struct input_event);
	int size_of_packet = 4 * packet_size;
	int bytes_read = 0;
	void *packet = NULL;


	packet = malloc(size_of_packet);

	if (!packet)
	{
		fprintf(stderr, "malloc failed\n");
		exit(1);
	}

	bytes_read = read(event3, packet, size_of_packet);

	if (bytes_read < packet_size)
	{
		fprintf(stderr, "fread failed\n");
		exit(1);
	}

	/* obtain the full packet */
	packet_memcpy_result = memcpy(&event_x,   packet,                   packet_size);
	packet_memcpy_result = memcpy(&event_y,   packet +     packet_size, packet_size);
	packet_memcpy_result = memcpy(&event_z,   packet + 2 * packet_size, packet_size);
	packet_memcpy_result = memcpy(&event_syn, packet + 3 * packet_size, packet_size);

	free(packet);

	if(skip_zero && (event_x.value == 0 || event_y.value == 0 || event_z.value == 0))
	{
		if(debug) printf("Bad packet!\n");
		return(0);
	}

	if (event_syn.type == EV_SYN)
	{
		x = event_x.value;
		y = event_y.value;
		z = event_z.value;

		return (1);
	}
	else
		return (0);
}

