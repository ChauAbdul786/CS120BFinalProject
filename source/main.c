/*	Author: Abdullah Chaudhry
 *  Partner(s) Name: 
 *	Lab Section: 021
 *	Assignment: Final Project
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */

//NOTE: This project was compiled using the Arduino IDE, by setting the core for the Arduino Uno to empty and using the Atmega1284p core provided at https://github.com/JChristensen/mighty-1284p/tree/v1.6.3
//The full tutorial for this process can be found here: http://www.technoblogy.com/show?19OV

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "nokia5110.h" //Provided by https://github.com/LittleBuster/avr-nokia5110
#include "timer.h"
#include "scheduler.h"
#include "SparkFun_MMA8452Q.h" // Provided by https://learn.sparkfun.com/tutorials/mma8452q-accelerometer-breakout-hookup-guide/all (Note: This library is arduino, which is why this project was compiled using Arduino IDE before being used as a sketch in Atmel Studio)

#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif


unsigned char tmpA, tmpC, tmpD;
void ADC_Init();         /* ADC Initialization function */ //Provided by https://www.avrfreaks.net/forum/ac-dimmer-1
int ADC_Read(char channel);  /* ADC Read function */    //Provided by https://www.avrfreaks.net/forum/ac-dimmer-1
char toChar(int); //Helper function to take in an int and return it's corresponding ASCII value
char withinTolerance(int a, int b); //Helper function to determine if the user's angle is close enough to the true angle
int convertToAngle(int g); //Helper function to convert raw g data provided by accelerometer to angle to be compared against true angle. Equation for conversion was found here: https://www.analog.com/en/app-notes/an-1057.html

enum states { Start, MainMenu, RoundStart, Game, Win, Lose } state;
unsigned int cursorPosition = 10;
unsigned int x, y, roundsToWin, restartButton;
unsigned int numRound = 1;
unsigned int tempsw;
unsigned int i, j, roundsLeft;
int angle, userAngle;
char sAngle[5], sRoundsLeft[5];
void Tick(){ 
    tempsw = ((PINA & 0x80) >> 7);
    restartButton = ((PINA & 0x40) >> 6); //Note: Bit shifting here isn't required per the 
                                            //logic of the program, but is done anyways
                                            //for adding potential future functionality
    y = ADC_Read(1);
    x = ADC_Read(0);

    accel.read();
    userAngle = accel.z; //Only care about z-axis in this game

    switch(state){ //State Transitions
        case Start:
            state = MainMenu;
            nokia_lcd_clear();
            i = 0;
            j = 0;
            break;
        case MainMenu:
            if(!restartButton){
                state = Start;
                cursorPosition = 10;
                numRound = 1;
                i = 0;
                j = 0;
            }

            if(!tempsw){
                switch(cursorPosition){
                    case(10):
                        cursorPosition = 20;
                        roundsToWin = 3;
                        break;
                    case(20):
                        cursorPosition = 20;
                        roundsToWin = 5;
                        break;
                    case(30):
                        cursorPosition = 20;
                        roundsToWin = 7;
                        break;
                    default:
                        cursorPosition = 20;
                        roundsToWin = 5;
                        break;
                }

                state = RoundStart;
            }else{
                state = MainMenu;
            }
            break;
        case RoundStart:
        if(!restartButton){
            state = Start;
            cursorPosition = 10;
            numRound = 1;
            i = 0;
            j = 0;
        }
            if(i <= 15){
                state = RoundStart;
            }else{
                i = 0;
                angle = rand() % 91; //Angle from 0 - 90
                itoa(angle, sAngle, 10);
                state = Game;
            }
            break;
        case Game:
        if(!restartButton){
            state = Start;
            cursorPosition = 10;
            numRound = 1;
            i = 0;
            j = 0;
        }
            if(i >= 100){
                if(j >= 50){
                    i = 0;
                    j = 0;
                    numRound++;
                    if(numRound > roundsToWin){ //Strict inequality becuase numRound starts at 1 (for visual purposes for the player)
                        state = Win;
                    }else{
                        state = RoundStart;
                    }
                }else{
                    i = 0;
                    j = 0;
                    state = Lose;
                }
            }else{
                state = Game;
            }
            break;
        case Win:
        if(!restartButton){
            state = Start;
            cursorPosition = 10;
            numRound = 1;
            i = 0;
            j = 0;
        }else{
            state = Win;
        }
            break;
        case Lose:
        if(!restartButton){
            state = Start;
            cursorPosition = 10;
            numRound = 1;
            i = 0;
            j = 0;
        }else{
            state = Lose;  
        }
            break;
        default:
            break;
    }

    switch(state){ //State Actions
        case Start:
            break;
        case MainMenu:
            if(y < 200){
                if(cursorPosition < 30){
                    cursorPosition += 10;
                }
            }else if(y > 800){
                if(cursorPosition > 10){
                    cursorPosition -= 10;
                }
            }

            nokia_lcd_clear();

            nokia_lcd_set_cursor(0, 0);
            nokia_lcd_write_string("ANGLE GAME!", 1);
            nokia_lcd_set_cursor(0, 10);
            nokia_lcd_write_string("Easy", 1);
            nokia_lcd_set_cursor(0, 20);
            nokia_lcd_write_string("Medium", 1);
            nokia_lcd_set_cursor(0, 30);
            nokia_lcd_write_string("Hard", 1);

            nokia_lcd_set_cursor(35, cursorPosition);
            nokia_lcd_write_string("<--", 1);

            nokia_lcd_render();

            break;
        case RoundStart:
            i++;

            nokia_lcd_clear();
            nokia_lcd_set_cursor(0, 20);
            nokia_lcd_write_string("ROUND: ", 2);
            nokia_lcd_write_char(toChar(numRound), 2);

            nokia_lcd_render();
            
            break;
        case Game:
            i++;

            roundsLeft = (roundsToWin - numRound + 1);
            itoa(roundsLeft, sRoundsLeft, 10);

            nokia_lcd_clear();
            nokia_lcd_set_cursor(0, 0);
            nokia_lcd_write_string("Rounds Left: ", 1);
            nokia_lcd_write_string(sRoundsLeft, 1);
            nokia_lcd_set_cursor(0, 20);
            nokia_lcd_write_string("Angle :", 1);
            nokia_lcd_write_string(sAngle, 1);
            nokia_lcd_render();

            if(withinTolerance(convertToAngle(userAngle), Angle)){
                j++; //If 500 j are accumulated (5 seconds) the round is won
            }

            
            break;
        case Win:
        if(y < 200){
            if(cursorPosition < 30){
                cursorPosition += 2;
            }
        }else if(y > 800){
            if(cursorPosition > 10){
                cursorPosition -= 2;
            }
        }

            nokia_lcd_clear();
            nokia_lcd_set_cursor(15, cursorPosition);
            nokia_lcd_write_string("YOU WIN!!!!", 1);
            nokia_lcd_render();
            break;
        case Lose:
        if(y < 200){
            if(cursorPosition < 30){
                cursorPosition += 2;
            }
        }else if(y > 800){
            if(cursorPosition > 10){
                cursorPosition -= 2;
            }
        }

            nokia_lcd_clear();
            nokia_lcd_set_cursor(15, cursorPosition);
            nokia_lcd_write_string("You lost :(", 1);
            nokia_lcd_render();

            break;
        default:
            break;
    }
}


int main(void) {
    /* Insert DDR and PORT initializations */
    DDRA = 0x00; PINA = 0xFF;
    DDRB = 0xFF; PORTB = 0x00;
    DDRC = 0x00; PINC = 0xFF;
    DDRD = 0x00; PIND = 0xFF;

    /* Insert your solution below */
    ADC_Init(); //ADC used for joystick
    nokia_lcd_init();

    MMA8452Q accel;
    accel.init();

    TimerSet(100);
    TimerOn();

    while (1) {
        tmpA = PINA;
        tmpC = PINC;
        tmpD = PIND;

        Tick();

        while(!TimerFlag) {};
        TimerFlag = 0;
    }
    return 1;
}


void ADC_Init(){
    DDRA = 0x00;        /* Make ADC port as input */
    ADCSRA = 0x87;      /* Enable ADC, with freq/128  */
    ADMUX = 0x40;       /* Vref: Avcc, ADC channel: 0 */
}

int ADC_Read(char channel){
    ADMUX = 0x40 | (channel & 0x07);/* set input channel to read */
    ADCSRA |= (1<<ADSC);    /* Start ADC conversion */
    while (!(ADCSRA & (1<<ADIF)));  /* Wait until end of conversion by polling ADC interrupt flag */
    ADCSRA |= (1<<ADIF);    /* Clear interrupt flag */
    _delay_ms(1);       /* Wait a little bit */
    return ADCW;        /* Return ADC word */
}

char toChar(int i){
    return (i += 48);
}

int withinTolerance(int a, int b){
    if((b - a <= 20) && (b - a >= -20)){
        return 1;
    }

    return 0;
}

int convertToAngle(int g){
    return (asin(g / 1));
}