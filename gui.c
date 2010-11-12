/* Structure holding the window and widgets */
typedef struct
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
