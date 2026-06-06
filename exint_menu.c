#include <lpc21xx.h>
#include "lcd.h"
#include "types.h"
#include "exint_menu.h"
#include "kpm.h"
#include "lcd.h"
#include "string.h"
#include "delay.h"
#include "i2c_eeprom.h"
#include "r305.h"
#include "l293d.h"

//#define NEW_ROM      // Enable for fresh EEPROM initialization
#define OLD_ROM        // Enable for reading existing EEPROM data

/*---------------------------------------------------------
 * Global Variables
 *---------------------------------------------------------*/
extern u8 menu_flag;    // Flag used for menu operations

static u8 flag = 0;    // Local status flag

u8 admin_id, ids = 0;  // Admin ID and total stored IDs

s8 pswd1[5], pswd2[5] = "2026"; // Password buffers

u32 id;                // Fingerprint/User ID variable


/*---------------------------------------------------------
 * Function Name : init_eint2
 * Description   : Configures External Interrupt 2 (EINT2)
 *                 and enables interrupt in VIC.
 *---------------------------------------------------------*/
void init_eint2(void)
{
        /* Configure P0.7 pin as EINT2 pin */

        // Clear bit pair 14 & 15 without affecting other bits
        PINSEL0 &= ~((u32)3 << 14);

        // Select EINT2 function for P0.7
        PINSEL0 |= EINT2_INPUT_PIN;

        /*---------------- VIC Configuration ----------------*/

        // Enable EINT2 interrupt channel in VIC
        VICIntEnable |= (1 << EINT2_VIC_CHNO);

        // Configure VIC slot1 for EINT2
        VICVectCntl1 = (1 << 5) | EINT2_VIC_CHNO;

        // Load ISR address into VIC
        VICVectAddr1 = (u32)eint2_isr;

        /*------------- External Interrupt Config ------------*/

        // Configure EINT2 as edge-triggered interrupt
        EXTMODE = (1 << 2);

        // Falling edge trigger (default EXPOLAR = 0)
}


/*---------------------------------------------------------
 * Function Name : init_ids
 * Description   : Initializes ID count from EEPROM.
 *                 NEW_ROM  -> Clears EEPROM and starts from 0
 *                 OLD_ROM  -> Reads stored ID count
 *---------------------------------------------------------*/
void init_ids(void)
{
#ifdef NEW_ROM

        // Fresh EEPROM initialization
        ids = 0;

        // Store initial ID count into EEPROM address 0x0000
        i2c_eeprom_write_byte(0x50, 0x0000, ids);

#endif


#ifdef OLD_ROM

        // Read previously stored ID count from EEPROM
        ids = i2c_eeprom_read_byte(0x50, 0x0000);

#endif
}
/*---------------------------------------------------------
 * Function Name : eint2_isr
 * Description   : Interrupt Service Routine for EINT2.
 *                 Sets menu flag when external interrupt
 *                 occurs and clears interrupt status.
 * Return Type   : void
 *---------------------------------------------------------*/
void eint2_isr(void) __irq
{
        // Set menu flag to indicate interrupt event
        menu_flag = 1;

        /* Clear EINT2 interrupt status in EXTINT register */
        EXTINT = (1 << 2);

        /* Clear interrupt in VIC */
        VICVectAddr = 0;
}


/*---------------------------------------------------------
 * Admin Details Storage using EEPROM
 *---------------------------------------------------------*/

/*
#define NEW_ROM

#ifdef NEW_ROM

-----------------------------------------------------------
 * Function Name : set_admin_detail
 * Description   : Stores admin ID and password into EEPROM.
 *                 Password must be entered twice for
 *                 confirmation.
 *-----------------------------------------------------------
*/
/*
void set_admin_detail(void)
{
        u8 id;
        u8 pswd1[5], pswd2[5];

        while(1)
        {
                // Clear LCD and ask for Admin ID 
                cmd_lcd(LCD_CLR);

                cmd_lcd(GOTO_LINE1_POS_0);
                str_lcd("enter admin ID:");

                cmd_lcd(GOTO_LINE2_POS_0);

                // Read numeric ID from keypad
                id = read_num();

                // Ask for password 
                cmd_lcd(LCD_CLR);
                str_lcd("enter password:");

                // Read password from keypad
                password_kpm(pswd1);

                // Confirm password 
                cmd_lcd(LCD_CLR);
                str_lcd("enter again:");

                password_kpm(pswd2);

                // Compare both passwords 
                if(strcmp(pswd1, pswd2) != 0)
                {
                        // Password mismatch
                        cmd_lcd(LCD_CLR);

                        str_lcd("wrong entry");

                        cmd_lcd(0xC0);
                        str_lcd("enter again");

                        delay_ms(500);
                }
                else
                {
                        // Store Admin ID into EEPROM 
                        i2c_eeprom_write_byte(0x50, 0x0001, id);

                        // Store Password into EEPROM 
                        i2c_eeprom_write_page(0x50,
                                              0x0002,
                                              pswd1,
                                              5);

                        // Display success message 
                        cmd_lcd(LCD_CLR);

                        str_lcd("..admin detail..");

                        cmd_lcd(0xC0);
                        str_lcd("....saved.....");

                        delay_ms(500);

                        // Update global admin ID
                        admin_id = id;

                        return;
                }
        }
}

#endif
*/


/*---------------------------------------------------------
 * EEPROM Function References
 *---------------------------------------------------------

// Write multiple bytes into EEPROM
void i2c_eeprom_write_page(u8 SlaveAddr,
                           u16 BuffAddr,
                           s8 *P,
                           u8 nbytes);

// Read multiple bytes sequentially from EEPROM
void i2c_eeprom_seq_read(u8 SlaveAddr,
                         u16 BuffAddr,
                         s8 *p,
                         u8 nbytes);

---------------------------------------------------------*/
/*---------------------------------------------------------
 * Function Name : is_id_in_db
 * Description   : Searches the entered ID in EEPROM database
 * Argument      : id -> ID to search
 * Return Value  :
 *                  EEPROM address if ID exists
 *                  0 if ID not found
 *---------------------------------------------------------*/
u16 is_id_in_db(char id)
{
        // Loop counter variable
        char i = 0;

        // Variable to store ID read from EEPROM
        char id2;

        // Check all stored IDs one by one
        for(i = 0; i < ids; i++)
        {
                // Read ID from EEPROM
                // Address calculation:
                // 0x0001 -> Starting address of first ID
                // Each record occupies 6 bytes
                id2 = i2c_eeprom_read_byte(
                                                0x50,
                                                (0x0001 + (i * 6))
                                                  );

                // Compare stored ID with entered ID
                if(id2 == id)
                {
                        // Return EEPROM address if ID found
                        return (0x0001 + (i * 6));
                }
        }

        // Return 0 if ID not found
        return 0;
}


/*---------------------------------------------------------
 * Function Name : check_admin
 * Description   : Verifies admin ID and password
 *                 Allows maximum 3 wrong attempts
 * Return Value  :
 *                  0 -> Access Granted
 *                  1 -> Access Denied
 *---------------------------------------------------------*/
s8 check_admin(void)
{
        // Variable to count wrong attempts
        u8 wrong = 0;


        /*================ ADMIN ID CHECK =================*/

ID:

        // Clear LCD display
        cmd_lcd(LCD_CLR);

        // Move cursor to first line
        cmd_lcd(GOTO_LINE1_POS_0);

        // Display message
        str_lcd("enter admin ID");

        // Move cursor to second line
        cmd_lcd(GOTO_LINE2_POS_0);

        // Read admin ID from keypad
        id = readnum();

        // Check entered ID with valid admin ID
        if(id != 11)
        {
                // Check whether maximum attempts reached
                if(wrong == 3)
                {
                        // Clear LCD
                        cmd_lcd(LCD_CLR);

                        // Display access denied message
                        str_lcd("ACCESS DENIED");

                        // Delay for visibility
                        delay_ms(1000);

                        // Return failure
                        return 1;
                }

                // Clear LCD
                cmd_lcd(LCD_CLR);

                // Move cursor to line 1
                cmd_lcd(GOTO_LINE1_POS_0);

                // Display wrong ID message
                str_lcd("wrong ID..!!");

                // Move cursor to line 2
                cmd_lcd(GOTO_LINE2_POS_0);

                // Display remaining chances
                char_lcd(2 - wrong + 48);

                // Display message
                str_lcd(" more chance");

                // Small delay
                delay_ms(500);

                // Retry again if chances available
                if(wrong != 3)
                {
                        // Increment wrong attempt count
                        wrong++;

                        // Jump back to ID checking
                        goto ID;
                }
        }

        // Reset wrong counter before password checking
        wrong = 0;


        /*================ PASSWORD CHECK =================*/

PSWD:

        // Clear LCD
        cmd_lcd(LCD_CLR);

        // Move cursor to line 1
        cmd_lcd(GOTO_LINE1_POS_0);

        // Display password prompt
        str_lcd("enter PASSWORD");

        // Move cursor to line 2
        cmd_lcd(GOTO_LINE2_POS_0);

        // Read password from keypad
        password_kpm(pswd1);

        // Compare entered password with original password
        if(strcmp((const char*)pswd1, "2026") != 0)
        {
                // Clear LCD
                cmd_lcd(LCD_CLR);

                // Move cursor to line 1
                cmd_lcd(GOTO_LINE1_POS_0);

                // Display wrong password message
                str_lcd("wrong password..!!");

                // Move cursor to line 2
                cmd_lcd(GOTO_LINE2_POS_0);

                // Display remaining chances
                char_lcd(2 - wrong + 48);

                // Display message
                str_lcd(" more chance");

                // Delay for visibility
                delay_ms(500);

                // Retry again if chances available
                if(wrong != 3)
                {
                        // Increment wrong attempt count
                        wrong++;

                        // Jump back to password checking
                        goto PSWD;
                }

                // Return failure
                return 1;
        }


        /*================ ACCESS SUCCESS =================*/

        // Clear LCD
        cmd_lcd(LCD_CLR);

        // Move cursor to line 1
        cmd_lcd(GOTO_LINE1_POS_0);

        // Display success message
        str_lcd("ACCESS GRANTED");

        // Delay for visibility
        delay_ms(500);

        // Return success
        return 0;
}
/*---------------------------------------------------------
 * Function Name : enroll_id
 * Description   : Enrolls new user details
 *                 -> Reads ID
 *                 -> Verifies duplicate ID
 *                 -> Stores password
 *                 -> Enrolls fingerprint
 *                 -> Saves data into EEPROM
 * Return Type   : void
 *---------------------------------------------------------*/
void enroll_id(void)
{

/*===================== ID ENTRY =====================*/

ID:

        // Clear LCD display
        cmd_lcd(0x01);

        // Display ID entry message
        str_lcd("ENTER ID(1-10):");

        // Move cursor to second line
        cmd_lcd(0xC0);

        // Read ID from keypad
        id = readnum();

        // Check whether ID is within valid range
        if(!((id >= 1) && (id <= 10)))
        {
                // Clear LCD
                cmd_lcd(0x01);

                // Display symbol
                char_lcd(0);

                // Display invalid message
                str_lcd("INVALID ENTRY");

                // Display symbol
                char_lcd(0);

                // Move cursor to second line
                cmd_lcd(0xC0);

                // Display retry message
                str_lcd("try again...");

                // Small delay
                delay_ms(500);

                // Jump back for ID entry
                goto ID;
        }

        // Check whether ID already exists in EEPROM
        if(is_id_in_db(id))
        {
                // Clear LCD
                cmd_lcd(0x01);

                // Display duplicate ID message
                str_lcd("IT ALREADY EXIST");

                // Move cursor to second line
                cmd_lcd(0xC0);

                // Ask user to enter another ID
                str_lcd("TRY OTHER ID");

                // Delay for visibility
                delay_s(1);

                // Jump back for new ID entry
                goto ID;
        }


/*=================== PASSWORD ENTRY ===================*/

PSWD:

        // Clear LCD
        cmd_lcd(0x01);

        // Display password prompt
        str_lcd("ENTER PASSWORD:");

        // Move cursor to second line
        cmd_lcd(0xC0);

        // Read password from keypad
        password_kpm(pswd1);

        // Clear LCD
        cmd_lcd(0x01);

        // Ask user to enter password again
        str_lcd("ENTER AGAIN:");

        // Move cursor to second line
        cmd_lcd(0xC0);

        // Read confirmation password
        password_kpm(pswd2);

        // Compare both passwords
        if(strcmp((const char*)pswd1,
                  (const char*)pswd2) != 0)
        {
                // Clear LCD
                cmd_lcd(0x01);

                // Display mismatch message
                str_lcd("NOT MATCHED");

                // Move cursor to second line
                cmd_lcd(0xC0);

                // Ask user to retry
                str_lcd("try again");

                // Small delay
                delay_ms(500);

                // Jump back for password entry
                goto PSWD;
        }
        else
        {
                // Clear LCD
                cmd_lcd(0x01);

                // Display symbol
                char_lcd(2);

                // Ask user to place finger
                str_lcd(" go to finger");

                // Delay for visibility
                delay_ms(500);
        }


/*================= FINGERPRINT ENROLL =================*/

FP1:

        // Enroll fingerprint with entered ID
        if(enroll(id) != 0)
        {
                // Clear LCD
                cmd_lcd(0x01);

                // Display failure message
                str_lcd("FAIL FINGER");

                // Move cursor to second line
                cmd_lcd(0xC0);

                // Ask user to retry
                str_lcd("PLEASE TRY AGAIN");

                // Small delay
                delay_ms(500);

                // Retry fingerprint enrollment
                goto FP1;
        }


/*================= EEPROM STORAGE =================*/

        // Store ID into EEPROM
        i2c_eeprom_write_byte(
                                        0x50,
                                        (0x0001 + (ids * 6)),
                                        id
                                                     );

        // Store password into EEPROM
        i2c_eeprom_write_page(
                                        0x50,
                                        (0x0002 + (ids * 6)),
                                        (s8*)pswd1,
                                        5
                                                    );

        // Increment total ID count
        ids++;

        // Update total IDs count in EEPROM
        i2c_eeprom_write_byte(0x50,
                              0x0000,
                              ids);


/*================= SUCCESS MESSAGE =================*/

        // Clear LCD
        cmd_lcd(0x01);

        // Display success message
        str_lcd("ENROLL DONE ");

        // Display success symbol
        char_lcd(1);

        // Delay for visibility
        delay_s(1);
}
/*---------------------------------------------------------
 * Function Name : login
 * Description   : User login function
 *                 -> Verifies ID
 *                 -> Verifies password
 *                 -> Verifies fingerprint
 *                 -> Opens and closes door
 * Return Type   : void
 *---------------------------------------------------------*/
void login(void)
{
        // Variable for entered login ID
        int l_id;

        // Loop counter variables
        int k;

        // Buffers for stored and entered passwords
        s8 buf1[5], buf2[5];

        // Variable used in countdown loop
        s8 l;

        // Retry counter
        s8 retry = 0;

        // Stores EEPROM address if ID found
        u16 found = 0;


/*==================== CHECK DATABASE ====================*/

        // Check whether any users are enrolled
        if(ids == 0)
        {
                // Display message only once
                if(flag == 0)
                {
                        // Set flag
                        flag = 1;

                        // Clear LCD
                        cmd_lcd(0x01);

                        // Display no data message
                        str_lcd("NO DATA FOUND");

                        // Move cursor to second line
                        cmd_lcd(0xC0);

                        // Ask user to enroll
                        str_lcd("PRESS SW TO ENROLL");
                }

                // Exit function
                return;
        }

        // Reset flag
        flag = 0;


/*====================== ID CHECK ======================*/

        // Clear LCD
        cmd_lcd(0x01);

        // Move cursor to first line
        cmd_lcd(0x80);

        // Display ID prompt
        str_lcd("ENTER THE ID:");

        // Read ID from keypad
        l_id = readnum();

        // Check for invalid input
        if(l_id == -1)
        {
                return;
        }

        // Check whether ID exists in database
        if((found = is_id_in_db(l_id)) == 0)
        {
                // Clear LCD
                cmd_lcd(0x01);

                // Display error message
                str_lcd("ID IS NOT FOUND");

                // Delay for visibility
                delay_ms(1000);

                // Exit function
                return;
        }

        // Clear LCD
        cmd_lcd(0x01);

        // Display success message
        str_lcd("ID IS FOUND");

        // Delay for visibility
        delay_ms(600);


        /* Read stored password from EEPROM */
        i2c_eeprom_seq_read(
                                0x50,
                                found + 1,
                                (s8 *)buf1,
                                5
                                                   );


/*=================== PASSWORD CHECK ===================*/

PSWD:

        // Clear LCD
        cmd_lcd(0x01);

        // Display password prompt
        str_lcd("ENTER PASSWORD:");

        // Move cursor to second line
        cmd_lcd(0xC0);

        // Read password from keypad
        password_kpm((s8 *)buf2);

        // Compare stored password with entered password
        if(strcmp((const char*)buf1,
                  (const char *)buf2) != 0)
        {
                // Clear LCD
                cmd_lcd(0x01);

                // Display wrong password message
                str_lcd("WRONG PASSWORD ");

                // Display symbol
                char_lcd(0);

                // Increment retry counter
                retry++;

                // Check maximum attempts
                if(retry == 3)
                {
                        // Clear LCD
                        cmd_lcd(0x01);

                        // Display failure message
                        str_lcd("LOGIN FAILS");

                        // Delay for visibility
                        delay_ms(1000);

                        // Exit function
                        return;
                }

                // Move cursor to second line
                cmd_lcd(0xC0);

                // Display remaining chances
                str_lcd("you have ");

                char_lcd(3 - retry + 48);

                str_lcd(" chances");

                // Delay for visibility
                delay_ms(1000);

                // Retry password entry
                goto PSWD;
        }

        // Clear LCD
        cmd_lcd(0x01);

        // Display success message
        str_lcd("PASSWORD VERIFED");

        // Delay for visibility
        delay_ms(500);

        // Reset retry counter
        retry = 0;


/*================= FINGERPRINT VERIFY =================*/

FP:

        // Clear LCD
        cmd_lcd(0x01);

        // Verify fingerprint
        if(verify(l_id) != 0)
        {
                // Clear LCD
                cmd_lcd(0x01);

                // Display failure message
                str_lcd("FINGER NOT FOUND");

                // Move cursor to second line
                cmd_lcd(0xC0);

                // Ask user to retry
                str_lcd("try again");

                // Delay for visibility
                delay_ms(600);

                // Increment retry counter
                retry++;

                // First retry
                if(retry == 1)
                {
                        cmd_lcd(0x01);

                        str_lcd("two more chance");

                        delay_ms(800);

                        goto FP;
                }

                // Second retry
                else if(retry == 2)
                {
                        cmd_lcd(0x01);

                        str_lcd("last chance");

                        delay_ms(800);

                        goto FP;
                }

                // Access denied after 3 attempts
                else
                {
                        cmd_lcd(0x01);

                        char_lcd(0);

                        str_lcd("ACCESS DENIED");

                        char_lcd(0);

                        delay_ms(1000);

                        return;
                }
        }


/*==================== DOOR OPENING ====================*/

        // Clear LCD
        cmd_lcd(0x01);

        // Display opening message
        str_lcd("DOOR OPENING ");

        // Display animation symbols
        char_lcd(3);
        char_lcd(5);
        char_lcd(4);

        // Rotate motor to open door
        door_open();

        // Wait until fully opened
        delay_ms(5000);

        // Stop motor
        motor_stop();

        // Clear LCD
        cmd_lcd(0x01);

        // Display door opened message
        str_lcd("DOOR OPENED ");

        char_lcd(4);

        delay_ms(500);


/*==================== AUTO CLOSE TIMER ====================*/

        // Countdown from 30 seconds
        for(l = 30; l > 0; l--)
        {
                // Move cursor to first line
                cmd_lcd(0x80);

                // Display countdown message
                str_lcd("DOOR CLOSE IN ");

                // Move cursor to second line
                cmd_lcd(0xC0);

                str_lcd("  ");

                // Display tens digit
                char_lcd((l / 10) + 48);

                // Display units digit
                char_lcd((l % 10) + 48);

                // Display seconds text
                str_lcd(" sec ");

                // Door animation loop
                for(k = 0; k < 9; k++)
                {
                        // Move cursor for animation
                        cmd_lcd(0xC0 + 9);

                        // Display symbols
                        char_lcd(4);

                        // Alternate animation
                        if(k & 1)
                        {
                                char_lcd(5);
                                char_lcd(' ');
                        }
                        else
                        {
                                char_lcd(' ');
                                char_lcd(5);
                        }

                        // Display symbol
                        char_lcd(3);

                        // Small animation delay
                        delay_ms(100);
                }
        }


/*==================== DOOR CLOSING ====================*/

        // Clear LCD
        cmd_lcd(0x01);

        // Display closing message
        str_lcd("DOOR IS CLOSEING");

        // Move cursor to second line
        cmd_lcd(0xC0);

        str_lcd("   ");

        // Display animation symbols
        char_lcd(4);
        char_lcd(5);
        char_lcd(3);

        // Rotate motor to close door
        door_close();

        // Wait until fully closed
        delay_ms(5000);

        // Stop motor
        motor_stop();


/*==================== DOOR CLOSED ====================*/

        // Clear LCD
        cmd_lcd(0x01);

        // Display success message
        str_lcd("DOOR IS CLOSED ");

        // Display symbol
        char_lcd(3);

        // Delay for visibility
        delay_ms(1000);
}
/*---------------------------------------------------------
 * Function Name : disable
 * Description   : Disables EINT2 interrupt in VIC
 * Return Type   : void
 *---------------------------------------------------------*/
void disable(void)
{
        // Disable EINT2 interrupt channel
        VICIntEnClr = 1 << EINT2_VIC_CHNO;
}


/*---------------------------------------------------------
 * Function Name : enable
 * Description   : Enables EINT2 interrupt in VIC
 * Return Type   : void
 *---------------------------------------------------------*/
void enable(void)
{
        // Enable EINT2 interrupt channel
        VICIntEnable |= 1 << EINT2_VIC_CHNO;
}


/*---------------------------------------------------------
 * Function Name : password_edit
 * Description   : Changes existing user password
 *                 -> Verifies ID
 *                 -> Verifies old password
 *                 -> Verifies fingerprint
 *                 -> Updates new password in EEPROM
 * Return Type   : void
 *---------------------------------------------------------*/
void password_edit(void)
{
        // Variable for entered ID
        s8 p_id;

        // Buffers for passwords
        s8 pw1[5], pw2[5];

        // Retry counter
        s8 retry = 0;

        // Stores EEPROM address if ID found
        u16 found = 0;


/*====================== ID CHECK ======================*/

        // Clear LCD
        cmd_lcd(0x01);

        // Display ID prompt
        str_lcd("ENTER UR ID:");

        // Read ID from keypad
        p_id = readnum();

        // Check whether ID exists
        if((found = is_id_in_db(p_id)) == 0)
        {
                // Clear LCD
                cmd_lcd(0x01);

                // Display error message
                str_lcd("ID IS NOT FOUND");

                // Delay for visibility
                delay_ms(1000);

                // Exit function
                return;
        }

        // Clear LCD
        cmd_lcd(0x01);

        // Display success message
        str_lcd("ID IS FOUND");

        // Delay for visibility
        delay_ms(700);


        /* Read stored password from EEPROM */
        i2c_eeprom_seq_read(
                                0x50,
                                found + 1,
                                pw1,
                                5
                                                   );


/*=================== OLD PASSWORD CHECK ===================*/

PSWD:

        // Clear LCD
        cmd_lcd(0x01);

        // Display password prompt
        str_lcd("ENTER OLD PASSWORD:");

        // Move cursor to second line
        cmd_lcd(0xC0);

        // Read old password
        password_kpm(pw2);

        // Compare entered password with stored password
        if(strcmp((const char*)pw2,
                  (const char *)pw1) != 0)
        {
                // Clear LCD
                cmd_lcd(0x01);

                // Display wrong password message
                str_lcd("WRONG PASSWORD");

                // Increment retry counter
                retry++;

                // Check maximum attempts
                if(retry == 3)
                {
                        // Clear LCD
                        cmd_lcd(0x01);

                        // Display failure symbol
                        char_lcd(0);

                        // Display failure message
                        str_lcd("   NOT MATCHED ");

                        // Display failure symbol
                        char_lcd(0);

                        // Move cursor to second line
                        cmd_lcd(0xC0);

                        // Display update denied message
                        str_lcd(" CAN'T CHANGED ");

                        // Delay for visibility
                        delay_ms(1000);

                        // Exit function
                        return;
                }

                // Move cursor to second line
                cmd_lcd(0xC0);

                // Display remaining attempts
                str_lcd("you have ");

                char_lcd(3 - retry + 48);

                str_lcd(" chances");

                // Delay for visibility
                delay_ms(1000);

                // Retry password entry
                goto PSWD;
        }

        // Clear LCD
        cmd_lcd(0x01);

        // Display verification success message
        str_lcd("PASSWORD VERIFED");

        // Delay for visibility
        delay_ms(500);

        // Reset retry counter
        retry = 0;


/*=================== FINGERPRINT VERIFY ===================*/

FP:

        // Clear LCD
        cmd_lcd(0x01);

        // Verify fingerprint
        if(verify(p_id) != 0)
        {
                // Clear LCD
                cmd_lcd(0x01);

                // Display failure message
                str_lcd("FINGER NOT FOUND");

                // Move cursor to second line
                cmd_lcd(0xC0);

                // Ask user to retry
                str_lcd("try again");

                // Delay for visibility
                delay_ms(600);

                // Increment retry counter
                retry++;

                // First retry
                if(retry == 1)
                {
                        cmd_lcd(0x01);

                        str_lcd("two more chance");

                        delay_ms(500);

                        goto FP;
                }

                // Second retry
                else if(retry == 2)
                {
                        cmd_lcd(0x01);

                        str_lcd("last chance");

                        delay_ms(500);

                        goto FP;
                }

                // Access denied after maximum retries
                else
                {
                        cmd_lcd(0x01);

                        // Display failure symbol
                        char_lcd(0);

                        // Display failure message
                        str_lcd("   NOT MATCHED ");

                        // Display failure symbol
                        char_lcd(0);

                        // Move cursor to second line
                        cmd_lcd(0xC0);

                        // Display update denied message
                        str_lcd("    CAN'T CHANGED ");

                        // Delay for visibility
                        delay_ms(1000);

                        return;
                }
        }


/*=================== NEW PASSWORD ENTRY ===================*/

        else
        {

PSWD1:

                // Clear LCD
                cmd_lcd(0x01);

                // Ask user to enter new password
                str_lcd("ENTER NEW PASSWORD:");

                // Move cursor to second line
                cmd_lcd(0xC0);

                // Read new password
                password_kpm(pw1);

                // Clear LCD
                cmd_lcd(0x01);

                // Ask user to confirm password
                str_lcd("ENTER AGAIN:");

                // Move cursor to second line
                cmd_lcd(0xC0);

                // Read confirmation password
                password_kpm(pw2);

                // Compare both passwords
                if(strcmp((const char*)pw1,
                          (const char*)pw2) != 0)
                {
                        // Clear LCD
                        cmd_lcd(0x01);

                        // Display mismatch message
                        str_lcd("NOT MATCHED");

                        // Move cursor to second line
                        cmd_lcd(0xC0);

                        // Ask user to retry
                        str_lcd("try again");

                        // Delay for visibility
                        delay_ms(500);

                        // Retry password entry
                        goto PSWD1;
                }

                else
                {
                        // Update password in EEPROM
                        i2c_eeprom_write_page(
                                                        0x50,
                                                        found + 1,
                                                        pw1,
                                                        5
                                                     );

                        // Clear LCD
                        cmd_lcd(0x01);

                        // Display success message
                        str_lcd(" NEW PASSWORD ");

                        // Move cursor to second line
                        cmd_lcd(0xC0);

                        // Display update message
                        str_lcd("UPDATED ");

                        // Display space
                        char_lcd(' ');

                        // Display success symbol
                        char_lcd(1);

                        // Delay for visibility
                        delay_ms(500);
                }
        }
}
/*---------------------------------------------------------
 * Function Name : finger_edit
 * Description   : Updates user fingerprint
 *                 -> Verifies ID
 *                 -> Verifies password
 *                 -> Deletes old fingerprint
 *                 -> Enrolls new fingerprint
 * Return Type   : void
 *---------------------------------------------------------*/
void finger_edit(void)
{
        // Variable for entered ID
        s8 f_id;

        // Buffers for password verification
        s8 pw1[5], pw2[5];

        // Retry counter
        s8 retry = 0;

        // Variable to store EEPROM address
        u16 found;


/*=========================================================
 *                      ID CHECK
 *=========================================================*/

        // Clear LCD
        cmd_lcd(0x01);

        // Display ID prompt
        str_lcd("ENTER THE ID:");

        // Read ID from keypad
        f_id = readnum();

        // Check whether ID exists in database
        if((found = is_id_in_db(f_id)) == 0)
        {
                // Clear LCD
                cmd_lcd(0x01);

                // Display error message
                str_lcd("ID IS NOT FOUND");

                // Delay for visibility
                delay_ms(1000);

                // Exit function
                return;
        }

        // Clear LCD
        cmd_lcd(0x01);

        // Display success message
        str_lcd("ID IS FOUND");

        // Delay for visibility
        delay_ms(900);


/*=========================================================
 *                  PASSWORD VERIFICATION
 *=========================================================*/

        // Read stored password from EEPROM
        i2c_eeprom_seq_read(
                                0x50,
                                found + 1,
                                pw2,
                                5
                                                   );

PSWD:

        // Clear LCD
        cmd_lcd(0x01);

        // Display password prompt
        str_lcd("ENTER PASSWORD:");

        // Move cursor to second line
        cmd_lcd(0xC0);

        // Read entered password
        password_kpm(pw1);

        // Compare entered password with stored password
        if(strcmp((const char*)pw2,
                  (const char *)pw1) != 0)
        {
                // Clear LCD
                cmd_lcd(0x01);

                // Display error message
                str_lcd("WRONG PASSWORD");

                // Delay for visibility
                delay_ms(1000);

                // Increment retry count
                retry++;

                // Check maximum attempts
                if(retry == 3)
                {
                        // Clear LCD
                        cmd_lcd(0x01);

                        // Display failure symbol
                        char_lcd(0);

                        // Display failure message
                        str_lcd("   NOT MATCHED ");

                        // Display failure symbol
                        char_lcd(0);

                        // Move cursor to second line
                        cmd_lcd(0xC0);

                        // Display update failure message
                        str_lcd("CAN'T CHANGED ");

                        // Delay for visibility
                        delay_ms(1000);

                        // Exit function
                        return;
                }

                // Move cursor to second line
                cmd_lcd(0xC0);

                // Display remaining attempts
                str_lcd("you have ");

                char_lcd(3 - retry + 48);

                str_lcd(" chances");

                // Delay for visibility
                delay_ms(1000);

                // Retry password entry
                goto PSWD;
        }


/*=========================================================
 *                FINGERPRINT UPDATION
 *=========================================================*/

FP1:

        // Delete old fingerprint template
        delete(f_id);

        // Enroll new fingerprint
        if(enroll(f_id) != 0)
        {
                // Clear LCD
                cmd_lcd(0x01);

                // Display failure message
                str_lcd("FAIL FINGER");

                // Move cursor to second line
                cmd_lcd(0xC0);

                // Ask user to retry
                str_lcd("please try again");

                // Delay for visibility
                delay_ms(500);

                // Retry fingerprint enrollment
                goto FP1;
        }


/*=========================================================
 *                  UPDATE SUCCESS MESSAGE
 *=========================================================*/

        // Clear LCD
        cmd_lcd(0x01);

        // Display success message
        str_lcd(" SUCCESSFULLY ");

        // Move cursor to second line
        cmd_lcd(0xC0);

        // Display updated message
        str_lcd("  UPDATED ");

        // Display success symbol
        char_lcd(1);
}
/*---------------------------------------------------------
 * Function Name : forget_password
 * Description   : Resets forgotten password
 *                 -> Verifies user ID
 *                 -> Verifies default password
 *                 -> Updates new password in EEPROM
 * Return Type   : void
 *---------------------------------------------------------*/
void forget_password(void)
{
        // Variable for entered user ID
        s8 fo_id;

        // Buffers for password storage
        s8 pw1[5], pw2[5];

        // Retry counter
        s8 retry = 0;

        // Variable to store EEPROM address
        u16 found;


/*=========================================================
 *                      ID CHECK
 *=========================================================*/

        // Clear LCD
        cmd_lcd(0x01);

        // Display ID prompt
        str_lcd("ENTER THE ID:");

        // Read ID from keypad
        fo_id = readnum();

        // Check whether ID exists
        if((found = is_id_in_db(fo_id)) == 0)
        {
                // Clear LCD
                cmd_lcd(0x01);

                // Display error message
                str_lcd("ID IS NOT FOUND");

                // Delay for visibility
                delay_ms(1000);

                // Exit function
                return;
        }

        // Clear LCD
        cmd_lcd(0x01);

        // Display success message
        str_lcd("ID IS FOUND");

        // Delay for visibility
        delay_ms(1000);


/*=========================================================
 *                DEFAULT PASSWORD CHECK
 *=========================================================*/

PSWD:

        // Clear LCD
        cmd_lcd(0x01);

        // Display default password message
        str_lcd("ENTER THE DEFAULT ");

        // Move cursor to second line
        cmd_lcd(0xC0);

        // Display password prompt
        str_lcd("PASSWORD:");

        // Read entered password
        password_kpm(pw1);

        // Compare entered password with default password
        if(strcmp((const char*)"1729",
                  (const char *)pw1) != 0)
        {
                // Clear LCD
                cmd_lcd(0x01);

                // Display wrong password message
                str_lcd("WRONG PASSWORD");

                // Delay for visibility
                delay_ms(1000);

                // Increment retry count
                retry++;

                // Check maximum attempts
                if(retry == 2)
                {
                        // Clear LCD
                        cmd_lcd(0x01);

                        // Display failure symbol
                        char_lcd(0);

                        // Display failure message
                        str_lcd("   NOT MATCHED ");

                        // Display failure symbol
                        char_lcd(0);

                        // Move cursor to second line
                        cmd_lcd(0xC0);

                        // Display update failure message
                        str_lcd(" CAN'T CHANGED ");

                        // Delay for visibility
                        delay_ms(1000);

                        // Exit function
                        return;
                }

                // Move cursor to second line
                cmd_lcd(0xC0);

                // Ask user to retry
                str_lcd("try one more");

                // Delay for visibility
                delay_ms(1000);

                // Retry password check
                goto PSWD;
        }


/*=========================================================
 *                  PASSWORD UPDATION
 *=========================================================*/

PSWD1:

        // Clear LCD
        cmd_lcd(0x01);

        // Ask user to enter new password
        str_lcd("ENTER NEW PASSWORD:");

        // Move cursor to second line
        cmd_lcd(0xC0);

        // Read new password
        password_kpm(pw1);

        // Clear LCD
        cmd_lcd(0x01);

        // Ask user to confirm password
        str_lcd("ENTER AGAIN:");

        // Move cursor to second line
        cmd_lcd(0xC0);

        // Read confirmation password
        password_kpm(pw2);

        // Compare both passwords
        if(strcmp((const char*)pw1,
                  (const char*)pw2) != 0)
        {
                // Clear LCD
                cmd_lcd(0x01);

                // Display mismatch message
                str_lcd("NOT MATCHED");

                // Move cursor to second line
                cmd_lcd(0xC0);

                // Ask user to retry
                str_lcd("try again");

                // Delay for visibility
                delay_ms(500);

                // Retry password entry
                goto PSWD1;
        }

        else
        {
                // Update password in EEPROM
                i2c_eeprom_write_page(
                                                0x50,
                                                found + 1,
                                                pw1,
                                                5
                                             );

                // Clear LCD
                cmd_lcd(0x01);

                // Display success message
                str_lcd(" NEW PASSWORD ");

                // Move cursor to second line
                cmd_lcd(0xC0);

                // Display updated message
                str_lcd("UPDATED ");

                // Display space
                char_lcd(' ');

                // Display success symbol
                char_lcd(1);

                // Delay for visibility
                delay_ms(500);
        }
}
/*---------------------------------------------------------
 * Function Name : delete_id
 * Description   : Deletes user ID from database
 *                 -> Deletes fingerprint from sensor
 *                 -> Removes EEPROM user record
 *                 -> Shifts remaining records upward
 *                 -> Updates total user count
 *                 -> Displays delete status on LCD
 * Return Type   : void
 *---------------------------------------------------------*/

void delete_id(void)
{
        // Variable to store entered ID
        s8 id;

        // Variable to store total users
        s8 total;

        // Loop counter
        s8 i;

        // Buffer to hold one user record
        s8 buf[6];

        // EEPROM addresses
        u16 found_addr;
        u16 next_addr;
        u16 last_addr;

ID:

        // Clear LCD display
        cmd_lcd(0x01);

        // Move cursor to first line
        cmd_lcd(0x80);

        // Display message
        str_lcd("ENTER ID:");

        // Read ID from keypad
        id = readnum();

        // Check whether ID exists in EEPROM
        found_addr = is_id_in_db(id);

        // If ID not found
        if(found_addr == 0)
        {
                // Clear LCD
                cmd_lcd(0x01);

                // Display error message
                str_lcd("ID NOT FOUND");

                // Wait for 1 second
                delay_ms(1000);

                // Ask user again
                goto ID;
        }

        // Delete fingerprint template from sensor
        if(delete(id) != 0)
        {
                // Clear LCD
                cmd_lcd(0x01);

                // Display failure message
                str_lcd("DELETE FAIL");

                // Delay for visibility
                delay_ms(1000);

                // Exit function
                return;
        }

        // Read total number of users
        total = i2c_eeprom_read_byte(0x50,0x0000);

        // Calculate address of last record
        last_addr = 0x0001 + ((total-1) * 6);

        // Shift records upward
        while(found_addr < last_addr)
        {
                // Calculate next record address
                next_addr = found_addr + 6;

                // Read next user record
                i2c_eeprom_seq_read
                (
                        0x50,
                        next_addr,
                        buf,
                        6
                );

                // Write next record into current location
                i2c_eeprom_write_page
                (
                        0x50,
                        found_addr,
                        buf,
                        6
                );

                // Move to next record
                found_addr += 6;
        }

        // Clear buffer data
        for(i=0;i<6;i++)
        {
                // Fill buffer with empty value
                buf[i] = 0xFF;
        }

        // Clear last EEPROM record
        i2c_eeprom_write_page
        (
                0x50,
                last_addr,
                buf,
                6
        );

        // Check total users count
        if(total > 0)
        {
                // Decrement user count
                total--;

                // Update total users in EEPROM
                i2c_eeprom_write_byte
                (
                        0x50,
                        0x0000,
                        total
                );
        }

        // Clear LCD display
        cmd_lcd(0x01);

        // Move cursor to first line
        cmd_lcd(0x80);

        // Display success message
        str_lcd("USER DELETED");

        // Move cursor to second line
        cmd_lcd(0xC0);

        // Display status
        str_lcd("SUCCESS");

        // Display custom symbol
        char_lcd(1);

        // Wait before exit
        delay_ms(1500);
}

/*---------------------------------------------------------
 * Function Name : edit
 * Description   : Edit menu function
 *                 -> Password change
 *                 -> Fingerprint update
 *                 -> Forgot password recovery
 *                 -> Exit option
 * Return Type   : void
 *---------------------------------------------------------*/
void edit(void)
{
        // Variable to store user menu choice
        u8 choice;


/*=========================================================
 *                  CHECK DATABASE STATUS
 *=========================================================*/

        // Check whether any user data exists
        if(ids == 0)
        {
                // Display message only once
                if(flag == 0)
                {
                        // Set flag
                        flag = 1;

                        // Clear LCD
                        cmd_lcd(0x01);

                        // Display no data message
                        str_lcd("NO DATA FOUND");

                        // Move cursor to second line
                        cmd_lcd(0xC0);

                        // Ask user to enroll first
                        str_lcd("PRESS SW TO ENROLL");
                }

                // Exit function
                return;
        }

        // Reset flag
        flag = 0;


/*=========================================================
 *                      EDIT MENU
 *=========================================================*/

        while(1)
        {
                // Clear LCD
                cmd_lcd(0x01);

                // Display first line menu
                str_lcd("1.PSSWRD 2.FINGER");

                // Move cursor to second line
                cmd_lcd(0xC0);

                // Display second line menu
                str_lcd("3.FORGOT 4.EXIT");

                // Read keypad input
                choice = keyscan();


/*=========================================================
 *                    MENU SELECTION
 *=========================================================*/

                switch(choice)
                {
                        // Password update option
                        case '1':

                                password_edit();

                                break;


                        // Fingerprint update option
                        case '2':

                                finger_edit();

                                break;


                        // Forgot password option
                        case '3':

                                forget_password();

                                break;


                        // Exit option
                        case '4':

                                break;
                }


/*=========================================================
 *                      EXIT CHECK
 *=========================================================*/

                // Exit from menu loop
                if(choice == '4')
                {
                        break;
                }
        }
}
/*---------------------------------------------------------
 * Function Name : menu
 * Description   : Main menu function
 *                 -> Enroll new user
 *                 -> Edit user details
 *                 -> Delete user
 *                 -> Exit menu
 * Return Type   : void
 *---------------------------------------------------------*/
void menu(void)
{
        // Variable to store menu choice
        u8 choice;

        // Clear menu interrupt flag
        menu_flag = 0;

        // Disable external interrupt while inside menu
        disable();

        // Reset display flag
        flag = 0;


/*=========================================================
 *                      MAIN MENU LOOP
 *=========================================================*/

        while(1)
        {
                // Clear LCD display
                cmd_lcd(LCD_CLR);

                // Move cursor to first line
                cmd_lcd(GOTO_LINE1_POS_0);

                // Display first line menu options
                str_lcd("1.ENROLL 2.EDIT");

                // Move cursor to second line
                cmd_lcd(GOTO_LINE2_POS_0);

                // Display second line menu options
                str_lcd("3.DELETE 4.EXIT");

                // Read keypad input
                choice = keyscan();


/*=========================================================
 *                    MENU SELECTION
 *=========================================================*/

                switch(choice)
                {

/*-------------------- ENROLL OPTION --------------------*/

                        case '1':

                                /*
                                 * Admin verification can be enabled
                                 * if required before enrollment
                                 */

                                /*
                                if(check_admin() == 1)
                                        break;
                                */

                                // Call enrollment function
                                enroll_id();

                                break;


/*---------------------- EDIT OPTION --------------------*/

                        case '2':

                                // Call edit menu function
                                edit();

                                break;


/*--------------------- DELETE OPTION -------------------*/

                        case '3':

                                // Call delete function
                                delete_id();

                                break;


/*---------------------- EXIT OPTION --------------------*/

                        case '4':

                                // Exit from switch
                                break;
                }


/*=========================================================
 *                      EXIT HANDLING
 *=========================================================*/

                // Check whether user selected EXIT
                if(choice == '4')
                {
                        // Clear LCD
                        cmd_lcd(0x01);

                        // Reset menu flag
                        menu_flag = 0;

                        // Re-enable external interrupt
                        enable();

                        // Display total enrolled IDs
                        char_lcd(ids + 48);

                        // Exit menu loop
                        break;
                }
        }
}