#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <jack/jack.h>
#include "jackmatrix.h"
#include "callbacks.h"
#include "dialogues.h"

/* callback function when button for a port couple is toggled */
void toggle_button_callback (GtkWidget *widget, gpointer  data)
{
	if (win.jackConnected == FALSE)
	{
		return;
	}
    portCouple* p = data;
    const char* actionString;
    char* buttonLabel;
    int returnCode;
    gboolean isToggled;
    isToggled = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
    if (isToggled == TRUE)
    {
        actionString = "CONNECT";
        buttonLabel = ON_CHAR;
        returnCode = jack_connect (win.jackClient, ports_out[p->out], ports_in[p->in]);
    }
    else
    {
        actionString = "DISCONNECT";
        buttonLabel = OFF_CHAR;
        returnCode = jack_disconnect (win.jackClient, ports_out[p->out], ports_in[p->in]);
    }
    if (returnCode == 0)
    {
        g_print ("%s -> %s -> %s\n", ports_out[p->out],actionString, ports_in[p->in]);
        gtk_button_set_label (GTK_BUTTON (widget), buttonLabel);
    }
    else
    {
        fprintf (stderr,"Cannot %s ports %s -> %s\n", ports_out[p->out], actionString, ports_in[p->in]);
        errorConenctDialogue (actionString, ports_out[p->out], ports_in[p->in]);    
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
    closeJackClient (&win);
	/* try opening jack client here in case the jack server has been closed */
    if ((openJackClient (&win)) == 0)
	{
    	/* TODO add a wait cursor because we may take long */
		win.jackConnected = TRUE;
    	win.tableMakeReturnCode = make_gui();
	}
}

/* callback function when quit button is clicked */
gboolean button_quit_clicked (GtkWidget *widget, gpointer data)
{
    closeJackClient (&win);
    gtk_main_quit ();
    return FALSE;
}

/* window delete callback */
gboolean delete_event ( GtkWidget *widget, GdkEvent  *event, gpointer data)
{
    closeJackClient (&win);
    gtk_main_quit ();
    return FALSE;
}
