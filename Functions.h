#include <SPI.h>
#include "Platform.h"
#include "App_Common.h"

void Flash_Light(int delay, int pin);
void Write_Text(Gpu_Hal_Context_t *phost,  int x, int y, int size, char text[]);
void Finish_Display(Gpu_Hal_Context_t *phost);
void Start_Set_Display(Gpu_Hal_Context_t *phost);
void insert_line(Gpu_Hal_Context_t *phost, int x1, int x2, int y1, int y2, int linewidth);
void insert_charging(Gpu_Hal_Context_t *phost, float charging); 
void draw_rect(Gpu_Hal_Context_t *phost, int x1, int y1, int x2, int y2, int r, int g, int b);
void driving_mode_icon(Gpu_Hal_Context_t *phost, int r, int g, int b, int mode, bool driving_mode_flag);
void volt_battery_icon(Gpu_Hal_Context_t *phost, int r, int g, int b);
void mc_icon(Gpu_Hal_Context_t *phost, int r, int g, int b);
void solar_cell_icon(Gpu_Hal_Context_t *phost, int r, int g, int b);
void error_icon(Gpu_Hal_Context_t *phost, int r, int g, int b);
void meter_icon(Gpu_Hal_Context_t *phost, int x1, int x2, int y1, int y2, float percentage);
void economy_icon(Gpu_Hal_Context_t *phost, int meter_status);
void insert_scissor(Gpu_Hal_Context_t *phost, int x1, int x2, int y1, int y2, int r, int g, int b);
void insert_single_line(Gpu_Hal_Context_t *phost, int x1, int x2, int y1, int y2, int linewidth, int r, int g, int b);
void high_beam(Gpu_Hal_Context_t *phost);
void high_voltage(Gpu_Hal_Context_t *phost);
int calc_potentials(float potential, float min, float max);
void cruise_control_icon(Gpu_Hal_Context_t *phost, float cruise_velocity);
double extractDataMPPT(String data, int startByte, int numBytes);
double extractSingleByte(String data, int startByte);
double extractBytesToDecimal(String data, int startByte, int numBytes);
void eco_or_racing_mode(Gpu_Hal_Context_t *phost, bool eco_or_racing_mode_flag); 