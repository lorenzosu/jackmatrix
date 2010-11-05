/*
 * jackmatrix.c
 * Copyright (C) Lorenzo Sutton 2010 <lsutton@libero.it>
 * 
 * jackmatrix is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * jackmatrix is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <jack/jack.h>

#define ON_CHAR "I"
#define OFF_CHAR "0"
#define MAX_PORTS_COL 128
#define MAX_PORTS_ROW 128

typedef struct {
    int in;
    int out;
    } portCouple;

/* Structure holding the window and widgets */    
typedef struct main_window_type {
    GtkWidget *window;
    GtkWidget *container_box;
    GtkWidget *toolbar;
    GtkToolItem *refresh;
    GtkToolItem *save;
    GtkToolItem *sep;
    GtkToolItem *exit;
    GtkWidget *button;
    GtkWidget *portButtonCol[MAX_PORTS_COL];	// I don't like this. And atm it works for rows
    GtkWidget *buttonColLabel;
    GtkWidget *portButtonRow[MAX_PORTS_ROW];
    GtkWidget *table;
    gint rows, cols, count;
    PangoFontDescription *sansFont; 
    GdkColor activeColor;
    GdkColor hoverColor;
} MainWindow;
    
/* Global variables */
const char **ports_in, **ports_out, **connections; 
jack_client_t *client;
jack_status_t status;
MainWindow win;
int ports_in_num, ports_out_num;

/* Error dialogue in case we can't connect or disconnect ports */
void errorConenctDialogue (char * connectType, const char* port_out, const char* port_in)
{
    GtkWidget *dialog;
    dialog = gtk_message_dialog_new ( NULL,
                         GTK_DIALOG_MODAL,
                         GTK_MESSAGE_ERROR,
                         GTK_BUTTONS_OK,
                         "Cannot %s port\n"
                         "%s -> %s.\n"
                         "Has an application disconnected or closed?",
                         connectType, port_out, port_in);
    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);    
}


/* toggle callback funcion */
static void toggle_button_callback (GtkWidget *widget, gpointer  data)
{
    portCouple* p = data;
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))) {
        if (jack_connect (client, ports_out[p->out], ports_in[p->in]) ) {
            fprintf (stderr,"Cannot connect ports %s -> %s\n",ports_out[p->out], ports_in[p->in]);          
            errorConenctDialogue ("connect", ports_out[p->out], ports_in[p->in]);
        } else {
            g_print ("%s CONNECT TO %s\n", ports_out[p->out], ports_in[p->in]);
            gtk_button_set_label (GTK_BUTTON (widget), ON_CHAR);
        }
    } else {
        if (jack_disconnect (client, ports_out[p->out], ports_in[p->in]) ) {
            fprintf (stderr,"Cannot disconnect ports %s -> %s\n", ports_out[p->out], ports_in[p->in]);
            errorConenctDialogue ("disconnect", ports_out[p->out], ports_in[p->in]);
        } else {
            g_print ("%s DISCONNECT FROM %s\n", ports_out[p->out], ports_in[p->in]);
            gtk_button_set_label (GTK_BUTTON (widget), OFF_CHAR);
        }
    }
}

/* callback to change button labels' background when hovering a cell button*/
static void button_hover ( GtkWidget *widget, gpointer data)
{
    portCouple* p = data;
    GdkColor c;
    gdk_color_parse ("light blue", &c);

    gtk_widget_modify_bg (win.portButtonRow[p->out], GTK_STATE_NORMAL, &c);
    gtk_widget_modify_bg (win.portButtonCol[p->in], GTK_STATE_NORMAL, &c);
    gtk_widget_modify_bg (widget, GTK_STATE_PRELIGHT, &c);
}
/* callback to reset button labels' background when leaving a cell button*/
static void button_leave ( GtkWidget *widget, gpointer data)
{
    portCouple* p = data;
    GtkStyle *style = gtk_rc_get_style(widget);
    GdkColor c;
    gtk_style_lookup_color(style, "bg_color", &c);
    gtk_widget_modify_bg (win.portButtonRow[p->out], GTK_STATE_NORMAL, &c);
    gtk_widget_modify_bg (win.portButtonCol[p->in], GTK_STATE_NORMAL, &c);
}
/* window delete callback */
static gboolean delete_event ( GtkWidget *widget, GdkEvent  *event, gpointer data)
{
    gtk_main_quit ();
    return FALSE;
}

void button_quit_clicked(GtkWidget *widget, gpointer data)
{
    gtk_main_quit ();
}

void get_jack_ports()
{
     ports_in     = jack_get_ports (client, NULL, "audio", JackPortIsInput);
     ports_out    = jack_get_ports (client, NULL, "audio", JackPortIsOutput);
}

void refresh_table()
{
    int i,j,k;
    gboolean connected;
    /* at least 1 row and one column for the button labels */
    win.rows = win.cols = 1;
    for (i = 0; ports_in[i]; ++i) {
        win.rows++;
    }
    ports_in_num = i; // total number if JACK input ports

    for (i = 0; ports_out[i]; ++i) {
        win.cols++;    
    }
    ports_out_num = i; // total number of JACK output ports

    /*printf ("    ports_in %d,    ports_out %d\n", ports_in_num,ports_out_num);*/

    gtk_table_resize(GTK_TABLE(win.table),ports_in_num,ports_out_num);

    /* 2D array of portCouple vars. This basically holds the indexes of the 
    couples of in-out ports represented by a 'cell' which in turn will be used
    by the callbacks for connecting/disconnecting ports, hover etc. */
    portCouple** port_couple_array;

    /* Allocate memory for the array.
    TODO check if there is a 'more gtk' way of doing it */
    port_couple_array = (portCouple**) malloc(ports_out_num * sizeof(portCouple**));
    for (i = 0; i < ports_out_num; i++) {
        port_couple_array[i] = (portCouple*)malloc(ports_in_num * sizeof(portCouple));
    }
    
    /* Double for loop that creates the columns and rows of the table. 
    Columns hold out ports, rows ins. The zero column and row hold 
    button (lables) for the ports. This is why both for loops start from 1 and
	not 0 and as a consequence all the use of i-1 and j-1 indexes.
    TODO Better to put the in lables in the last row instead of the top?
    */
    
    /* All the size, font, colour forcing we do in here is usually bad in gtk.
    Here we do it to try and to gain as much space as possible. 
    TODO Have all this stuff (fonts, sizes, colours) configurable: rc? */    
    for (i = 1; i < win.cols; i++) {
            /* Setup, create and show a gtk button functioning as label for the current out port*/
            const gchar *labelText = g_strdup_printf ("%s",ports_out[i-1]);
            win.portButtonRow[i-1] = gtk_button_new_with_label (labelText);           
            //gtk_label_set_line_wrap_mode (GTK_LABEL (win.portLabelRow), PANGO_WRAP_WORD_CHAR);
            //gtk_label_set_line_wrap (GTK_LABEL (win.portLabelRow), TRUE);
            gtk_widget_modify_font (GTK_WIDGET (win.portButtonRow[i-1]), win.sansFont);
            gtk_widget_set_size_request (GTK_WIDGET (win.portButtonRow[i-1]), 200, 25);
            gtk_table_attach_defaults (GTK_TABLE (win.table), win.portButtonRow[i-1], 0, 1, i,i+1);
            gtk_widget_show (win.portButtonRow[i-1]);
        for (j = 1; j < win.rows; j++) {                 
            /* Setup, create and show a gtk button functioning as label for the current in port*/
            labelText = g_strdup_printf ("%s",ports_in[j-1]);

            win.portButtonCol[j-1] = gtk_button_new ();
			win.buttonColLabel = gtk_label_new (labelText);
			gtk_container_add (GTK_CONTAINER (win.portButtonCol[j-1]), win.buttonColLabel);

            gtk_table_attach_defaults (GTK_TABLE (win.table), win.portButtonCol[j-1], j, j+1, 0,1);
            gtk_widget_modify_font (GTK_WIDGET (win.buttonColLabel), win.sansFont); 
            gtk_widget_set_size_request (GTK_WIDGET (win.buttonColLabel), 40,80);
    	    gtk_label_set_line_wrap_mode (GTK_LABEL (win.buttonColLabel), PANGO_WRAP_WORD_CHAR);
	        gtk_label_set_line_wrap (GTK_LABEL (win.buttonColLabel), TRUE);
            gtk_widget_show (win.buttonColLabel );
            gtk_widget_show (win.portButtonCol[j-1]);

            /* Check if the two ports are already connected. 
            If so connected is set to TRUE and toggle button will be down */
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

            gchar *tipText;
            tipText = g_strdup_printf ("%s\n\t|\n%s",ports_out[i-1],ports_in[j-1]);

            buttonLabel = g_strdup_printf ("%s",connected ? ON_CHAR : OFF_CHAR);
            win.button = gtk_toggle_button_new_with_label(buttonLabel);

            gtk_widget_set_tooltip_text (win.button, tipText);
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (win.button), connected);
                      
            /* put the indexes (ints) in the array */
            port_couple_array[i-1][j-1].out = i-1;
            port_couple_array[i-1][j-1].in = j-1;

            /* connect the click event to the callback passing the copuples' array */
            g_signal_connect (win.button,
                            "clicked",
                            G_CALLBACK (toggle_button_callback),
                            (gpointer) &port_couple_array[i-1][j-1]);
			/* Connect the enter (hover) and leave events*/                            
            g_signal_connect (win.button,
                            "enter",
                            G_CALLBACK (button_hover),
                           (gpointer) &port_couple_array[i-1][j-1]);                          
            g_signal_connect (win.button,
                            "leave",
                            G_CALLBACK (button_leave),
                            (gpointer) &port_couple_array[i-1][j-1]);

			/* Change the style of the toggle buttons*/
            gtk_widget_modify_font (GTK_WIDGET (win.button), win.sansFont); 
            gtk_widget_set_size_request (GTK_WIDGET (win.button), 30, 10);
            gdk_color_parse ("yellow", &win.activeColor); // TODO hard-coding like this is bad! make a var
            gtk_widget_modify_bg (GTK_WIDGET (win.button), GTK_STATE_ACTIVE, &win.activeColor);

            gtk_table_attach_defaults (GTK_TABLE (win.table),win.button, j , j+1, i, i+1);
            gtk_widget_show (win.button);
        }
    }

    /* Modify the table and show it */
    gtk_table_set_col_spacings(GTK_TABLE (win.table),4);
    /* Allow for cells of diffent widths and heigths.
    Makes sense here as we don't want for e.g. the connections cells
    to be as wide as the label ones. */
    gtk_table_set_homogeneous (GTK_TABLE (win.table),0);
    gtk_widget_show (win.table);
}

void button_refresh_clicked(GtkWidget *widget, gpointer data)
{
    get_jack_ports(); 
    refresh_table();
}


int main(int argc, char *argv[] )
{
	printf ("\n --- jackmatrix: matrix layout JACK control ---\n\n");

    win.sansFont = pango_font_description_from_string ("Mono Sans 8");
    
    /* jack stuff */
    char *server_name = NULL;

    /* init gtk and create a new window */
    gtk_init (&argc, &argv);
    win.window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (win.window), "jackmatrix");

    /* connect delete event handler for the window */
    g_signal_connect (win.window, "delete-event", G_CALLBACK (delete_event), NULL);

    /*gtk_container_set_border_width (GTK_CONTAINER (win.window), 1);*/

    /* Open a client connection to the JACK server.  Don't start new server (JackNoStartServer option)
       Return if there's an error. */
    client = jack_client_open ("jackmatrix", JackNoStartServer, &status, server_name);
    if (client == NULL) {
            fprintf (stderr, "\nCannot connect to a JACK server. Is JACK server running?\n\n"
                             "jackmatrix requires a running JACK server to work!\n");
        if (! (status & JackServerFailed)) {
            fprintf (stderr, "jack_client_open() failed with status = 0x%2.0x\n", status);
        }
        return 1;
    }

    /* Get input and output ports, only audio ones */
    get_jack_ports();
    win.table = gtk_table_new(1, 1, TRUE);
    win.container_box = gtk_vbox_new(FALSE,2);
    gtk_container_add(GTK_CONTAINER (win.window), win.container_box);
    gtk_box_pack_end(GTK_BOX(win.container_box),win.table,TRUE,TRUE,0);
    gtk_widget_show(win.container_box);
    refresh_table();
  
    /* Toolbar
    */
    win.toolbar = gtk_toolbar_new();
    gtk_toolbar_set_style(GTK_TOOLBAR(win.toolbar), GTK_TOOLBAR_BOTH);
    /*gtk_container_set_border_width(GTK_CONTAINER(win.toolbar), 2);*/
    
    win.refresh = gtk_tool_button_new_from_stock(GTK_STOCK_REFRESH);
    gtk_toolbar_insert(GTK_TOOLBAR(win.toolbar), win.refresh, -1);
    g_signal_connect(G_OBJECT(win.refresh), "clicked", 
        G_CALLBACK(button_refresh_clicked), NULL);
    
    win.save = gtk_tool_button_new_from_stock(GTK_STOCK_SAVE);
    gtk_toolbar_insert(GTK_TOOLBAR(win.toolbar), win.save, -1);
    
    win.sep = gtk_separator_tool_item_new();
    gtk_toolbar_insert(GTK_TOOLBAR(win.toolbar), win.sep, -1); 
    
    win.exit = gtk_tool_button_new_from_stock(GTK_STOCK_QUIT);
    gtk_toolbar_insert(GTK_TOOLBAR(win.toolbar), win.exit, -1);
    g_signal_connect(G_OBJECT(win.exit), "clicked", 
        G_CALLBACK(button_quit_clicked), NULL);

    gtk_box_pack_start(GTK_BOX(win.container_box), win.toolbar, FALSE, FALSE, 0);



    /* Finally do last additions to window and show it */
    gtk_window_set_position(GTK_WINDOW(win.window), GTK_WIN_POS_CENTER);
	// add an icon
	const gchar * iconFilename = "icon128.png";
	GdkPixbuf *pixbuf;
	GError *iconError = NULL;
	pixbuf = gdk_pixbuf_new_from_file(iconFilename, &iconError);
	if(!pixbuf) {
		fprintf(stderr, "%s\n", iconError->message);
		g_error_free(iconError);
	}

    gtk_widget_show_all(win.window);
	gtk_window_set_icon(GTK_WINDOW(win.window), pixbuf);
    gtk_widget_show (win.window); 
    gtk_main ();

    return 0;
}
