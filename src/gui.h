#ifndef GUI_H_
#define GUI_H_

#define ON_CHAR "I"
#define OFF_CHAR "0"

#define MAX_PORTS_COL 128
#define MAX_PORTS_ROW 128

#define JM_JACK_SERVER_ERROR "Cannot connect to a JACK server. Is JACK server running?\njackmatrix requires a running JACK server to work!"

/* Structure holding the window and widgets as well as other stuff */
typedef struct
{
    GtkWidget *window;
    GtkWidget *container_box;
    GtkWidget *toolbar;
    GtkToolItem *refresh;
    GtkToolItem *save;
    GtkToolItem *sep;
    GtkToolItem *quit;
    GtkWidget *table;    
    GtkWidget *button;
    GtkWidget *portButtonCol[MAX_PORTS_COL];    // I don't like this. And atm it works for only rows
    GtkWidget *portButtonRow[MAX_PORTS_ROW];
    GtkWidget *buttonColLabel;
    gint rows;
    gint cols;
    gint count;
    PangoFontDescription *sansFont;
    GdkColor activeColor;
    GdkColor hoverColor;
    gboolean firstRun;
} MainWindow;

static void toggle_button_callback (GtkWidget *widget, gpointer data);
static void button_hover (GtkWidget *widget, gpointer data);
static void button_leave (GtkWidget *widget, gpointer data);
static void button_refresh_clicked (GtkWidget *widget, gpointer data);
static gboolean button_quit_clicked (GtkWidget *widget, gpointer data);
static gboolean delete_event ( GtkWidget *widget, GdkEvent  *event, gpointer data);

void errorConenctDialogue (char * connectType, const char* port_out, const char* port_in);
void errorJackserverDialogue (void);

#endif //GUI_H_
