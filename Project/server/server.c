#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define SERIAL_PORT "/dev/cu.usbserial-0001"   // macOS device

int serial_fd;

/* Keep the last good line for display */
static char last_good_line[128] = "No Data";

/* Write string to UART */
void send_to_serial(const char *msg) {
    write(serial_fd, msg, strlen(msg));
}

/* Read at most one complete line from UART and update last_good_line */
static void read_latest_valid_line(void) {
    static char acc[256];
    static int len = 0;
    char ch;
    int r;

    while ((r = read(serial_fd, &ch, 1)) == 1) {
        if (ch == '\r') continue;
        if (ch == '\n') {
            acc[len] = '\0';
            double t; int s;
            if (sscanf(acc, "Temp:%lfC Speed:%d", &t, &s) == 2) {
                snprintf(last_good_line, sizeof(last_good_line),
                         "Temp:%.2fC   Speed:%d", t, s);
            }
            len = 0;
            break;
        }
        if ((unsigned char)ch >= 32 && (unsigned char)ch < 127) {
            if (len < (int)sizeof(acc) - 1) acc[len++] = ch;
        } else {
            len = 0; // drop garbage
        }
    }
}

/* Handle HTTP client */
void handle_client(int client_fd) {
    char buffer[2048];
    int n;

    /* Get newest valid line */
    read_latest_valid_line();

    /* Read HTTP request */
    n = read(client_fd, buffer, sizeof(buffer) - 1);
    if (n < 0) n = 0;
    buffer[n] = '\0';

    /* LED actions (active-low on MCU) */
    if (strstr(buffer, "led=1")) send_to_serial("1"); // LED1 ON
    if (strstr(buffer, "led=2")) send_to_serial("2"); // LED2 ON
    if (strstr(buffer, "led=3")) send_to_serial("3"); // LED3 ON
    if (strstr(buffer, "led=4")) send_to_serial("4"); // LED1 OFF
    if (strstr(buffer, "led=5")) send_to_serial("5"); // LED2 OFF
    if (strstr(buffer, "led=6")) send_to_serial("6"); // LED3 OFF

    /* LCD message */
    char *msg = strstr(buffer, "msg=");
    if (msg) {
        char lcd_msg[64];
        msg += 4;
        sscanf(msg, "%63[^& ]", lcd_msg);
        strcat(lcd_msg, "#"); // '#' terminator
        send_to_serial(lcd_msg);
    }

    /* Build styled webpage */
    char response[4096];
    snprintf(response, sizeof(response),
        "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
        "<!DOCTYPE html><html><head>"
        "<meta http-equiv='refresh' content='5'>"
        "<style>"
        "body{font-family:Arial,sans-serif;background:#f1f3f5;text-align:center}"
        "h1{color:#2c3e50;margin-top:24px}"
        ".card{margin:24px auto;padding:24px;width:360px;background:#fff;"
        "border-radius:12px;box-shadow:0 6px 16px rgba(0,0,0,.1)}"
        ".row{margin:6px}button{padding:10px 18px;margin:6px;border:0;"
        "border-radius:8px;background:#3498db;color:#fff;font-weight:600}"
        "button:hover{background:#2d83be}"
        "input{padding:10px;border:1px solid #ccd2d8;border-radius:8px}"
        "</style></head><body>"
        "<h1>LPC2129 Web Control</h1>"
        "<div class='card'><h3>Sensor Data</h3>"
        "<div style='font-size:20px;font-weight:700'>%s</div></div>"
        "<form method='POST'>"
        "<div class='row'>"
        "<button name='led' value='1'>LED1 ON</button>"
        "<button name='led' value='4'>LED1 OFF</button></div>"
        "<div class='row'>"
        "<button name='led' value='2'>LED2 ON</button>"
        "<button name='led' value='5'>LED2 OFF</button></div>"
        "<div class='row'>"
        "<button name='led' value='3'>LED3 ON</button>"
        "<button name='led' value='6'>LED3 OFF</button></div>"
        "<div class='row'>Message to LCD: "
        "<input name='msg' placeholder='Hello world'>"
        "<button type='submit'>Send</button>"
        "</div></form></body></html>",
        last_good_line
    );

    write(client_fd, response, strlen(response));
    close(client_fd);
}

int main(int argc, char *argv[]) {
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int server_fd;

    if (argc < 2) {
        printf("Usage: %s <IP address>\n", argv[0]);
        printf("Example: %s 192.168.1.45\n", argv[0]);
        exit(1);
    }

    /* Open Serial Port */
    serial_fd = open(SERIAL_PORT, O_RDWR | O_NOCTTY | O_SYNC);
    if (serial_fd < 0) { perror("Serial open"); exit(1); }

    struct termios tty;
    if (tcgetattr(serial_fd, &tty) < 0) { perror("tcgetattr"); exit(1); }

    cfsetispeed(&tty, B9600);
    cfsetospeed(&tty, B9600);
    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    tty.c_cflag |= (CREAD | CLOCAL);
    tty.c_iflag = 0;
    tty.c_oflag = 0;
    tty.c_lflag = 0;
    tty.c_cc[VMIN]  = 0;
    tty.c_cc[VTIME] = 10; // 1s timeout
    if (tcsetattr(serial_fd, TCSANOW, &tty) < 0) { perror("tcsetattr"); exit(1); }
    tcflush(serial_fd, TCIFLUSH);

    /* Create HTTP Socket */
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("Socket"); exit(1); }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) <= 0) {
        perror("Invalid IP address");
        exit(1);
    }

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(1);
    }

    if (listen(server_fd, 5) < 0) {
        perror("Listen failed");
        exit(1);
    }

    printf("Server running on http://%s:%d\n", argv[1], PORT);

    while (1) {
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);
        if (client_fd >= 0) handle_client(client_fd);
    }

    close(serial_fd);
    close(server_fd);
    return 0;
}

