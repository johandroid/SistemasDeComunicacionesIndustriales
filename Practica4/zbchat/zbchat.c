#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>		//Used for UART
#include <fcntl.h>		//Used for UART
#include <termios.h>	//Used for UART
#include <string.h>
#include <time.h>

#define UART_DEV "/dev/ttyO4"
#define BUFF_SIZE 256
#define CR '\r'

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
    int nbytes = 0;
    int retries = 25;

    memset(buffer, 0x00, BUFF_SIZE);

    while (idx < BUFF_SIZE && retries--) {
       nbytes = read(fstream, &buffer[idx], BUFF_SIZE - idx - 1);
       if (nbytes > 0) idx += nbytes;
       //fprintf(stderr, "El idx es %d y el buffer es %s \n",idx,buffer);
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
        return 0;
    }

    nbytes = read_line(fstream, rbuffer);
    if (nbytes <= 0) {
        //fprintf(stderr, "Error - Rx error y  el fstream es: %d y el rbuffer %s \n",fstream,rbuffer);
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
    "atni JOHAN\r",
    "atsm 4\r",
    "atso 2\r",
    "atct 258\r",
    "atac\r",
    "atwr\r",
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

void uart_cfg(int uart_fstream) {

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

int zb_cfg_node(int fstream) {
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

int zb_get_panid(int fstream) {
    char rbuffer[BUFF_SIZE];
    at_cmd(fstream, "atoi\r", &rbuffer[0]);
    return strtol(rbuffer, NULL, 16);
}

int enter_at_mode(int fstream) {
    return at_cmd_ok(fstream, "+++");
}

int exit_at_mode(int fstream) {
    return at_cmd_ok(fstream, "atcn\r");
}

int zb_dump_info(int fstream) {
    int i = 0;
    char rbuffer[BUFF_SIZE];

    while(status_at_commands[i]) {
        at_cmd(fstream, status_at_commands[i], &rbuffer[0]);
        if (info_fmt[i])
            printf(info_fmt[i], rbuffer);
        i++;
    }

    return 1;
}

int uart_open(const char * dev) {

    // Intentamos abrir en modo non-blocking read/write
    int uart_fstream = open(dev, O_RDWR | O_NOCTTY | O_NDELAY);
    if (uart_fstream == -1) {
        fprintf(stderr,
                "Error - Unable to open UART '%s'.  "
                "Ensure it is not in use by another application\n", UART_DEV);
    }

    return uart_fstream;
}

int main() {
    int uart_fd = uart_open(UART_DEV);
    if (uart_fd == -1)
        return 1;

    uart_cfg(uart_fd);

    char rbuffer[BUFF_SIZE];

    while (1) {
        if(enter_at_mode(uart_fd)) {
            usleep(1e5);
            printf("Configurando nodo...\n");
            zb_cfg_node(uart_fd);

            printf("Obteniendo información de la red...\n");
            int pan_id;
            while ((pan_id = zb_get_panid(uart_fd)) == 0xffff) {
                // Esperamos una conexión
                usleep(1e5);
            }

            zb_dump_info(uart_fd);
            exit_at_mode(uart_fd);
        } else {
            usleep(1e5);
            continue;
        }

        // Lectura datos enviados por la red
        time_t wdt = clock();
        float seconds = 0.0;
        while (seconds < 5) {
            memset(rbuffer, 0x00, BUFF_SIZE);
            int n = read(uart_fd, &rbuffer[0], BUFF_SIZE);
            if (n > 0){
                wdt = clock();
                fprintf(stderr, "%s", rbuffer);
            }

            seconds = (clock() - wdt) / CLOCKS_PER_SEC;
            //printf("%0.3f s\r", seconds);
            //fflush(stdout);
            //usleep(1e3);
        }
    }
    close(uart_fd);
    return 0;
}
