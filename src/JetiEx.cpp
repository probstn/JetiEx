#include "JetiEx.h"

using namespace std;

#define BAUD 9600

#define INTERRUPT_PIN 17

#define RING_BUFFER_SIZE 256

unsigned long timings[RING_BUFFER_SIZE];
unsigned int syncIndex1 = 0; // index of the first sync signal
unsigned int syncIndex2 = 0; // index of the second sync signal
bool received = false;

void ex_setup()
{
    pinMode(INTERRUPT_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), ex_handler, CHANGE);
    Serial.println("[EX] Setup");
}

bool _isSync(unsigned int timing)
{
    if (timing > SYNC_LENGTH)
        return true;
    return false;
}

void ex_handler()
{
    static unsigned int duration = 0;
    static unsigned long start = 0;
    static unsigned long stop = 0;
    static unsigned int ringIndex = 0;
    static unsigned int syncCount = 0;
    // ignore if we haven't processed the previous received signal
    if (received == true)
    {
        return;
    }
    // calculating timing since last change
    (start < stop) ? start = micros() : stop = micros(); // set start / stop for next measure
    (start < stop) ? duration = (stop - start) / 100 : duration = (start - stop) / 100;

    // store data in ring buffer
    ringIndex = (ringIndex + 1) % RING_BUFFER_SIZE;
    timings[ringIndex] = duration;
    // detect sync signal
    if (_isSync(timings[ringIndex]))
    {
        syncCount++;
        // first time sync is seen, record buffer index
        if (syncCount == 1)
        {
            syncIndex1 = (ringIndex + 1) % RING_BUFFER_SIZE;
        }
        else if (syncCount == 2)
        {
            // second time sync is seen, start bit conversion
            syncCount = 0;
            syncIndex2 = (ringIndex) % RING_BUFFER_SIZE;
            unsigned int changeCount = (syncIndex2 < syncIndex1) ? (syncIndex2 + RING_BUFFER_SIZE - syncIndex1) : (syncIndex2 - syncIndex1);
            // changeCount must be 66 -- 32 bits x 2 + 2 for sync
            received = true;
        }
    }
}

void ex_process()
{
    if (!received)
        return;

    uint8_t data = 0;
    int dataIndex = 0;

    int process_state = 0;
    bool isHigh = true;
    Serial.println("[EX] Received");
    // disable interrupt to avoid new data corrupting the buffer
    detachInterrupt(INTERRUPT_PIN);

    // loop over buffer data
    Serial.println("[RX] ");
    for (unsigned int i = syncIndex1, y = 0; i != syncIndex2; i = (i + 1) % RING_BUFFER_SIZE, y++)
    {
        isHigh = !isHigh;
        for (int bit = 0; bit < timings[i]; bit++)
        {
            if (dataIndex > 7)
            {
                Serial.println(data, HEX);
                dataIndex = 0;
                process_state++;
            }

            switch (process_state)
            {
            case 0:
                /*Serial.print("Start: ");
                Serial.println(isHigh);*/
                // 1 start bit
                process_state++;
                break;

            case 1:
                // 8 data-bits
                /*Serial.print("Data ");
                Serial.print(dataIndex);
                Serial.print(": ");
                Serial.println(isHigh);*/
                isHigh ? data |= (1 << dataIndex) : data &= ~(1 << dataIndex);
                dataIndex++;
                break;

            case 2:
                // 1 mode-bit
                /*Serial.print("Mode: ");
                Serial.println(isHigh);*/
                process_state++;
                break;

            case 3:
                // 1 odd-parity-bit
                /*Serial.print("Parity: ");
                Serial.println(isHigh);*/
                process_state++;
                break;

            case 4:
                // 1 stop-bit
                /*Serial.print("Stop: ");
                Serial.println(isHigh);*/
                process_state = 0;
                break;
            }
        }
    }

    // ready for next data
    received = false;
    syncIndex1 = 0;
    syncIndex2 = 0;
    attachInterrupt(INTERRUPT_PIN, ex_handler, CHANGE);
}