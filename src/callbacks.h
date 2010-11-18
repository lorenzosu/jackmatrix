#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <jack/jack.h>

/* callback function when button for a port couple is toggled */
void toggle_button_callback (GtkWidget *widget, gpointer  data);

/* callback to change button labels' background when hovering a cell button*/
void button_hover ( GtkWidget *widget, gpointer data);

/* callback to reset button labels' background when leaving a cell button*/
void button_leave ( GtkWidget *widget, gpointer data);

/* callback function when refresh button is clicked */
void button_refresh_clicked (GtkWidget *widget, gpointer data);

/* callback function when quit button is clicked */
gboolean button_quit_clicked(GtkWidget *widget, gpointer data);

/* window delete callback */
gboolean delete_event ( GtkWidget *widget, GdkEvent  *event, gpointer data);
