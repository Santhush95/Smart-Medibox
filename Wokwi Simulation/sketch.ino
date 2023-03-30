//Libraries
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHTesp.h>
#include <WiFi.h>
#include <time.h>

//Definitions
#define NTP_SERVER     "pool.ntp.org"
#define UTC_OFFSET     0
#define UTC_OFFSET_DST 0

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C

#define BUZZER 18
#define LED_1 15
#define LED_2 2
#define DHT 12
#define CANCEL 34
#define UP 35
#define DOWN 32
#define OK 33

//Object declarations
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
DHTesp dhtSensor;

//Variables
int days = 0;
int hours = 0;
int minutes = 0;
int seconds = 0;

unsigned long time_now = 0;
unsigned long time_last = 0;

bool alarm_enabled = true;
int no_alarms = 2;
int alarm_hours[] = {0, 0};
int alarm_minutes[] = {1, 10};
bool alarm_triggered[] = {false, false};

//Buzzer Notes
int no_notes = 8;
int C = 268;
int D = 294;
int E = 330;
int F = 349;
int G = 392;
int A = 440;
int B = 494;
int C_H = 523;
int notes[] = {C, D, E, F, G, A, B, C_H};

//modes
int current_mode = 0;
int max_modes = 4;
String options[] = {"1 - Set Time", "2 - Set Alarm 1", "3 - Set Alarm 2", "4 - Disable Alarms"};

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  pinMode(BUZZER, OUTPUT);
  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);
  pinMode(CANCEL, INPUT);
  pinMode(UP, INPUT);
  pinMode(DOWN, INPUT);
  pinMode(OK, INPUT);

  dhtSensor.setup(DHT, DHTesp::DHT22);

  // initialize the OLED object
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  //Initialize WiFi
  WiFi.begin("Wokwi-GUEST", "", 6);
  while (WiFi.status() != WL_CONNECTED) {
    display_text("Connecting to WiFI", 2, 0, 0, 1);
    delay(250);
  }

  display_text("Connected to WiFI", 2, 0, 0, 1);
  delay(2000);

  configTime(UTC_OFFSET, UTC_OFFSET_DST, NTP_SERVER);
 
  display_text("SMART MEDIBOX", 2, 0, 0, 1);
  display_text("V1.0", 2, 40, 0, 0);
  delay(2000);
}

void loop() {
  update_time_with_check_alarm();
  if(digitalRead(CANCEL) == LOW){
    delay(200);
    go_to_menu();
  }

  check_temp();
}


void display_text(String text, int text_size, int row, int column, bool isClear) {
  if (isClear) {
    display.clearDisplay();
  }
  display.setTextSize(text_size);
  display.setTextColor(WHITE);
  display.setCursor(column,row);
  display.println(text);
  display.display();
}

void display_time(void){
  String time = String(days) + ":" + String(hours) +":" + String(minutes) +":" + String(seconds);
  display_text(time, 2, 0, 0, 1);
}

void update_time(void){
  struct tm timeinfo;
  getLocalTime(&timeinfo);

  char day_str[8];
  char hour_str[8];
  char minute_str[8];
  char second_str[8];
  strftime(day_str, 8, "%d", &timeinfo);
  strftime(hour_str, 8, "%H", &timeinfo);
  strftime(minute_str, 8, "%M", &timeinfo);
  strftime(second_str, 8, "%S", &timeinfo);
  days = atoi(day_str);
  hours = atoi(hour_str);
  minutes = atoi(minute_str);
  seconds = atoi(second_str);

  // time_now = millis() / 1000;  //No of seconds since boot up
  // seconds = time_now - time_last;  //No of seconds passed after the last update_time function call
  // if(seconds >= 60){
  //   time_last += 60;
  //   minutes += 1;
  // }
  // if(minutes >= 60){
  //   minutes = 0;
  //   hours += 1;
  // }
  // if(hours >= 24){
  //   hours = 0;
  //   days += 1;
  // }
}

void ring_alarm(void){
  display_text("Medicine Time", 2, 0, 0, 1);

  //Light Up LED
  digitalWrite(LED_1, HIGH);

  //Ring Buzzer
  while(digitalRead(CANCEL) == HIGH){
    for(int i = 0; i < no_notes; i++) {
      if(digitalRead(CANCEL) == LOW){
        delay(200);
        break;
      }
      tone(BUZZER, notes[i]);
      delay(500);
      noTone(BUZZER);
      delay(2);
    }
  }
  delay(200);

  //Light Down LED
  digitalWrite(LED_1, LOW);
}

void update_time_with_check_alarm(void){
  update_time();
  display_time();

  //check for alarms
  if(alarm_enabled == true) {
    for(int i= 0; i <  no_alarms; i++) {
      if (alarm_triggered[i] == false && alarm_hours[i] == hours && alarm_minutes[i] == minutes){
        ring_alarm();
        alarm_triggered[i] = true;
      }
    }
  }
}

int wait_for_button_press(void){
  while(true){
    if(digitalRead(UP) == LOW){
      delay(200);
      return UP;
    }
    else if(digitalRead(DOWN) == LOW){
      delay(200);
      return DOWN;
    }
    else if(digitalRead(OK) == LOW){
      delay(200);
      return OK;
    }
    else if(digitalRead(CANCEL) == LOW){
      delay(200);
      return CANCEL;
    }
    update_time();
  }
}

void go_to_menu(void){
  while(digitalRead(CANCEL) == HIGH){
    display_text(options[current_mode], 2, 0, 0, 1);

    int pressed = wait_for_button_press();
    if(pressed == UP){
      current_mode += 1;
      current_mode = current_mode % max_modes;
      delay(200);
    }
    if(pressed == DOWN){
      current_mode -= 1;
      if(current_mode < 0){
        current_mode = max_modes - 1;
      }
      delay(200);
    }
    if(pressed == OK){
      delay(200);
      run_mode(current_mode);     
    }

    if(pressed == CANCEL){
      break;    
    }

  }
}

void run_mode(int mode){
  int temp_hours = 0;
  int temp_minutes = 0;
  if(mode == 0){  

    while(digitalRead(CANCEL) == HIGH){
      //Set hour
      display_text("Enter Hours : " + String(temp_hours), 2, 0, 0, 1);

      int pressed = wait_for_button_press();

      if (pressed == UP){
        temp_hours += 1;
        temp_hours = temp_hours % 24;
        delay(200);
      }

      else if (pressed == DOWN){
        temp_hours -= 1;
        if(temp_hours < 0){
          temp_hours = 23;
        }
        delay(200);
      }

      else if (pressed == OK){
        hours = temp_hours;
        delay(200);
        break;
      }
    }
    while(digitalRead(CANCEL) == HIGH){
      //Set minute
      display_text("Enter Minutes : " + String(temp_minutes), 2, 0, 0, 1);

      int pressed = wait_for_button_press();

      if (pressed == UP){
        temp_minutes += 1;
        temp_minutes = temp_minutes % 60;
        delay(200);
      }

      else if (pressed == DOWN){
        temp_minutes -= 1;
        if(temp_minutes < 0){
          temp_minutes = 59;
        }
        delay(200);
      }

      else if (pressed == OK){
        minutes = temp_minutes;
        delay(200);
        break;
      }

    }

    display_text("Time Has Been Set", 2, 0, 0, 1);
    delay(2000);
    
  }

  else if(mode == 1 || mode == 2){  

    while(digitalRead(CANCEL) == HIGH){
      //Set hour
      display_text("Enter Hour : " + String(temp_hours), 2, 0, 0, 1);

      int pressed = wait_for_button_press();

      if (pressed == UP){
        temp_hours += 1;
        temp_hours = temp_hours % 24;
        delay(200);
      }

      else if (pressed == DOWN){
        temp_hours -= 1;
        if(temp_hours < 0){
          temp_hours = 23;
        }
        delay(200);
      }

      else if (pressed == OK){
        alarm_hours[mode-1] = temp_hours;
        delay(200);
        break;
      }
    }
    while(digitalRead(CANCEL) == HIGH){
      //Set minute
      display_text("Enter Minute : " + String(temp_minutes), 2, 0, 0, 1);

      int pressed = wait_for_button_press();

      if (pressed == UP){
        temp_minutes += 1;
        temp_minutes = temp_minutes % 60;
        delay(200);
      }

      else if (pressed == DOWN){
        temp_minutes -= 1;
        if(temp_minutes < 0){
          temp_minutes = 59;
        }
        delay(200);
      }

      else if (pressed == OK){
        alarm_minutes[mode-1] = temp_minutes;
        alarm_triggered[mode-1] = false;
        delay(200);
        break;
      }
    }
    display_text("Alarm Has Been Set", 2, 0, 0, 1);
    delay(2000);
  }

  else if(mode == 3) {
    if (alarm_enabled) {
      alarm_enabled = false;
      display_text("Alarms Disabled", 2, 0, 0, 1);
      options[3] = "4 - Enable Alarms";
      delay(2000);
    }
    else {
      alarm_enabled = true;
      display_text("Alarms Enabled", 2, 0, 0, 1);
      options[3] = "4 - Disable Alarms";
      delay(2000);
    }
    
  }
}

void check_temp(void){
  TempAndHumidity data = dhtSensor.getTempAndHumidity();

  if(data.temperature > 35){
    display_text("Temperature High", 1, 40, 0, 0);
    digitalWrite(LED_2, HIGH);
  }
  else if(data.temperature < 25){
    display_text("Temperature Low", 1, 40, 0, 0);
    digitalWrite(LED_2, HIGH);
  }
  else if(data.humidity > 35){
    display_text("Humidity High", 1, 40, 0, 0);
    digitalWrite(LED_2, HIGH);
  }
  else if(data.humidity < 25){
    display_text("Humidity Low", 1, 40, 0, 0);
    digitalWrite(LED_2, HIGH);
  }
  else {
    digitalWrite(LED_2, LOW);
  }
}
