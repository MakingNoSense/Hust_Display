#include "Platform.h"
#include "App_Common.h"
#include "Functions.h"
#include <Wire.h>
#include "buffer.h"

#define INCREAMENT_CRUISE_CONTROL_2 2 // Increament curise control, switch screen
#define HIGH_BEAM_3 3 // High Beam 
#define REVERSE_MODE_4 4 // Mode: reverse, 2
#define ECO_OR_RACING_5 5 // ECO or Racing mode
#define CRUISE_CONTROL_6 6 // Cruise control
#define DRIVE_MODE_7 7 // Mode: Drive, 1
#define HORN_BUTTON_8 8 
#define HIGH_VOLTAGE_9 9 
#define DECREAMENT_CRUISE_CONTROL_17 17 // Decreament cruise control 
#define BLINK_LEFT_20 20
#define NEUTRAL_MODE_21 21 // Mode: Neutral, 2



//Saved
/* In Functions.cpp the functions wirtten written beyond riverdi github can be found*/


/*----- I2C ECU --------*/
/* Transmit on channel 9 and receive on channel 10*/


/* Global used for buffer optimization */
Gpu_Hal_Context_t host, *phost;
void start_screen();
void main_screen();
void values_screen();


static byte rgb_yellow[] = {230, 230, 50};
static byte rbg_red[] = {250, 20, 1};
static byte rgb_white[] = {250, 250, 250};
float max_gas = 1;
float min_gas = 2000;
float max_break = 0;
float min_break = 2000;
float current = 0;
float voltage_diff = 20;
float mc_watt = 20;
//current voltage
float mc_watt_max = 100;
float solar_watt = 20;
float solar_watt_max = 100;
float gas_potential;
float break_potential;
float last_brake_potential = 1/1337;
int economy_value = 0;
/*---------------------- CANBUS ----------------------*/
// Motor Controller
double HeatsinkTemp = 0.0; //
double MotorTemp = 0.0; //

double BusCurrent = 0.0; //
double BusVoltage = 0.0; //

double MotorVelocity = 0.0; //
double VehicleVelocity = 0.0;  //

// BMS (Orion 2)
static double PackVoltageMax = 159.6;
double PackVoltage = 0.0; // 
double PackCurrent = 0.0; // 
double PackAverageCurrent = 0.0; //

//double PackSOH = 0.0; //
//double PackSOC = 0.0; //

double LowCellVoltage = 0.0; //
double HighCellVoltage = 0.0; //
double AvgCellVoltage = 0.0; //

double HighTemperature = 0.0; //
double LowTemperature = 0.0; //
double AvgTemperature = 0.0; //
double InternalTemperature = 0.0; //

// MPPT (SEC-B175-7A TPEE)
double MPPTInputVoltage = 0.0; //
double MPPTInputCurrent = 0.0; //

double MPPTOutputVoltage = 0.0; //
double MPPTOutputCurrent = 0.0; // 
double MMPTOutputPower = 0.0;
static double MPPT_power_max = 2000;

/*-------------------------------------------------*/

CircularBuffer gas_buffer(10);
CircularBuffer break_buffer(10);


int counter_gas = 0;
int counter_break = 0;
int pot_counter = 0;
int driving_mode_counter = 2;
int cruise_control_velocity = 100;


bool intro = true;
bool data_received_flag = false;
bool update_screen_flag = true;
bool start_screen_flag = true;
bool main_screen_flag = false;
bool init_i2c_channel_10 = true;
bool high_beam_flag = false;
bool high_voltage_flag = false;
bool battery_error_flag = false;
bool mc_error_flag = false;
bool solar_error_flag = false;
bool driving_mode_error_flag = false;
bool error_flag = false;
bool cruise_control_flag = false;
/*--- Eco 0 Race 1 ---*/
bool eco_or_race_mode_flag = true;
bool right_blinker_flag = false;
bool left_blinker_flag = false;
bool hazard_lights_flag = false;
bool potentiometer_flag = false;

/*----- Variables for handeling buttons ----*/
unsigned long lastPressed = 0.0;
unsigned long timeATM = 0.0;


String global_data = "";


/* setup */
void setup() {
  phost = &host;
  SPI.begin();
  /* Init HW Hal */
  App_Common_Init(&host);
  Serial.begin(19200);
  init_buttons();


  Wire.begin(8);
  Wire.onRequest(i2c_request);
  Wire.onReceive(I2C_receive);
}


void i2c_request() {
  float gas_to_send = calc_potentials(gas_potential, min_gas, max_gas);
  float break_to_send = calc_potentials(break_potential, min_break, max_break);

  int gas = int(gas_to_send);
  Serial.println("Gas: " + String(gas));
  int breaks = int(break_to_send);
  Serial.println("Break: " + String(breaks));
  int driving = int(driving_mode_counter);
  Serial.println("Mode: " + String(driving));
  int cruise = int(cruise_control_flag);
  Serial.println("Cruise control: " + String(cruise_control_flag));
  int eco = int(eco_or_race_mode_flag);
  Serial.println("Race mode: " + String(eco_or_race_mode_flag));
  int start = int(start_screen_flag);
  Serial.println("Start mode: " + String(start_screen_flag));
  int cruise_velocity = int(cruise_control_velocity);
  Serial.println("Cruise velocity: " + String(cruise_velocity));
  int right_blinker = int(right_blinker_flag);
  Serial.println("Right blinker: " + String(right_blinker_flag));
  int left_blinker = int(left_blinker_flag);
  Serial.println("Left blinker: " + String(left_blinker_flag));
  int hazard_lights = int(hazard_lights_flag);
  Serial.println("Hazard lights: " + String(hazard_lights_flag));
  int high_beam = int(high_beam_flag);
  Serial.println("High beam: " + String(high_beam_flag));
  int HV = int(high_voltage_flag);
  Serial.println("High Voltage: " + String(high_voltage_flag); 
  //Wire.beginTransmission();
  Wire.write(gas);
  Wire.write(breaks);
  Wire.write(driving);
  Wire.write(cruise);
  Wire.write(eco);
  Wire.write(start);
  Wire.write(cruise_velocity);
  Wire.write(right_blinker);
  Wire.write(left_blinker);
  Wire.write(hazard_lights);
  Wire.write(high_beam);
  Wire.write(HV);
  //Wire.endTransmission();
}


void I2C_receive() 
{
  String data = "";
  String data_segment = "";
  while(Wire.available()) // To read in data backwards (1744 18 0 0 5 188 0 199 114) -> (114 199 0 188 5 0 0 18 1744)
  {
    char c = Wire.read();
    if(c == ' ')
    {
      data += data_segment + ' ';
      data_segment = "";
    }
    else
    {
      data_segment += c;
    }
  }

  String ID = "";
  for(int i = 0; i < data.length(); i++)
  {    
    String c = data.substring(i, i+1);
    if(c != " ")
    {
      ID += c;
    }
    else
    {
      data = data.substring(i+1);
      break;
    }
  }
  Serial.println("--------------");
  Serial.println(ID + " " + data);
  update_data(ID, data);  
}


/* This funciton activates the flag when high_voltage is off and drving mode is not neutral*/
void handle_driving_mode_flag() {
  if(!high_voltage_flag && driving_mode_counter != 2) {
    driving_mode_error_flag = true;
  } else {
    driving_mode_error_flag = false;
  }
}


void init_buttons() { 
  pinMode(INCREAMENT_CRUISE_CONTROL_2, INPUT_PULLUP);
  pinMode(HIGH_BEAM_3, INPUT_PULLUP);
  pinMode(REVERSE_MODE_4, INPUT_PULLUP);
  pinMode(ECO_OR_RACING_5, INPUT_PULLUP);
  pinMode(CRUISE_CONTROL_6, INPUT_PULLUP);
  pinMode(DRIVE_MODE_7, INPUT_PULLUP);
  pinMode(HORN_BUTTON_8, INPUT_PULLUP);
  pinMode(HIGH_VOLTAGE_9, INPUT_PULLUP);
  pinMode(DECREAMENT_CRUISE_CONTROL_17, INPUT_PULLUP);
  pinMode(NEUTRAL_MODE_21, INPUT_PULLUP);
}


/* Updates gas and break potentials */
void update_minmax_potentials(float mean, int identifier) {
  // 1 for gas
  if (identifier == 1) {
    if (mean > max_gas) {
      max_gas = mean;
      update_screen_flag = true;
    }
    if (mean < min_gas) {
      min_gas = mean;
      update_screen_flag = true;
    }
  }

  // 2 for break
  if (identifier == 2) {
    if (mean > max_break) {
      max_break = mean;
      update_screen_flag = true;
    }
    if (mean < min_break) {
      min_break = mean;
      update_screen_flag = true;
    }
  }
}


/* loop */
void loop() {
  /* Show Bridgetek logo */
  if (intro) {
    //App_Show_Logo(&host);
    intro = false;
    Serial.println("Start");
    //start_screen();
  }
  
   /* ------------- Start screen ---------------*/   
   if(start_screen_flag) {
     start_screen();
     gas_buffer.add(analogRead(A1));
     break_buffer.add(analogRead(A2));
     pot_counter++;


     byte switch_screen_button = digitalRead(INCREAMENT_CRUISE_CONTROL_2);
     if(switch_screen_button == LOW) {
       if(potentiometer_flag) {
         main_screen_flag = true;
         start_screen_flag = false;
       }       
       Serial.println("Screen changing");
     }
   }

   gas_potential = analogRead(A1);
   break_potential = analogRead(A2);

   if(pot_counter >= 10 && start_screen_flag) {
     float mean_gas = gas_buffer.get_mean();
     float mean_break = break_buffer.get_mean();
     update_minmax_potentials(mean_gas, 1);
     update_minmax_potentials(mean_break, 2);
   }

   
  /* calc new mean for gas */
  if(counter_gas >= 5 && start_screen_flag) {
    float mean = gas_buffer.get_mean();
    update_minmax_potentials(mean, 1);
    counter_gas = 0;
  }

  /* calc new mean for break */
  if(counter_break >= 5 && start_screen_flag) {
    float mean = break_buffer.get_mean();
    update_minmax_potentials(mean, 2);
    counter_break = 0;
  }
  

  /* Update screen if flags */
  if(update_screen_flag && start_screen_flag) {
    start_screen();
    update_screen_flag = false;
  }

  /*----- Switch Screen Button ----------*/
  if((max_break - min_break) >= 500 && (max_gas - min_gas) >= 500) {
    potentiometer_flag = true;
  }

  


  /*---- if statement for button time control ------*/
  timeATM = millis();
  if(timeATM - lastPressed > 300) {

    /*------ High voltage and Horn Button --------*/
    /*---- Horn button is redundant? ---------*/
    byte horn_button = digitalRead(HORN_BUTTON_8);
    if(horn_button == LOW) {
      Serial.println("Tuuut");      
    }

    byte high_voltage_button = digitalRead(HIGH_VOLTAGE_9);
    if(high_voltage_button == LOW) {
      if(high_voltage_flag) {
        high_voltage_flag = false;
      } else {
        high_voltage_flag = true;
      }
    } 

    /*----- Driving Mode Button Button ----------*/

    byte drive_mode_button = digitalRead(DRIVE_MODE_7);
    if(drive_mode_button == LOW) {
      driving_mode_counter = 1;
    }

    byte reverse_mode_button = digitalRead(REVERSE_MODE_4);
    if(reverse_mode_button == LOW) {
      driving_mode_counter = 0;
    }

    byte neutral_mode_button = digitalRead(NEUTRAL_MODE_21);
    if(neutral_mode_button == LOW) {
      driving_mode_counter = 2;
    }

    /*----- Cruise Control Button ----------*/
    byte cruise_control_button = digitalRead(CRUISE_CONTROL_6);
    if(cruise_control_button == LOW) {
      if(cruise_control_flag) {
        cruise_control_flag = false;
      } else {
        cruise_control_flag = true;
      }    
      Serial.println(cruise_control_flag);
    }

    byte increment_cruise_control_button = digitalRead(INCREAMENT_CRUISE_CONTROL_2);
    if(increment_cruise_control_button == LOW && cruise_control_flag) {
      cruise_control_velocity++;
      Serial.println(cruise_control_velocity);
    } 

    byte decrement_cruise_control_button = digitalRead(DECREAMENT_CRUISE_CONTROL_17);
    if(decrement_cruise_control_button == LOW && cruise_control_flag) {
      if(cruise_control_velocity <= 0) {
        Serial.println("Cant decrease");
      } else {
        cruise_control_velocity--;
        Serial.println(cruise_control_velocity);
      }  
    }

    
    /*----- Eco mode or racing mode Button ----------*/
    byte eco_or_racing_button = digitalRead(ECO_OR_RACING_5);
    if(eco_or_racing_button == LOW) {
      if(eco_or_race_mode_flag) {
        eco_or_race_mode_flag = false;
      } else {
        eco_or_race_mode_flag = true;
      }
      Serial.println(eco_or_race_mode_flag);
    }

    /*------------------- High Beam Button -------------------*/
    byte high_beam_button = digitalRead(HIGH_BEAM_3);
    if(high_beam_button == LOW) {
      if(high_beam_flag) {
        high_beam_flag = false;
      } else {
        high_beam_flag = true;
      }
      Serial.println(high_beam_flag);
    }

    /*------------------------ BLINKER BUTTONS ------------------------*/
    /*
    byte right_blinker_button = digitalRead(BLINK_RIGHT);
    if(right_blinker_button == LOW) {
      if(right_blinker_flag) {
        right_blinker_flag = false;
      } else {
        right_blinker_flag = true;
      }
      Serial.println("Right blinker");
      delay(300);
    }

    byte left_blinker_button = digitalRead(BLINK_LEFT);
    if(left_blinker_button == LOW) {
      if(left_blinker_flag) {
        left_blinker_flag = true; 
      } else {
        left_blinker_flag = false;
      }
      Serial.println("Left blinker");
      delay(300);
    }
    */
    
    int buttonValue = analogRead(A0);
    //Serial.println(buttonValue);
    if(buttonValue >= 1000) {
      if(hazard_lights_flag) {
        hazard_lights_flag = false;
      } else {
        hazard_lights_flag = true;
      }
      Serial.println("Hazard lights: " + String(hazard_lights_flag));
      //delay(100);
    } else if(buttonValue >= 800 && buttonValue < 1000) {
      if(right_blinker_flag) {
        right_blinker_flag = false;
      } else {
        right_blinker_flag = true;
      }    
      Serial.println("Right blinker: " + String(right_blinker_flag));
      //delay(100);
    } else if(buttonValue > 150 && buttonValue < 800) {
      if(left_blinker_flag) {
        left_blinker_flag = false;
      } else {
        left_blinker_flag = true;
      }
      Serial.println("Left blinker: " + String(left_blinker_flag));
    }

    lastPressed = millis();
  }

  /*---- Crusie control ------*/
  if(!cruise_control_flag)
  {
    cruise_control_velocity = VehicleVelocity;
  } 

  if((break_potential-40 > last_brake_potential) && last_brake_potential != 1/
  1337) {cruise_control_flag = 0;}
  last_brake_potential = break_potential;
  
  

  /* Calculations for economy meter */
  
  solar_watt = MPPTOutputCurrent*MPPTOutputVoltage;
  mc_watt = BusCurrent*BusVoltage;
  economy_value = map(solar_watt-mc_watt, -10000, 10000, 180, 300);
  

  /* ---------------- Handeling for error flags ------------------ */
  /* Check all the temperatures and update flags */
  /* ------------- MC ----------- */
  if(HeatsinkTemp > 50 || MotorTemp > 50) {
    mc_error_flag = true;
  } else {
    mc_error_flag = false;
  }

  /* ------------ BMS ----------- */
  /* Insert logic for these variables
  HighTemperature
  LowTemperature
  AvgTemperature
  InternalTemperature*/
    
  if(HighTemperature >= 60 || InternalTemperature >= 60 || HighTemperature - LowTemperature >= 20) {
    battery_error_flag = true;
  } else {
    battery_error_flag = false;
  }
  /*
  solar_error_flag = true;
  bool driving_mode_error_flag = true;
  bool error_flag = true;
  */
    

  /* ------------ MPPT ----------- */
  /* Insert logic for handeling MPPT error flag*/
  

  /*------- Main error flag ------*/
  /* Insert logic for handeling main error flag*/

  /* ------------- Main screen --------------- */
  if(main_screen_flag) {
    main_screen();
  }  

  /* ------- Handle error flags ------------- */
  if(!high_voltage_flag) {
    handle_driving_mode_flag();
  }


  delay(2);
}

/* Function for start screen*/
void start_screen() {
  char maxgas_char[20];
  char mingas_char[20];
  char maxbreak_char[20];
  char minbreak_char[20];
  char current_char[20];

  sprintf(maxgas_char, "Max:%s", String(max_gas).c_str());
  sprintf(mingas_char, "Min:%s", String(min_gas).c_str());
  sprintf(maxbreak_char, "Max:%s", String(max_break).c_str());
  sprintf(minbreak_char, "Min:%s", String(min_break).c_str());
  sprintf(current_char, "Current:%s", String(current).c_str());

  Start_Set_Display(phost);

  /*---- Set boxes for text ---------*/
  draw_rect(phost, 10, 60, 200, 100, 70, 200, 200);
  draw_rect(phost, 280, 60, 470, 100, 70, 200, 200);
  draw_rect(phost, 10, 130, 200, 170, 70, 200, 200);
  draw_rect(phost, 280, 130, 470, 170, 70, 200, 200);
  //draw_rect(phost, 140, 200, 340, 230, 70, 200, 200);

  /*--------- Input text to screen -------------*/
  Write_Text(phost, 195, 0, 30, "HUST");
  Write_Text(phost, 50, 10, 25, "Gas");
  Write_Text(phost, 360, 10, 25, "Break");
  Write_Text(phost, 10, 65, 25, maxgas_char);
  Write_Text(phost, 10, 135, 25, mingas_char);
  Write_Text(phost, 280, 65, 25, maxbreak_char);
  Write_Text(phost, 280, 135, 25, minbreak_char);
  //Write_Text(phost, 140, 195, 25, current_char);

  if(!potentiometer_flag) {
    Write_Text(phost, 10, 240, 21, "Set potentiometers to change display");
    //Write_Text(phost, )
  }

  Finish_Display(phost);
}

/* Function for main screen*/
void main_screen() {
    
    char velocity_char[25];
    char voltage_diff_char[20];
    char current_char[20];


    sprintf(velocity_char, "%s [mph]", String(VehicleVelocity).c_str());
    sprintf(voltage_diff_char, "Volt diff:%s", String(voltage_diff).c_str());
    sprintf(current_char, "Current:%s", String(current).c_str());

    Start_Set_Display(phost);
    //draw_rect(phost, 120 , 35, 300, 75); 
    //Write_Text(phost, 195, 0, 30, "Hust");
    Write_Text(phost, 210, 80, 31, String(int(VehicleVelocity)).c_str());
    Write_Text(phost, 190, 115, 30, "mph");
    //Write_Text(phost, 10, 80, 22, voltage_diff_char);
    //Write_Text(phost, 10, 100, 22, current_char);
    //insert_line(phost, 440, 470, 10, 262);
    //insert_charging(phost, 0.7);

    /*---- For driving mode icon -----*/
    if(driving_mode_error_flag) {
      driving_mode_icon(phost, rbg_red[0], rbg_red[1], rbg_red[2], driving_mode_counter, driving_mode_error_flag);
    } else {
      driving_mode_icon(phost, rgb_white[0], rgb_white[1], rgb_white[2], driving_mode_counter, driving_mode_error_flag);
    }
    /*----- For battery icon --------*/
    if(battery_error_flag) {
      volt_battery_icon(phost, rbg_red[0], rbg_red[1], rbg_red[2]);
    } else {
      volt_battery_icon(phost, rgb_white[0], rgb_white[1], rgb_white[2]);
    }
  
    /*---- For MC icon -------*/
    if(mc_error_flag) {
      mc_icon(phost, rbg_red[0], rbg_red[1], rbg_red[2]);
    } else {
      mc_icon(phost, rgb_white[0], rgb_white[1], rgb_white[2]);
    }
    
    /*---- For Solar cell icon -----*/
    if(solar_error_flag) {
      solar_cell_icon(phost, rbg_red[0], rbg_red[1], rbg_red[2]);
    } else {
      solar_cell_icon(phost, rgb_white[0], rgb_white[1], rgb_white[2]);
    }
    
    /*------- For main error flag -------*/
    if(error_flag) {
      error_icon(phost, rbg_red[0], rbg_red[1], rbg_red[2]);
    } else {
      error_icon(phost, rgb_white[0], rgb_white[1], rgb_white[2]);
    }
    

    if(cruise_control_flag) {
      cruise_control_icon(phost, cruise_control_velocity);
    }
    /*----------- Economy --------------*/
    economy_icon(phost, economy_value);
    
    /*---------- Solar ----------*/
    meter_icon(phost, 410, 465, 30, 200, solar_watt/solar_watt_max);

    /*---------- MC ----------*/
    meter_icon(phost, 340, 395, 30, 200, mc_watt/mc_watt_max);
  
    /*---------- voltage ----------*/
    meter_icon(phost, 15, 70, 30, 200, PackVoltage/PackVoltageMax);

    /*----- High Beam Symbol ------*/
    if(high_beam_flag) {
      high_beam(phost);   
    }
    /*---- High voltage symbol -----*/
    if(high_voltage_flag)
    {
      high_voltage(phost);
    }
    /*---- ECO or RACE mode --------*/
    eco_or_racing_mode(phost, eco_or_race_mode_flag);

    Finish_Display(phost);
}

/* Function for values in vehicle*/
/* This screen might be redudant but should be double checkt in order to see if the values are correct*/
void values_screen() {
  char mc_temperature_char[20];
  char heat_sink_char[20];
  char High_temperature_bms_char[20];
  char internal_temperature_bms_char[20];
  char avarage_temperature_bms_char[20];
  char cell_diff_min_max_char[20];

  sprintf(mc_temperature_char, "MC:%s", String(MotorTemp).c_str());
  sprintf(heat_sink_char, "Sink:%s", String(HeatsinkTemp).c_str());

  sprintf(High_temperature_bms_char, "High:%s", String(HighTemperature).c_str());
  sprintf(internal_temperature_bms_char, "Internal:%s", String(InternalTemperature).c_str());
  sprintf(avarage_temperature_bms_char, "Avg:%s", String(AvgTemperature).c_str());
  sprintf(cell_diff_min_max_char, "Celldiff:%s", String(HighCellVoltage-LowCellVoltage));

  Start_Set_Display(phost);

  /*---- Set boxes for text ---------*/
  draw_rect(phost, 10, 60, 200, 100, 70, 200, 200);
  draw_rect(phost, 10, 115, 200, 155, 70, 200, 200);
  draw_rect(phost, 10, 170, 200, 210, 70, 200, 200);
  draw_rect(phost, 10, 225, 200, 265, 70, 200, 200);
  draw_rect(phost, 280, 60, 470, 100, 70, 200, 200);
  draw_rect(phost, 280, 115, 470, 155, 70, 200, 200);
  draw_rect(phost, 280, 170, 470, 210, 70, 200, 200);
  draw_rect(phost, 280, 225, 470, 265, 70, 200, 200);

  /*--------- Input text to screen -------------*/
  Write_Text(phost, 195, 0, 30, "Hust");
  Write_Text(phost, 50, 10, 25, "Temperatures");
  Write_Text(phost, 360, 10, 25, "Tempratures");
  Write_Text(phost, 10, 65, 25, mc_temperature_char);
  Write_Text(phost, 10, 115, 25, heat_sink_char);
  Write_Text(phost, 10, 170, 25, High_temperature_bms_char);
  Write_Text(phost, 280, 65, 25, internal_temperature_bms_char);
  Write_Text(phost, 280, 115, 25, avarage_temperature_bms_char);
  Write_Text(phost, 140, 170, 25, cell_diff_min_max_char);

  Finish_Display(phost);
}



/*---------------- CAN BUS -------------------*/
void update_data(String ID, String data)
{
  //Serial.println("Update values");
  if(ID == "1027") //0x403 Velocity Measurement
  {
    //double vehicle_velocity = extractBytesToDecimal(data, 0, 4);
    //double motor_velocity = extractBytesToDecimal(data, 4, 4);
    VehicleVelocity = extractBytesToDecimal(data, 4, 4);

    Serial.println("Vehicle velocity: " + String(VehicleVelocity));
    Serial.println("--------------------------------");
    //Serial.println("Motor velocity: " + String(MotorVelocity));
  }

  if(ID == "1035") //0x40B MC Temperatures
  {
    double heat_sink_temp = extractBytesToDecimal(data, 0, 4);
    double motor_temp = extractBytesToDecimal(data, 4, 4);
    HeatsinkTemp = extractBytesToDecimal(data, 0, 4);
    MotorTemp = extractBytesToDecimal(data, 4, 4);

    Serial.println("Heatsink: " + String(HeatsinkTemp));
    Serial.println("Motor temp: " + String(MotorTemp));
  }

  // ---------------- BMS ------------------------
  if(ID == "1536")
  {
    PackCurrent = 0.1*extractDataMPPT(data, 0, 2);
    PackVoltage = 0.1*extractDataMPPT(data, 2, 2);
    PackAverageCurrent = 0.1*extractDataMPPT(data, 4, 2);
    //PackSOH = extractSingleByte(data, 6);
    //PackSOC = extractSingleByte(data, 7);
  }

  if(ID == "1537")
  {
    LowCellVoltage = 0.0001*extractDataMPPT(data, 0, 2);
    HighCellVoltage = 0.0001*extractDataMPPT(data, 2, 2);
    AvgCellVoltage = 0.0001*extractDataMPPT(data, 4, 2);

    //Serial.println("Avg cell: " + String(AvgCellVoltage));
  }

  if(ID == "1538")
  {
    HighTemperature = extractSingleByte(data, 0);
    LowTemperature = extractSingleByte(data, 1);
    AvgTemperature = extractSingleByte(data, 2);
    InternalTemperature = extractSingleByte(data, 3);
  }
  
  // ----------------- MPPT ----------------------
  if(ID == "512") 
  { // InputVolt, InputCurrent, OutputVolt, OutputCurrent
    MPPTInputVoltage = 0.01*extractDataMPPT(data, 0, 2);
    MPPTInputCurrent = 0.0005*extractDataMPPT(data, 2, 2);
    MPPTOutputVoltage = 0.01*extractDataMPPT(data, 4, 2);
    MPPTOutputCurrent = 0.0005*extractDataMPPT(data, 6, 2);
  }
}