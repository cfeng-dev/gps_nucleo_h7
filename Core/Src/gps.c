/*
 * gps.c
 *
 *  Created on: Nov 15, 2019
 *      Author: Bulanov Konstantin
 *
 *  Contact information
 *  -------------------
 *
 * e-mail   :  leech001@gmail.com
 */

/*
 * |---------------------------------------------------------------------------------
 * | Copyright (C) Bulanov Konstantin,2019
 * |
 * | This program is free software: you can redistribute it and/or modify
 * | it under the terms of the GNU General Public License as published by
 * | the Free Software Foundation, either version 3 of the License, or
 * | any later version.
 * |
 * | This program is distributed in the hope that it will be useful,
 * | but WITHOUT ANY WARRANTY; without even the implied warranty of
 * | MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * | GNU General Public License for more details.
 * |
 * | You should have received a copy of the GNU General Public License
 * | along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * |---------------------------------------------------------------------------------
 */

/*
 * Modifications:
 * - Redesigned the GPS parsing logic to include additional NMEA messages (e.g., $GNGGA, $GNRMC, $GNVTG).
 * - Added functions for converting raw UTC time and date into human-readable format.
 * - Added function descriptions to improve code documentation.
 *
 * Modified by: Chin-I Feng, 2024
 */

#include "gps.h"
#include <string.h>
#include <stdio.h>

uint8_t rx_data = 0;
uint8_t rx_buffer[GPSBUFSIZE];
uint8_t rx_index = 0;

GPS_t GPS;

#if (GPS_DEBUG == 1)
void GPS_print(char *data){
	char buf[GPSBUFSIZE] = {0,};
	sprintf(buf, "%s\n", data);
	CDC_Transmit_FS((unsigned char *) buf, (uint16_t) strlen(buf));
}
#endif

/**
 * @brief Initialize GPS UART reception
 */
void GPS_Init() {
    HAL_UART_Receive_IT(GPS_USART, &rx_data, 1); // Start UART interrupt
}

void GPS_UART_CallBack() {
    if (rx_data != '\n' && rx_index < sizeof(rx_buffer)) {
        rx_buffer[rx_index++] = rx_data;
    } else {

		#if (GPS_DEBUG == 1)
		GPS_print((char*)rx_buffer);
		#endif

        if (GPS_validate((char*)rx_buffer)) {
            GPS_parse((char*)rx_buffer);
        }
        rx_index = 0;
        memset(rx_buffer, 0, sizeof(rx_buffer));
    }
    HAL_UART_Receive_IT(GPS_USART, &rx_data, 1);
}

/**
 * @brief Validate a GPS NMEA message by checking its format and checksum.
 * @param nmeastr Pointer to the GPS NMEA message string.
 * @return 1 if the message is valid, 0 otherwise.
 *
 * This function verifies that the NMEA message starts with a '$',
 * calculates its checksum by XORing all characters between '$' and '*',
 * and compares the calculated checksum with the checksum at the end of the message.
 * The function ensures the message length does not exceed 75 characters
 * and contains the required checksum format ('*XX').
 */
int GPS_validate(char *nmeastr) {
    char check[3];
    char checkcalcstr[3];
    int i;
    int calculated_check;

    i = 0;
    calculated_check = 0;

    // check to ensure that the string starts with a $
    if (nmeastr[i] != '$') {
        return 0;
    }
    i++;

    //No NULL reached, 75 char largest possible NMEA message, no '*' reached
    while((nmeastr[i] != 0) && (nmeastr[i] != '*') && (i < 75)) {
        calculated_check ^= nmeastr[i]; // calculate the checksum
        i++;
    }

    if (i >= 75) {
        return 0; // the string was too long so return an error
    }

    if (nmeastr[i] == '*') {
        check[0] = nmeastr[i+1];    // put hex chars in check string
        check[1] = nmeastr[i+2];
        check[2] = 0;
    }
    else {
        return 0; // no checksum separator found there for invalid
    }

    sprintf(checkcalcstr, "%02X", calculated_check);
    return((checkcalcstr[0] == check[0])
        && (checkcalcstr[1] == check[1])) ? 1 : 0;
}

// Modified to support additional NMEA messages (e.g., $GNGGA, $GNRMC, $GNVTG)
/**
 * @brief Parse GPS NMEA messages and extract data into the GPS structure.
 * @param GPSstrParse Pointer to the NMEA message string.
 *
 * Extracts data based on message type:
 * - $GNGGA: Position, satellites, altitude, and UTC time.
 * - $GNRMC: Position, speed, course, UTC time, and date.
 * - $GNVTG: Course and speed data.
 *
 * Skips invalid or unrecognized messages.
 */
void GPS_parse(char *GPSstrParse){
    if(!strncmp(GPSstrParse, "$GNGGA", 6)){
    	if (sscanf(GPSstrParse, "$GNGGA,%f,%f,%c,%f,%c,%d,%d,%f,%f,%c", &GPS.utc_time, &GPS.nmea_latitude, &GPS.ns, &GPS.nmea_longitude, &GPS.ew, &GPS.lock, &GPS.satelites, &GPS.hdop, &GPS.msl_altitude, &GPS.msl_units) >= 1) {
    		GPS.dec_latitude = GPS_nmea_to_dec(GPS.nmea_latitude, GPS.ns);
    		GPS.dec_longitude = GPS_nmea_to_dec(GPS.nmea_longitude, GPS.ew);
    		GPS_ConvertTime(GPS.utc_time, &GPS.hours, &GPS.minutes, &GPS.seconds);

    		return;
    	}
    }
    else if (!strncmp(GPSstrParse, "$GNRMC", 6)){
    	if(sscanf(GPSstrParse, "$GNRMC,%f,%c,%f,%c,%f,%c,%f,%f,%d", &GPS.utc_time, &GPS.rmc_status, &GPS.nmea_latitude, &GPS.ns, &GPS.nmea_longitude, &GPS.ew, &GPS.speed_k, &GPS.course_d, &GPS.date) >= 1) {
    		GPS_ConvertTime(GPS.utc_time, &GPS.hours, &GPS.minutes, &GPS.seconds);
    		GPS_ConvertDate(GPS.date, &GPS.day, &GPS.month, &GPS.year);

    		return;
    	}
    }
    else if (!strncmp(GPSstrParse, "$GNVTG", 6)){
        if(sscanf(GPSstrParse, "$GNVTG,%f,%c,%*[^,],%c,%f,%c,%f,%c", &GPS.course_t, &GPS.course_t_unit, &GPS.course_m_unit, &GPS.speed_k, &GPS.speed_k_unit, &GPS.speed_km, &GPS.speed_km_unit) >= 1) {
            return;
        }
    }
}

float GPS_nmea_to_dec(float deg_coord, char nsew) {
    int degree = (int)(deg_coord/100);
    float minutes = deg_coord - degree*100;
    float dec_deg = minutes / 60;
    float decimal = degree + dec_deg;
    if (nsew == 'S' || nsew == 'W') { // return negative
        decimal *= -1;
    }
    return decimal;
}

/**
 * @brief Convert raw UTC time (hhmmss.sss) into hours, minutes, and seconds.
 * @param utc_time Raw UTC time in hhmmss.sss format.
 * @param hours Pointer to store hours.
 * @param minutes Pointer to store minutes.
 * @param seconds Pointer to store seconds.
 */
void GPS_ConvertTime(float utc_time, int *hours, int *minutes, int *seconds) {
    *hours = (int)(utc_time / 10000);           // Extract hours
    *minutes = (int)((utc_time / 100)) % 100;   // Extract minutes
    *seconds = (int)(utc_time) % 100;           // Extract seconds

    // Adjust for the timezone
    *hours += GPS_TIMEZONE_OFFSET;

    // Handle overflow if hours exceed 23
    if (*hours >= 24) {
        *hours -= 24;
    } else if (*hours < 0) {
        *hours += 24;
    }
}

/**
 * @brief Convert raw date (ddmmyy) into day, month, and year.
 * @param date Raw date in ddmmyy format.
 * @param day Pointer to store day.
 * @param month Pointer to store month.
 * @param year Pointer to store year.
 */
void GPS_ConvertDate(int date, int *day, int *month, int *year) {
    *day = date / 10000;                        // Extract day
    *month = (date / 100) % 100;                // Extract month
    *year = (date % 100) + 2000;                // Convert year to four digits
}
