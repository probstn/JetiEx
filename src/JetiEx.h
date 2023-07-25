#include <Arduino.h>

#define INTERRUPT_PIN 14

#define RING_BUFFER_SIZE 256

#define TEMP_START 16
#define TEMP_END 28

#define SYNC_LENGTH 9000
#define SYNC_LENGTH_TOL 500

#define SEP_LENGTH 600
#define SEP_LENGTH_TOL 250

#define BIT1_LENGTH 3850
#define BIT1_LENGTH_TOL 500

#define BIT0_LENGTH 1900
#define BIT0_LENGTH_TOL 500

#define DATA_LENGTH 36

double ex_getTemp();

void ex_run(void * pvParameters);

void ex_setup();

void ex_handler();

bool _isSync(unsigned int idx);

void ex_process();