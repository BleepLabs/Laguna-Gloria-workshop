
/*
  todo : comment!
*/

const byte play_pin[2] = {6, 7};
const byte rec_pin[2] = {5, 8};
const byte act_pin[2] = {4, 9};
const byte pot_pin[2] = {3, 10};
const byte out_pin[3] = {0, 1};
#define led_pin 2

#include <Bounce2.h>
Bounce * rec_button_bounce = new Bounce[2];
Bounce * play_button_bounce = new Bounce[2];
Bounce * act_button_bounce = new Bounce[2];

uint32_t current_time;
uint32_t prev_time[8];
int note_len = 50;
uint32_t sol_time[4];
int rate1 = 200;

#define rec_bank_len 10000
byte rec[2][rec_bank_len];
int bank_index_length[2] = {rec_bank_len, rec_bank_len};
int bank_index[2];
int pot[2], prev_pot[2];
byte playing[2];
byte recording[2];
int out[2];
int prev_out[2];
int change[2];
float indicate = 0;
int mappot1;
byte servo_mode[2] = {0, 0};
byte direct_act[2];
int rpot;

#include <Servo.h>
Servo myservo;  // create servo object to control a servo

#include <Adafruit_NeoPixel.h>
Adafruit_NeoPixel leds(2, led_pin, NEO_GRB + NEO_KHZ800);

//1.0 is VERY bright if you're powering it off of 5V
// this needs to be declared and set to something >0 for the LEDs to work
const float max_brightness = 0.05;
//led biz end

void setup() {
  leds.begin();
  for (byte j = 0; j < 2; j++) {
    pinMode(out_pin[j], OUTPUT);
    rec_button_bounce[j].attach( rec_pin[j] , INPUT_PULLUP  );
    rec_button_bounce[j].interval(10);
    play_button_bounce[j].attach( play_pin[j] , INPUT_PULLUP  );
    play_button_bounce[j].interval(10);
    act_button_bounce[j].attach( act_pin[j] , INPUT_PULLUP  );
    act_button_bounce[j].interval(10);
  }


  // pinMode(13, OUTPUT);
  // digitalWrite(13, 0);
  //myservo.attach(out_pin2);  // attaches the servo on pin 9 to the servo object

}

void loop() {
  current_time = millis();


  if (current_time - prev_time[0] > 15) {
    prev_time[0] = current_time;


    prev_pot[0] = pot[0];
    pot[0] = analogRead(pot_pin[0]);

    prev_pot[1] = pot[1];
    pot[1] = 1023 - analogRead(pot_pin[1]);
    mappot1 = map(pot[1], 0, 1023, 0, 180);


    for (byte j = 0; j < 2; j++) {
      rec_button_bounce[j].update();
      play_button_bounce[j].update();
      act_button_bounce[j].update();

      if (play_button_bounce[j].fell()) {
        playing[j] = !playing[j];
        recording[j] = 0;
        bank_index[j] = 0;
        if (playing[j] == 0) {
          digitalWrite(out_pin[j], 0);
          myservo.detach();
          servo_mode[1] = 0;
        }
      }

      if (rec_button_bounce[j].fell()) {
        bank_index[j] = 0;
        bank_index_length[j] = rec_bank_len;
        if (j == 1) {
          if (play_button_bounce[j].read() == 1) {
            myservo.detach();
            servo_mode[1] = 0;
          }
          else {
            myservo.attach(out_pin[1]);
            servo_mode[1] = 1;
          }
        }
      }
      if (act_button_bounce[j].fell()) {
        if (recording[j] == 0) {
          direct_act[j] = 1;
        }
      }
      if (act_button_bounce[j].rose()) {
        if (recording[j] == 0) {
          direct_act[j] = 0;
        }
      }

      if (rec_button_bounce[j].read() == 0) {
        recording[j] = 1;
        playing[j] = 1;
      }
      if (rec_button_bounce[j].rose()) {
        playing[j] = 1;
        recording[j] = 0;
        bank_index_length[j] = bank_index[j];
      }

    }

  }

  if (current_time - prev_time[1] > 15) {
    prev_time[1] = current_time;

    for (byte j = 0; j < 2; j++) {
      if (playing[j] == 1) {
        bank_index[j]++;
        if (bank_index[j] > bank_index_length[j]) {
          bank_index[j] = 0;
          if (recording[j] == 1) {
            recording[j] = 0;
          }
        }
        if (recording[j] == 1) {
          if (servo_mode[j] == 0) {
            rec[j][bank_index[j]] = !act_button_bounce[j].read();
          }
          if (servo_mode[j] == 1) {
            if (!act_button_bounce[j].read() == 1) {
              mappot1 = 180;
            }
            rec[j][bank_index[j]] = mappot1;

          }
        }

        prev_out[j] = out[j];
        out[j] = rec[j][bank_index[j]];
        change[j] = 0;
        if (prev_out[j] != out[j]) {
          change[j] = 1;
        }

        if (out[j] == 1 && change[j] == 1) {
          int r2 = random(255);
          rpot = (pot[j] >> 2) - 15;
          if (r2 > rpot) {
            digitalWrite(out_pin[j], 1);
            indicate = 1;
          }
          else {
            digitalWrite(out_pin[j], 0);
            indicate = 0;
          }
        }
        if (out[j] == 0) {
          digitalWrite(out_pin[j], 0);
          indicate = 0;
        }


        if (out[j] != 0 && recording[j] == 0) {
          if (indicate == 0) {
            leds.setPixelColor(j, 0, 1, 0);
          }
          else {
            leds.setPixelColor(j, 10, 10, 10);
          }
        }

        else if (out[j] == 0 && recording[j] == 1) {
          set_LED(j, 0, 1, .25);
        }
        else  if (out[j] != 0 && recording[j] == 1) {
          set_LED(j, 0, 0, .9);
        }
        else {
          set_LED(j, .3, 1, .25);
        }

      }
      if (playing[j] == 0) {
        set_LED(j, .7, 1, .1);
        if (direct_act[j] == 1) {
          set_LED(j, 0, 0, .9);
        }
        digitalWrite(out_pin[j], direct_act[j]);
      }

      if (recording[j] == 1) {
        if (servo_mode[j] == 1) {
          myservo.write(out[j]);
          set_LED(j, .15, 1, .2 + (out[j] / 240.0));
        }
      }
      if (playing[j] == 1 && recording[j] == 0) {
        if (servo_mode[j] == 1) {
          set_LED(j, .75, .5 + (out[j] / 180.0), .2);
          myservo.write(out[j]);
        }
      }

      leds.show();



      //int p1 = analogRead(ch1_pot_pin);
      //analogWrite(out_pin2,p1>>2);
      //pwm(out_pin2, 50, p1 >> 2);
      //myservo.write(rec[1][byte_index]);
      if (0) {
        //Serial.print(out[0]);
        //        Serial.print(" " );
        //        Serial.print(bank_index[0]);
        //        Serial.println();
        Serial.println(bank_index[0]);
        Serial.println();
      }
    }
  }





}//end of loop




//This function is a little different than you might see in other libraries but it works pretty similar
// instead of 0-255 you see in other libraries this is all 0-1.0
// you can copy this to the bottom of any code as long as the declarations at the top in "led biz" are done

//set_pixel_HSV(led to change, hue,saturation,value aka brightness)
// led to change is 0-63
// all other are 0.0 to 1.0
// hue - 0 is red, then through the ROYGBIV to 1.0 as red again
// saturation - 0 is fully white, 1 is fully colored.
// value - 0 is off, 1 is the value set by max_brightness
// (it's not called brightness since, unlike in photoshop, we're going from black to fully lit up

//based on https://stackoverflow.com/questions/3018313/algorithm-to-convert-rgb-to-hsv-and-hsv-to-rgb-in-range-0-255-for-both

void set_LED(int pixel, float fh, float fs, float fv) {
  byte RedLight;
  byte GreenLight;
  byte BlueLight;

  byte h = fh * 255;
  byte s = fs * 255;
  byte v = fv * max_brightness * 255;

  h = (h * 192) / 256;  // 0..191
  unsigned int i = h / 32;   // We want a value of 0 thru 5
  unsigned int f = (h % 32) * 8;   // 'fractional' part of 'i' 0..248 in jumps

  unsigned int sInv = 255 - s;  // 0 -> 0xff, 0xff -> 0
  unsigned int fInv = 255 - f;  // 0 -> 0xff, 0xff -> 0
  byte pv = v * sInv / 256;  // pv will be in range 0 - 255
  byte qv = v * (256 - s * f / 256) / 256;
  byte tv = v * (256 - s * fInv / 256) / 256;

  switch (i) {
    case 0:
      RedLight = v;
      GreenLight = tv;
      BlueLight = pv;
      break;
    case 1:
      RedLight = qv;
      GreenLight = v;
      BlueLight = pv;
      break;
    case 2:
      RedLight = pv;
      GreenLight = v;
      BlueLight = tv;
      break;
    case 3:
      RedLight = pv;
      GreenLight = qv;
      BlueLight = v;
      break;
    case 4:
      RedLight = tv;
      GreenLight = pv;
      BlueLight = v;
      break;
    case 5:
      RedLight = v;
      GreenLight = pv;
      BlueLight = qv;
      break;
  }
  leds.setPixelColor(pixel, RedLight, GreenLight, BlueLight);
}
