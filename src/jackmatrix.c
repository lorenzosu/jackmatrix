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
#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <jack/jack.h>

#include "jackmatrix.h"
#include "gui.h"

void get_jack_ports()
{
    ports_in = jack_get_ports (client, NULL, "audio", JackPortIsInput);
    ports_out = jack_get_ports (client, NULL, "audio", JackPortIsOutput);
}

/* Function that makes the table based on the ports */
void make_table()
{
    int i,j,k;
    gboolean connected;
    /* at least 1 row and one column for the button labels */
    win.rows = win.cols = 1;
    for (i = 0; ports_in[i]; ++i)
    {
        win.rows++;
    }
    ports_in_num = i; // total number if JACK input ports

    for (i = 0; ports_out[i]; ++i)
    {
        win.cols++;
    }
    ports_out_num = i; // total number of JACK output ports

    /* here we destroy the table and re-create it if we are refreshing */
    if (win.firstRun == TRUE)
    {
        gtk_table_resize(GTK_TABLE(win.table),ports_in_num,ports_out_num);
    }
    else
    {
        gtk_widget_hide (win.table);
        gtk_widget_destroy (win.table);
        gtk_window_resize (GTK_WINDOW (win.window), 1,1);
        win.table = gtk_table_new(ports_in_num,ports_out_num, TRUE);
        gtk_box_pack_end(GTK_BOX(win.container_box),win.table,TRUE,TRUE,0);
    }
    /* 2D array of portCouple vars. This basically holds the indexes of the
    couples of in-out ports represented by a 'cell' which in turn will be used
    by the callbacks for connecting/disconnecting ports, hover etc. */
    portCouple** port_couple_array;

    /* Allocate memory for the array.
    TODO check if there is a 'more gtk' way of doing it */
    port_couple_array = (portCouple**) malloc(ports_out_num * sizeof(portCouple**));
    for (i = 0; i < ports_out_num; i++)
    {
        port_couple_array[i] = (portCouple*)malloc(ports_in_num * sizeof(portCouple));
    }

    /* Double for loop that creates the columns and rows of the table.
    Columns hold out ports, rows ins. The zero column and row hold
    button (labels) for the ports. This is why both for loops start from 1 and
    not 0 and as a consequence all the use of i-1 and j-1 indexes.
    TODO Better to put the in labels in the last row instead of the top?
    */

    /* All the size, font, color forcing we do in here is usually bad in gtk.
    Here we do it to try and to gain as much space as possible.
    TODO Have all this stuff (fonts, sizes, colours) configurable: rc? */
    for (i = 1; i < win.cols; i++)
    {
        /* Setup, create and show a gtk button functioning as label for the current out port*/
        const gchar *labelText = g_strdup_printf ("%s",ports_out[i-1]);
        win.portButtonRow[i-1] = gtk_button_new_with_label (labelText);
        //gtk_label_set_line_wrap_mode (GTK_LABEL (win.portLabelRow), PANGO_WRAP_WORD_CHAR);
        //gtk_label_set_line_wrap (GTK_LABEL (win.portLabelRow), TRUE);
        gtk_widget_modify_font (GTK_WIDGET (win.portButtonRow[i-1]), win.sansFont);
        gtk_widget_set_size_request (GTK_WIDGET (win.portButtonRow[i-1]), 200, 25);
        gtk_table_attach_defaults (GTK_TABLE (win.table), win.portButtonRow[i-1], 0, 1, i,i+1);
        gtk_widget_show (win.portButtonRow[i-1]);
        for (j = 1; j < win.rows; j++)
        {
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
            if ((connections = jack_port_get_all_connections (client, jack_port_by_name(client, ports_in[j-1]))) != 0)
            {
                for (k = 0; connections[k]; k++)
                {
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
    /* Allow for cells of different widths and heights.
    Makes sense here as we don't want for e.g. the connections cells
    to be as wide as the label ones. */
    gtk_table_set_homogeneous (GTK_TABLE (win.table),0);
    gtk_widget_show (win.table);
}

int main(int argc, char *argv[] )
{
    printf ("\n --- jackmatrix: matrix layout JACK control version %s---\n\n",PACKAGE STRING);

    win.sansFont = pango_font_description_from_string ("Mono Sans 8");
    /* jack stuff */
    char *server_name = NULL;

    /* init gtk and create a new window */
    gtk_init (&argc, &argv);
    win.window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (win.window), "jackmatrix");

    /* connect delete event handler for the window */
    g_signal_connect (win.window, "delete-event", G_CALLBACK (delete_event), NULL);

    /* Open a client connection to the JACK server.
       Don't start new server (JackNoStartServer option)
       Return if there's an error. */
    client = jack_client_open ("jackmatrix", JackNoStartServer, &status, server_name);
    if (client == NULL)
    {
        fprintf (stderr, JM_JACK_SERVER_ERROR);
        errorJackserverDialogue ();
        if (! (status & JackServerFailed))
        {
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
    win.firstRun = TRUE;
    make_table();

    /* Toolbar with stock buttons */
    win.toolbar = gtk_toolbar_new();
    gtk_toolbar_set_style(GTK_TOOLBAR(win.toolbar), GTK_TOOLBAR_BOTH);
    win.refresh = gtk_tool_button_new_from_stock(GTK_STOCK_REFRESH);
    gtk_toolbar_insert(GTK_TOOLBAR(win.toolbar), win.refresh, -1);
    g_signal_connect(G_OBJECT(win.refresh),
                     "clicked",
                     G_CALLBACK(button_refresh_clicked),
                     NULL);
    win.save = gtk_tool_button_new_from_stock(GTK_STOCK_SAVE);
    gtk_toolbar_insert(GTK_TOOLBAR(win.toolbar), win.save, -1);
    /* TODO add save functionality */
    win.sep = gtk_separator_tool_item_new();
    gtk_toolbar_insert(GTK_TOOLBAR(win.toolbar), win.sep, -1);
    win.quit = gtk_tool_button_new_from_stock(GTK_STOCK_QUIT);
    gtk_toolbar_insert(GTK_TOOLBAR(win.toolbar), win.quit, -1);
    g_signal_connect(G_OBJECT(win.quit), "clicked",
                     G_CALLBACK(button_quit_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(win.container_box), win.toolbar, FALSE, FALSE, 0);

    /* Finally do last additions to window and show it */
    gtk_window_set_position(GTK_WINDOW(win.window), GTK_WIN_POS_CENTER);
    // add an icon
    const gchar * iconFilename = "icon128.png";
    GdkPixbuf *pixbuf;
    GError *iconError = NULL;
    pixbuf = gdk_pixbuf_new_from_file(iconFilename, &iconError);
    if(!pixbuf)
    {
        fprintf(stderr, "%s\n", iconError->message);
        g_error_free(iconError);
    }

    gtk_widget_show_all(win.window);
    gtk_window_set_icon(GTK_WINDOW(win.window), pixbuf);
    gtk_widget_show (win.window);
    gtk_main ();

    return 0;
}
