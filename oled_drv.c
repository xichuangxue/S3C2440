#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/arch/regs-gpio.h>
#include <asm/hardware.h>



static struct class *oled_class;

static struct class_device *oled_class_dev;


static char leds_status = 0x0;
static DECLARE_MUTEX(leds_lock);//DT?? leds_status¨º¡À¡ê?D¨¨¨°a??¨¨???


#define OLED_SCLK_Clr() s3c2410_gpio_setpin(S3C2410_GPE14,0)
#define OLED_SCLK_Set() s3c2410_gpio_setpin(S3C2410_GPE14,1)

#define OLED_SDIN_Clr() s3c2410_gpio_setpin(S3C2410_GPE15,0)
#define OLED_SDIN_Set() s3c2410_gpio_setpin(S3C2410_GPE15,1)



static int s3c24xx_oled_open (struct inode *inode, struct file *file)
{
	s3c2410_gpio_cfgpin(S3C2410_GPE14,S3C2410_GPE14_OUTP);
	s3c2410_gpio_cfgpin(S3C2410_GPE15,S3C2410_GPE15_OUTP);

	s3c2410_gpio_setpin(S3C2410_GPE14,1);
	s3c2410_gpio_setpin(S3C2410_GPE15,1);

	return 0;
}

static ssize_t s3c24xx_oled_read (struct file *filp, char __user *buff, size_t count, loff_t *offp)
{

	return count;
}


static void Write_IIC_Byte(unsigned char IIC_Byte)
{
	unsigned char i;
	unsigned char m,da;
	da=IIC_Byte;
	
	OLED_SCLK_Clr();
	for(i=0;i<8;i++)		
	{
		m=da;
		m=m&0x80;
		if(m==0x80)
			OLED_SDIN_Set();
		else 
			OLED_SDIN_Clr();
		da=da<<1;
		OLED_SCLK_Set();
		OLED_SCLK_Clr();
	}

}


static void IIC_Start()
{

	OLED_SCLK_Set() ;
	OLED_SDIN_Set();
	OLED_SDIN_Clr();
	OLED_SCLK_Clr();
}

static void IIC_Stop()
{
	OLED_SCLK_Set() ;
//	OLED_SCLK_Clr();
	OLED_SDIN_Clr();
	OLED_SDIN_Set();
	
}

static void IIC_Wait_Ack()
{
	OLED_SCLK_Set() ;
	OLED_SCLK_Clr();
}


/*
buf[0]  
  0		//write cmd
  1		//write data
*/
ssize_t s3c24xx_oled_write (struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	unsigned char val[2];

	if(count == 2)
		copy_from_user(&val,buf,2);
	else
		return -1;

	IIC_Start();
	Write_IIC_Byte(0x78);
	IIC_Wait_Ack();	
	if(val[0])			
		Write_IIC_Byte(0x40);			//write data
	else
		Write_IIC_Byte(0x00);			//write cmd
	IIC_Wait_Ack();	
	Write_IIC_Byte(val[1]);
	IIC_Wait_Ack();	
	IIC_Stop();	

}



static struct file_operations s3c24xx_oled_fops ={
	.owner	= THIS_MODULE,
	.open	= s3c24xx_oled_open,
	.read	= s3c24xx_oled_read,
	.write	= s3c24xx_oled_write,
	
};



int major;
static int __init  s3c24xx_oled_init(void)
{
	int ret;
	int minor;

	major = register_chrdev(0,"oled",&s3c24xx_oled_fops);

	
	oled_class = class_create(THIS_MODULE,"oled");
	if(IS_ERR(oled_class))
		return PTR_ERR(oled_class);

	oled_class_dev = class_device_create(oled_class,NULL,MKDEV(major,0),NULL,"oled");


	printk("initialized\n");
	return 0;
}





static void __exit  s3c24xx_oled_exit(void)
{
	int minor;
	unregister_chrdev(major,"oled");
	class_device_unregister(oled_class_dev);
	class_destroy(oled_class);
}



module_init(s3c24xx_oled_init);
module_exit(s3c24xx_oled_exit);

MODULE_LICENSE("GPL");



