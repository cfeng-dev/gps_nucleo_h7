/*
 * gps.h
 *
 *  Created on: Nov 15, 2019
 *      Author: Bulanov Konstantin
 *
 * Modifications:
 * - Changed UART interface from `huart2` to `huart1` (User-specific stuff).
 * - Added timezone offset definition (`GPS_TIMEZONE_OFFSET`).
 * - Added additional converted time and date fields in the `GPS_t` struct.
 *
 * Modified by: Chin-I Feng, 2024
 */


#ifndef GPS_H
#define GPS_H

#include "main.h"

#define GPS_USART      &huart1
#define GPSBUFSIZE     128       // GPS buffer size
#define GPS_TIMEZONE_OFFSET 1 	 // Define the timezone offset (e.g., +1 for Germany in winter)
#define GPS_DEBUG	0

typedef struct{

    // calculated values
    float dec_longitude;
    float dec_latitude;
    float altitude_ft;

    // GNGGA - Global Positioning System Fixed Data
    float nmea_longitude;
    float nmea_latitude;
    float utc_time;
    char ns, ew;
    int lock;
    int satelites;
    float hdop;
    float msl_altitude;
    char msl_units;

    // GNRMC - Recommended Minimmum Specific GNS Data
    char rmc_status;
    float speed_k;
    float course_d;
    int date;

    // GNVTG - Course over ground, ground speed
    float course_t; // ground speed true
    char course_t_unit;
    float course_m; // magnetic
    char course_m_unit;
    char speed_k_unit;
    float speed_km; // speek km/hr
    char speed_km_unit;

    // Converted time
    int hours;                 // Hours
    int minutes;               // Minutes
    int seconds;               // Seconds

    // Converted date
    int day;                   // Day
    int month;                 // Month
    int year;                  // Year (four digits)
} GPS_t;

extern GPS_t GPS;
extern UART_HandleTypeDef huart1;
extern uint8_t rx_data;
extern uint8_t rx_buffer[GPSBUFSIZE];
extern uint8_t rx_index;

void GPS_Init(void);
void GSP_USBPrint(char *data);
void GPS_print_val(char *data, int value);
void GPS_UART_CallBack();
int GPS_validate(char *nmeastr);
void GPS_parse(char *GPSstrParse);
float GPS_nmea_to_dec(float deg_coord, char nsew);
void GPS_ConvertTime(float utc_time, int *hours, int *minutes, int *seconds);
void GPS_ConvertDate(int date, int *day, int *month, int *year);

#endif // GPS_H
