#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <unistd.h>
#include <wiringPi.h>

// 5v:         red
// gnd:        brown
// LED_PIN:    yellow
// RADIO_PIN:  orange

// Signals read:
//
//       "1-on          : 0x65a5a969659a9655\n",
//       "1-off         : 0x65a5a969659a9555\n",
//       "2-on          : 0x65a5a969659a9656\n",
//       "2-off         : 0x65a5a969659a9556\n",
//       "3-on          : 0x65a5a969659a9659\n",
//       "3-off         : 0x65a5a969659a9559\n",
//       "0-all_off     : 0x65a5a969659a9955\n"
//
// It follows that all signals should include:
// on1 & off1 & on2 & off2 & on3 & off3 == 0x65a5a969659a9450
//
// unit codes:
//
//    In [56]: hex(base ^ (on1 & off1))
//    Out[56]: '0x5'
//
//    In [57]: hex(base ^ (on2 & off2))
//    Out[57]: '0x6'
//
//    In [58]: hex(base ^ (on3 & off3))
//    Out[58]: '0x9'
//
// signal codes:
//
//    In [59]: hex(base ^ (on1 & on2 & on3))
//    Out[59]: '0x200'
//
//    In [60]: hex(base ^ (off1 & off2 & off3))
//    Out[60]: '0x100'
//

#define LED_PIN 0
#define RADIO_PIN 7

#define PULSE_WIDTH 250
#define PULSE_WIDTH_WIDE 1250

#define LOCK_FILE "/dev/shm/radio.lock"

static inline void writePulse(int pin, unsigned int length)
{
    digitalWrite(pin, LOW);
    delayMicroseconds(length);
    digitalWrite(pin, HIGH);
    delayMicroseconds(PULSE_WIDTH);
}

static inline void writeMarker(int pin)
{
    writePulse(pin, PULSE_WIDTH * 10);
}

static inline void writePause(int pin)
{
    writePulse(pin, PULSE_WIDTH * 40);
}

/* mutex to allow only one process at a time, I'm pretty sure I bricked one
 * unit by sending two codes at once! */
int aquire_lock()
{
    int i = 0, lock = -1, f = -1;
    while (i < 20 && lock != 0) {
        f = open(LOCK_FILE, O_CREAT|O_RDWR, 0600);
        if (f > 0)
            lock = flock(f, LOCK_EX|LOCK_NB);
        if (lock != 0)
            sleep(1);
        i++;
    }
    return lock;
}

void sendNumber(int pin, uint64_t num, int reps)
{
    writePulse(pin, PULSE_WIDTH);
    for (int i = 0; i < reps; i++)
    {
        writePause(pin);
        writeMarker(pin);
        for (int j = 63; j >= 0; j--)
        {
            if ((num>>j) & 0x1) {
                writePulse(pin, PULSE_WIDTH_WIDE);
            } else {
                writePulse(pin, PULSE_WIDTH);
            }
        }
    }
    writePause(pin);
    digitalWrite(pin, LOW);
}

void clean()
{
  digitalWrite(LED_PIN, LOW);
  digitalWrite(RADIO_PIN, LOW);
}

int usage(char* execName, int fail)
{
      printf("Usage: %s [protocol] [signal]\n\n", execName);
      printf("Only supported protocol is \"nexa\"\n");
      clean();
      return fail;
}

int main(int argc, char* argv[])
{
  int lock = aquire_lock();
  if (lock) {
      printf("Failed to aquire lock\n");
      return 1;
  }
  wiringPiSetup () ;
  pinMode (LED_PIN, OUTPUT) ;
  pinMode (RADIO_PIN, OUTPUT) ;

  if (argc < 3) {
      return usage(argv[0], 1);
  }

  if (strcmp(argv[1], "nexa")) {
      return usage(argv[0], 1);
  }

  uint64_t code = strtoull(argv[2], NULL, 16);
  printf("Sending 0x%lx\n", code);
  digitalWrite(LED_PIN, HIGH);
  sendNumber(RADIO_PIN, code, 5);
  clean();
  return 0 ;
}
