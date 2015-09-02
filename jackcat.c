/** @file simple_client.c
 *
 * @brief This simple client demonstrates the most basic features of JACK
 * as they would be used by many applications.
 */

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <jack/jack.h>

#define NPORTS 4

jack_port_t *input_ports[NPORTS];
jack_port_t *output_ports[NPORTS];
jack_client_t *client;

/**
 * The process callback for this JACK application is called in a
 * special realtime thread once for each audio cycle.
 *
 * This client does nothing more than copy data from its input
 * port to its output port. It will exit when stopped by 
 * the user (e.g. using Ctrl-C on a unix-ish operating system)
 */
int
process (jack_nframes_t nframes, void *arg)
{
	jack_default_audio_sample_t *in, *out;
	
	int i;

	for (i=0; i < NPORTS; i++) {
	  in = jack_port_get_buffer (input_ports[i], nframes);
	  out = jack_port_get_buffer (output_ports[i], nframes);
	  memcpy (out, in,
		  sizeof (jack_default_audio_sample_t) * nframes);
	}

	return 0;      
}

/**
 * JACK calls this shutdown_callback if the server ever shuts down or
 * decides to disconnect the client.
 */
void
jack_shutdown (void *arg)
{
	exit (1);
}

int
main (int argc, char *argv[])
{
	const char **ports;
	const char *client_name = argv[0];
	const char *server_name = NULL;
	jack_options_t options = JackNullOption;
	jack_status_t status;
	
	/* open a client connection to the JACK server */

	client = jack_client_open (client_name, options, &status, server_name);
	if (client == NULL) {
		fprintf (stderr, "jack_client_open() failed, "
			 "status = 0x%2.0x\n", status);
		if (status & JackServerFailed) {
			fprintf (stderr, "Unable to connect to JACK server\n");
		}
		exit (1);
	}
	if (status & JackServerStarted) {
		fprintf (stderr, "JACK server started\n");
	}
	if (status & JackNameNotUnique) {
		client_name = jack_get_client_name(client);
		fprintf (stderr, "unique name `%s' assigned\n", client_name);
	}

	/* tell the JACK server to call `process()' whenever
	   there is work to be done.
	*/

	jack_set_process_callback (client, process, 0);

	/* tell the JACK server to call `jack_shutdown()' if
	   it ever shuts down, either entirely, or if it
	   just decides to stop calling us.
	*/

	jack_on_shutdown (client, jack_shutdown, 0);

	/* display the current sample rate. 
	 */

	printf ("engine sample rate: %" PRIu32 "\n",
		jack_get_sample_rate (client));

	/* create NPORTS ports */
	
	int i;
	
	
	for (i = 0; i < NPORTS; i++) {
	  char * ipname = malloc(20); // NOTE: if NPORTS IS MORE THAN 10^4 this is a bug
	  snprintf(ipname,9,"input_%d",i);	  
	  char * opname = malloc(20); // NOTE: if NPORTS IS MORE THAN 10^4 this is a bug
	  snprintf(opname,9,"output_%d",i);
	  input_ports[i] = jack_port_register (client, ipname ,
					   JACK_DEFAULT_AUDIO_TYPE,
					   JackPortIsInput, 0);
	  output_ports[i] = jack_port_register (client, opname,
						JACK_DEFAULT_AUDIO_TYPE,
						JackPortIsOutput, 0);
	  printf("Ok %d %s %s\n",i,ipname,opname);
	  if ((input_ports[i] == NULL) || (output_ports[i] == NULL)) {
	    fprintf(stderr, "no more JACK ports available\n");
	    exit (1);
	  }
	}
	  /* Tell the JACK server that we are ready to roll.  Our
	   * process() callback will start running now. */

	if (jack_activate (client)) {
		fprintf (stderr, "cannot activate client");
		exit (1);
	}

	sleep (-1);

	/* this is never reached but if the program
	   had some other way to exit besides being killed,
	   they would be important to call.
	*/

	jack_client_close (client);
	exit (0);
}
