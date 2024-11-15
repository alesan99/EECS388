#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "eecs388_lib.h"

#define SERVO_PULSE_MAX 2400 /* 2400 us */
#define SERVO_PULSE_MIN 544 /* 544 us */

void auto_brake(int devid)
{
    // Task-1: 
    // Your code here (Use Lab 02 - Lab 04 for reference)
    // Use the directions given in the project document
    uint16_t dist = 0;
    if ('Y' == ser_read(devid) && 'Y' == ser_read(devid)) { // read 1st & 2nd byte headers
        // Read
        uint8_t dist_L = ser_read(devid); // 3rd byte, containing half of distance data
        uint8_t dist_H = ser_read(devid); // 4th byte, containing rest
        // Process
        dist = dist_L + (dist_H << 8); // combine to get full 16 bit distance

        // Brake light indicator
        if (dist <= 60) { // 60 cm
            // Too close, must stop
            uint64_t timer = get_cycles();
            int red_light_on = 1;
            if ((timer / 100000) % 2 == 0) { // 100ms
                red_light_on = 0;
            }

            gpio_write(RED_LED, red_light_on);
            gpio_write(GREEN_LED, 0);
            gpio_write(BLUE_LED, 0);
        } else if (dist <= 100) { // 100 cm
            //Very close, break hard
            gpio_write(RED_LED, 1);
            gpio_write(GREEN_LED, 0);
            gpio_write(BLUE_LED, 0);
        } else if (dist <= 200) { // 200 cm
            // Close, brake lightly
            gpio_write(RED_LED, 1);
            gpio_write(GREEN_LED, 1);
            gpio_write(BLUE_LED, 0);
        } else { // > 200 cm
            //Safe distance, no braking
            gpio_write(RED_LED, 0);
            gpio_write(GREEN_LED, 1);
            gpio_write(BLUE_LED, 0);
        }
    }
}

int read_from_pi(int devid)
{
    // Task-2: 
    // You code goes here (Use Lab 09 for reference)
    // After performing Task-2 at dnn.py code, modify this part to read angle values from Raspberry Pi.
    char str[100];
    int angle = 0;
    if (ser_isready(devid)) { // is UART channel ready?
        ser_readline(devid, 100, str);
        
        sscanf(str, "%d", &angle);
        return angle;
    }
}

void steering(int gpio, int pos)
{
    // Task-3: 
    // Your code goes here (Use Lab 05 for reference)
    // Check the project document to understand the task
    int pulse_time = (pos * (SERVO_PULSE_MAX - SERVO_PULSE_MIN)/180) + SERVO_PULSE_MIN;
    // pulse high
    gpio_write(gpio, 1);
    delay_usec(pulse_time);

    // pulse low
    gpio_write(gpio, 0);
    delay_usec(20000-pulse_time);
}


int main()
{
    // initialize UART channels
    ser_setup(0); // uart0
    ser_setup(1); // uart1
    int pi_to_hifive = 1; //The connection with Pi uses uart 1
    int lidar_to_hifive = 0; //the lidar uses uart 0
    
    printf("\nUsing UART %d for Pi -> HiFive", pi_to_hifive);
    printf("\nUsing UART %d for Lidar -> HiFive", lidar_to_hifive);
    
    //Initializing PINs
    gpio_mode(PIN_19, OUTPUT);
    gpio_mode(RED_LED, OUTPUT);
    gpio_mode(BLUE_LED, OUTPUT);
    gpio_mode(GREEN_LED, OUTPUT);

    printf("Setup completed.\n");
    printf("Begin the main loop.\n");

    int tick_timer = 0;

    while (1) {
        tick_timer++;


        auto_brake(lidar_to_hifive); // measuring distance using lidar and braking
        int angle = read_from_pi(pi_to_hifive); //getting turn direction from pi
        printf("\nangle=%d", angle) 
        int gpio = PIN_19; 
        for (int i = 0; i < 10; i++){
            // Here, we set the angle to 180 if the prediction from the DNN is a positive angle
            // and 0 if the prediction is a negative angle.
            // This is so that it is easier to see the movement of the servo.
            // You are welcome to pass the angle values directly to the steering function.
            // If the servo function is written correctly, it should still work,
            // only the movements of the servo will be more subtle
            if(angle>0){
                steering(gpio, 180);
            }
            else {
                steering(gpio,0);
            }
            
            // Uncomment the line below to see the actual angles on the servo.
            // Remember to comment out the if-else statement above!
            // steering(gpio, angle);
        }

    }
    return 0;
}
