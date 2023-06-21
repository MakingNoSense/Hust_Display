#include <SPI.h>
#include "Platform.h"
#include "App_Common.h"

void Flash_Light(int x, int pin)
{
  pinMode(pin, OUTPUT);

  digitalWrite(pin, HIGH);
  delay(x);
  digitalWrite(pin, LOW);
  delay(x);

  pinMode(pin, INPUT);
}

/* Function for writing a text on the screen */
void Write_Text(Gpu_Hal_Context_t *phost, int x, int y, int size, char text[])
{
  App_WrDl_Buffer(phost, BEGIN(BITMAPS));
  App_WrDl_Buffer(phost, COLOR_RGB(1, 1, 1));
  int increment = 8;

  for(int i = 0; text[i] != '\0'; i++)
  {
    
    App_WrDl_Buffer(phost, VERTEX2II(x, y, size, text[i]));

    if(isUpperCase(text[i])){
      x += size;
    } else if(text[i] == 'r' || text[i] == 'i' || text[i] == 'e') {
      x += size - size/3*1.5;
    } else  if(text[i] == 'm') {
      x += size + 1;
    } else { 
      x += size - increment;
    }
  }
  App_WrDl_Buffer(phost, END());
}

/* Starts a display list */
void Start_Set_Display(Gpu_Hal_Context_t *phost)
{
  App_WrDl_Buffer(phost, CLEAR_COLOR_RGB(255, 255, 255));
  App_WrDl_Buffer(phost, CLEAR(1, 1, 1));
  ///App_WrDl_Buffer(phost, BEGIN(BITMAPS));
}

/* Swaps the disply list */
void Finish_Display(Gpu_Hal_Context_t *phost)
{
    App_WrDl_Buffer(phost, END());
    App_WrDl_Buffer(phost, COLOR_RGB(255, 1, 1));
    App_WrDl_Buffer(phost, DISPLAY());
        
    Gpu_Hal_DLSwap(phost,DLSWAP_FRAME);
    App_Flush_DL_Buffer(phost);    
}


void insert_single_line(Gpu_Hal_Context_t *phost, int x1, int x2, int y1, int y2, int linewidth, int r, int g, int b) {
  App_WrDl_Buffer(phost, BEGIN(LINES));
  App_WrDl_Buffer(phost, LINE_WIDTH(linewidth));
  App_WrDl_Buffer(phost, COLOR_RGB(r, g, b));
  
  App_WrDl_Buffer(phost, VERTEX2II(x1, y1, 0, 0));
  App_WrDl_Buffer(phost, VERTEX2II(x2, y2, 0, 0));  
  App_WrDl_Buffer(phost, END());
}


/* Creates a box on the screen */
void insert_line(Gpu_Hal_Context_t *phost, int x1, 
                      int x2, int y1, int y2, int linewidth) 
  {
  App_WrDl_Buffer(phost, BEGIN(LINES));
  App_WrDl_Buffer(phost, LINE_WIDTH(linewidth));
  App_WrDl_Buffer(phost, COLOR_RGB(10, 50, 10));
  
  App_WrDl_Buffer(phost, VERTEX2II(x1, y1, 0, 0));
  App_WrDl_Buffer(phost, VERTEX2II(x1, y2, 0, 0));
  
  App_WrDl_Buffer(phost, VERTEX2II(x1, y1, 0, 0));
  App_WrDl_Buffer(phost, VERTEX2II(x2, y1, 0, 0));
  
  App_WrDl_Buffer(phost, VERTEX2II(x2, y2, 0, 0));
  App_WrDl_Buffer(phost, VERTEX2II(x2, y1, 0, 0));
  
  App_WrDl_Buffer(phost, VERTEX2II(x2, y2, 0, 0));
  App_WrDl_Buffer(phost, VERTEX2II(x1, y2, 0, 0));

  App_WrDl_Buffer(phost, VERTEX2II(x2, y1, 0, 0));
  App_WrDl_Buffer(phost, VERTEX2II(x1, y1, 0, 0));


  
  App_WrDl_Buffer(phost, END());
}


void insert_scissor(Gpu_Hal_Context_t *phost, int x1, int x2, int y1, int y2, int r, int g, int b) {
  App_WrDl_Buffer(phost, COLOR_RGB(r, g, b));
  App_WrDl_Buffer(phost, SCISSOR_XY(x1, y1));
  App_WrDl_Buffer(phost, SCISSOR_SIZE(x2, y2));
}


void draw_rect(Gpu_Hal_Context_t *phost, int x1, int y1, int x2, int y2, int r, int g, int b) {
  App_WrDl_Buffer(phost, BEGIN(RECTS));
  App_WrDl_Buffer(phost, COLOR_RGB(r, g, b));
  App_WrDl_Buffer(phost, LINE_WIDTH(100));
  
  /* Using * 16 becuase input to VERTEX2F is 1/16 pixel*/    
  App_WrDl_Buffer(phost, VERTEX2F(x1 * 16, y1 * 16));
  App_WrDl_Buffer(phost, VERTEX2F(x2 * 16, y2 * 16));

  App_WrDl_Buffer(phost, END());
}


void driving_mode_icon(Gpu_Hal_Context_t *phost, int r, int g, int b, int mode, bool driving_mode_flag) {

  char letter;

  switch (mode) {
    case 0:
      letter = 'R';
      break;
    case 1:
      letter = 'D';
      break;
    case 2:
      letter = 'N';
      break;
    default:
      break;
  }
  if(driving_mode_flag) {
    draw_rect(phost, 210, 220, 270, 260, r, g, b);
  } 

  /* ---Fix so that ints corresponds to string--- */

  Write_Text(phost, 225, 170, 31, String(letter).c_str());
  Write_Text(phost, 210, 230, 23, "Mode");
  insert_line(phost, 205, 275, 215, 265, 20);
}


void volt_battery_icon(Gpu_Hal_Context_t *phost, int r, int g, int b) {
  draw_rect(phost, 15, 220, 70, 260, r, g, b);
  Write_Text(phost, 18, 220, 28, "- +");
  Write_Text(phost, 30, 230, 25, "V");
  insert_line(phost, 10, 75, 215, 265, 20);
}


void mc_icon(Gpu_Hal_Context_t *phost, int r, int g, int b) {
  draw_rect(phost, 340, 220, 395, 260, r, g, b);
  Write_Text(phost, 345, 225, 30, "MC");
  insert_line(phost, 335, 400, 215, 265, 20);
}


void solar_cell_icon(Gpu_Hal_Context_t *phost, int r, int g, int b) {
  draw_rect(phost, 410, 220, 465, 260, r, g, b);
  /* Make a grid in rect*/

  insert_line(phost, 410, 465, 220, 260, 20);

  insert_line(phost, 418, 428, 226, 236, 20);
  insert_line(phost, 433, 443, 226, 236, 20);
  insert_line(phost, 448, 458, 226, 236, 20);

  insert_line(phost, 418, 428, 243, 253, 20);
  insert_line(phost, 433, 443, 243, 253, 20);
  insert_line(phost, 448, 458, 243, 253, 20);
}


void error_icon(Gpu_Hal_Context_t *phost, int r, int g, int b) {
  draw_rect(phost, 87, 220, 188, 260, r, g, b);
  Write_Text(phost, 85, 230, 23, "ERROR");
  insert_line(phost, 82, 193, 215, 265, 20);
}


void meter_icon(Gpu_Hal_Context_t *phost, int x1, int x2, int y1, int y2, float percentage) {
  insert_line(phost, x1, x2, y1, y2, 25);
  float counter = 0.9;  
 
  for(int i = y1 + 15; i <= y2-10; i += (y2-y1)/10) {
    if(counter < percentage) {
      draw_rect(phost, x1+10, i, x2-10, i+2, 1, 1, 1);
    }  
    counter -= 0.1;
  }
}


void economy_icon(Gpu_Hal_Context_t *phost, int meter_status) {
  draw_rect(phost, 177, 13, 191, 56, 255, 1, 1);
  draw_rect(phost, 204, 13, 219, 56, 255, 200, 1);
  draw_rect(phost, 232, 13, 248, 56, 255, 255, 1);
  draw_rect(phost, 261, 13, 276, 56, 200, 230, 1);
  draw_rect(phost, 289, 13, 303, 56, 100, 255, 1);

  /* 
    Interval: 177-280
  */

  insert_single_line(phost, meter_status, meter_status, 10, 60, 40, 10, 50 ,10);

  insert_line(phost, 170, 310, 8, 60, 40);
}


void high_beam(Gpu_Hal_Context_t *phost) {

  App_WrDl_Buffer(phost, BEGIN(BITMAPS));
  App_WrDl_Buffer(phost, COLOR_RGB(20, 20, 230));
  App_WrDl_Buffer(phost, VERTEX2II(120, 150, 31, 'D'));
  App_WrDl_Buffer(phost, END());

  insert_single_line(phost, 100, 118, 162, 162, 40, 15, 20, 250);
  insert_single_line(phost, 100, 118, 168, 168, 40, 15, 20, 250);
  insert_single_line(phost, 100, 118, 174, 174, 40, 15, 20, 250);
  insert_single_line(phost, 100, 118, 180, 180, 40, 15, 20, 250);
  insert_single_line(phost, 100, 118, 186, 186, 40, 15, 20, 250);
  //insert_line(phost, 92, 150, 155, 195, 20);  
}


void high_voltage(Gpu_Hal_Context_t *phost) {
  Write_Text(phost, 95, 110, 30, "HV");
}


void cruise_control_icon(Gpu_Hal_Context_t *phost, float cruise_velocity) {
  char cruise[20];
  sprintf(cruise, "Cruise:%s", String(int(cruise_velocity)).c_str());

  Write_Text(phost, 150, 150, 24, cruise);
}


void eco_or_racing_mode(Gpu_Hal_Context_t *phost, bool eco_or_racing_mode_flag) {
  if(!eco_or_racing_mode_flag) {
    Write_Text(phost, 85, 90, 23, "ECO");
  } else {
    Write_Text(phost, 85, 90, 23, "RACE");
  }
}


int calc_potentials(float potential, float min, float max) {
  //int mappedValue = map(potential, 0, 1023, min, max);
  int mappedValue = map(potential, min, max, 0, 1023);
  //Serial.println(mappedValue/6);
  return int(mappedValue/5);
}

/*------------------ CAN BUS MESSAGES ---------------------*/

double extractBytesToDecimal(String data, int startByte, int numBytes) {

  // Calculate startbyte index position ex. startByte: 4 = index: 14 (65 160 0 0 68 (250 0 0 1027))
  int startIndex = 0;
  int byteCounter = 0; // Bytes inc. for each " "
  for(int i = 0; i < data.length(); i++)
  {

    if(byteCounter == startByte)
    {
      startIndex = i;
      break;
    }

    if(data.substring(i, i+1) == " ")
    {
      byteCounter++;
    }
  
  }

  //debugln("Start index: " + String(startIndex));

  byte bytes[numBytes];

  byteCounter = 0;
  String byte_data = "";
  for(int i = startIndex; i < data.length(); i++)
  {

    String data_substr = data.substring(i, i+1);

    if(byteCounter == numBytes)
    {
      break;
    }
    else if(data_substr == " ")
    {
      //debugln(byte_data);
      bytes[byteCounter] = (byte) strtoul(byte_data.c_str(), NULL, 10);
      byteCounter++;
      byte_data = "";
    }
    else
    {
      byte_data += data_substr; 
    }

  }
  /* For debugging of output bytes
  for(int i = 0; i < numBytes; i++)
  {
    debug(bytes[i]);
    debug(" ");
  }
  debugln();
  */
  
  double value;
  memcpy(&value, bytes, numBytes);
  // Return the decimal value
  return value;
}


double extractSingleByte(String data, int startByte)
{
  // Calculate startbyte index position ex. startByte: 4 = index: 14 (65 160 0 0 68 (250 0 0 1027))
  int startIndex = 0;
  int byteCounter = 0; // Bytes inc. for each " "
  for(int i = 0; i < data.length(); i++)
  {

    if(byteCounter == startByte)
    {
      startIndex = i;
      break;
    }

    if(data.substring(i, i+1) == " ")
    {
      byteCounter++;
    }
  }

  byteCounter = 0;
  String byte_data = "";
  double MPPTdata = 0;
  for(int i = startIndex; i < data.length(); i++)
  {

    String data_substr = data.substring(i, i+1);


    if(data_substr == " ")
    {
      byteCounter++;

      if(byteCounter == 1)
      {
        MPPTdata = byte_data.toDouble();
        break;
      }
    }
    else
    {
      byte_data += data_substr; 
    }
  }

  return MPPTdata;
}

double extractDataMPPT(String data, int startByte, int numBytes)
{
  // Calculate startbyte index position ex. startByte: 4 = index: 14 (65 160 0 0 68 (250 0 0 1027))
  int startIndex = 0;
  int byteCounter = 0; // Bytes inc. for each " "
  for(int i = 0; i < data.length(); i++)
  {

    if(byteCounter == startByte)
    {
      startIndex = i;
      break;
    }

    if(data.substring(i, i+1) == " ")
    {
      byteCounter++;
    }
  
  }
  //debugln("Start index: " + String(startIndex));

  byteCounter = 0;
  String byte_data = "";
  double MPPTdata = 0;
  for(int i = startIndex; i < data.length(); i++)
  {

    String data_substr = data.substring(i, i+1);

    if(byteCounter == numBytes)
    {
      break;
    }
    else if(data_substr == " ")
    {
      byteCounter++;

      if(byteCounter == 1)
      {
        MPPTdata += 256*byte_data.toDouble();
      }
      else
      {
        MPPTdata += byte_data.toDouble();
      }
      byte_data = "";
    }
    else
    {
      byte_data += data_substr; 
    }
  }

  return MPPTdata;
}


