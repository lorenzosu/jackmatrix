

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

/* Error dialogue in case we can't start a jack server */
void errorJackserverDialogue (void)
{
    GtkWidget *dialog;
    dialog = gtk_message_dialog_new ( NULL,
                                      GTK_DIALOG_MODAL,
                                      GTK_MESSAGE_ERROR,
                                      GTK_BUTTONS_OK,
                                      JM_JACK_SERVER_ERROR,
                                      connectType, port_out, port_in);
    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);
}
