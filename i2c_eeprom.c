/*---------------------------------------------------------
 * Function Name : i2c_eeprom_write_byte
 * Description   : Writes single byte into EEPROM
 *                 -> Sends EEPROM slave address
 *                 -> Sends memory address
 *                 -> Writes one byte of data
 *                 -> Generates stop condition
 * Return Type   : void
 *---------------------------------------------------------*/

void i2c_eeprom_write_byte(u8 SlaveAddr,
                                                   u16 BuffAddr,
                                                   u8 data)
{
                // Generate start condition
                i2c_start();

                // Send slave address with write bit
                i2c_write(SlaveAddr<<1);

                // Send higher byte of EEPROM address
                i2c_write(BuffAddr>>8);

                // Send lower byte of EEPROM address
                i2c_write(BuffAddr);

                // Send data byte
                i2c_write(data);

                // Generate stop condition
                i2c_stop();

                // EEPROM internal write delay
                delay_ms(10);
}


/*---------------------------------------------------------
 * Function Name : i2c_eeprom_read_byte
 * Description   : Reads single byte from EEPROM
 *                 -> Sends EEPROM address
 *                 -> Generates repeated start
 *                 -> Reads one byte of data
 *                 -> Returns received byte
 * Return Type   : u8
 *---------------------------------------------------------*/

u8 i2c_eeprom_read_byte(u8 SlaveAddr,
                                                u16 BufferAddr)
{
        // Variable to store received data
        u8 data;

        // Generate start condition
        i2c_start();

        // Send slave address with write bit
        i2c_write(SlaveAddr<<1);

        // Send higher byte of EEPROM address
        i2c_write(BufferAddr>>8);

        // Send lower byte of EEPROM address
        i2c_write(BufferAddr);

        // Generate repeated start
        i2c_restart();

        // Send slave address with read bit
        i2c_write(SlaveAddr<<1|1);

        // Read data byte without acknowledgment
        data = i2c_nack();

        // Generate stop condition
        i2c_stop();

        // Return received data
        return data;
}


/*---------------------------------------------------------
 * Function Name : i2c_eeprom_write_page
 * Description   : Writes multiple bytes into EEPROM
 *                 -> Sends EEPROM address
 *                 -> Writes page data sequentially
 *                 -> Generates stop condition
 * Return Type   : void
 *---------------------------------------------------------*/

void i2c_eeprom_write_page(u8 SlaveAddr,
                                                   u16 BuffAddr,
                                                   s8 *p,
                                                   u8 nbytes)
{
        // Loop counter
        u8 i;

        // Generate start condition
        i2c_start();

        // Send slave address with write bit
        i2c_write(SlaveAddr<<1);

        // Send higher byte of EEPROM address
        i2c_write(BuffAddr>>8);

        // Send lower byte of EEPROM address
        i2c_write(BuffAddr);

        // Write multiple bytes into EEPROM
        for(i=0;i<nbytes;i++)
        {
                // Send data byte
                i2c_write(p[i]);
        }

        // Generate stop condition
        i2c_stop();

        // EEPROM internal write delay
        delay_ms(10);
}


/*---------------------------------------------------------
 * Function Name : i2c_eeprom_seq_read
 * Description   : Reads multiple bytes sequentially
 *                 from EEPROM memory
 *                 -> Sends EEPROM address
 *                 -> Reads bytes continuously
 *                 -> Sends ACK for intermediate bytes
 *                 -> Sends NACK for last byte
 * Return Type   : void
 *---------------------------------------------------------*/

void i2c_eeprom_seq_read(u8 SlaveAddr,
                                                 u16 BuffAddr,
                                                 s8 *p,
                                                 u8 nbytes
                                                )
{
                // Loop counter
                u8 i;

                // Generate start condition
                i2c_start();

                // Send slave address with write bit
                i2c_write(SlaveAddr<<1);

                // Send higher byte of EEPROM address
                i2c_write(BuffAddr>>8);

                // Send lower byte of EEPROM address
                i2c_write(BuffAddr);

                // Generate repeated start
                i2c_restart();

                // Send slave address with read bit
                i2c_write(SlaveAddr<<1|1);

                // Read bytes with acknowledgment
                for(i=0;i<nbytes-1;i++)
                {
                        // Read data byte
                        p[i] = i2c_mack();
                }

                        // Read last byte without acknowledgment
                        p[i] = i2c_nack();

                        // Generate stop condition
                        i2c_stop();

                        // Delay after read operation
                        delay_ms(10);
}
