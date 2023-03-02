/*LCD1602 (HD44780) LKM
*
* Copyright (C) 2023 Petrov Danila (pda761@mail.ru).
*
*  This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public Licence
* as published by the Free Software Foundation; either version 3
* of the Licence, or (at your option) any later version.
*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/of_device.h>
#include <linux/gpio/consumer.h>
#include <linux/platform_device.h>
#include <linux/mod_devicetable.h>
#include <linux/property.h>
#include <linux/of_device.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>




// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

#define LOW 0
#define HIGH 1
//#define BITMODE_8



/* Variables for lcd1602 */
static const char *label;
static char lcd_buff[32];

static struct gpio_desc *_rs_pin; // LOW: command. HIGH: character.
static struct gpio_desc *_enable_pin; // activated by a HIGH pulse.

#ifdef BITMODE_8
static struct gpio_desc *_data_pins[8];
#else
static struct gpio_desc *_data_pins[4];
#endif

static uint8_t _displayfunction;
static uint8_t _displaycontrol;
static uint8_t _displaymode;

static uint8_t _numlines;
static uint8_t _row_offsets[4];


/* Variables for device and device class */
static dev_t my_device_nr;
static struct class *my_class;
static struct cdev my_device;




/* LCD1602 specific functional */


/************ low level data pushing commands **********/

static void pulseEnable(void) {
	gpiod_set_value(_enable_pin, LOW);
	usleep_range(1, 1);    
	gpiod_set_value(_enable_pin, HIGH);
	usleep_range(1, 1);    // enable pulse must be >450 ns
	gpiod_set_value(_enable_pin, LOW);
	usleep_range(100, 100);   // commands need >37 us to settle
}


static void write4bits(uint8_t value) {
	int i;
	for (i = 0; i < 4; i++) {
		gpiod_set_value(_data_pins[i], ((value >> i) & 0x01));
	}

	pulseEnable();
}


static void write8bits(uint8_t value) {
	int i;
	for (i = 0; i < 8; i++) {
		gpiod_set_value(_data_pins[i], ((value >> i) & 0x01));
	}
	
	pulseEnable();
}


// write either command or data, with automatic 4/8-bit selection
static void send(uint8_t value, uint8_t mode) {
	gpiod_set_value(_rs_pin, mode);

	if (_displayfunction & LCD_8BITMODE) {
		write8bits(value); 
	} else {
		write4bits(value>>4);
		write4bits(value);
	}
}




/*********** mid level commands, for sending data/cmds */

static void command(uint8_t value) {
  	send(value, LOW);
}

static size_t write(uint8_t value) {
	send(value, HIGH);
	return 1; // assume success
}




/********** high level commands, for the user! */

static void clear(void) {
	command(LCD_CLEARDISPLAY);  // clear display, set cursor position to zero
	usleep_range(2000, 2000);  // this command takes a long time!
}


static void display(void) {
	_displaycontrol |= LCD_DISPLAYON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}


static void setCursor(uint8_t col, uint8_t row) {
	const size_t max_lines = sizeof(_row_offsets) / sizeof(*_row_offsets);
	if ( row >= max_lines ) {
		row = max_lines - 1;    // we count rows starting w/ 0
	}
	if ( row >= _numlines ) {
		row = _numlines - 1;    // we count rows starting w/ 0
	}
	
	command(LCD_SETDDRAMADDR | (col + _row_offsets[row]));
}




static void setRowOffsets(int row0, int row1, int row2, int row3){
	_row_offsets[0] = row0;
	_row_offsets[1] = row1;
	_row_offsets[2] = row2;
	_row_offsets[3] = row3;
}





static void begin(uint8_t cols, uint8_t lines, uint8_t dotsize) {

	if (lines > 1) {
		_displayfunction |= LCD_2LINE;
	}
	_numlines = lines;


	setRowOffsets(0x00, 0x40, 0x00 + cols, 0x40 + cols);


	// for some 1 line displays you can select a 10 pixel high font
	if ((dotsize != LCD_5x8DOTS) && (lines == 1)) {
		_displayfunction |= LCD_5x10DOTS;
	}


	// SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
	// according to datasheet, we need at least 40 ms after power rises above 2.7 V
	// before sending commands. Arduino can turn on way before 4.5 V so we'll wait 50
	usleep_range(50000, 50500); 
	// Now we pull both RS and R/W low to begin commands
	gpiod_set_value(_rs_pin, LOW);
	gpiod_set_value(_enable_pin, LOW);
	
	
	//put the LCD into 4 bit or 8 bit mode
	if (! (_displayfunction & LCD_8BITMODE)) {
		// this is according to the Hitachi HD44780 datasheet
		// figure 24, pg 46


		// we start in 8bit mode, try to set 4 bit mode
		write4bits(0x03);
		usleep_range(4500, 4500); // wait min 4.1ms


		// second try
		write4bits(0x03);
		usleep_range(4500, 4500); // wait min 4.1ms
		

		// third go!
		write4bits(0x03); 
		usleep_range(150, 150);


		// finally, set to 4-bit interface
		write4bits(0x02); 
	} else {
		// this is according to the Hitachi HD44780 datasheet
		// page 45 figure 23


		// Send function set command sequence
		command(LCD_FUNCTIONSET | _displayfunction);
		usleep_range(4500, 4500);  // wait more than 4.1 ms


		// second try
		command(LCD_FUNCTIONSET | _displayfunction);
		usleep_range(150, 150);


		// third go
		command(LCD_FUNCTIONSET | _displayfunction);
	}


	// finally, set # lines, font size, etc.
	command(LCD_FUNCTIONSET | _displayfunction);  


	// turn the display on with no cursor or blinking default
	_displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;  
	display();


	// clear it off
	clear();


	// Initialize to default text direction (for romance languages)
	_displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
	// set the entry mode
	command(LCD_ENTRYMODESET | _displaymode);

}




static void init(uint8_t cols, uint8_t rows) {
  
	#ifdef BITMODE_8
	_displayfunction = LCD_8BITMODE | LCD_1LINE | LCD_5x8DOTS;
	#else
	_displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;
	#endif
	
	begin(cols, rows, LCD_5x8DOTS);  
	setCursor(0, 0);
}








/* State module functional */
static ssize_t lcd1602_write(struct file *filp, const char __user *user_buff, size_t count, loff_t *offp) {
	int to_copy, not_copied, delta, i;

	//unsigned long copy_from_user(void *to, const void __user *from, unsigned long count);
	



	/* Get amount of data to copy */
	to_copy = min(count, sizeof(lcd_buff));

	/* Copy data to user */
	not_copied = copy_from_user(lcd_buff, user_buff, to_copy);

	/* Calculate data */
	delta = to_copy - not_copied;

	/* Set the new data to the display */
	clear();
	setCursor(0, 0);

	
	for(i=0; i<to_copy; i++) {
		if (lcd_buff[i] == '#') {	// Use '#' as line divider
			setCursor(0, 1);
		}
		else{
			write(lcd_buff[i]);
		}
	}

	return delta;
}




static struct file_operations lcd1602_fops = {
	.owner = THIS_MODULE,
	.write = lcd1602_write,
};




static int lcd1602_init_probe(struct platform_device *pdev) {
	
    printk("lcd1602 - driver init\n");







	/*GPIOs init*/
	/* "rs" label is matching the device tree declaration. OUT_LOW is the value at init */
    _rs_pin = devm_gpiod_get(&pdev->dev, "rs", GPIOD_OUT_LOW);
    if(IS_ERR(_rs_pin)) {
		printk("lcd1602 - Error! Could not setup the GPIO RS\n");
		return -1 * IS_ERR(_rs_pin);
	}

	/* "en" label is matching the device tree declaration. OUT_LOW is the value at init */
    _enable_pin = devm_gpiod_get(&pdev->dev, "en", GPIOD_OUT_LOW);
    if(IS_ERR(_enable_pin)) {
		printk("lcd1602 - Error! Could not setup the GPIO EN\n");
		return -1 * IS_ERR(_enable_pin);
	}
	




	/* "d0" label is matching the device tree declaration. OUT_LOW is the value at init */
    _data_pins[0] = devm_gpiod_get(&pdev->dev, "d0", GPIOD_OUT_LOW);
    if(IS_ERR(_data_pins[0])) {
		printk("lcd1602 - Error! Could not setup the GPIO D0\n");
		return -1 * IS_ERR(_data_pins[0]);
	}

	/* "d1" label is matching the device tree declaration. OUT_LOW is the value at init */
    _data_pins[1] = devm_gpiod_get(&pdev->dev, "d1", GPIOD_OUT_LOW);
    if(IS_ERR(_data_pins[1])) {
		printk("lcd1602 - Error! Could not setup the GPIO D1\n");
		return -1 * IS_ERR(_data_pins[1]);
	}

	/* "d2" label is matching the device tree declaration. OUT_LOW is the value at init */
    _data_pins[2] = devm_gpiod_get(&pdev->dev, "d2", GPIOD_OUT_LOW);
    if(IS_ERR(_data_pins[2])) {
		printk("lcd1602 - Error! Could not setup the GPIO D2\n");
		return -1 * IS_ERR(_data_pins[2]);
	}

	/* "d3" label is matching the device tree declaration. OUT_LOW is the value at init */
    _data_pins[3] = devm_gpiod_get(&pdev->dev, "d3", GPIOD_OUT_LOW);
    if(IS_ERR(_data_pins[3])) {
		printk("lcd1602 - Error! Could not setup the GPIO D3\n");
		return -1 * IS_ERR(_data_pins[3]);
	}

	
	#ifdef BITMODE_8
	/* "d4" label is matching the device tree declaration. OUT_LOW is the value at init */
    _data_pins[4] = devm_gpiod_get(&pdev->dev, "d4", GPIOD_OUT_LOW);
    if(IS_ERR(_data_pins[4])) {
		printk("lcd1602 - Error! Could not setup the GPIO D4\n");
		return -1 * IS_ERR(_data_pins[4]);
	}

	/* "d5" label is matching the device tree declaration. OUT_LOW is the value at init */
    _data_pins[5] = devm_gpiod_get(&pdev->dev, "d5", GPIOD_OUT_LOW);
    if(IS_ERR(_data_pins[5])) {
		printk("lcd1602 - Error! Could not setup the GPIO D5\n");
		return -1 * IS_ERR(_data_pins[5]);
	}

	/* "d6" label is matching the device tree declaration. OUT_LOW is the value at init */
    _data_pins[6] = devm_gpiod_get(&pdev->dev, "d6", GPIOD_OUT_LOW);
    if(IS_ERR(_data_pins[6])) {
		printk("lcd1602 - Error! Could not setup the GPIO D6\n");
		return -1 * IS_ERR(_data_pins[6]);
	}

	/* "d7" label is matching the device tree declaration. OUT_LOW is the value at init */
    _data_pins[7] = devm_gpiod_get(&pdev->dev, "d7", GPIOD_OUT_LOW);
    if(IS_ERR(_data_pins[7])) {
		printk("lcd1602 - Error! Could not setup the GPIO D7\n");
		return -1 * IS_ERR(_data_pins[7]);
	}
	#endif







	/* Check for device properties */
	if(!device_property_present(&pdev->dev, "label")) {
		printk("lcd1602 - Error! Device property 'label' not found!\n");
		return -1;
	}

	/* Read device properties */
	if(device_property_read_string(&pdev->dev, "label", &label)) {
		printk("lcd1602 - Error! Could not read 'label'\n");
		return -1;
	}
	printk("lcd1602 - label: %s\n", label);





    /* Allocate a device nr */
	if(alloc_chrdev_region(&my_device_nr, 0, 1, label) < 0) {
		printk("lcd1602: %s - Device Nr. could not be allocated!\n", label);
		return -1;
	}
	printk("lcd1602: %s - Device Nr. Major: %d, Minor: %d was registered!\n", label, my_device_nr >> 20, my_device_nr && 0xfffff);


	/* Create device class */
	if((my_class = class_create(THIS_MODULE, label)) == NULL) {
		printk("lcd1602: %s - Device class can not be created!\n", label);
		unregister_chrdev_region(my_device_nr, 1);
		return -1;
	}


	/* create device file */
	if(device_create(my_class, NULL, my_device_nr, NULL, label) == NULL) {
		printk("lcd1602: %s - Can not create device file!\n", label);
		class_destroy(my_class);
		return -1;
	}


	/* Initialize device file */
	cdev_init(&my_device, &lcd1602_fops);


	/* Regisering device to kernel */
	if(cdev_add(&my_device, my_device_nr, 1) == -1) {
		printk("mygpio: %s - Registering of device to kernel failed!\n", label);
		device_destroy(my_class, my_device_nr);
		return -1;
	}




	
	/*LCD1602 init*/
	init(16, 2);

	int i;
	char text[] = "Hello STM32MP1!";
  	for(i=0; i<sizeof(text)-1;i++) {
    	write(text[i]);
	}



	printk("lcd1602 - Initialization driver successfull!\n");

	return 0;
}




static int lcd1602_exit_remove(struct platform_device *pdev) {

	clear();
    cdev_del(&my_device);
	device_destroy(my_class, my_device_nr);
	class_destroy(my_class);
	unregister_chrdev_region(my_device_nr, 1);


    printk("lcd1602 - driver remove\n");
   
    return 0;
}




/* this structure does the matching with the device tree */
/* if it does not match the compatible field of DT, nothing happens */
static struct of_device_id lcd1602_of_match[] = {
    {
        .compatible = "petroff,lcd1602"
    },{/* end node */}
};
MODULE_DEVICE_TABLE(of, lcd1602_of_match);



static struct platform_driver lcd1602_driver = {
    .probe = lcd1602_init_probe,
    .remove = lcd1602_exit_remove,
    .driver = {
        .name = "lcd1602_driver",
        .owner = THIS_MODULE,
        .of_match_table = lcd1602_of_match,
    }
};
 



module_platform_driver(lcd1602_driver);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("pet_r_off");
MODULE_DESCRIPTION("A simple loadable LCD1602 (HD44780) Linux module.");
MODULE_VERSION("0.1");

