#include "serial.h"
#include <pthread.h>
#include <stdio.h> //stdin

struct termios tty;
char serial_buffer[256];
int serial_port;
pthread_t reading_serial_thread;

void *reading_serial(void *vargp)
{
    while (1)
    {
        printf("Start reading\n");
        int num_bytes = reading_serial_port(serial_port, serial_buffer);
        if (num_bytes < 0)
        {
            printf("Error reading: %i %s\n", errno, strerror(errno));
            return NULL;
        }
        else
        {
            printf("Read %i bytes. Received message: %s\n", num_bytes, serial_buffer);
        }
    }
    return NULL;
}

int main()
{
    serial_port = open_serial_port("/dev/ttyS0", &tty);

    pthread_create(&reading_serial_thread, NULL, reading_serial, NULL);

    char input_buffer[5];
    while (1)
    {
        printf("Input you data here: ");
        if (fgets(input_buffer, 5, stdin))
        {
            writing_serial_port(serial_port, input_buffer, 5);
        }
    }

    pthread_join(reading_serial_thread, NULL);
    close_serial_port(serial_port);
    return 0;
}
