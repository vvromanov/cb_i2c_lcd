#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "i2c-dev.h"

#define LCD_BL 1<<3
#define LCD_EN 1<<2
#define LCD_RW 1<<1
#define LCD_RS 1<<0
#define LCD_D4 1<<4
#define LCD_D5 1<<5
#define LCD_D6 1<<6
#define LCD_D7 1<<7

int fd=-1;

uint32_t lcd_backlight=LCD_BL;

int init_i2c_bus(uint8_t bus_number, uint16_t device_addr)
{
    unsigned long funcs;
    char device[100];
    snprintf(device, sizeof(device), "/dev/i2c-%u", bus_number);
    fd=open(device, O_RDWR);
    if(fd<0){
        perror("Failed to open I2C device!");
        exit(1);
    }
 
//    ioctl(fd, I2C_TIMEOUT, 3);    // i2c timeout, 1 for 10ms ; if too small, i2c may lose response
//   ioctl(fd, I2C_RETRIES, 3);    // i2c retry limit
  /* check adapter functionality */
    if (ioctl(fd, I2C_FUNCS, &funcs) < 0) {
        fprintf(stderr, "Error: Could not get the adapter functionality matrix: %s\n", strerror(errno));
        exit(1);
    }
//   printf("Functionality matrix %lx\n", funcs);
    if (!(funcs & I2C_FUNC_SMBUS_WRITE_BYTE)) {
        fprintf(stderr, "Error: Adapter for i2c bus %d does not have byte write capability\n", bus_number);
        exit(1);
    }
    if (ioctl(fd, I2C_SLAVE, device_addr) < 0) {
        fprintf(stderr, "Error: Could not set address to 0x%02x: %s\n", device_addr, strerror(errno));
        close(fd);
        exit(1);
    }
    return fd;
}

static uint8_t current_value;

int32_t i2c_write_byte(uint8_t value)
{
    int32_t res = i2c_smbus_write_byte(fd, value);
    if (res<0) {
        fprintf(stderr, "Error: %s\n", strerror(errno));
        exit(1);
    }
    current_value=value;
}


void write_pulse(uint8_t value) {
    i2c_write_byte(value | lcd_backlight);
    i2c_write_byte(value | LCD_EN | lcd_backlight);
    usleep(1);
    i2c_write_byte(value | lcd_backlight);
}

void write_cmd(uint8_t value) {
    write_pulse((value >> 4) << 4);
    usleep(5000);
    write_pulse((value & 0xF) << 4);
    usleep(1);
//    i2c_write_byte(lcd_backlight);
}

void write_char(uint8_t value) {
    write_pulse(LCD_RS | (value >> 4)<<4);
    usleep(200);
    write_pulse(LCD_RS | (value & 0xF)<<4);
    usleep(1);
//    i2c_write_byte(lcd_backlight);
}

void write_line(int line, const char* s) {
    switch(line) {
        case 0: 
            write_cmd(0x80);
            break;
        case 1: 
            write_cmd(0xC0);
            break;
        case 2: 
            write_cmd(0x94);
            break;
        case 3: 
            write_cmd(0xD4);
            break;
    }
    int len=strlen(s);
    if (len>20) {
        len=20;
    }
    int i;
    for (i=0;i<len;i++) {
        write_char(*((uint8_t*)(s+i)));
    }
    for (i=len;i<20;i++) {
        write_char(0x20);
    }
}

void lcd_clear() {
    write_cmd(0x1);
    write_cmd(0x2);
}

void pulse() {
    i2c_write_byte(current_value | LCD_EN);
    usleep(1);
    i2c_write_byte(current_value & ~LCD_EN);
}

int main(int argc, char* argv[])
{
    int fd=init_i2c_bus(1, 0x27);
    if (argc==1) {
        write_pulse(LCD_D5 | LCD_D4);
        usleep(5000);
        pulse();
        usleep(200);
        pulse();
        usleep(200);
        write_pulse(LCD_D5);
        usleep(5000);
        write_cmd(0x28);
        write_cmd(0x06);
        write_cmd(0x0C);
    } else {
        if (argc>=2) {
           write_line(0,argv[1]);
        }
        if (argc>=3) {
           write_line(1,argv[2]);
        }
        if (argc>=4) {
           write_line(2,argv[3]);
        }
        if (argc>=5) {
           write_line(3,argv[4]);
        }
    }
    close(fd);
}
