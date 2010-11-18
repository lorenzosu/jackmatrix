#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <jack/jack.h>
#include "jackmatrix.h"
#include "callbacks.h"

/* callback function when button for a port couple is toggled */
void toggle_button_callback (GtkWidget *widget, gpointer  data)
{
    portCouple* p = data;
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
    {
        if (jack_connect (client, ports_out[p->out], ports_in[p->in]) )
        {
            fprintf (stderr,"Cannot connect ports %s -> %s\n",ports_out[p->out], ports_in[p->in]);
            errorConenctDialogue ("connect", ports_out[p->out], ports_in[p->in]);
        }
        else
        {
            g_print ("%s CONNECT TO %s\n", ports_out[p->out], ports_in[p->in]);
            gtk_button_set_label (GTK_BUTTON (widget), ON_CHAR);
        }
    }
    else
    {
        if (jack_disconnect (client, ports_out[p->out], ports_in[p->in]) )
        {
            fprintf (stderr,"Cannot disconnect ports %s -> %s\n", ports_out[p->out], ports_in[p->in]);
            errorConenctDialogue ("disconnect", ports_out[p->out], ports_in[p->in]);
        }
        else
        {
            g_print ("%s DISCONNECT FROM %s\n", ports_out[p->out], ports_in[p->in]);
            gtk_button_set_label (GTK_BUTTON (widget), OFF_CHAR);
        }
    }
}

/* callback to change button labels' background when hovering a cell button*/
void button_hover ( GtkWidget *widget, gpointer data)
{
    portCouple* p = data;
    GdkColor c;
    gdk_color_parse ("light blue", &c);

    gtk_widget_modify_bg (win.portButtonRow[p->out], GTK_STATE_NORMAL, &c);
    gtk_widget_modify_bg (win.portButtonCol[p->in], GTK_STATE_NORMAL, &c);
    gtk_widget_modify_bg (widget, GTK_STATE_PRELIGHT, &c);
}

/* callback to reset button labels' background when leaving a cell button*/
void button_leave ( GtkWidget *widget, gpointer data)
{
    portCouple* p = data;
    GtkStyle *style = gtk_rc_get_style(widget);
    GdkColor c;
    gtk_style_lookup_color(style, "bg_color", &c);
    gtk_widget_modify_bg (win.portButtonRow[p->out], GTK_STATE_NORMAL, &c);
    gtk_widget_modify_bg (win.portButtonCol[p->in], GTK_STATE_NORMAL, &c);
}

/* callback function when refresh button is clicked */
void button_refresh_clicked (GtkWidget *widget, gpointer data)
{
    win.firstRun = FALSE;
    get_jack_ports();
    make_table();
}

/* callback function when quit button is clicked */
gboolean button_quit_clicked(GtkWidget *widget, gpointer data)
{
    gtk_main_quit ();
    return FALSE;
}

/* window delete callback */
gboolean delete_event ( GtkWidget *widget, GdkEvent  *event, gpointer data)
{
    gtk_main_quit ();
    return FALSE;
}
