#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <jack/jack.h>

#define ON_CHAR "I"
#define OFF_CHAR "0"
#define MAX_PORTS_COL 128
#define MAX_PORTS_ROW 128

typedef struct
{
    int in;
    int out;
} portCouple;

/* Structure holding the window and widgets */
typedef struct main_window_type
{
    GtkWidget *window;
    GtkWidget *container_box;
    GtkWidget *toolbar;
    GtkToolItem *refresh;
    GtkToolItem *save;
    GtkToolItem *sep;
    GtkToolItem *quit;
    GtkWidget *button;
    GtkWidget *portButtonCol[MAX_PORTS_COL];    // I don't like this. And atm it works for only rows
    GtkWidget *buttonColLabel;
    GtkWidget *portButtonRow[MAX_PORTS_ROW];
    GtkWidget *table;
    gint rows, cols, count;
    PangoFontDescription *sansFont;
    GdkColor activeColor;
    GdkColor hoverColor;
    gboolean firstRun;
} MainWindow;

/* Global variables */
const char **ports_in, **ports_out, **connections;
jack_client_t *client;
jack_status_t status;
MainWindow win;
int ports_in_num, ports_out_num;

/* * jackmatrix functions */

void errorConenctDialogue (char * connectType, const char* port_out, const char* port_in);
/* get the jack ports - only audio ones*/
void get_jack_ports();

/* function which makes the (gtk) table with all the buttons of the matrix */
void make_table();
