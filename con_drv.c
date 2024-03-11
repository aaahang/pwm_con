#include "asm/gpio.h"
#include "asm/io.h"
#include "asm/uaccess.h"
#include "linux/cdev.h"
#include "linux/clockchips.h"
#include "linux/dmi.h"
#include "linux/err.h"
#include "linux/export.h"
#include "linux/gpio.h"
#include "linux/i2c.h"
#include "linux/jump_label.h"
#include "linux/kdev_t.h"
#include "linux/mod_devicetable.h"
#include "linux/node.h"
#include "linux/of.h"
#include "linux/printk.h"
#include "linux/pwm.h"
#include "linux/stddef.h"
#include "linux/types.h"
#include <linux/module.h>
#include <linux/poll.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/miscdevice.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/mutex.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/stat.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/tty.h>
#include <linux/kmod.h>
#include <linux/gfp.h>
#include <linux/gpio/consumer.h>
#include <linux/platform_device.h>
#include <linux/of_gpio.h>
#include <linux/of_irq.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/slab.h>
#include <linux/fcntl.h>
#include <linux/timer.h>


struct pwm_device *pwm_con_de =NULL;
int con1_gpio ,con2_gpio,con_pwm_gpio;
static struct of_device_id pwm2_con_de[] = 
{
    {
        .compatible = "pwm_con"
    },
};
int pwm_open (struct inode *id, struct file *fe)
{
    printk("\n open form driver \n");
    return 0;
}
union 
{
    struct {
        int pwm,con;
    }ik;
    char uk[16];
}pwm_k;
ssize_t pwm_write (struct file *fe, const char __user * buf , size_t size , loff_t * lof)
{

    copy_from_user(pwm_k.uk,buf,16);
    pwm_config(pwm_con_de,pwm_k.ik.pwm,5000);
    // pwm_enable(pwm_con_de);
    //  gpio_set_value(con_pwm_gpio,pwm_k.ik.pwm);
    switch (pwm_k.ik.con)
    {
       case 1: 
        gpio_set_value(con1_gpio,1);
        gpio_set_value(con2_gpio,0);
        break;
       case 0: 
        gpio_set_value(con1_gpio,0);
        gpio_set_value(con2_gpio,1);
        break;
    
    default:
        gpio_set_value(con1_gpio,0);
        gpio_set_value(con2_gpio,0);
        break;
    }
    return 0;
    
}

static struct file_operations pwm_op=
{
    .owner =  THIS_MODULE,
    .open = pwm_open,
    .write = pwm_write
};
struct cdev * pwm_cdev;
struct device_node  *pwm_con,*child;

dev_t pwm_m =0 ;
unsigned int major; 
struct class * pwm_class = NULL;

int con_probe(struct platform_device * pd)
{
    int ret;
    // pwm 注册
    pwm_con = of_find_node_by_path("/pwm2_con");
    child = of_get_next_child(pwm_con,NULL);
    if(pwm_con == NULL)
    {
        printk(KERN_EMERG "\t  get tem_device_node failed!  \n");
        goto out_err_0;
    }
    pwm_con_de = devm_of_pwm_get(&pd->dev,child,"pwm2_con_pwnm");
     if (IS_ERR(pwm_con_de))
    {
            printk(KERN_ERR" pwm_test,get pwm  error!!\n");
            return -1;
    }
    if(pwm_con_de == NULL)
    {
        printk( "get pwms failed!  \n");
        goto out_err_0;

    }
    pwm_config(pwm_con_de,1000,5000);
    pwm_set_polarity(pwm_con_de,PWM_POLARITY_NORMAL);
    pwm_enable(pwm_con_de);

    //gpio注册
    // con_pwm_gpio = of_get_named_gpio(pwm_con,"conpwm",0);
    // gpio_direction_output(con_pwm_gpio,0);
    con1_gpio = of_get_named_gpio(pwm_con,"con1",0);
    gpio_direction_output(con1_gpio,0);
    con2_gpio = of_get_named_gpio(pwm_con,"con2",0);
    gpio_direction_output(con2_gpio,0);

    ret=  alloc_chrdev_region(&pwm_m,0,1,"pwm_De");
    if(ret < 0)
    {
        printk(KERN_INFO "alloc_chrdev_region fail\n");
        goto out_err_0;
    }
    printk(KERN_INFO "MAJOR %d\n", MAJOR(pwm_m)); //打印主设备号
    pwm_cdev = cdev_alloc();
    cdev_init(pwm_cdev,&pwm_op);
    ret = cdev_add(pwm_cdev,pwm_m,1);
    if(ret)
    {
         printk(KERN_INFO "cdev_add error\n");
        goto out_err_1;
    }
    pwm_class = class_create(THIS_MODULE,"PWM_cLASS");
    if (IS_ERR(pwm_class)) {
        printk(KERN_ERR "can't register device mydevice class\n");
        goto out_err_2;
    }
    device_create(pwm_class,NULL,pwm_m,NULL,"device_pwm");
    if (IS_ERR(pwm_class)) {
        printk(KERN_ERR "can't register device mydevice class\n");
        goto out_err_2;
    }

    return 0;

    // out_err_3:
    //     class_destroy(pwm_class);
    out_err_2:
        cdev_del(pwm_cdev);
    out_err_1:
        unregister_chrdev_region(pwm_m, 1);
    out_err_0:
        return -EINVAL;

}
int con_remove(struct platform_device *pf)
{
    pwm_disable(pwm_con_de);
    device_destroy(pwm_class,pwm_m);
    class_destroy(pwm_class);
    cdev_del(pwm_cdev);
    unregister_chrdev_region(pwm_m,1);

    return 0;
}
static struct platform_driver pwm2_con_dr =
{
    .probe = con_probe,
    .remove = con_remove,
    .driver =
    {
        .owner = THIS_MODULE,
        .name  = "pwm2_con_drive",
        .of_match_table = pwm2_con_de
    }
};

static int __init tem_getdata_driver_init(void)
{
    int error;
    
    error = platform_driver_register(&pwm2_con_dr);

    printk(KERN_EMERG "\tDriverState = %d\n",error);
    return 0;
}
static void __exit tem_getdata_driver_exit(void)
{
    printk(KERN_EMERG "platform_driver_exit!\n");
    platform_driver_unregister(&pwm2_con_dr);
}

module_init(tem_getdata_driver_init);
module_exit(tem_getdata_driver_exit);
MODULE_LICENSE("GPL");

