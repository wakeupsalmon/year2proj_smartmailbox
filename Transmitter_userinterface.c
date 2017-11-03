#include <avr/interrupt.h>
#include <avr/io.h>

#define zero 0xFC
#define one 0x60
#define two 0xDA
#define three 0xF2
#define four 0x66
#define five 0xB6
#define six 0xBE
#define seven 0xE0
#define eight 0xFE
#define nine 0xE6


//The Array for calling the 7 seg number
unsigned int Array[10]={zero, one, two, three, four, five, six, seven, eight, nine};

//Pulse Width Modulation code from Ziqi and Juntian
//Setting up the Delay Function

void Delay (unsigned int Delay) {
    int i;
    for (i=0; i < Delay;i++){
        TCCR2A = 0;
        TCCR2B = (1<<CS12) | (1<<CS10);
        OCR2A = 114;
        TCNT2 = 0;
        TIFR2 = (1<<OCF2A);
        while ( !(TIFR2 & (1<<OCF2A)));
    }
    TCCR2B = 0;
}

void output(void) {
    TCNT0=0;
    TCCR0A=(1<<WGM01)|(1<<COM0A0);
    TCCR0B= (1<<CS00);
}

int main(void)
{
    //Set up the PWM
    //PWM, Phase and Frequency Correct
    TCCR1A=(1<<COM1A1); //This calls COM1A1
    TCCR1B= (1<<CS11) | (1<<WGM13); //This calls WGM13 and CS11
    TCNT1= 0;
    ICR1=18432;
    int open, closed; // FOR THE SERVO MOTOR
    open = 1148;
    closed = 1648;
    OCR1A=closed;
    //Data Direction Registers

    DDRC=0x00; //from comparator
    DDRA=0xFF; //7 seg
    DDRD = (1<<PD6) | (1<<PD5) ; //LEDS and servo

    //Variable Declaration and Initiliazation
    int click=0; //Debouncer for 1 press at a time
    int Command=0; //Button on the left, corresponds to Assignment
    int Assignment=20;
    int ID=0;
    int i;
    int transmit = 1;
    int Student=0;
    int once = 3;
    int Disable = 1;
    int Receive = 0;
    
    //The Part of the Code that Runs Over and Over
    while(1){
        transmit =1;
        while (transmit){

            while (PIND & (1<<PD1))//While the on Switch is set to the positive line
            {
                Student=0;
                click=0; //the click counter for single button presses - rises indefinitely but only adds to count on 100th millisecond
                Delay(1);
                while (PIND & (1<<PD4))// the ID button is pressed
                {
                    click++; // Software debounce and single click
                    Delay(1); //Ensure 100 clicks happen before displaying a new ID value to the user
                    if (click == 100){
                        if (ID > 9){
                            ID=0;
                        }
                        PORTA = Array[ID];
                        ID++; //Value of ID is now 1 greater than the value displayed
                        once=0;
                    }
                }
                while (PIND & (1<<PD6))//Assignment button is pressed
                {
                    click++;
                    Delay(1);
                    if (click == 100)
                    {
                        if ((Command>=5) && (ID!=1)){
                            Command=0;
                        }
                        if (Command > 9){
                            Command = 0;
                        }
                        once=0;
                        PORTA=Array[Command];
                        Command++; //This means that the real value of Command is 1 less - remember to subtract 1 from Command in next step
                    }
                }// End of button 5 (assignment) loop
            } // End of While Switch ON

            //Officially After the Switch is Opened and Next phase of User Interface Begins

            //The Maths behind the ID and Assignments
            // Must fix the values of ID and COMMAND since they're 1 too high
            if (once==0){
                ID--;
                Command--;
                once=4;
            }
            if (Command%2 == 0){
                Disable = 0;
            }
            if ((ID == 0) && (once==4) && (Student!=1)){
                Assignment = Command/2; // Function (integers round down, no need to subtract 1)
                PORTA = Array[Assignment]; //checking myself if the values worked
                Student=1;
            }
            click=0;


            // Still in the main loop but outside of math
            //If the teacher isn't using the box...
            
            if ((Command==Assignment) && (Disable == 0) && (Receive==0)){

                OCR1A = open;
                i=0;
                while(i<=10000){ // waiting for the assignment input for 10 seconds
                    i++;
                    Delay(1);

                    while (PINC & (1<<PC3)){//while there is no current entering the pin(LED covered)
                        click++; //Debounce
                        Delay(1);
                        if (click == 100){ // wait for the pin to be open for at least 100 ms
                            Delay(5000);
                            OCR1A=closed; // close after waiting for full paper submission
                            transmit=0;
                            i=10000;
                        }// end of the debouncer if

                    } // End of the while comparator = 0 loops
                }// End of the 10 second timer for statement
                OCR1A=closed; //close of the timer ran out
            }// End of the if ID is not 0 statement
            
        }// End bracket of the while 1 statement
        
        DDRB=(1<<PB3);
        TCNT0=0;
        TCCR0A=0x00;
        TCCR0B=0x00;
        int binary[12];
        binary[0]=1;
        
        for (i=6;i>=1;i--){
            binary[i]= ID%2;
            ID=ID/2;
        }
        for (i=12;i>=7;i--){
            binary[i]=Assignment%2;
            Assignment=Assignment/2;
        }
        //next part of code needs to read the array back and output a frequency
        output();
        for (i=0; i<=12 ; i++){
            if (binary[i] == 0){
                OCR0A=80;
            }
            if (binary[i] == 1){
                OCR0A=60;
            }
            Delay(1);
        }
        OCR0A = 200;
    }
            
}
