#ifndef JACKMATRIX_H_
#define JACKMATRIX_H_

#define MAX_PORTS_COL 128
#define MAX_PORTS_ROW 128

typedef struct
{
    int in;
    int out;
} portCouple;

/* Global variables */
MainWindow win;
const char **ports_in, **ports_out, **connections;
jack_client_t *client;
jack_status_t status;
int ports_in_num, ports_out_num;

void get_jack_ports(void);
void make_table(void);

#endif //GUI_H_
