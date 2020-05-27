#include <stdlib.h>
#include "all.h"

u8 Red, Green, Blue;                                              //now colors
u8 tempR, tempG, tempB;                           //target colors
u8 deltaR, deltaG, deltaB;       //�������� �������� �� ���
u16 tempRG, tempRB, tempGB, temp4; 

void GenerateColours(void)
{
        tempR = (u8) rand() & 0xFF;
        tempG = (u8) rand() & 0xFF;
        tempB = (u8) rand() & 0xFF;
}

/*
void delay(unsigned int delayt)               //�������� delay_ms() �� ���� ��������� ������ ���������( 
{
  while (delayt != 0)
  {
    _delay_ms(2);
    delayt--;
  };
};

*/

u16 ChangeColors(LED *leds)
{ 
  u16 time=0;
       do{
        GenerateColours();
        tempRG = (abs(tempR - tempG));
        tempRB = (abs(tempR - tempB));
        tempGB = (abs(tempG - tempB));
       }while (( tempRG < 150 ) && (tempRB < 150) && (tempGB < 150));  
     //���� ��� ����� ����� �������� ������ �� ����� ����� ��� �� ���������
      
     //c ������������ 0,4 (32767) �������� ���� ����
        if (rand() < 13100)                                         
        {
            do{
            temp4 = rand() & 0x000F;
            }
            while ((temp4 == 0) || (temp4 > 3));
            
            if (temp4 == 1)                                            //�� ������� ��������
            {                                                          //����� �� 0 - 20% �� ���������
                do{ temp4 = rand() & 0x0FFF;
                }
                while ((temp4 == 0) || (temp4 > 3276));
                
                tempR = (temp4 / 32767) * tempR;
            };
            
            if (temp4 == 2)
            {
                do{
                temp4 = rand() & 0x0FFF;
                }
                while ((temp4 == 0) || (temp4 > 3276));
                
                tempG = (temp4 / 32767) * tempG;
            }
            
            if (temp4 == 3)
            {
                do{
                temp4 = rand() & 0x0FFF;
                }
                while ((temp4 == 0) || (temp4 > 3276));
                
                tempB = (temp4 / 32767) * tempB;
            };        
        }
       
															//�������� ��������. ������� �� ������� ����� �� 255 �����
       deltaR = (tempR - Red) >> 8;
       deltaG = (tempG - Green) >> 8;
       deltaB = (tempB - Blue) >> 8; 
      
       
       do{
          temp4 = rand() & 0x00FF;
        }while ((temp4 < 50) || (temp4 > 100));                    	//��������� �������� �������� �� 5 �� 10�
       time = (temp4 >> 8) * 10;              		//�������� ������ ���� �����
       
       
            
       do 
       {
        if (tempR != Red)
            Red += deltaR;
        if (tempG != Green)
            Green += deltaG;
        if (tempB != Blue)
            Blue += deltaB;
       
        for(u8 r=0; r<6; r++){
          leds->R[r]=Red;
          leds->G[r]=Green;  
          leds->B[r]=Blue;
        }
        
        //return (time*200);
        _delay_ms(time);
       }
       while ((tempR != Red) || (tempG != Green) || (tempB != Blue)); 
       
       return ((time << 8));               							//���������� � ����� �����, �������� ���      
}