/*---------------------------------------------------------
 * File Name    : keypad.c
 * Description  : 4x4 Matrix Keypad Driver Functions
 *                 -> Keypad initialization
 *                 -> Key scanning
 *                 -> Password input
 *                 -> ID input
 *                 -> Numeric input handling
 *---------------------------------------------------------*/

#include "kpm.h"
#include "types.h"
#include "macros.h"
#include "delay.h"
#include "lcd.h"
#include <lpc21xx.h>

// Menu flag variable
u8 menu_flag = 0;

// Enable keypad mode
#define KEYPAD


/*---------------------------------------------------------
 * Keypad Lookup Table
 *---------------------------------------------------------*/

#ifdef KEYPAD

s32 kpmlut[4][4] =
{
        {'1','2','3','A'},
        {'4','5','6','B'},
        {'7','8','9','-'},
        {'*','0','=','B'}
};

#else

s32 kpmlut[4][4] =
{
        {'1','2','3','4'},
        {'5','6','7','8'},
        {'9','0','+','-'},
        {'*','/','=','B'}
};

#endif


/*---------------------------------------------------------
 * Function Name : init_kpm
 * Description   : Initializes keypad interface
 *                 -> Configures row pins as output
 * Return Type   : void
 *---------------------------------------------------------*/

void init_kpm(void)
{
        // Configure keypad row pins as output
        WNIBBLE(IODIR1,row0,15);
}


/*---------------------------------------------------------
 * Function Name : colscan
 * Description   : Scans keypad columns
 *                 -> Detects key press
 * Return Type   : u32
 *---------------------------------------------------------*/

u32 colscan(void)
{
        // Return 0 if key pressed
        // Return 1 if no key pressed
        return (RNIBBLE(IOPIN1,col0)<15)?0:1;
}


/*---------------------------------------------------------
 * Function Name : rowcheck
 * Description   : Detects keypad row
 *                 -> Activates rows one by one
 * Return Type   : u32
 *---------------------------------------------------------*/

u32 rowcheck(void)
{
        // Variable for row number
        u32 rno;

        // Scan all rows
        for(rno=0;rno<4;rno++)
        {
                // Activate one row
                WNIBBLE(IOPIN1,row0,~(1<<rno));

                // Check key press
                if(colscan()==0)
                {
                        break;
                }
        }

        // Clear all rows
        WNIBBLE(IOPIN1,row0,0X0);

        // Return row number
        return rno;
}


/*---------------------------------------------------------
 * Function Name : colcheck
 * Description   : Detects keypad column
 *                 -> Scans column lines
 * Return Type   : u32
 *---------------------------------------------------------*/

u32 colcheck(void)
{
        // Variable for column number
        u32 cno;

        // Scan all columns
        for(cno=0;cno<4;cno++)
        {
                // Check active column
                if(RBIT(IOPIN1,(col0+cno))==0)
                {
                        break;
                }
        }

        // Return column number
        return cno;
}


/*---------------------------------------------------------
 * Function Name : keyscan
 * Description   : Scans keypad and returns pressed key
 *                 -> Waits for key press
 *                 -> Detects row and column
 *                 -> Reads key from lookup table
 *                 -> Waits for key release
 * Return Type   : s32
 *---------------------------------------------------------*/

s32 keyscan(void)
{
        // Variables for key value, row and column
        s32 keyv,rno,cno;

        // Wait until key is pressed
        while(colscan() && menu_flag==0);

        // Exit condition
        if(menu_flag==1)
        {
                return -1;
        }

        // Detect row
        rno = rowcheck();

        // Detect column
        cno = colcheck();

        // Read key from lookup table
        keyv = kpmlut[rno][cno];

        // Wait until key release
        while(!colscan());

        // Debounce delay
        delay_ms(100);

        // Return key value
        return keyv;
}


/*---------------------------------------------------------
 * Function Name : readnum
 * Description   : Reads numeric input from keypad
 *                 -> Accepts digits only
 *                 -> Supports backspace
 *                 -> Returns entered number
 * Return Type   : s32
 *---------------------------------------------------------*/

s32 readnum(void)
{
        // Variable for keypad input
        s32 key;

        // Digit counter
        s32 cnt = 0;

        // Variable for final number
        int num = 0;

        while(1)
        {
                        // Read keypad input
                        key = keyscan();

                        // Exit condition
                        if(key == -1)
                        {
                                cnt = 0;

                                return -1;
                        }

                        // Accept numeric digits
                        if((key>='0') && (key<='9') && (cnt<4))
                        {
                                 // Increment count
                                 cnt++;

                                 // Display digit
                                 char_lcd(key);

                                 // Convert ASCII to integer
                                 num = num*10 + (key-48);
                        }

                        // Backspace operation
                        else if((key=='B') && (cnt!=0))
                        {
                                // Decrement count
                                cnt--;

                                // Move cursor left
                                cmd_lcd(0x10);

                                // Clear character
                                char_lcd(' ');

                                // Move cursor back
                                cmd_lcd(0x10);

                                // Remove last digit
                                num /= 10;
                        }

                        // Enter key pressed
                        else if((key == '=') && (cnt!=0))
                        {
                                // Reset counter
                                cnt = 0;

                                // Return entered number
                                return num;
                        }
        }
}


/*---------------------------------------------------------
 * Old Functions / Backup Code
 *---------------------------------------------------------*/

/*
void password_by_kpm(char *p)
{
        while(1)
        {
                *p = keyscan();

                char_lcd(*p);

                delay_us(100);

                cmd_lcd(0x10);

                char_lcd('*');
        }
}
*/


/*
u32 readop(void)
{
        u32 key;

label:

        cmd_lcd(0x80);

        str_lcd("enter the operator:");

        key = keyscan();

        cmd_lcd(0xc0);

        char_lcd(key);

        if((key == '+')||(key == '-')||(key =='/')||(key == '*'))
        {
                return key;
        }

        cmd_lcd(0x01);

        cmd_lcd(0x80);

        str_lcd("entered wrong operator...");

        cmd_lcd(0xc0);

        str_lcd("AGAIN enter operator");

        delay_ms(1000);

        goto label;
}
*/


/*
void id_by_kpm(u8 *ptr)
{
        char i;

        for(i=0;i<4;i++)
        {
                ptr[i] = keyscan();

                char_lcd(ptr[i]);
        }
}
*/


/*---------------------------------------------------------
 * Function Name : password_kpm
 * Description   : Reads password from keypad
 *                 -> Accepts 4 digit password
 *                 -> Displays '*' for security
 *                 -> Supports backspace
 * Return Type   : void
 *---------------------------------------------------------*/

void password_kpm(char *ptr)
{
        // Variable for keypad input
        u8 a;

        // Digit counter
        u8 cnt = 0;

        while(1)
        {
                // Read keypad input
                a = keyscan();

                // Accept numeric digits
                if((a>='0') && (a<='9') && (cnt!=4))
                {
                        // Increment counter
                        cnt++;

                        // Display digit
                        char_lcd(a);

                        // Delay for visibility
                        delay_ms(300);

                        // Move cursor left
                        cmd_lcd(0x10);

                        // Display '*'
                        char_lcd('*');

                        // Store password digit
                        *ptr = a;

                        // Move pointer
                        ptr++;
                }

                // Backspace operation
                else if((a=='B') && (cnt!=0))
                {
                        // Move cursor left
                        cmd_lcd(0x10);

                        // Clear character
                        char_lcd(' ');

                        // Move cursor back
                        cmd_lcd(0x10);

                        // Decrement counter
                        cnt--;

                        // Move pointer backward
                        ptr--;
                }

                // Enter key pressed
                else if((a=='=') && (cnt==4))
                {
                        // Reset counter
                        cnt = 0;

                        // Null terminate string
                        *ptr = '\0';

                        // Exit function
                        return;
                }
        }
}


/*---------------------------------------------------------
 * Function Name : id_kpm
 * Description   : Reads user ID from keypad
 *                 -> Accepts 4 digit ID
 *                 -> Supports backspace
 *                 -> Stores ID as string
 * Return Type   : void
 *---------------------------------------------------------*/

void id_kpm(s8 *ptr)
{
        // Variable for keypad input
        s8 a;

        // Digit counter
        s8 cnt = 0;

        while(1)
        {
                // Read keypad input
                a = keyscan();

                // Accept numeric digits
                if((a>='0') && (a<='9') && (cnt!=4))
                {
                        // Display digit
                        char_lcd(a);

                        // Store digit
                        ptr[cnt] = a;

                        // Increment counter
                        cnt++;
                }

                // Backspace operation
                else if((a=='B') && (cnt!=0))
                {
                        // Decrement counter
                        cnt--;

                        // Move cursor left
                        cmd_lcd(0x10);

                        // Clear character
                        char_lcd(' ');

                        // Move cursor back
                        cmd_lcd(0x10);
                }

                // Enter key pressed
                else if((a == '=') && (cnt==4))
                {
                        // Null terminate string
                        ptr[cnt] = '\0';

                        // Exit function
                        return;
                }
        }
}
