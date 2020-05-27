#include "all.h"
#include "ds13072.h"
#include "i2c2.h"
#include <stdlib.h>

__eeprom __no_init u8 LEADZERO;   //показывать ли "0" в часу
__eeprom __no_init u8 SEPARATOR;  //показывать ли неоновые разделители
__eeprom __no_init u8 DELAY;      //время каждого режима в Mix

volatile u8 button;                             //флаги установки кнопок  
volatile u8 protectKeys[3]={0,0,0};             //"запиратели" кнопок
volatile u8 timerButton[2]={0,0};               //счетчик нажатий кнопки + -
volatile u16 setCountBtn=0;                     //счетчик нажатий кнопки set

u8 H, M, S,  d, m, y;             //час мин сек   число месяц год

volatile u8 out[9] = {10, 10, 10, 10, 10, 10};  //число на каждую лампу
volatile u8 changedDigit[6];                    // какой разряд изменился
u8 lev[6] = {255, 1, 1, 255, 255, 255};
u8 tempScreen[6]; 
u8 brig[2] = {1, 250};

volatile u8 outR[6], outG[6], outB[6];          //текущий цвет
u8 targetR[6], targetG[6], targetB[6];          //целевой цвет
u8 reachR = 0,  reachG = 0,  reachB = 0;        //флаги равности текущего и целевого цвета
u8 speedR = 0,  speedG = 0,  speedB = 0;        //++ или -- чтоб достичь целевого цвета
u8 R = 0, G = 0, B = 60;

volatile u8 flagBin=0;                          // для смены бинарных
volatile u8 set = 0;                            //режим настройки

u8 lightType=0;                                 //тип подсветки RGB
u16 hsv_temp=0;
u8 binaryType=0;                                //тип бинарных часов
u8 smoothVar=0;                                 //время для плавного изменения RGB
u8 hzVar=0;                                     //для режима Mix - кол-во секунд  

//счетчики
volatile u16 waitTimer=0;     //таймер паузы
volatile u8 smoothTimer=0;    //таймер времени плавного перехода RGB
  u8 counter = 0;             //для вывода на 6 цифр
  u8 dig = 0;
u8 Hz2 = 0;                   //1 раз обновление времени в секунду
volatile u8 junction = 0;     //циклы для плавного изменения цифр
volatile u8 ms = 0;           //время для плавного изменения цифр

//******************************************************************************
//******************************************************************************
__flash u8 dayBright[25]={1,21,42,63,84,105,126,147,168,189,210,231,245,231,210,189,168,147,126,105,84,63,42,21,1}; // ступени яркости день
__flash u8 nightBright[25]={105,116,127,138,149,162,175,186,198,210,222,234,245,234,222,210,198,186,175,162,149,138,127,116,105}; // ступени яркости ночь

/*__flash u8 bright[65]=
{ 
  0,1,2,3,4,5,6,7,8,9,10,12,14,16,18,20,
  23,26,29,31,34,37,40,42,44,46,49,51,53,56,59,61,
  64,67,70,72,75,79,82,85,89,92,96,100,104,108,112,117,
  122,127,132,137,143,149,156,163,171,179,188,198,209,222,236,253,255
};*/

__flash u8 TABLE[][18] = {
{ 1,0,0, 1,0,0, 1,0,0, 1,0,1, 1,0,1, 1,1,0}, //FIRST COLOR
{ 0,1,0, 0,1,0, 0,1,0, 1,1,0, 1,1,1, 0,0,1}, //FIRST COLOR
{ 0,0,1, 0,0,1, 0,0,1, 0,1,1, 0,1,0, 1,1,1}, //FIRST COLOR
{ 0,1,1, 0,0,0, 0,1,1, 0,1,0, 0,1,1, 1,0,1}, //SECOND COLOR
{ 1,0,0, 0,0,1, 1,0,1, 0,0,1, 1,1,0, 1,1,0}, //SECOND COLOR
{ 0,0,0, 1,1,0, 1,1,0, 1,0,0, 1,0,1, 0,1,1}  //SECOND COLOR
}; //таблица контрастов

#define ZERO  for(u8 l=0; l<6; l++){\
              outR[l]=0;\
              outG[l]=0;\
              outB[l]=0;}  
//******************************************************************************
//******************************************************************************

void Initialization()
{
       //PORT init
        DDRA = 0xEF;  DDRB = 0x0B;  DDRC = 0xFF;  DDRD = 0x7B;
        PORTA = 0x00; PORTB = 0x00; PORTC = 0x00; PORTD = 0x04;
       //инициализация DS1307
        ds1307_Init(); 
       //PWM
        TCCR0 |= (1<<WGM00)|(1<<WGM01)|(0<<COM00)|(1<<COM01)|(1<<CS00);
        TCCR1A |= (1<<COM1A1)|(1<<COM1B1)|(0<<COM1A0)|(0<<COM1B0)|(1<<WGM10)|(0<<WGM11);
        TCCR1B |= (1<<WGM12)|(0<<WGM13)|(0<<CS12)|(0<<CS11)|(1<<CS10);   
       //Timer2 - прерывания
        TCCR2 |= (1<<CS20)|(1<<CS21);
        TIMSK |= (1<<OCIE2)|(1<<TOIE2);        
                          //rtc_set_time(15, 46,37);
                          //rtc_set_date(19, 4, 13);
        __enable_interrupt();
              
        ZERO;        
        _delay_ms(500);
              
        set = 0; //обычный режим
        rtc_get_date(&d, &m, &y); //?чтение даты для нормально работы режимов отображения  
        rtc_get_time(&H, &M, &S);
}

//ПЛАВНАЯ СМЕНА
void soft(char D1, char D2, char D3) { // эффект плавной смены цифр, здесь идет поиск тех цифр, которые надо менять
        u8 i;
        
        if(set == 0){
              if(!LEADZERO && D1 < 10) tempScreen[0] = 10;
              else tempScreen[0] = D1 / 10; 
              tempScreen[1] = D1 % 10;    tempScreen[2] = D2 / 10;
              tempScreen[3] = D2 % 10;    tempScreen[4] = D3 / 10;
              tempScreen[5] = D3 % 10;
        
                      for (i = 0; (i < 6); i++) // сравнили с тем, что было
                              if (tempScreen[i] != out[i]) // узнаём в каком разряде поменялась цифра
                                      changedDigit[i] = 1; // цифра изменилась
                              else
                                      changedDigit[i] = 0; // цифра осталась
              junction = 0;
        }else{
               for (i = 0; (i < 6); i++) changedDigit[i] = 0; // во время настройки часов эффект отключен
        }
}
  

void sprint(char D1, char D2, char D3) { // эффект "пробежки" цифр
	tempScreen[0] = D1 / 10;    tempScreen[1] = D1 % 10;
	tempScreen[2] = D2 / 10;    tempScreen[3] = D2 % 10;
	tempScreen[4] = D3 / 10;    tempScreen[5] = D3 % 10;

        for (u8 i = 0; i < 6; i++){ // сравниваем текущую цифру с новой
            if (tempScreen[i] > out[i]) changedDigit[i] = 2; // новая больше
            if (tempScreen[i] < out[i]) changedDigit[i] = 3; // новая меньше

            for (u8 j = 0; j < 9; j++){ // делаем "пробежку"
                if(changedDigit[i] == 2) {if(out[i] != tempScreen[i]) out[i]++;} // новая цифра больше, плюсуем
                if(changedDigit[i] == 3) {if(out[i] != tempScreen[i]) out[i]--;} // новая цифра меньше, вычитаем
                if(out[i] == tempScreen[i]) break; //  // новая цифра равна, возвращаемся в начало

                _delay_ms(70);
            }
        }
}

  
void shift_l(u16 T){ // эффект "сдвиг влево"
        
	for(volatile u8 i = 5; i > 0; i--){   // сдвигаем влево текущие цифры
            out[0] = out[1];    out[1] = out[2];
            out[2] = out[3];    out[3] = out[4];
            out[4] = out[5];    out[i] = 10;

            _delay_ms(50);       //время сдвига времени))
	}
	out[0] = 10;

	for(volatile u8 i = 0; i <= 3; i++){  // запоминаем T и двигаем ее на нужное место
            out[5-i] = T/100;   out[6-i] = T/10%10;
            out[7-i] = T%10;    out[8-i] = 10;

            _delay_ms(50);       //время сдвига цифры T
	}
        
        _delay_ms(200);           //задержка показа цифры T
}

  
void shift_r(){ // эффект "сдвиг вправо"
	if(out[0] == 0){ // сдвигаем вправо текущие цифры
	    for(u8 i = 0; i <= 5; i++) // для отрицательной температуры
            { 
                out[5] = out[4];    out[4] = out[3];
                out[3] = out[2];    out[2] = out[1];
                out[1] = out[0];    out[i] = 10;
                _delay_ms(50);
	    }
	}
        else
        {
            for(u8 i = 0; i <= 3; i++) // для положительной температуры
            {
                out[5] = out[4];    out[4] = out[3];
                out[3] = out[2];    out[2] = out[1];

                out[i] = 10;
                _delay_ms(50);
            }
	}

	rtc_get_time(&H, &M, &S); // получаем время

	tempScreen[0] = H / 10;   tempScreen[1] = H % 10;
	tempScreen[2] = M / 10;   tempScreen[3] = M % 10;
	tempScreen[4] = S / 10;   tempScreen[5] = S % 10;

	for(u8 i = 0; i <= 5; i++) // двигаем время вправо
        { 
            out[5] = out[4];    out[4] = out[3];
            out[3] = out[2];    out[2] = out[1];
            out[1] = out[0];    out[0] = tempScreen[5-i];
            _delay_ms(50);
	}
}

// получаем время с DS1307
void get_time(){       
        rtc_get_time(&H, &M, &S);
        soft(H, M, S); // вызываем плавную смену цифр   
}

// получаем дату с DS1307
void get_date(){        
        rtc_get_date(&d, &m, &y); 
            PORTA_Bit5=0; PORTC_Bit6=0;
            for(u8 i=0;i<6;i++) lev[i] = brig[1];   
            ZERO;
        sprint(d, m, y); 
          _delay_s(2);
          waitTimer = 0;
}

//  Преобразование HSL->RGB
static void HSVtoRGB( u8 *r, u8 *g, u8 *b, u16 i_hue )
{
        u16 ii;
        double fr, hue;
        u8 c2, c3;
        
        while (i_hue >= 360) i_hue -= 360;
        
        hue = i_hue;
        ii = (int)(hue /= 60.0);
        fr = hue - ii;
        c2 = (u8)(255 - 255 * fr);
        c3 = (u8)(255 * fr);
        
        switch (ii)
        { 
            case 0: *r = 255; *g = c3;  *b = 0;   break;
            case 1: *r = c2;  *g = 255; *b = 0;   break;
            case 2: *r = 0;   *g = 255; *b = c3;  break;
            case 3: *r = 0;   *g = c2;  *b = 255; break;
            case 4: *r = c3;  *g = 0;   *b = 255; break;
            case 5: *r = 255; *g = 0;   *b = c2;  break;
        }
}
//------------------------------------------------------------------------------------

void BinarySeconds( u8 num, u8 bit, u8 mode ) //SS - LED bin.format  bit:[0-5] mode:[0-11]
{ 
        u8 vv=5-bit;

        if(BitIsSet(num, vv)) targetR[bit] = TABLE[0][mode]*255;  
          else targetR[bit] = TABLE[3][mode]*255;
        if(BitIsSet(num, vv)) targetG[bit] = TABLE[1][mode]*255;  
          else targetG[bit] = TABLE[4][mode]*255;
        if(BitIsSet(num, vv)) targetB[bit] = TABLE[2][mode]*255;  
          else targetB[bit] = TABLE[5][mode]*255;
               
        if(outR[bit] > targetR[bit]) ClearBit(speedR, bit); else SetBit(speedR, bit); //set direction
        if(outG[bit] > targetG[bit]) ClearBit(speedG, bit); else SetBit(speedG, bit); 
        if(outB[bit] > targetB[bit]) ClearBit(speedB, bit); else SetBit(speedB, bit);        
}

//цветовая гамма
void HSVchange(u8 dir)           
{
        if(dir)
        {
              hsv_temp += 2;
              if(hsv_temp>=360) hsv_temp=0;
              HSVtoRGB(&R,&G,&B,hsv_temp);
        }else{
              if(hsv_temp ==0) hsv_temp=360; else hsv_temp -= 2;
              HSVtoRGB(&R,&G,&B,hsv_temp);
        } 
}

//плавное изменение яркости подсветки
void Smooth( u8 load )
{
        if( smoothTimer == 0 )
        {
              for(u8 x=0; x<6; x++)
              {
                    if(outR[x] == targetR[x]) SetBit(reachR, x); else ClearBit(reachR, x);
                    if(outG[x] == targetG[x]) SetBit(reachG, x); else ClearBit(reachG, x);
                    if(outB[x] == targetB[x]) SetBit(reachB, x); else ClearBit(reachB, x);
                    
                    if(BitIsClear(reachR, x)) //если цвет не достигнут на "х" канале
                    {
                        if(BitIsSet(speedR, x)) outR[x]++; //++
                           else outR[x]--; //--
                           
                        if ((BitIsSet(speedR, x)) && ( outR[x] >= targetR[x] )) 
                            { outR[x] = targetR[x]; SetBit(reachR,x); }  //цвет достигнут 
                        if ((BitIsClear(speedR, x)) && ( outR[x] <= targetR[x] )) 
                            { outR[x] = targetR[x]; SetBit(reachR,x); }  //цвет достигнут 
                    }
                    if(BitIsClear(reachG, x)) //если цвет не достигнут на "х" канале
                    {
                        if(BitIsSet(speedG, x)) outG[x]++; //++
                           else outG[x]--; //--
                           
                        if ((BitIsSet(speedG, x)) && ( outG[x] >= targetG[x] ))
                            { outG[x] = targetG[x]; SetBit(reachG,x); }  //цвет достигнут 
                        if ((BitIsClear(speedG, x)) && ( outG[x] <= targetG[x] )) 
                            { outG[x] = targetG[x]; SetBit(reachG,x); }  //цвет достигнут 
                    }
                    if(BitIsClear(reachB, x)) //если цвет не достигнут на "х" канале
                    {
                        if(BitIsSet(speedB, x)) outB[x]++; //++
                           else outB[x]--; //--
                           
                        if ((BitIsSet(speedB, x)) && ( outB[x] >= targetB[x] ))
                            { outB[x] = targetB[x]; SetBit(reachB,x); }  //цвет достигнут 
                        if ((BitIsClear(speedB, x)) && ( outB[x] <= targetB[x] )) 
                            { outB[x] = targetB[x]; SetBit(reachB,x); }  //цвет достигнут 
                    }
                    
              }
              smoothTimer = load;
        }
}

//все LED подсветки - один случайный цвет
void AllRandom()
{
        if(!waitTimer && !smoothVar)
        {
            HSVtoRGB(&R, &G, &B , (rand() % 360));
            for(u8 bit=0; bit<6; bit++){
                targetR[bit]=R; targetG[bit]=G; targetB[bit]=B;
                if(outR[bit] > targetR[bit]) ClearBit(speedR, bit); else SetBit(speedR, bit);
                if(outG[bit] > targetG[bit]) ClearBit(speedG, bit); else SetBit(speedG, bit); 
                if(outB[bit] > targetB[bit]) ClearBit(speedB, bit); else SetBit(speedB, bit);
            }
            smoothVar = rand() % 30;
            Smooth(smoothVar);  
        }
        
        if(reachR==0x3F && reachG==0x3F && reachB==0x3F && !waitTimer) //ожидание без изменения цвета
        {
            do{ 
               waitTimer = (rand() % 3492); 
            }while (waitTimer < 1000); 
            smoothVar = 0;
        }
}

//выбирается 1 случайный LED и на нем меняется случайный цвет
void EachRandom()
{
        if(!waitTimer && !smoothVar)
        {
            u8 shoot;
            
            HSVtoRGB(&R, &G, &B , (rand() % 360));
            shoot = rand() % 6;
            for(u8 bit=0; bit<6; bit++){
                 if(shoot== bit) {targetR[bit]=R; targetG[bit]=G; targetB[bit]=B;}
                  if(outR[bit] > targetR[bit]) ClearBit(speedR, bit); else SetBit(speedR, bit);
                  if(outG[bit] > targetG[bit]) ClearBit(speedG, bit); else SetBit(speedG, bit); 
                  if(outB[bit] > targetB[bit]) ClearBit(speedB, bit); else SetBit(speedB, bit);
            }
            smoothVar = rand() % 20;
            Smooth(smoothVar);  
        }
        
        if(reachR==0x3F && reachG==0x3F && reachB==0x3F && !waitTimer) //ожидание без изменения цвета
        {
            do{ 
               waitTimer = (rand() % 1000); 
            }while (waitTimer < 300); 
            smoothVar = 0;
        }
}

//выбирается 1 случайный LED и на нем меняется фиксированный цвет
void EachRandomFixed()
{
        if(!waitTimer && !smoothVar)
        {
            u8 choose, shoot;       
            choose=rand()%6; choose += 6;
            shoot=rand()%6;
            
            for(u8 bit=0; bit<6; bit++){
                if(shoot == bit) {
                    targetR[bit]=TABLE[0][choose]*255; 
                    targetG[bit]=TABLE[1][choose]*255; 
                    targetB[bit]=TABLE[2][choose]*255;
                }
                if(outR[bit] > targetR[bit]) ClearBit(speedR, bit); else SetBit(speedR, bit);
                if(outG[bit] > targetG[bit]) ClearBit(speedG, bit); else SetBit(speedG, bit); 
                if(outB[bit] > targetB[bit]) ClearBit(speedB, bit); else SetBit(speedB, bit);
            }
            smoothVar = rand()%20;
            Smooth(smoothVar);  
        }
        
        if(reachR==0x3F && reachG==0x3F && reachB==0x3F && !waitTimer) //ожидание без изменения цвета
        {
            do{ 
                waitTimer = (rand() % 1200); 
            } while (waitTimer < 200); 
            smoothVar=0;
        }
}

//LED подсветки поочередно меняются случайным цветом
void ShiftColor()
{
        static u8 bit=0;
        if(!waitTimer && !smoothVar)
        {
            HSVtoRGB(&R, &G, &B , (rand() % 360));
            
            targetR[bit]=R; targetG[bit]=G; targetB[bit]=B;
            for(u8 bit=0; bit<6; bit++){        
                    if(outR[bit] > targetR[bit]) ClearBit(speedR, bit); else SetBit(speedR, bit);
                    if(outG[bit] > targetG[bit]) ClearBit(speedG, bit); else SetBit(speedG, bit); 
                    if(outB[bit] > targetB[bit]) ClearBit(speedB, bit); else SetBit(speedB, bit);
            }
            smoothVar = (rand() % 15) + 5;
            Smooth(smoothVar);  
            if(++bit == 6) bit = 0;
        }
        
        if(reachR==0x3F && reachG==0x3F && reachB==0x3F && !waitTimer) //ожидание без изменения цвета
        {
            /*do{ 
             waitTimer = (rand() % 1000); 
            } while (waitTimer < 200); */
            waitTimer = 40;
            smoothVar = 0;
        }
}

//все режимы по порядку
void Mix()
{
        static u8 mixVar=1, binColor=0;
        switch(mixVar)
        {
            case 1: if(flagBin) {for(u8 f=0; f<6; f++) BinarySeconds(S, f, binColor);} 
                    flagBin=0; Smooth(2); break;
            case 2: AllRandom();        if(!waitTimer) Smooth(smoothVar); break; 
            case 3: EachRandom();       if(!waitTimer) Smooth(smoothVar);  break; 
            case 4: EachRandomFixed();  if(!waitTimer) Smooth(smoothVar); break; 
            case 5: ShiftColor();       if(!waitTimer) Smooth(smoothVar); break; 
        }
        
        if(hzVar == DELAY)
        {
            if(++mixVar == 7) mixVar = 1;
            if(mixVar == 1) binColor = rand() % 18;
            smoothVar = 0; smoothTimer = 0;
            hzVar = 0;
        }
}

// режим настройки часов
void Settings(){ 
	if(set < 4  &&  SQW_IN){ // нужно для вывода данных во время настройки
		out[0] = H/10; out[1] = H%10;
		out[2] = M/10; out[3] = M%10;
		out[4] = S/10; out[5] = S%10;
	}
	if(set > 3 && set <7  &&  SQW_IN) {
		out[0] = d/10; out[1] = d%10;
		out[2] = m/10; out[3] = m%10;
		out[4] = y/10; out[5] = y%10;
	}
        if(set > 6  &&  SQW_IN) {
		out[0] = LEADZERO; out[1] = SEPARATOR;
		out[2] = DELAY/10; out[3] = DELAY%10; //out[2] = m/10; out[3] = m%10;
		out[4] = 10; out[5] = 10; //out[4] = y/10; out[5] = y%10;
	}

	switch(set) // включена настройка
	{
	case 1: // настройка часов
                if(SQW_IN){out[0] = H/10; out[1] = H%10;} // мигаем часами
                 else{out[0] = 10; out[1] = 10;}
          
		if(BitIsSet(button, 2)){ 
                  H++; if(H > 23) H = 0;   protectKeys[2]=5; out[0] = H/10; out[1] = H%10; button &= ~(1<<2);}
		if(BitIsSet(button, 1)){ 
                  if(!H) H = 23; else H--; protectKeys[1]=5; out[0] = H/10; out[1] = H%10; button &= ~(1<<1);}
	break;

	case 2: // настройка минут
                if(SQW_IN){out[2] = M/10; out[3] = M%10;} // мигаем минутами
                 else{out[2] = 10; out[3] = 10;}
                
		if(BitIsSet(button, 2)){ 
                  M++; if(M > 59) M = 0;   protectKeys[2]=5; out[2] = M/10; out[3] = M%10; button &= ~(1<<2);}
		if(BitIsSet(button, 1)){ 
                  if(!M) M = 59; else M--; protectKeys[1]=5; out[2] = M/10; out[3] = M%10; button &= ~(1<<1);}
        break;
        
        case 3: // настройка секунд
                if(SQW_IN){out[4] = S/10; out[5] = S%10;} // мигаем секундами
		 else{out[4] = 10; out[5] = 10;}
                
		if(BitIsSet(button, 2)){ 
                  S=0; protectKeys[2]=5; out[4] = S/10; out[5] = S%10; button &= ~(1<<2);}
		if(BitIsSet(button, 1)){ 
                  S=0; protectKeys[1]=5; out[4] = S/10; out[5] = S%10; button &= ~(1<<1);}
        break;

	case 4: // настройка дня
                if(SQW_IN){out[0] = d/10; out[1] = d%10;} // мигаем днем
		 else{out[0] = 10; out[1] = 10;}
                
		if(BitIsSet(button, 2)){ 
                  d++; if(d > 31) d = 1; out[0] = d/10; out[1] = d%10; protectKeys[2]=5; button &= ~(1<<2);}
		if(BitIsSet(button, 1)){ 
                  d--; if(d < 1) d = 31; out[0] = d/10; out[1] = d%10; protectKeys[1]=5; button &= ~(1<<1);}
        break;

	case 5: // настройка месяца
                if(SQW_IN){ out[2] = m/10; out[3] = m%10;} // мигаем месяцем
		 else{ out[2] = 10; out[3] = 10;}
                
		if(BitIsSet(button, 2)){
                  m++; if(m > 12) m = 1; out[2] = m/10; out[3] = m%10; protectKeys[2]=5; button &= ~(1<<2);}
		if(BitIsSet(button, 1)){
                  m--; if(  !m ) m = 12; out[2] = m/10; out[3] = m%10; protectKeys[1]=5; button &= ~(1<<1);}
	break;

	case 6: // настройка года
                if(SQW_IN){ out[4] = y/10; out[5] = y%10;} // мигаем годом
		 else{ out[4] = 10; out[5] = 10;}
                 
		if(BitIsSet(button, 2)){
                  y++; out[4] = y/10; out[5] = y%10; protectKeys[2]=5; button &= ~(1<<2);}
		if(BitIsSet(button, 1)){
                  y--; out[4] = y/10; out[5] = y%10; protectKeys[1]=5; button &= ~(1<<1);}
        break;
        
        case 7: // настройка значащего нуля
                if(SQW_IN){ out[0] = LEADZERO;}
		 else{ out[0] = 10;}
                 
		if(BitIsSet(button, 2)){
                  LEADZERO=1; out[0] = 1; protectKeys[2]=5; button &= ~(1<<2);}
		if(BitIsSet(button, 1)){
                  LEADZERO=0; out[0] = 0; protectKeys[1]=5; button &= ~(1<<1);}
        break;
        
        case 8: // настройка flash/light неоновых разделителей
                if(SQW_IN){ out[1] = SEPARATOR;}
		 else{ out[1] = 10;}
                 
		if(BitIsSet(button, 2)){
                  SEPARATOR=1; out[1] = 1; protectKeys[2]=5; button &= ~(1<<2);}
		if(BitIsSet(button, 1)){
                  SEPARATOR=0; out[1] = 0; protectKeys[1]=5; button &= ~(1<<1);}
        break;
        
        case 9: // настройка flash/light неоновых разделителей
                if(SQW_IN){ out[2] = DELAY/10; out[3] = DELAY%10;}
		 else{ out[2] = 10; out[3] = 10;}
                 
		if(BitIsSet(button, 2)){
                  if(DELAY < 90) DELAY++; out[2] = DELAY/10; out[3] = DELAY%10; protectKeys[2]=5; button &= ~(1<<2);}
		if(BitIsSet(button, 1)){
                  if(DELAY > 5) DELAY--;  out[2] = DELAY/10; out[3] = DELAY%10; protectKeys[1]=5; button &= ~(1<<1);}
        break;
	}
}


void Buttons()
{
       // если нажата кнопка set
        if(BitIsSet(button, 0) && !set)
        {  
            if(++lightType == 8) lightType=0;
            
            if(lightType == 1) HSVtoRGB(&R,&G,&B,hsv_temp);
            if(lightType > 2 && lightType < 7) {smoothVar=0; smoothTimer=0;}
            if(!lightType) {ZERO;}
            button &= ~(1<<0); 
            shift_l(lightType);
            shift_r();
            protectKeys[0]=5;      
        }
        
       // если нажата кнопка set во время установки        
                  if(BitIsSet(button, 0) && set){              
                    if(set<4) rtc_set_time(H, M, S);          //Write DS1307 time
                    if(set<7 && set>3) rtc_set_date(d, m, y); //Write DS1307 date
                    set++;
                        ZERO;
                        switch(set){
                            case 1: outB[0]=255; outB[1]=255; break;
                            case 2: outG[2]=255; outG[3]=255; break;
                            case 3: outR[4]=255; outR[5]=255; break;
                            
                            case 4: outB[0]=255; outB[1]=255; outR[0]=255; outR[1]=255; break;
                            case 5: outG[2]=255; outG[3]=255; outR[2]=255; outR[3]=255; break;
                            case 6: outG[4]=255; outG[5]=255; outB[4]=255; outB[5]=255; break;
                            
                            case 7: outB[0]=255; break;
                            case 8: outR[1]=255; outG[1]=255; break;
                            case 9: outB[2]=255; outG[3]=255; break;
                        }
                            if(set > 3 && set <7) {
                              out[0] = d/10; out[1] = d%10;
                              out[2] = m/10; out[3] = m%10;
                              out[4] = y/10; out[5] = y%10;
                            }
                            if(set > 6) {
                              out[0] = LEADZERO; out[1] = SEPARATOR;
                              out[2] = DELAY/10; out[3] = DELAY%10; 
                              out[4] = 10; out[5] = 10; //out[4] = y/10; out[5] = y%10;
                            }
                    if(set == 10) set=0; 
                    button &= ~(1<<0); protectKeys[0]=5;
                  }
       //кнопка -
        if(BitIsSet(button, 1) && !set)
        {  
            switch(lightType)
            {
                case 0: button &= ~(1<<1); protectKeys[1]=10; get_date(); break;
                case 1: binaryType++; if(binaryType==18) {binaryType=0;} button &= ~(1<<1); 
                                      for(u8 f=0; f<6; f++) BinarySeconds(S, f, binaryType);  
                                      protectKeys[1]=5; break;
                case 2: HSVchange(0); button &= ~(1<<1); break;
                case 3: button &= ~(1<<1); protectKeys[1]=10; get_date(); break;
                case 4: button &= ~(1<<1); protectKeys[1]=10; get_date(); break;
                case 5: button &= ~(1<<1); protectKeys[1]=10; get_date(); break;
                case 6: button &= ~(1<<1); protectKeys[1]=10; get_date(); break;
                case 7: button &= ~(1<<1); protectKeys[1]=10; get_date(); break;
            }
        }
       //кнопка + 
        if(BitIsSet(button, 2) && !set)
        {  
            switch(lightType)
            {
                case 0: button &= ~(1<<2); protectKeys[2]=10; get_date(); break; 
                case 1: if(binaryType==0) {binaryType=17;}
                        else{binaryType--;} button &= ~(1<<2);
                        for(u8 f=0; f<6; f++) BinarySeconds(S, f, binaryType); 
                        protectKeys[2]=5; break;
                case 2: HSVchange(1); button &= ~(1<<2); break;
                case 3: button &= ~(1<<2); protectKeys[2]=10; get_date(); break;
                case 4: button &= ~(1<<2); protectKeys[2]=10; get_date(); break;
                case 5: button &= ~(1<<2); protectKeys[2]=10; get_date(); break;
                case 6: button &= ~(1<<2); protectKeys[2]=10; get_date(); break;
                case 7: button &= ~(1<<2); protectKeys[2]=10; get_date(); break;
            }
        }
}

void Display()
{
        if(!set)
        {
            switch(lightType)
            {
                case 1: if(flagBin){for(u8 f=0; f<6; f++) BinarySeconds(S, f, binaryType);} 
                        flagBin=0; Smooth(2); break;
                case 2: for(u8 k=0; k<6; k++) {outR[k]=R; outG[k]=G; outB[k]=B;} break;
                case 3: AllRandom(); if(!waitTimer) Smooth(smoothVar); break; 
                case 4: EachRandom(); if(!waitTimer) Smooth(smoothVar);  break; 
                case 5: EachRandomFixed(); if(!waitTimer) Smooth(smoothVar); break; 
                case 6: ShiftColor(); if(!waitTimer) Smooth(smoothVar); break; 
                case 7: Mix(); break;
            }
        }
        
        if(SEPARATOR && !set) {PORTA_Bit5=1; PORTC_Bit6=1;} //неоновые разделители
        else                  {PORTA_Bit5=0; PORTC_Bit6=0;}       
}

//===================================================================
//===================    M    A    I    N    ========================
//===================================================================
void main()
{
        Initialization(); 
     
        while(1)
        {         
          if(set) Settings(); 
          if(SQW_IN && set == 0)  //сигнал с частотой 1ГЦ
          { 
                if(Hz2 < 1)
                {
                      Hz2++; //delay++;
                     // установка уровня яркости
                      if(H < 7) {brig[0] = 0; brig[1] = 105;} // ночь
                      else {brig[0] = 1; brig[1] = 10;} // день  
                      
                      get_time();
                      if(lightType==7) hzVar++;              
                }        
          }
          else Hz2 = 0;
        
          Display();
          Buttons();  
        }
}


//==============================================================================
//=================  I  N  T  E  R  R  U  P  T  S  =============================
//==============================================================================
#pragma vector = TIMER2_OVF_vect 
__interrupt void Over() // реализация ШИМ управления яркостью ламп и подсветки
{
    static u8 pwmCnt = 0;         
        
          //скидываем дешифратор на "OUT disable"
        PORTC_Bit4=1; PORTC_Bit2=0; PORTC_Bit3=1; PORTC_Bit5=0;
        PORTA_Bit1=1; PORTA_Bit3=0; PORTA_Bit2=1; PORTA_Bit0=0;
        
        OCR2 = lev[dig]; // подготавливаемся к выводу следуйщего разряда 255=min. 1=max
        
        if (junction < 25){ // плавная смена яркости цифр
            if (++ms == 40) // 18 = ~38ms
            { 
                ms = 0;
                for (u8 i = 0; i < 6; i++) 
                {
                    if ((changedDigit[i] == 1) || (junction == 0))
                    {
                        if(brig[0] == 1) lev[i] = dayBright[junction]; else lev[i] = nightBright[junction];
                    }
                    if (junction == 12 && set == 0)
                    {
                        out[i] = tempScreen[i];
                        flagBin=1; //для смены при режиме бинарные часы
                    }
                }
                junction++;
            }
        }                
        
//ШИМ подсветка - ЦИФРЫ
        pwmCnt++;
        if(pwmCnt == 7) 
        {
          switch(counter)
          {
              case 0: PORTD_Bit3=1; PORTD_Bit6=0; PORTC_Bit7=0; PORTA_Bit6=0; PORTB_Bit0=0; PORTB_Bit1=0; break;
              case 1: PORTD_Bit3=0; PORTD_Bit6=1; PORTC_Bit7=0; PORTA_Bit6=0; PORTB_Bit0=0; PORTB_Bit1=0; break;
              case 2: PORTD_Bit3=0; PORTD_Bit6=0; PORTC_Bit7=1; PORTA_Bit6=0; PORTB_Bit0=0; PORTB_Bit1=0; break;
              case 3: PORTD_Bit3=0; PORTD_Bit6=0; PORTC_Bit7=0; PORTA_Bit6=1; PORTB_Bit0=0; PORTB_Bit1=0; break;
              case 4: PORTD_Bit3=0; PORTD_Bit6=0; PORTC_Bit7=0; PORTA_Bit6=0; PORTB_Bit0=1; PORTB_Bit1=0; break;
              case 5: PORTD_Bit3=0; PORTD_Bit6=0; PORTC_Bit7=0; PORTA_Bit6=0; PORTB_Bit0=0; PORTB_Bit1=1; break;  
          }
          pwmCnt=0;
          OCR1A = outR[counter];
          OCR0  = outG[counter];
          OCR1B = outB[counter];
          
          if(waitTimer) waitTimer--;
        }    
        
///КНОПКИ        
    //кнопка 0    
        DDRB_Bit4 = 1; PORTB_Bit4 = 0; DDRB_Bit4 = 0;     //устанавливаем "0" >>
        __delay_cycles(15);
       //если кнопка подтянулась до питания ->
        if(PINB_Bit4 == 0) setCountBtn++; 
        else
        {
                    if(setCountBtn > 1000)
                    { //врубаем режим настройки - длительное нажатие кнопки
                            button &= ~(1<<0);
                            setCountBtn = 0; protectKeys[0] = 0;
                            //PORTA_Bit5=0; PORTC_Bit6=0;
                            ZERO;
                            outB[0] = 255; outB[1] = 255;     
                            set = 1; 
                    }
                    if(setCountBtn < 1000 && setCountBtn > 200)
                    { //просто меняем режим подсветки
                            button |= (1<<0);
                            setCountBtn = 0;
                            protectKeys[0] = 0;
                    }
        }   
        
    //кнопка 1    
        DDRD_Bit7 = 1; PORTD_Bit7 = 0; DDRD_Bit7 = 0;     //устанавливаем "0" >>
        __delay_cycles(15);
       //если кнопка подтянулась до питания ->
        if(PIND_Bit7==0)        {timerButton[0]++;}    
        else                    {timerButton[0] = 0; protectKeys[1] = 0;}
        if(timerButton[0] == 150) {button |= (1<<1); timerButton[0] = 0;} //240 - 100ms удержания
        
    //кнопка 2    
        DDRB_Bit2 = 1; PORTB_Bit2 = 0; DDRB_Bit2 = 0;     //устанавливаем "0" >>
        __delay_cycles(15);
       //если кнопка подтянулась до питания ->
        if(PINB_Bit2==0)        {timerButton[1]++;}    
        else                    {timerButton[1] = 0; protectKeys[2] = 0;}
        if(timerButton[1] == 150) {button |= (1<<2); timerButton[1] = 0;} //240 - 100ms удержания
        
        
        if(protectKeys[0]) {button &= ~(1<<0);} //если "запиратели" кнопки активированы
        if(protectKeys[1]) {button &= ~(1<<1);} //то выключить нажатую кнопку)
        if(protectKeys[2]) {button &= ~(1<<2);} //запиратели очищаются при отпускании кнопки
        
    static u8 counterBtn = 0;
        
        counterBtn++;                      //1 ед. "запирателя" - 100мс
        if(counterBtn == 240){
          counterBtn=0;
          if(protectKeys[0]) protectKeys[0]--;
          if(protectKeys[1]) protectKeys[1]--;
          if(protectKeys[2]) protectKeys[2]--;
        }    
        
        if(smoothTimer) smoothTimer--;
}

#pragma vector = TIMER2_COMP_vect 
__interrupt void Compare() // реализация ШИМ управления яркостью ламп и подсветки
{  
       //переключение анодов - ключевые транзисторы
        if(counter==0 || counter==3) {PORTD_Bit1 = 1; PORTD_Bit0 = 0; PORTA_Bit7 = 0;}
        if(counter==1 || counter==4) {PORTD_Bit1 = 0; PORTD_Bit0 = 1; PORTA_Bit7 = 0;}
        if(counter==2 || counter==5) {PORTD_Bit1 = 0; PORTD_Bit0 = 0; PORTA_Bit7 = 1;}
       //левая половина ламп - дешифратор
        if(dig < 3) switch(out[dig]) 
        {
          case 0: PORTC_Bit4=0; PORTC_Bit2=0; PORTC_Bit3=1; PORTC_Bit5=0; break;
          case 1: PORTC_Bit4=0; PORTC_Bit2=0; PORTC_Bit3=1; PORTC_Bit5=1; break;
          case 2: PORTC_Bit4=0; PORTC_Bit2=0; PORTC_Bit3=0; PORTC_Bit5=1; break;
          case 3: PORTC_Bit4=0; PORTC_Bit2=1; PORTC_Bit3=0; PORTC_Bit5=1; break;
          case 4: PORTC_Bit4=0; PORTC_Bit2=0; PORTC_Bit3=0; PORTC_Bit5=0; break;
          case 5: PORTC_Bit4=1; PORTC_Bit2=0; PORTC_Bit3=0; PORTC_Bit5=0; break;
          case 6: PORTC_Bit4=1; PORTC_Bit2=0; PORTC_Bit3=0; PORTC_Bit5=1; break;
          case 7: PORTC_Bit4=0; PORTC_Bit2=1; PORTC_Bit3=1; PORTC_Bit5=0; break;
          case 8: PORTC_Bit4=0; PORTC_Bit2=1; PORTC_Bit3=0; PORTC_Bit5=0; break;
          case 9: PORTC_Bit4=0; PORTC_Bit2=1; PORTC_Bit3=1; PORTC_Bit5=1; break;
          case 10: PORTC_Bit4=1; PORTC_Bit2=0; PORTC_Bit3=1; PORTC_Bit5=0; break;
        }
       //правая половина ламп - дешифратор
        if(dig > 2) switch(out[dig]) 
        {
          case 0: PORTA_Bit1=1; PORTA_Bit3=0; PORTA_Bit2=0; PORTA_Bit0=1; break;
          case 1: PORTA_Bit1=1; PORTA_Bit3=0; PORTA_Bit2=0; PORTA_Bit0=0; break;
          case 2: PORTA_Bit1=0; PORTA_Bit3=1; PORTA_Bit2=1; PORTA_Bit0=1; break;
          case 3: PORTA_Bit1=0; PORTA_Bit3=1; PORTA_Bit2=0; PORTA_Bit0=1; break;
          case 4: PORTA_Bit1=0; PORTA_Bit3=1; PORTA_Bit2=1; PORTA_Bit0=0; break;
          case 5: PORTA_Bit1=0; PORTA_Bit3=0; PORTA_Bit2=1; PORTA_Bit0=1; break;
          case 6: PORTA_Bit1=0; PORTA_Bit3=0; PORTA_Bit2=1; PORTA_Bit0=0; break;
          case 7: PORTA_Bit1=0; PORTA_Bit3=0; PORTA_Bit2=0; PORTA_Bit0=0; break;
          case 8: PORTA_Bit1=0; PORTA_Bit3=1; PORTA_Bit2=0; PORTA_Bit0=0; break;
          case 9: PORTA_Bit1=0; PORTA_Bit3=0; PORTA_Bit2=0; PORTA_Bit0=1; break;
          case 10: PORTA_Bit1=1; PORTA_Bit3=0; PORTA_Bit2=1; PORTA_Bit0=0; break;
        }
                
        counter++;
        if(counter == 6) counter = 0;
        if(counter <  3) dig = counter;
        if(counter >= 3) dig = 8 - counter;
}





