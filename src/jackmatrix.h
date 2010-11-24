#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <jack/jack.h>

#define ON_CHAR "I"
#define OFF_CHAR "o"
#define JM_CONNECTED_COLOUR "yellow"
#define JM_ICON_FILE "jackmatrix.png"

#define MAX_PORTS_COL 128
#define MAX_PORTS_ROW 128

typedef struct
{
    int in;
    int out;
} portCouple;

/* Structure holding the widgets and jack stuff */
typedef struct main_window_type
{
    GtkWidget *window;
    GtkWidget *vertical_container_box;
    GtkWidget *toolbar;
	GtkWidget *scrolledWindow; 
    GtkWidget *table;

    GtkWidget *button;
    GtkWidget *portButtonCol[MAX_PORTS_COL];    // This is not very elegant
    GtkWidget *portButtonRow[MAX_PORTS_ROW];
    GtkWidget *buttonColLabel;
    GtkWidget *buttonRowLabel;

    GtkToolItem *refresh;
    GtkToolItem *save;
    GtkToolItem *sep;
    GtkToolItem *quit;

    gint rows, cols, count;

    PangoFontDescription *sansFont;
    GdkColor activeColor;
    GdkColor hoverColor;

    gboolean firstRun;
    int tableMakeReturnCode;
    
    jack_client_t *jackClient;
    jack_status_t *jackStatus;
    gint ports_in_num;
    gint ports_out_num;
	gboolean jackConnected;
} MainWindow;

/* Global variables */
const char **ports_in, **ports_out;
MainWindow win;

/* jack functions */
/* open a client in the MainWinow structure */
int openJackClient (MainWindow *win);
/* close a client in the MainWinow structure */
int closeJackClient (MainWindow *win);
/* get the jack ports - only audio ones*/
void get_jack_ports();

/* GUI functions */
/* function which makes the gui based on the jack ports; */
int make_gui();
