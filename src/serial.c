#include "serial.h"

// https://blog.mbedded.ninja/programming/operating-systems/linux/linux-serial-ports-using-c-cpp/

void setup_serial_port(struct termios *tty)
{
    /*
    Parity
    */
    tty->c_cflag &= ~PARENB; // Clear parity bit, disabling parity (most common)
    // tty->c_cflag |= PARENB;  // Set parity bit, enabling parity

    /*
    Num. Stop Bits
    */
    tty->c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in communication (most common)
    // tty->c_cflag |= CSTOPB;  // Set stop field, two stop bits used in communication

    /*
    Number Of Bits Per Byte
    */
    // tty->c_cflag &= ~CSIZE; // Clear all the size bits, then use one of the statements below
    // tty->c_cflag |= CS5;    // 5 bits per byte
    // tty->c_cflag |= CS6;    // 6 bits per byte
    // tty->c_cflag |= CS7;    // 7 bits per byte
    tty->c_cflag |= CS8; // 8 bits per byte (most common)

    /*
    Flow Control 
    */
    tty->c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control (most common)
    // tty->c_cflag |= CRTSCTS;  // Enable RTS/CTS hardware flow control

    /*
    CREAD and CLOCAL
    */
    tty->c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)

    /*
    Disabling Canonical Mode
    */
    tty->c_lflag &= ~ICANON;

    /*
    Echo
    */
    tty->c_lflag &= ~ECHO;   // Disable echo
    tty->c_lflag &= ~ECHOE;  // Disable erasure
    tty->c_lflag &= ~ECHONL; // Disable new-line echo

    /*
    Disable Signal Chars
    */
    tty->c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP

    /*
    Software Flow Control (IXOFF, IXON, IXANY)
    */
    tty->c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl

    /*
    Disabling Special Handling Of Bytes On Receive
    */
    tty->c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL); // Disable any special handling of received bytes

    /*
    Output Modes
    */
    tty->c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
    tty->c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
    // tty->c_oflag &= ~OXTABS; // Prevent conversion of tabs to spaces (NOT PRESENT IN LINUX)
    // tty->c_oflag &= ~ONOEOT; // Prevent removal of C-d chars (0x004) in output (NOT PRESENT IN LINUX)

    /*
    VMIN and VTIME
    */
    tty->c_cc[VTIME] = 10; // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
    tty->c_cc[VMIN] = 0;

    /*
    Baud Rate
    B0,  B50,  B75,  B110,  B134,  B150,  B200, B300, B600, B1200, B1800, B2400, B4800, B9600, B19200, B38400, B57600, B115200, B230400, B460800
    */
    // cfsetispeed(tty, B9600);
    // cfsetospeed(tty, B9600);
    cfsetspeed(tty, B9600); // Both
}

int open_serial_port(const char *SERIAL_PORT, struct termios *tty)
{
    int serial_port = open(SERIAL_PORT, O_RDWR);
    if (serial_port < 0)
    {
        printf("Error %i from open: %s\n", errno, strerror(errno));
    }

    setup_serial_port(tty);
    if (tcgetattr(serial_port, tty) != 0)
    {
        printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
    }
    // Save tty settings, also checking for error
    if (tcsetattr(serial_port, TCSANOW, tty) != 0)
    {
        printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
    }
    printf("%s Opened Successfully\n", SERIAL_PORT);
    return serial_port;
}

void writing_serial_port(int serial_port, unsigned char *msg, int length)
{
    write(serial_port, msg, length);
}

int reading_serial_port(int serial_port, char *buffer)
{
    return read(serial_port, &buffer, sizeof(buffer));
}

void close_serial_port(int serial_port)
{
    close(serial_port);
}