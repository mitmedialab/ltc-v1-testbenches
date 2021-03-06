/*
  IO testing is done by mating two LTC stickers to each other, such that
  A0<->A5, A1<->A4 ... A5<->A0. There is a male half and a female half.
  The male half outputs test stimulus, and the female half analyzes
  the stimulus. Between the two, this testbench can validate the 
  plumbing of most IO functions. 

  Specifically, this test does *not* check for the full analog performance 
  of the ADC, it just confirms that the ADC API calls work. A separate
  "true analog" (vs PWM) voltage source is needed to characterize the
  analog performance of the ADC.

  Setup: 
    Mate two LTC stickers to each other, such that A0<->A5, A1<->A4 ... A5<->A0.
    This is the female half of the IO tester. Power on the male side first.

  Expected result:
    The female half measures the voltages coming from the male half. It measures
    based on tuplets of {0,1,2} and {3,4,5}. While measuring {0,1,2}, it drives
    outputs on {3,4,5}, and vice-versa. It will take turns stimulating outputs
    {3,4,5} as digital out or analog out.

    If things are working correctly, the pattern visible on the stimulated output
    is fade-flash-fade-flash. 

*/

#include "ChibiOS.h"

int blink_chan = 3;
int blink_last = 3;

#define FADE_RATE 1

static THD_WORKING_AREA(blinky_area, 64);
static THD_FUNCTION(do_blinky, arg) {
  (void)arg;
  int depth = 0;
  int state = 0;

  while (1) {
    if( state == 1 ) {
      for( depth = 0; depth < 255; depth += FADE_RATE ) {
	analogWrite(blink_chan, depth);
	delay(1);
      }
      for( depth = 255; depth > 0; depth -= FADE_RATE ) {
	analogWrite(blink_chan, depth);
	delay(1);
      }
      state = 0;
    } else {
      digitalWrite(blink_chan, 0);
      //      threadSleep(50);  // threadSleep seems to be unstable
      delay(200);
      digitalWrite(blink_chan, 1);
      //      threadSleep(50);
      delay(200);
      digitalWrite(blink_chan, 0);
      //      threadSleep(50);
      delay(200);
      state = 1;
    }
  }
}

void setup() {
  int i;

  for( i = 0; i < 6; i++ ) {
    pinMode(i, INPUT);
  }

  createThread(blinky_area, sizeof(blinky_area), 20, do_blinky, NULL);
  pinMode(blink_chan, OUTPUT);
}

void loop() {
  int i;

  if( (digitalRead(0) == HIGH) && (blink_last != 0)) {
    blink_chan = 3;
  } else if( (digitalRead(1) == HIGH) && (blink_last != 1)) {
    blink_chan = 4;
  } else if( (digitalRead(2) == HIGH) && (blink_last != 2)) {
    blink_chan = 5;
  } else if( (digitalRead(3) == HIGH) && (blink_last != 3)) {
    blink_chan = 0;
  } else if( (digitalRead(4) == HIGH) && (blink_last != 4)) {
    blink_chan = 1;
  } else if( (digitalRead(5) == HIGH) && (blink_last != 5)) {
    blink_chan = 2;
  }
  if( blink_chan != blink_last ) {
    pinMode(blink_last, INPUT);
    pinMode(blink_chan, OUTPUT);
    blink_last = blink_chan;
  }
  delay(10);
  yieldThread();
}


