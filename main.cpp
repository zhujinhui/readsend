#include "readsend.h"

#define BUFFERS_MAX         3
#define BUFFER_SIZE         128*1024

#define WAIT_READ_BUF       10      // 10ms
#define SEND_COST_TM        20      // 20ms

int main()
{
    LoopBuffers buffer(BUFFER_SIZE, BUFFERS_MAX);
    TestReadThread thread("TestRead", buffer);

    thread.StartThread();

    for (int i=0; i<30; i++)
    {
        // send
        BufferObj obj;
        bool have = buffer.checkout(obj, false);

        if (have == false)
        {
            // should not be here, since READ is far more quick than SEND
            while (true)
            {
                have = buffer.wait(WAIT_READ_BUF);
                if (have == false) continue;
                else break;
            }
            buffer.checkout(obj, false);
        }
        MPSleep(SEND_COST_TM); // sending
        buffer.checkin(obj, false);
    }

    thread.StopThread();
}

