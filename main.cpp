#include "readsend.h"

int main()
{
    LoopBuffers buffer(128*1024, 3);
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
                have = buffer.wait(10);
                if (have == false) continue;
                else break;
            }
            buffer.checkout(obj, false);
        }
        MPSleep(20); // sending
        buffer.checkin(obj, false);
    }

    thread.StopThread();
}

