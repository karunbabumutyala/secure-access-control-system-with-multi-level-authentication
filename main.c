/*---------------------------------------------------------
 * File Name    : main.c
 * Description  : Main function for Secure Access
 *                Control System
 *---------------------------------------------------------*/

#include "lcd.h"                     // LCD driver header
#include "kpm.h"                     // Keypad driver header
#include "i2c.h"                     // I2C driver header
#include "i2c_eeprom.h"              // EEPROM driver header
#include "uart.h"                    // UART driver header
#include "r305.h"                    // Fingerprint sensor header
#include "delay.h"                   // Delay functions
#include "exint_menu.h"              // External interrupt header
#include "i2c_eeprom.h"              // EEPROM header
#include "types.h"                   // User defined data types
#include "l293d.h"                   // Motor driver header

extern u8 menu_flag;                 // External menu flag variable


/*---------------------------------------------------------
 * Function Name : main
 * Description   : Main program execution function
 *                 -> Initializes all peripherals
 *                 -> Displays project title
 *                 -> Handles menu and login operations
 * Return Type   : int
 *---------------------------------------------------------*/

main()
{
        // u8 data;                  // Variable for EEPROM testing

        // char buf[16];             // Buffer for EEPROM testing

        init_lcd();                  // Initialize LCD

        init_kpm();                  // Initialize keypad

        init_uart0();                // Initialize UART0

        init_i2c();                  // Initialize I2C

        bulid_symbols();             // Create custom LCD symbols

        init_eint2();                // Initialize external interrupt

        init_ids();                  // Initialize stored IDs

        init_l293d();                // Initialize motor driver

        // init_i2c();               // Optional I2C initialization


/*---------------------------------------------------------
 * LCD Symbol Testing Code
 *---------------------------------------------------------*/

/*
        str_lcd("abcdef ");          // Display sample string

        char_lcd(0);                 // Display custom symbol 0

        char_lcd(' ');               // Display space

        char_lcd(1);                 // Display custom symbol 1

        char_lcd(' ');               // Display space

        char_lcd(2);                 // Display custom symbol 2

        delay_ms(1000);              // Delay
*/


        title1();                    // Display first project title

        delay_ms(1000);              // Delay

        title2();                    // Display second project title

        delay_ms(1000);              // Delay


/*---------------------------------------------------------
 * EEPROM Testing Code
 *---------------------------------------------------------*/

/*
i2c_eeprom_write_byte(0x50,0x0000,'A'); // Write byte to EEPROM

data = i2c_eeprom_read_byte(0x50,0x00); // Read byte from EEPROM

cmd_lcd(0x01);                           // Clear LCD

char_lcd(data);                          // Display EEPROM data

i2c_eeprom_write_page(0x50,0x0001," karthik",9); // Write page data

i2c_eeprom_seq_read(0x50,0x0001,buf,9); // Read page data

str_lcd((u8 *)buf);                      // Display EEPROM string
*/


        // menu();                   // Optional menu call

        // delay_ms(1000);          // Delay


/*
cmd_lcd(0x01);                         // Clear LCD

char_lcd(data);                        // Display data

i2c_eeprom_write_page(0x50,0x0001," karthik",9); // Write page data

i2c_eeprom_seq_read(0x50,0x0002,buf,5); // Read page data

str_lcd((u8 *)buf);                    // Display string

cmd_lcd(0x01);                         // Clear LCD

str_lcd(" run");                       // Display message

door_open();                           // Open door
*/


/*---------------------------------------------------------
 * Main Program Loop
 *---------------------------------------------------------*/

        while(1)
        {
                if(menu_flag==1)      // Check menu interrupt flag
                {
                        menu();       // Open menu
                }

                login();              // Perform login operation
        }
}
