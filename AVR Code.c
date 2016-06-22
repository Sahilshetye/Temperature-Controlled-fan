
#include<avr/io.h> //standard AVR header
#include<util/delay.h> //delay header
#include<avr/interrupt.h> //interrupt header

#define F_CPU 12000000

#define KEY_PRT PORTB//keyboard PORT
#define KEY_DDR DDRB//keyboard DDR
#define KEY_PIN PINB//keyboard PIN

#define LCD_DPRT PORTD //LCD DATA PORT
#define LCD_DDDR DDRD //LCD DATA DDR
#define LCD_DPIN PIND //LCD DATA PIN
#define LCD_CPRT PORTC// LCD COMMANDS PORT
#define LCD_CDDR DDRC//LCD COMMANDS DDR
#define LCD_CPIN PINC//LCD COMMANDS PIN
#define LCD_RS 0 //LCD RS
#define LCD_RW 1 //LCD RW
#define LCD_EN 7 //LCD EN

void lcdinit(void);
void lcd_gotoxy(unsigned char,unsigned char );
void lcdprint(char*);
void lcdcommand(unsigned char);
void delay_us(unsigned int);
void delay_ms(unsigned int );
void lcddata(unsigned char);
unsigned char keypadcall(void);

unsigned char keypad[4][4]={'A','3','2','1',//keypad declaration as a 4x4 matrix
                             'B','6','5','4',
                             'C','9','8','7',
                             'D','#','0','*'};

void main(void)
{
  int ADCH_temp,x,y,a,b,c,d,e,ADC_temp=0,i;
    DDRA=0b00000110;                                                                      //PINA1 and PINA2 are outputs and the rest of the pins of PORTA are inputs 
    ADCSRA=0X87;                                                                               //make ADC enable and select clk/128*
    ADMUX=0XE0;                                                                                 //2.56Vref ,single ended channel ADC0,data will be left justified
    sei();                                                                                                   //enable interrupt
    lcdinit();                                                                                              //LCD initialisation function
    lcd_gotoxy(1,1);                                                                                //LCD cursor positioning function
    lcdprint("set temp=");                                                                      //LCD print function
    delay_us(1000);                                                                               //wait
    a= keypadcall();                                                                                //keypad call for the first digit of temperature
    b= keypadcall();                                                                                //keypad call for the second digit of temperature
    c=atoi(&a);                                                                                         //conversion from character to integer
    d=atoi(&b);                                                                                        //conversion from character to integer
    e=10*c+d;                                                                                          //the set temp entered  finally converted to integer form
      
     while(1)                                                                                             //stay here forever
   
     { ADC_temp=0;
        for(i=0;i<16;i++)                                                                            //do the ADC conversion 16 times to improve the accuracy
        {
         ADCSRA|=(1<<ADSC);                                                               //start conversion
       while((ADCSRA&(1<<ADIF))==0);                                              //wait for end of conversion,ADIF flag set
       ADCH_temp=ADCH;                                                                    //the variable holds the current temperature
       ADC_temp+=ADCH_temp;                                                         //accumulate results(16 samples)
        }
        ADC_temp=ADC_temp>>4;                                                       //average the 16 samples   
       x=ADC_temp/10+48;                            //required to convert the temperature(first digit) to ascii(decimal) value in order to display it on LCD
       y=ADC_temp%10+48;                         //required to convert the temperature(second digit) to ascii(decimal) value in order to display it on LCD
       
       lcd_gotoxy(1,2);
       lcddata(x);                                                                                       //pass the ascii value of first digit in order to display it on the LCD 
       lcddata(y);                                                                                       //pass the ascii value of second digit in order to display it on the LCD
       lcdprint("deg celcius");
       delay_us(1000);                                                                            //wait
       if(ADC_temp<=(e-2))                                                                    //if current temp is 2 degree celsius less than the set temperature
       PORTC=PORTC|0b01000000;                                                  //turn on the heater connected to PORTA6
       if(ADC_temp==e)                                                                          //if current temp is equal to the set temperature 
       {
           PORTC=PORTC&0b00000000;                                           //turn off the heater
           PORTA=PORTA&0b00000000;                                             //turn off the fan
       }
       if (ADC_temp>=(e+2))                                                                 //if current temp is 2 degree celsius more  than the set temperature
       PORTA=PORTA|0b00000100;                                                   //turn on the fan
       
       
     }         
    
    
}

void lcdinit()
{
    LCD_DDDR= 0XFF;
    LCD_CDDR= 0XFF;
    LCD_CPRT &=~(1<<LCD_EN);                                                     //LCD_EN=0
    delay_us(2000);//wait for init
    lcdcommand(0X38);                                                                         //init LCD 2 line, 5X7 matrix
    lcdcommand(0x0E);                                                                         //display on,cursor on
    lcdcommand(0X01);                                                                         //clear LCD
    delay_us(2000);//wait
    lcdcommand(0X06);                                                                         //shift cursor right
}

void lcdcommand(unsigned char cmnd)
{
    LCD_DPRT=cmnd;                                                                         //send cmnd to data port
    LCD_CPRT&=~(1<<LCD_RS);                                                    //RS=0 for command
    LCD_CPRT&=~(1<<LCD_RW);                                                   //RW=0 for write
    LCD_CPRT|=(1<<LCD_EN);                                                        //EN=1 for H-to-L pulse
    delay_us(1);                                                                                      //wait to make enable wide
    LCD_CPRT&=~(1<<LCD_EN);                                                    //EN=0 for H-to-L pulse
    delay_us(100);                                                                                 //wait to make enable wide
    
}


void delay_us(unsigned int d)//delay in micro secs
{
    _delay_us(d);
}

void delay_ms(unsigned int d)/*delay in milli secs*/
{
    
    _delay_ms(d);
}


void lcd_gotoxy(unsigned char x,unsigned char y)
{
    
    unsigned char firstcharadr[]={0x80,0xC0,0x94,0xD4};
        lcdcommand(firstcharadr[y-1]+x-1);
        delay_us(100);
}

void lcdprint(char *str)
{
    
    unsigned char i=0;
    while(str[i]!=0)
    {
        lcddata(str[i]);
        i++;
    }
    
}

void lcddata(unsigned char data)
{
    LCD_DPRT=data;                                                                        //send data to data port
    LCD_CPRT|=(1<<LCD_RS);                                                     //RS=1 for data
    LCD_CPRT&=~(1<<LCD_RW);                                                //RW=0 for write
    LCD_CPRT|=(1<<LCD_EN);                                                     //EN=1 for H-to-L pulse
    delay_us(1);                                                                                  //wait to make enable wide
    LCD_CPRT&=~(1<<LCD_EN);                                                //EN=0 for H-to-L pulse
    delay_us(100);                                                                             //wait to make enable wide
}

unsigned char keypadcall(void)
{
    unsigned char colloc,rowloc;
                                                                                                             //keyboard routine.this sends the ASCII code for the pressed key to PORTB
    KEY_DDR=0XF0;
    KEY_PRT=0XFF;
    while(1)                                                                                          //repeat forever
    {do 
    {
        KEY_PRT &=0X0F;                                                                  //ground all rows at once
        colloc=(KEY_PIN&0X0F);                                                       //read columns
    }while(colloc!=0X0F);                                                                  //check until all keys released
    
    do 
    {
        do 
        {    delay_ms(20);                                                                      //call delay
            colloc=(KEY_PIN&0X0F);                                                    //see if any key is pressed
        } while (colloc==0X0F);                                                            //keep checking for key press
        
        delay_ms(20);                                                                          //call delay for debounce
        colloc=(KEY_PIN&0X0F);                                                       //read columns
    } while (colloc==0X0F);                                                               //wait for key press
    
    while(1)
    
    {KEY_PRT=0XEF;//ground rows
    colloc=(KEY_PIN&0X0F);                                                           //read all the columns
    if(colloc!=0X0F)                                                                             //col detected
    {
        rowloc=0;                                                                                  //save row loc
        break;                                                                                        //exit while loop
    }
    
    
    KEY_PRT=0XDF;                                                                        //ground row1
    colloc=(KEY_PIN&0X0F);                                                         //read the columns
    if(colloc!=0X0F)                                                                           //col detected
    {
        rowloc=1;                                                                                //save row loc
        break;                                                                                      //exit while loop
    }
    
    KEY_PRT=0XBF;                                                                      //ground row 2
    colloc=(KEY_PIN&0X0F);                                                       //read the columns
    if(colloc!=0X0F)                                                                        //col detected
    {
        rowloc=2;                                                                              //save row loc
        break;                                                                                    //exit while loop
    }
    
    KEY_PRT=0X7F;                                                                     //ground row 3
    colloc=(KEY_PIN&0X0F);                                                       //read the columns
    rowloc=3;                                                                                   //save row loc
    break;//exit while loop
    }
                                                                                                          //check column and send the result for display on LCD,
                                                                                                          // also return the value to main function in order to convert the character into integer
    if (colloc==0X0E)
    {lcddata(keypad[rowloc][0]);
    return(keypad[rowloc][0]);
    }    
        
    else if (colloc==0x0D)
    {lcddata(keypad[rowloc][1]);
    return(keypad[rowloc][1]);
    }    
    
    else if (colloc==0X0B)
    {lcddata(keypad[rowloc][2]);
    return(keypad[rowloc][2]);
    }    
    else
    {lcddata(keypad[rowloc][3]);
    return(keypad[rowloc][3]);
    }    
    
    break;//exit while loop
    }    
    


    
} 