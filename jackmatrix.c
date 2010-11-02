#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <jack/jack.h>

typedef struct {
	int in;
	int out;
	} portCouple;

/* Global variables */
const char **ports_in, **ports_out, **connections; 
jack_client_t *client;
jack_status_t status;

/* toggle callback funcion */
void toggle_button_callback (GtkWidget *widget, gpointer  data)
{
	portCouple* p = data;
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))) {
		if (jack_connect (client, ports_out[p->out], ports_in[p->in]) ) {
			fprintf (stderr,"Cannot connect ports");
		} else {
	        g_print ("%s CONNECTED TO %s DOWN\n", ports_out[p->out], ports_in[p->in]);
	        gtk_button_set_label (GTK_BUTTON (widget), "o");
		}
    } else {
		if (jack_disconnect (client, ports_out[p->out], ports_in[p->in]) ) {
			fprintf (stderr,"Cannot disconnect ports");
		} else {
	        g_print ("%s DISCONNECTS %s DOWN\n", ports_out[p->out], ports_in[p->in]);
	        gtk_button_set_label (GTK_BUTTON (widget), "x");
		}
    }
}

/* delete callback */
static gboolean delete_event( GtkWidget *widget, GdkEvent  *event, gpointer   data)
{
    gtk_main_quit ();
    return FALSE;
}

 
int main( int   argc, char *argv[] )
{
    int i,j,k;

    /* Gtk stuff */
    GtkWidget *window;
    GtkWidget *button;
    GtkWidget *portLabel;
    GtkWidget *table;
    gint rows, cols, count;
    PangoFontDescription *sansFont = NULL; 
    sansFont = pango_font_description_from_string ("Sans 10");
	GdkColor color;
    
    /* jack stuff */
	int ports_in_num, ports_out_num;
  	char *server_name = NULL;
    gboolean connected;

    /* init gtk and create a new window */
	gtk_init (&argc, &argv);
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (window), "jackmatrix");

    /* connect delete event handler for he windows */
    g_signal_connect (window, "delete-event", G_CALLBACK (delete_event), NULL);

    gtk_container_set_border_width (GTK_CONTAINER (window), 1);

	/* Open a client connection to the JACK server.  Don't start new server (JackNoStartServer option)
	   Retrn if there's an error.   */
	client = jack_client_open ("jackmatrix", JackNoStartServer, &status, server_name);
	if (client == NULL) {
		if (status & JackServerFailed) {
			fprintf (stderr, "JACK server not running.\n"
			                 "jackmatrix currently requires a running server to work!\n");
		} else {
			fprintf (stderr, "jack_client_open() failed, status = 0x%2.0x\n", status);
		}
		return 1;
	}

    /* Get input and output ports, only audio ones */
	ports_in = jack_get_ports (client, NULL, "audio", JackPortIsInput);
	ports_out = jack_get_ports (client, NULL, "audio", JackPortIsOutput);

	/* at least 1 row and one column for the labels*/
    rows = cols = 1;
    for (i = 0; ports_in[i]; ++i) {
        rows++;
    }
 	ports_in_num = i; // total number if input ports

    for (i = 0; ports_out[i]; ++i) {
        cols++;    
    }
	ports_out_num = i; // total number of output ports

	printf ("	ports_in %d,	ports_out %d\n", ports_in_num,ports_out_num);

    count = 0;

    /* Setup and create the gtk table for the matrix */
    table = gtk_table_new (rows, cols, TRUE);
    gtk_container_add (GTK_CONTAINER (window), table);

    /* 2D array of portCouple vars. This basically holds the indexes of the 
    couples of in-out ports represented by a 'cell' which in turn will be used
    by the callback for connecting/disconnecting ports */
	portCouple** port_couple_array;

    /* Allocate memory for the array.
    TODO check if there a 'more gtk' way of doing it */
	port_couple_array = (portCouple**) malloc(ports_out_num * sizeof(portCouple**));
	for (i = 0; i < ports_out_num; i++) {
        port_couple_array[i] = (portCouple*)malloc(ports_in_num * sizeof(portCouple));
	}
    
    /* Double for loop that creates the columns and rows of the table. 
    Columns hold out ports, rosw ins. The zero column and row hold 
    lables for the ports. This is why both for loops start from 1 and not 0 and 
    as a consequence all the use of i-1 and j-1 indexes.
    TODO Would it be better to put the in lables in the last row?*/
    
    /* All the size, font, colour forcing we do in here is usually bad in gtk.
    Here we do it try and to gain as much space as possible. 
	TODO Have all this stuff (fonts, sizes, colours) configurable in some way */    
    for (i = 1; i < cols; i++) {
            /* Setup, create and show a gtk label for the current out port*/
            gchar *labelText = g_strdup_printf ("%s",ports_out[i-1]);
            portLabel = gtk_label_new(labelText);           
            gtk_label_set_line_wrap_mode (GTK_LABEL (portLabel), PANGO_WRAP_WORD_CHAR);
            gtk_label_set_line_wrap (GTK_LABEL (portLabel), TRUE);
            gtk_widget_modify_font (GTK_WIDGET (portLabel), sansFont); 
            gtk_table_attach_defaults (GTK_TABLE (table), portLabel, 0, 1, i,i+1);
            gtk_widget_show (portLabel);
        for (j = 1; j < rows; j++) {                 
            /* Setup, create and show a gtk label for the current in port*/
            labelText = g_strdup_printf ("%s",ports_in[j-1]);
            portLabel = gtk_label_new(labelText);
            gtk_table_attach_defaults (GTK_TABLE (table), portLabel, j, j+1, 0,1);
            gtk_widget_modify_font (GTK_WIDGET (portLabel), sansFont); 
            gtk_widget_set_size_request (GTK_WIDGET (portLabel), 40,100);
            gtk_label_set_line_wrap_mode (GTK_LABEL (portLabel), PANGO_WRAP_WORD_CHAR);
            gtk_label_set_line_wrap (GTK_LABEL (portLabel), TRUE);
            gtk_widget_show (portLabel);

			/* Check if the two ports are already connected. 
			If so connected is set to true and toggle button will be down */
            connected = FALSE;
			if ((connections = jack_port_get_all_connections (client, jack_port_by_name(client, ports_in[j-1]))) != 0) {
				for (k = 0; connections[k]; k++) {
				    if (ports_out[i-1] == connections[k])
                        connected = TRUE;
				}
				free (connections);
			}

			/* Set up and create the toggle button for the in-out couple */
            gchar *buttonLabel;
            buttonLabel = g_strdup_printf ("%s",connected ? "o" : "x");
            button = gtk_toggle_button_new_with_label(buttonLabel);
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), connected);
			
			/* put the indexes  (ints) in the array */
			port_couple_array[i-1][j-1].out = i-1;
			port_couple_array[i-1][j-1].in = j-1;

            /* connect the click action to the callback passing the copuples' array */
            g_signal_connect (button,
							"clicked",
							G_CALLBACK (toggle_button_callback),
							(gpointer) &port_couple_array[i-1][j-1]);

            gtk_widget_modify_font (GTK_WIDGET (button), sansFont); 
            gtk_widget_set_size_request (GTK_WIDGET (button), 40, 20);
	  		gdk_color_parse ("yellow", &color);
			gtk_widget_modify_bg (GTK_WIDGET (button), GTK_STATE_ACTIVE, &color);

            gtk_table_attach_defaults (GTK_TABLE (table),button, j , j+1, i, i+1);
            gtk_widget_show (button);
        }
    }

    /* Settings for the table and show it */
    gtk_table_set_col_spacings(GTK_TABLE (table),5);
    /* Allow for cells of diffent widths and heigths.
    Makes sense here as we don't want for e.g.  the connections cells
    to be as wide as the label ones. */
    gtk_table_set_homogeneous (GTK_TABLE (table),0);
    gtk_widget_show (table);

    /* Finally show the window and start the gtk loop*/
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_widget_show (window); 
    gtk_main ();

	for (i = 0; i < ports_out_num; i++)	{
	  free(port_couple_array[i]);
	}
	free(port_couple_array);

    return 0;
}
