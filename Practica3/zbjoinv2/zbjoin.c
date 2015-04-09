#include <stdio.h>
#include <unistd.h>		//Used for UART
#include <fcntl.h>		//Used for UART
#include <termios.h>	//Used for UART
#include <string.h>

#define UART_DEV "/dev/ttyO4"
#define BUFF_SIZE 256
#define CR '\r'

/// @brief Configura la uart, damos esto por correcto
void cfg_uart(int uart_fstream);
/// @brief Configura el nodo ZigBee
int cfg_zb_client_node(int fstream);
/*
void replace(char* buffer, char old_char, char new_char) {
    char *found = 0;
    while (1) {
        found = strchr(buffer, old_char);
        if (found < buffer) break;
        *found = new_char;
    }
}
*/

int read_line(int fstream, char *buffer) {
    int idx = 0;
    int retries = 15;

    memset(buffer, 0x00, BUFF_SIZE);

    while (idx < BUFF_SIZE && retries--) {
       idx += read(fstream, &buffer[idx], BUFF_SIZE - idx - 1);
       if (idx > 0 && buffer[idx - 1] == CR) {
           buffer[idx - 1] = '\0';
           break;
       }
       usleep(1e5); // 100ms
    }

    return idx;
}

int at_cmd(int fstream, char *req, char *rbuffer) {
    int nbytes = 0;

    char *p_req;
    p_req = req;
    while (*p_req)
        p_req++;

    nbytes = write(fstream, req, (p_req - req));
    if (nbytes <= 0) {
        fprintf(stderr, "Error - Tx error\n");
    }

    nbytes = read_line(fstream, rbuffer);
    if (nbytes <= 0) {
        fprintf(stderr, "Error - Rx error\n");
        return 0;
    }

    return nbytes;
}

int at_cmd_ok(int fstream, char *req) {
    char rbuffer[BUFF_SIZE];
    int rbytes = at_cmd(fstream, req, &rbuffer[0]);
    return rbytes > 0 && strstr(rbuffer, "OK") != NULL ? 1: 0;
}

// Comandos de configuración
char *cfg_at_commands[] = {
    "atni Johan\r",
    "atsm 4\r",
    "atso 2\r",
    "atct 258\r",
    "atac\r",
    "atwr\r",
    "atcn\r",
    NULL
};

// Comandos para obtener el estado de la red
char *status_at_commands[] = {
    "atai\r",
    "atsh\r",
    "atsl\r",
    "atch\r",
    "atdb\r",
    "atop\r",
    "atoi\r",
    "atni\r",
    "atmy\r",
    "atnc\r",
    "atcn\r",
    NULL
};

// Formato para mostrar el estado de la red
char *info_fmt[] = {
    "AI %02s, ",
    "SH %s, ",
    "SL %s, ",
    "CH %s, ",
    "DB %s, ",
    "OP %14s, ",
    "OI %s, ",
    "NI %s, ",
    "MY %s, ",
    "NC %s\n",
    NULL
};

/*char* get_zb_panid(int fstream) {
    if(!at_cmd_ok(fstream, "+++"))
        return 0;

    char rbuffer[BUFF_SIZE];
    char *panid = malloc(5);

    usleep(1e5);

    // Esperamos in id de red válido
    at_cmd(fstream, "atoi\r", &rbuffer[0]);
    strncpy(panid, rbuffer, 4);
    panid[4] = '\0';

    return panid;
}

void dump_zb_info() {

    while(status_at_commands[i]) {
        at_cmd(uart_fstream, status_at_commands[i], &rbuffer[0]);
        if (info_fmt[i])
            printf(info_fmt[i], rbuffer);
        i++;
    }
}*/

int main() {

    // Intentamos abrir en modo non-blocking read/write
    int uart_fstream = open(UART_DEV, O_RDWR | O_NOCTTY | O_NDELAY);
    if (uart_fstream == -1) {
        fprintf(stderr,
                "Error - Unable to open UART.  "
                "Ensure it is not in use by another application\n");

        return 1;
    }

    cfg_uart(uart_fstream);

    int i = 0;
    char rbuffer[BUFF_SIZE];

    while (1) {
        cfg_zb_client_node(uart_fstream);
        printf("------------------------------------------------------------\n");

        //get_zb_panid(netid, i, rbuffer, uart_fstream);

        // Lectura del estado de la red
        //i = 0;

        // Lectura datos enviados por la red
        int cnt = 70;
        while (cnt-- > 0) {
            memset(rbuffer, 0x00, BUFF_SIZE);
            i = read(uart_fstream, &rbuffer[0], BUFF_SIZE);

            if (i> 0) {
                printf("%s", rbuffer);
                fflush(stdout);
                cnt = 70;
            }

            usleep(1e5);
        }
    }

    close(uart_fstream);
    return 0;
}


void cfg_uart(int uart_fstream) {

    struct termios options;
    tcgetattr(uart_fstream, &options);
    // Configure port
    options.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
    options.c_iflag = IGNPAR;
    options.c_oflag = 0;
    options.c_lflag = 0;
    tcflush(uart_fstream, TCIFLUSH);
    tcsetattr(uart_fstream, TCSANOW, &options);
}

int cfg_zb_client_node(int fstream) {

    if(!at_cmd_ok(fstream, "+++"))
        return 0;

    usleep(1e5);

    int i = 0;
    char rbuffer[BUFF_SIZE];

    // Configuración del nodo ZigBee
    while(cfg_at_commands[i]) {
        at_cmd(fstream, cfg_at_commands[i], &rbuffer[0]);
        // printf(">> %s\n", cfg_at_commands[i]);
        // printf("<< %s\n", rbuffer);
        i++;
    }
    return 1;
}
