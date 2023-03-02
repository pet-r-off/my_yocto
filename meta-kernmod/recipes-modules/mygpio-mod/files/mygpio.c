/*GPIO LKM
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



/* Variables for gpio */
static const char *label;
static struct gpio_desc *my_gpio;


/* Variables for device and device class */
static dev_t my_device_nr;
static struct class *my_class;
static struct cdev my_device;




static ssize_t gpio_read(struct file *filp, char __user *user_buff, size_t count, loff_t *offp) {

	//unsigned long copy_to_user(void __user *to, const void *from, unsigned long count);




	int to_copy, not_copied, delta;
	char tmp[3] = " \n";


	/* Get amount of data to copy */
	to_copy = min(count, sizeof(tmp));


	/* Read value of button */
	tmp[0] = gpiod_get_value(my_gpio) + '0';


	/* Copy data to user */
	not_copied = copy_to_user(user_buff, &tmp, to_copy);


	/* Calculate data */
	delta = to_copy - not_copied;


	return delta;
}




static ssize_t gpio_write(struct file *filp, const char __user *user_buff, size_t count, loff_t *offp) {

	//unsigned long copy_from_user(void *to, const void __user *from, unsigned long count);




	int to_copy, not_copied, delta;
	char value;


	/* Get amount of data to copy */
	to_copy = min(count, sizeof(value));


	/* Copy data to user */
	not_copied = copy_from_user(&value, user_buff, to_copy);

	/* Calculate data */
	delta = to_copy - not_copied;

	/* Setting the LED */
	switch(value) {
		case '0':
			gpiod_set_value(my_gpio, 0);
			break;
		case '1':
			gpiod_set_value(my_gpio, 1);
			break;
	}




	return delta;
}




static struct file_operations gpio_fops = {
	.owner = THIS_MODULE,
	.read = gpio_read,
	.write = gpio_write,
};




static int gpio_init_probe(struct platform_device *pdev) {
    printk("mygpio - driver init\n");

   /* "ledgpio" label is matching the device tree declaration. OUT_LOW is the value at init */
    my_gpio = devm_gpiod_get(&pdev->dev, "ledgpio", GPIOD_OUT_LOW);
    if(IS_ERR(my_gpio)) {
		printk("mygpio - Error! Could not setup the GPIO\n");
		return -1 * IS_ERR(my_gpio);
	}





	/* Check for device properties */
	if(!device_property_present(&pdev->dev, "label")) {
		printk("mygpio - Error! Device property 'label' not found!\n");
		return -1;
	}

	/* Read device properties */
	if(device_property_read_string(&pdev->dev, "label", &label)) {
		printk("mygpio - Error! Could not read 'label'\n");
		return -1;
	}
	printk("mygpio - label: %s\n", label);





    /* Allocate a device nr */
	if(alloc_chrdev_region(&my_device_nr, 0, 1, label) < 0) {
		printk("mygpio: %s - Device Nr. could not be allocated!\n", label);
		return -1;
	}
	printk("mygpio: %s - Device Nr. Major: %d, Minor: %d was registered!\n", label, my_device_nr >> 20, my_device_nr && 0xfffff);


	/* Create device class */
	if((my_class = class_create(THIS_MODULE, label)) == NULL) {
		printk("mygpio: %s - Device class can not be created!\n", label);
		unregister_chrdev_region(my_device_nr, 1);
		return -1;
	}



	/* Create device file */
	if(device_create(my_class, NULL, my_device_nr, NULL, label) == NULL) {
		printk("mygpio: %s - Can not create device file!\n", label);
		class_destroy(my_class);
		return -1;
	}


	/* Initialize device file */
	cdev_init(&my_device, &gpio_fops);

	/* Regisering device to kernel */
	if(cdev_add(&my_device, my_device_nr, 1) == -1) {
		printk("mygpio: %s - Registering of device to kernel failed!\n", label);
		device_destroy(my_class, my_device_nr);
		return -1;
	}

	return 0;	
}




static int gpio_exit_remove(struct platform_device *pdev) {
    cdev_del(&my_device);
	device_destroy(my_class, my_device_nr);
	class_destroy(my_class);
	unregister_chrdev_region(my_device_nr, 1);

    printk("mygpio: %s - driver remove\n", label);
   
    return 0;
}




/* this structure does the matching with the device tree */
/* if it does not match the compatible field of DT, nothing happens */
static struct of_device_id mygpio_of_match[] = {
    {
        .compatible = "petroff,mygpio"
    },{/* end node */}
};
MODULE_DEVICE_TABLE(of, mygpio_of_match);



static struct platform_driver mygpio_driver = {
    .probe = gpio_init_probe,
    .remove = gpio_exit_remove,
    .driver = {
        .name = "mygpio_driver",
        .owner = THIS_MODULE,
        .of_match_table = mygpio_of_match,
    },
};
 



module_platform_driver(mygpio_driver);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("pet_r_off");
MODULE_DESCRIPTION("A simple loadable GPIO Linux module.");
MODULE_VERSION("0.1");

