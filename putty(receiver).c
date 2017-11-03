#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

unsigned int first, last,time,over,pulse;
void Delay (unsigned int Delay) {
    int i;
    for (i=0; i < Delay;i++){
        TCCR2A = 0;
        TCCR2B = (1<<CS12) | (1<<CS10);
        OCR2A = 26;
        TCNT2 = 0;
        TIFR2 = (1<<OCF2A);
        while ( !(TIFR2 & (1<<OCF2A)));
    }
    TCCR2B = 0;
}
ISR (TIMER1_OVF_vect){
    over=1;
}
ISR(TIMER1_CAPT_vect){
    first=ICR1;
    if (over) {
        time=65536 - last + first;
        over = 0;
    }
    else {
        time= first - last;
    }
    last = first;
}

void capture() {
    TCCR1B = (1<<ICNC1) | ( 1<<ICES1) | ( 1<<CS10);
    TCNT1=0;
    TIMSK1 = (1<<TOIE1) | (1<<ICIE1);
}
void disable () {
    TIMSK1=0;
}
int uart_putchar(char c, FILE *stream);
int uart_getchar(FILE *stream);
FILE mystdout = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);
FILE mystdin = FDEV_SETUP_STREAM(NULL, uart_getchar, _FDEV_SETUP_READ);
/******************************************************************************
******************************************************************************/
int uart_putchar(char c, FILE *stream)
{
    loop_until_bit_is_set(UCSR0A, UDRE0);
    UDR0 = c;
    return 0;
}
/******************************************************************************
******************************************************************************/
int uart_getchar(FILE *stream)
{
    /* Wait until data exists. */
    loop_until_bit_is_set(UCSR0A, RXC0);
    return UDR0;
}
/******************************************************************************
******************************************************************************/
void init_uart(void)
{
    UCSR0B = (1<<RXEN0) | (1<<TXEN0);
    UBRR0 = 95;
    stdout = &mystdout;
    stdin = &mystdin;
}
/******************************************************************************
******************************************************************************/
int main(void){
    init_uart(); // initialization
    int Array[15];
    DDRD=0x00;
    int j=0;
    int i, multiplier = 32;
    double k;
    int ID=0, Assignment =0, converter;
    capture();
    printf("\n ******************************************");
    printf("\n Waiting for first assignment submission. \n");
    while (1){
        sei();
        k = (14700000/time);
        if (k>50000){
            Delay(5);
            for (i=0; i<=11; i++){
                k=14700000/time;
                if (k < 92000){
                    Array[i]=0;
                }
                else {
                    Array[i]=1;
                }
                Delay(4);
            }            
            for (i=0; i<=5; i++){
                converter = Array[i] * multiplier;
                multiplier = multiplier/2;
                ID = converter + ID;
            }
            multiplier = 32;
            printf("\n Student ID  = %d", ID);
            for (i=6; i<=11; i++){
                converter = Array[i] * multiplier;
                multiplier = multiplier/2;
                Assignment = converter + Assignment;
            }
            multiplier = 32;
            printf("\n Assignment = %d", Assignment);
            j++;
            printf("\n # of submissions so far = %d \n", j);
            printf("\n Waiting for next submission");
        }
            
        k=0;
        ID=0;
        Assignment=0;
    }
}
