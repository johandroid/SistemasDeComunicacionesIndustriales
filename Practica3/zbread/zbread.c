#include <stdio.h>
#include <unistd.h>		//Used for UART
#include <fcntl.h>			//Used for UART
#include <termios.h>		//Used for UART
#include <string.h>

#define UART_DEV "/dev/ttyO4"
#define BUFF_SIZE 256
#define CR '\r'

int read_line(int fstream, char *buffer) {
    int idx = 0;
    int retries = 25;
    while (idx < BUFF_SIZE && retries--) {
       idx += read(fstream, &buffer[idx], BUFF_SIZE - idx - 1);
       if (buffer[idx - 1] == CR)  {
           idx -= 1;
           break;
       }
       usleep(100000);
    }

    buffer[idx] = '\0';
    return idx;
}

int at_cmd(int fstream, char *req, char *resp) {
    int tx_bytes = 0;
    int rx_bytes = 0;

    char *p_req;
    p_req = req;
    while (*p_req)
        p_req++;

    tx_bytes = write(fstream, req, (p_req - req));
    if (tx_bytes < 0) {
        fprintf(stderr, "Error - Tx error\n");
    }

    if (read_line(fstream, resp) == 0) {
        fprintf(stderr, "Error - Rx error\n");
        return 0;
    }
    /*
    if (strncmp((const char*)resp, "OK\r", 2) != 0) {
        fprintf(stderr, "Error - Rx error\n");
        return 0;
    }
    */

    return rx_bytes;
}

int main() {
    int uart_fstream = -1;

    //Open in non blocking read/write mode
    uart_fstream = open(UART_DEV, O_RDWR | O_NOCTTY | O_NDELAY);
    if (uart_fstream == -1) {
        fprintf(stderr, "Error - Unable to open UART.  Ensure it is not in use by another application\n");
    }

    struct termios options;
    tcgetattr(uart_fstream, &options);
    // Configure port
    options.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
    options.c_iflag = IGNPAR;
    options.c_oflag = 0;
    options.c_lflag = 0;
    tcflush(uart_fstream, TCIFLUSH);
    tcsetattr(uart_fstream, TCSANOW, &options);

    char *cfg_at_commands[] = {
        "+++",
        "atvr\r",
        "athv\r",
        "atni PUESTO09\r",
        "atsm 4\r",
        "atso 2\r",
        "atct 258\r",
        "atac\r",
        "atwr\r",
        "atcn\r",
        NULL
    };


    // Configuración
    int i = 0;
    char resp[BUFF_SIZE];
    while(cfg_at_commands[i]) {
        printf(">> %s\n", cfg_at_commands[i]);
        at_cmd(uart_fstream, cfg_at_commands[i], &resp[0]);
        printf("<< %s\n", resp);
        i++;
    }

    char *status_at_commands[] = {
        "+++",
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

    char *info_fmt[] = {
        NULL,
        "AI %s, ",
        "SH %s, ",
        "SL %s, ",
        "CH %s, ",
        "DB %s, ",
        "OP %10s, ",
        "OI %s, ",
        "NI %s, ",
        "MY %s, ",
        "NC %s\n",
        NULL};

    printf("-------------------------------------\n");
    while(1) {
        i = 0;
        sleep(2);
        while(status_at_commands[i]) {
            at_cmd(uart_fstream, status_at_commands[i], &resp[0]);
            if (info_fmt[i])
                printf(info_fmt[i], resp);
            i++;
        }
    }

    /*
    // leemos lo que envia el server
    unsigned char buffer[BUFF_SIZE];
    while (1) {
        if (read_line(uart_fstream, buffer))
            printf("<< %s\n", buffer);
    }
    */

    close(uart_fstream);
	return 0;
}
