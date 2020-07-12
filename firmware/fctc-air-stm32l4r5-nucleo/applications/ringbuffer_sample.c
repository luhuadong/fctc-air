#include <rtthread.h>
#include <rtdevice.h>
#include <string.h>

#define RING_BUFFER_LEN        8

static struct rt_ringbuffer *rb;
static char  *str = "Hello, World";

static void consumer_thread_entry(void *arg)
{
    char ch;

    while (1)
    {
        if (1 == rt_ringbuffer_getchar(rb, &ch))
        {
            rt_kprintf("[Consumer] <- %c\n", ch);
        }
        rt_thread_mdelay(500);
    }
}

static void ringbuffer_sample(int argc, char** argv)
{
    rt_thread_t tid;
    rt_uint16_t i = 0;

    rb = rt_ringbuffer_create(RING_BUFFER_LEN);
    if (rb == RT_NULL)
    {
        rt_kprintf("Can't create ringbffer");
        return;
    }

    tid = rt_thread_create("consumer", consumer_thread_entry, RT_NULL,
                           1024, RT_THREAD_PRIORITY_MAX/3, 20);
    if (tid == RT_NULL)
    {
        rt_ringbuffer_destroy(rb);
    }
    rt_thread_startup(tid);


    while (str[i] != '\0')
    {
        rt_kprintf("[Producer] -> %c\n", str[i]);
        rt_ringbuffer_putchar(rb, str[i++]);
        rt_thread_mdelay(500);
    }

    rt_thread_delete(tid);
    rt_ringbuffer_destroy(rb);
    
}
#ifdef RT_USING_FINSH
MSH_CMD_EXPORT(ringbuffer_sample, Start a producer and a consumer with a ringbuffer);
#endif