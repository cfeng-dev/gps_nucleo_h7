# GPS STM32 library

This project demonstrates how to use the **STM32 NUCLEO-H7A3ZI-Q** with the **Adafruit Ultimate GPS HAT** to read and process GPS data via UART. The code leverages the [leech001/gps repository](https://github.com/leech001/gps) as a foundation for GPS data parsing and communication.

This repository now serves as documentation of how I configured the GPS module with the microcontroller. I hope it can help others who are starting with GPS and STM32 development.

## Hardware and Software Configuration

### Step 1: Open the `.ioc` File in STM32CubeIDE

-   Launch **STM32CubeIDE** and open the `.ioc` file for the project.

### Step 2: Enable and Configure USART1

1. Navigate to **Pinout & Configuration**.
2. Under the peripherals list (A → Z), search for **USART1**.
3. Enable **USART1** in **Mode** as **Asynchronous**.
4. Go to **NVIC Settings** and check the box for **USART1 global interrupt** (as shown in the image below).

![USART1 NVIC Settings](/img/nvic_settings.png)

5. In **Parameter Settings**, set the **Baud Rate** to **9600 Bits/s** (as shown in the image below).

![USART1 Parameter Settings](/img/parameter_settings.png)

6. Adjust **GPIO Settings** for **USART1** as shown in the image below.

![USART1 GPIO Settings](/img/gpio_settings.png)

### Step 3: Adjust Code Generation Settings

1. Go to **Project Manager** → **Code Generator**.
2. Uncheck the box for **Generate peripheral initialization as a pair of `.c/.h` files per peripheral**.

### Step 4: Save the Configuration

-   Press `Ctrl + S` to save the `.ioc` file, which will regenerate the code with the updated configuration.

### Step 5: Add Flags for Float Support in printf and scanf

To enable floating-point support in `printf` and `scanf`, follow these steps:

1. Go to **Project** → **Properties** in STM32CubeIDE.
2. Navigate to **C/C++ Build** → **Settings**.
3. Under the **Tool Settings** tab, find **Miscellaneous** (under **MCU GCC Linker**).
4. In the **Other Flags** field, add the following flags:
    - `-u _printf_float`
    - `-u _scanf_float`
5. Click **Apply and Close** to save the changes.

This step ensures that floating-point values can be properly handled in formatted input/output functions like `printf` and `scanf`.

### Step 6: Add the Required Files to Your Project

Add these files to your project:

-   Place `gps.c` in the **Core/Src** folder.
-   Place `gps.h` in the **Core/Inc** folder.

### Step 7: Import the Library in `main.c`

1. Open `main.c` in STM32CubeIDE.
2. Add the following includes inside the `/* USER CODE BEGIN Includes */` section:

```c
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "string.h"
#include "gps.h"
/* USER CODE END Includes */
```

### Step 8: Add GPS Setup Code

-   In the `/_ USER CODE BEGIN 2 _/` section of `main.c`, add the following setup code to initialize the BNO055 IMU sensor:

```c
/* USER CODE BEGIN 2 */
GPS_Init();
/* USER CODE END 2 */
```

### Step 9: Write a Function to Fetch and Transmit GPS Data

1.  Inside the `/_ USER CODE BEGIN 0 _/` section of `main.c`, add the following function:

```c
/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart == GPS_USART) GPS_UART_CallBack();
}

void send_gps_data() {
	char buffer[200];

	// GPS data
	snprintf(buffer, sizeof(buffer),
			"[GPS Data] Time: %02d:%02d:%02d, Latitude: %.6f %c, Longitude: %.6f %c, Speed: %.2f knots, Status: %c\r\n",
			GPS.hours, GPS.minutes, GPS.seconds,  // Use pre-converted time
			GPS.dec_latitude,                     // Decimal latitude
			GPS.ns,                               // N/S indicator
			GPS.dec_longitude,                    // Decimal longitude
			GPS.ew,                               // E/W indicator
			GPS.speed_k,                          // Speed in knots
			GPS.rmc_status                        // RMC status ('A' = active, 'V' = void)
			);

	HAL_UART_Transmit(&huart3, (uint8_t*) buffer, strlen(buffer), HAL_MAX_DELAY);
	HAL_Delay(100); // 10 Hz
}
/* USER CODE END 0 */
```

2. Call the function inside the main loop to continuously fetch and transmit data:

```c
/* Infinite loop */
/* USER CODE BEGIN WHILE */
while (1)
{
    /* USER CODE END WHILE */
    send_gps_data();
    /* USER CODE BEGIN 3 */
}
/* USER CODE END 3 */
```

### Step 10: Circuit Connections

To connect the **Adafruit Ultimate GPS HAT** to the **STM32 NUCLEO-H7A3ZI-Q**, use the following wiring:

| **GPS HAT Pin** | **NUCLEO Pin** |
| --------------- | -------------- |
| 3.3V            | 3V3            |
| GND             | GND            |
| TX              | TX (PB6)       |
| RX              | RX (PB15)      |

Ensure the connections are secure to avoid communication issues. The image below illustrates the wiring:

![Circuit Diagram for BNO055 and STM32 NUCLEO-H7A3ZI-Q](/img/gps_circuit_diagram.png)

For more detailed information about the external header connections of the **NUCLEO-H7A3ZI-Q**, refer to the image below:

![NUCLEO-H7A3ZI-Q External Header Connections](/img/extension_connectors_stm32_h7.png)

For more detailed information about the external header connections of the **Adafruit Ultimate GPS HAT**, refer to the image below:

![NUCLEO-H7A3ZI-Q External Header Connections](/img/extension_connectors_adafruit_gps.png)

## Tools

-   **IDE**: STM32CubeIDE (1.16.1)
-   **Microcontroller**: [STM32 NUCLEO-H7A3ZI-Q](https://www.st.com/en/evaluation-tools/nucleo-h7a3zi-q.html)
-   **GPS Module**: [Adafruit Ultimate GPS HAT for Raspberry Pi A+/B+/Pi 2/3/4/Pi 5 - Mini Kit](https://www.adafruit.com/product/2324)

## Third-Party Code

This project includes code from the `gps.c` and `gps.h` files by Bulanov Konstantin, originally sourced from the [leech001/gps repository](https://github.com/leech001/gps). The code is licensed under the GNU General Public License (GPL) v3.

For the full terms of the GPL, see the `LICENSE` file included in this repository.

## Modifications

-   Extended the GPS parsing logic to include additional NMEA messages:
    -   `$GNGGA`: Position and satellite data
    -   `$GNRMC`: Position, speed, and time data
-   Added helper functions:
    -   `GPS_ConvertTime`: Converts raw UTC time to hours, minutes, and seconds.
    -   `GPS_ConvertDate`: Converts raw date to day, month, and year.
