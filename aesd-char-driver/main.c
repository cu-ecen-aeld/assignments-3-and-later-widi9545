/**
 * @file aesdchar.c
 * @brief Functions and data related to the AESD char driver implementation
 *
 * Based on the implementation of the "scull" device driver, found in
 * Linux Device Drivers example code.
 *
 * @author Dan Walkes
 * @date 2019-10-22
 * @copyright Copyright (c) 2019
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/fs.h> // file_operations
#include "aesdchar.h"
#include "aesd-circular-buffer.h"
int aesd_major =   0; // use dynamic major
int aesd_minor =   0;

MODULE_AUTHOR("William Diment"); /** TODO: fill in your name **/
MODULE_LICENSE("Dual BSD/GPL");

static struct aesd_dev aesd_device;

int aesd_open(struct inode *inode, struct file *filp)
{
    /**
     * TODO: handle open
     */
    PDEBUG("open");

    struct aesd_dev *dev; 
    dev = container_of(inode->i_cdev, struct aesd_dev, cdev);
    filp->private_data=dev; 


    return 0;
}

int aesd_release(struct inode *inode, struct file *filp)
{
    PDEBUG("release");
    /**
     * TODO: handle release
     */
    return 0;
}
ssize_t aesd_read(struct file *filp, char __user *buf, size_t count,
                loff_t *f_pos)
{
    ssize_t retval;

    size_t offset;
    size_t read;
    struct aesd_buffer_entry *entry;
    struct aesd_dev *dev = filp -> private_data;
    PDEBUG("read %zu bytes with offset %lld",count,*f_pos);




    mutex_lock(&dev->lock);
    entry = aesd_circular_buffer_find_entry_offset_for_fpos(&aesd_device.buffer, *f_pos, &offset);



    if(!entry){
        mutex_unlock(&dev->lock);
        return 0;
    }

    read = entry->size - offset;
    if(read > count){
        PDEBUG("This is the count and offest: %zu and ", count, *f_pos);
        read = count;
    }


    if (copy_to_user(buf, entry->buffptr + offset, read))
    {
        PDEBUG("Copying to user: illegal");
        mutex_unlock(&dev->lock);
        return 1;
    }

    *f_pos += read;
    retval = read;
    mutex_unlock(&dev->lock);
    return retval;
}



ssize_t aesd_write(struct file *filp, const char __user *buf, size_t count,
                loff_t *f_pos)
{  
    //https://man7.org/linux/man-pages/man3/memchr.3.html
    ssize_t retval = -ENOMEM;
    PDEBUG("write %zu bytes with offset %lld",count,*f_pos);
    char * kernel_buffer;
    char * new_buffer;

    
    size_t new_size;
    struct aesd_buffer_entry new_entry;
    struct aesd_dev *dev = filp -> private_data;

    kernel_buffer = kmalloc(count, GFP_KERNEL);
    if(!kernel_buffer){
        PDEBUG("No kernel buffer - exit 1");
        return 1;
    }

    //unsigned long __copy_from_user(void * to, const void __user * from, unsigned long n); - const void__user * from = buf 
    if (copy_from_user(kernel_buffer, buf, count)) {
        PDEBUG("Copying from user: illegal");
        kfree(kernel_buffer);
        return 1;
    }


    new_entry.buffptr = kernel_buffer;
    new_entry.size = count;
    mutex_lock(&dev->lock);

    if(&dev->buffer_entry.buffptr){
        PDEBUG("In Buffer");
        new_size = dev->buffer_entry.size + count;
        new_buffer = krealloc(dev->buffer_entry.buffptr, new_size, GFP_KERNEL);

        if(!new_buffer){
            kfree(kernel_buffer);
            mutex_unlock(&dev->lock);
            return retval;
        }
        memcpy(new_buffer + dev->buffer_entry.size, kernel_buffer, count);
        kfree(kernel_buffer);
        dev->buffer_entry.buffptr = new_buffer;
        dev->buffer_entry.size = new_size;

    }
    else{

        dev->buffer_entry.buffptr = kernel_buffer;
        dev->buffer_entry.size = count;
    }

    if(dev->buffer_entry.buffptr[dev->buffer_entry.size - 1] == '\n'){
        PDEBUG("Ending Write");
        new_entry.buffptr = dev->buffer_entry.buffptr; 
        new_entry.size = dev->buffer_entry.size;

        aesd_circular_buffer_add_entry(&dev->buffer, &new_entry);

        dev->buffer_entry.buffptr = NULL;
        dev->buffer_entry.size = 0;
    }
    mutex_unlock(&dev->lock);
    retval = count;
    return retval;
}





struct file_operations aesd_fops = {
    .owner =    THIS_MODULE,
    .read =     aesd_read,
    .write =    aesd_write,
    .open =     aesd_open,
    .release =  aesd_release,
};

static int aesd_setup_cdev(struct aesd_dev *dev)
{
    int err, devno = MKDEV(aesd_major, aesd_minor);

    cdev_init(&dev->cdev, &aesd_fops);
    dev->cdev.owner = THIS_MODULE;
    dev->cdev.ops = &aesd_fops;
    err = cdev_add (&dev->cdev, devno, 1);
    if (err) {
        printk(KERN_ERR "Error %d adding aesd cdev", err);
    }
    return err;
}



int aesd_init_module(void)
{
    dev_t dev = 0;
    int result;
    result = alloc_chrdev_region(&dev, aesd_minor, 1,
            "aesdchar");
    aesd_major = MAJOR(dev);
    if (result < 0) {
        printk(KERN_WARNING "Can't get major %d\n", aesd_major);
        return result;
    }
    memset(&aesd_device,0,sizeof(struct aesd_dev));

    /**
     * TODO: initialize the AESD specific portion of the device
     */
    mutex_init(&aesd_device.lock);
    aesd_circular_buffer_init(&aesd_device.buffer);
 

    result = aesd_setup_cdev(&aesd_device);

    if( result ) {
        unregister_chrdev_region(dev, 1);
    }
    return result;

}

void aesd_cleanup_module(void)
{
    dev_t devno = MKDEV(aesd_major, aesd_minor);
    int index;
    struct aesd_circular_buffer *buffer = &aesd_device.buffer;
    struct aesd_buffer_entry *entry; 

    cdev_del(&aesd_device.cdev);

    /**
     * TODO: cleanup AESD specific poritions here as necessary
     */
    AESD_CIRCULAR_BUFFER_FOREACH(entry, buffer, index){
        if((entry -> size > 0) && (entry->buffptr != NULL)){
            kfree(entry->buffptr);
            entry->size = 0;
        }
    }

    unregister_chrdev_region(devno, 1);
}



module_init(aesd_init_module);
module_exit(aesd_cleanup_module);
