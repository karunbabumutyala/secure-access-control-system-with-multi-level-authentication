/*---------------------------------------------------------
 * File Name    : lcd.c
 * Description  : LCD Driver Functions
 *                 -> LCD initialization
 *                 -> Command transmission
 *                 -> Data transmission
 *                 -> String display
 *                 -> Number display
 *---------------------------------------------------------*/

#include <lpc21XX.h>                 // LPC214x header file
#include "macros.h"                  // Macro definitions
#include "types.h"                   // User defined data types
#include "delay.h"                   // Delay functions
#include "lcd.h"                     // LCD function declarations


/*---------------------------------------------------------
 * Function Name : write_lcd
 * Description   : Writes data/command to LCD
 * Return Type   : void
 *---------------------------------------------------------*/

void write_lcd(u8 data)
{
         G_SETBIT(IOCLR0,RW);        // Select write operation

         WBYTE(IOPIN0,DATA,data);    // Send data to LCD pins

         G_SETBIT(IOSET0,EN);        // Enable LCD

         delay_us(1);                // Small delay

         G_SETBIT(IOCLR0,EN);        // Disable enable pin

         delay_ms(2);                // LCD processing delay
}


/*---------------------------------------------------------
 * Function Name : cmd_lcd
 * Description   : Sends command to LCD
 * Return Type   : void
 *---------------------------------------------------------*/

void cmd_lcd(u8 cmd)
{
        G_SETBIT(IOCLR0,RS);         // Select command register

        write_lcd(cmd);              // Send command to LCD
}


/*---------------------------------------------------------
 * Function Name : init_lcd
 * Description   : Initializes LCD module
 * Return Type   : void
 *---------------------------------------------------------*/

void init_lcd()
{
        WBYTE(IODIR0,DATA,0XFF);     // Configure data pins as output

        SETBIT(IODIR0,RS);           // Configure RS pin as output

        SETBIT(IODIR0,RW);           // Configure RW pin as output

        SETBIT(IODIR0,EN);           // Configure EN pin as output

        delay_ms(15);                // LCD power ON delay

        cmd_lcd(MODE_8BIT_LINE1);    // Initialize LCD in 8-bit mode

        delay_ms(5);                 // Delay after command

        cmd_lcd(0x30);               // Function set command

        delay_us(100);               // Small delay

        cmd_lcd(MODE_8BIT_LINE1);    // Reinitialize LCD

        cmd_lcd(MODE_8BIT_LINE2);    // Configure LCD in 2-line mode

        cmd_lcd(DISP_ON);            // Turn ON display

        cmd_lcd(LCD_CLR);            // Clear LCD screen

        cmd_lcd(DISP_SHIFT_OFF_INC); // Cursor increment mode
}


/*---------------------------------------------------------
 * Function Name : char_lcd
 * Description   : Displays single character on LCD
 * Return Type   : void
 *---------------------------------------------------------*/

void char_lcd(u8 character)
{
        SETBIT(IOSET0,RS);           // Select data register

        write_lcd(character);        // Send character to LCD
}


/*---------------------------------------------------------
 * Function Name : str_lcd
 * Description   : Displays string on LCD
 * Return Type   : void
 *---------------------------------------------------------*/

void str_lcd(u8 *p)
{
        while(*p)                    // Repeat until null character
                char_lcd(*p++);      // Display character
}


/*---------------------------------------------------------
 * Function Name : u32_lcd
 * Description   : Displays unsigned integer on LCD
 * Return Type   : void
 *---------------------------------------------------------*/

void u32_lcd(u32 n)
{
        u8 a[10];                    // Array to store digits

        s32 i=0;                     // Loop variable

        if(n==0)                     // Check for zero
        {
                char_lcd('0');       // Display zero
        }

        else
        {
                while(n)             // Extract digits
                {
                        a[i]=(n%10)+48; // Convert digit to ASCII

                        i++;         // Increment index

                        n/=10;       // Remove last digit
                }

                for(--i;i>=0;i--)    // Display digits in correct order
                {
                        char_lcd(a[i]); // Display digit
                }
        }
}
/*---------------------------------------------------------
 * Function Name : hex_lcd
 * Description   : Displays hexadecimal value on LCD
 * Return Type   : void
 *---------------------------------------------------------*/

void hex_lcd(u32 n)
{
   u8 c[10],rem;                     // Array for hex digits and remainder

   s32 i=0;                          // Loop variable

   if(n==0)                          // Check for zero
   {
        char_lcd('0');               // Display zero
   }

   else
   {
                while(n)             // Extract hexadecimal digits
                {
                        rem = n%16;  // Get remainder

                        c[i] = (rem<10)?rem+48:rem+55; // Convert to ASCII

                        i++;         // Increment index

                        n/=16;       // Remove last digit
                }

                for(--i;i>=0;i--)    // Display digits in correct order
                {
                        char_lcd(c[i]); // Display hexadecimal digit
                }
   }
}


/*---------------------------------------------------------
 * Function Name : bin_lcd
 * Description   : Displays binary value on LCD
 * Return Type   : void
 *---------------------------------------------------------*/

void bin_lcd(u32 n,u8 bit)
{
        s32 i=0;                     // Loop variable

        for(i=bit-1;i>=0;i--)        // Scan each bit
        {
                char_lcd(((n>>i)&1)+48); // Display binary bit
        }
}


/*---------------------------------------------------------
 * Function Name : oct_lcd
 * Description   : Displays octal value on LCD
 * Return Type   : void
 *---------------------------------------------------------*/

void oct_lcd(u32 n)
{
        u8 c[15]={0};                // Array for octal digits

        s32 i=0;                     // Loop variable

        if(n==0)                     // Check for zero
        {
                char_lcd('0');       // Display zero
        }

        else
        {
                while(n)             // Extract octal digits
                {
                        c[i] = n%8;  // Store remainder

                        i++;         // Increment index

                        n/=8;        // Remove last digit
                }

                for(--i;i>=0;i--)    // Display digits in correct order
                {
                        char_lcd(c[i]); // Display octal digit
                }
        }
}


/*---------------------------------------------------------
 * Function Name : bulid_CGRAM
 * Description   : Stores custom characters in CGRAM
 * Return Type   : void
 *---------------------------------------------------------*/

void bulid_CGRAM(u8 *p,u8 nb)
{
        s32 i;                       // Loop variable

        cmd_lcd(GOTO_CGRAM);         // Select CGRAM address

        for(i=0;i<=nb;i++)           // Write custom pattern data
        {
                char_lcd(p[i]);      // Store pattern byte
        }

        cmd_lcd(GOTO_LINE1_POS_0);   // Return cursor to line 1
}
/*---------------------------------------------------------
 * Function Name : float_lcd
 * Description   : Displays floating point value on LCD
 * Return Type   : void
 *---------------------------------------------------------*/

void float_lcd(f32 f,s32 np)
{
        s32 i;                       // Variable for integer part

        if(f<0)                      // Check for negative number
        {
                char_lcd('-');       // Display negative sign

                f*=-1;               // Convert into positive value
        }

        i=f;                         // Extract integer part

        u32_lcd(i);                  // Display integer part

        char_lcd('.');               // Display decimal point

        f = f-i;                     // Extract fractional part

        while(np)                    // Shift decimal digits
        {
                f*=10;               // Multiply by 10

                np--;                // Decrement precision count
        }

        i = f;                       // Convert fractional part to integer

        u32_lcd(i);                  // Display fractional part
}


/*---------------------------------------------------------
 * Function Name : s32_lcd
 * Description   : Displays signed integer on LCD
 * Return Type   : void
 *---------------------------------------------------------*/

void s32_lcd(s32 n)
{
        if(n<0)                      // Check for negative number
        {
                char_lcd('-');       // Display negative sign

                n*=-1;               // Convert into positive value
        }

        u32_lcd(n);                  // Display integer value
}


/*---------------------------------------------------------
 * Function Name : bulid_symbols
 * Description   : Stores custom symbols into LCD CGRAM
 * Return Type   : void
 *---------------------------------------------------------*/

void bulid_symbols(void)
{
        u8 i,sy[60]=                 // Array for custom symbols
        {
                                 0x00,0x11,0x0A,0x04,0x0A,0x11,0x00,0x00,  // X symbol

                                 0x00,0x01,0x02,0x14,0x08,0x00,0x00,0x00,  // Correct symbol

                                 0x02,0x02,0x0e,0x1f,0x1f,0x1f,0x0e,0x00,  // Finger symbol

                                 0x00,0x1F,0X11,0X11,0X1F,0X1B,0X1F,0X00,  // Locked symbol

                                 0X1F,0X11,0X01,0X01,0X1F,0X1B,0X1F,0X00,  // Unlocked symbol

                                 0x00,0x04,0x06,0x1F,0X1F,0X06,0X04,0X00,  // Arrow symbol

                                 0X00,0X04,0X0C,0X1F,0X1F,0X0C,0X04,0X00   // Reverse arrow symbol
                                 };

        cmd_lcd(GOTO_CGRAM);         // Select CGRAM address

        for(i=0;i<=55;i++)           // Store custom symbols
        {
                char_lcd(sy[i]);     // Write symbol data
        }

        cmd_lcd(0x80);               // Return cursor to first line
}


/*---------------------------------------------------------
 * Function Name : title1
 * Description   : Displays animated project title
 * Return Type   : void
 *---------------------------------------------------------*/

void title1(void)
{
        char i;                      // Loop variable

        char line1[] = " SECURE ACCESS "; // First line message

        char line2[] = " CONTROL SYSTEM "; // Second line message

        cmd_lcd(0x01);               // Clear LCD

        for(i=0;line1[i]!=0;i++)     // Display first line animation
        {
                cmd_lcd(0x80+i);     // Move cursor position

                char_lcd(5);         // Display animation symbol

                delay_ms(150);       // Delay for animation

                cmd_lcd(0x80+i);     // Return cursor position

                char_lcd(line1[i]);  // Display actual character

                delay_ms(120);       // Character delay
        }

        for(i=0;line2[i]!=0;i++)     // Display second line animation
        {
                cmd_lcd(0xCF-i);     // Move cursor from reverse direction

                char_lcd(6);         // Display animation symbol

                delay_ms(150);       // Delay for animation

                cmd_lcd(0xCF-i);     // Return cursor position

                char_lcd(line2[strlen(line2)-i-1]); // Display character

                delay_ms(120);       // Character delay
        }
}
/*---------------------------------------------------------
 * Function Name : title2
 * Description   : Displays animated authentication title
 * Return Type   : void
 *---------------------------------------------------------*/

void title2(void)
{
        char i;                                      // Loop variable

        char line1[] = "WITH MULTI-LEVEL";           // First line text

        char line2[] = " AUTHENTICATION ";           // Second line text

        cmd_lcd(0x01);                               // Clear LCD screen

        for(i=0;line1[i]!=0;i++)                     // Display first line animation
        {
                cmd_lcd(0x80+i);                     // Move cursor position

                char_lcd(5);                         // Display animation symbol

                delay_ms(150);                       // Animation delay

                cmd_lcd(0x80+i);                     // Return cursor position

                char_lcd(line1[i]);                  // Display actual character

                delay_ms(120);                       // Character display delay
        }

        for(i=0;line2[i]!=0;i++)                     // Display second line animation
        {
                cmd_lcd(0xCF-i);                     // Move cursor in reverse direction

                char_lcd(6);                         // Display animation symbol

                delay_ms(150);                       // Animation delay

                cmd_lcd(0xCF-i);                     // Return cursor position

                char_lcd(line2[strlen(line2)-i-1]); // Display actual character

                delay_ms(120);                       // Character display delay
        }
}
