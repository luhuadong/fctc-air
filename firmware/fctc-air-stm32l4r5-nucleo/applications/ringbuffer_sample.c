#include <rtthread.h>

#define RING_BUFFER_LEN     64

struct rt_ringbuffer *rb;

static void producer_thread_entry(void *arg)
{

}

static void consumer_thread_entry(void *arg)
{

}

static void ringbuffer_sample(int argc, char** argv)
{
    rt_thread_t tid;
    rt_uint16_t count = 0;

    rb = rt_ringbuffer_create(RING_BUFFER_LEN);
    if (rb == RT_NULL)
    {
        rt_kprintf("Can't create ringbffer");
        return;
    }

    tid = rt_thread_create("consumer", producer_thread_entry, RT_NULL,
                           1024, RT_THREAD_PRIORITY_MAX/3, 20);
    if (tid == RT_NULL)
    {
        rt_ringbuffer_destroy(rb);
    }
    rt_thread_startup(tid);


    while (count--)
    {
        
    }

    
}
#ifdef RT_USING_FINSH
MSH_CMD_EXPORT_ALIAS(ringbuffer_sample, Start a producer and a consumer with a ringbuffer);
#endif