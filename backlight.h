/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef BACKLIGHT_H
#define BACKLIGHT_H

  
/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
struct SmoothType{
    uint8_t speed;
    uint8_t target;
    uint8_t current;
    uint8_t state;
    uint16_t waitTimer;
};

struct SmoothSettings
{
    uint8_t (*pGetTarget)(uint8_t);
    uint8_t (*pGetSpeed)(uint8_t);
    uint16_t (*pGetWaitTime)(uint8_t);
};


/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
#define SMOOTH_NUM_CHANNELS  18


/* Exported functions ------------------------------------------------------- */
void BackLightProcess();
void BackLightChange();
void MixBL();
void MixBLReduceTime();
uint8_t BLGetType();

void ZeroBL();

void HSVtoRGB(uint8_t*, uint8_t*, uint8_t*, uint16_t);
void HSVchange(uint8_t);

void SmoothInitAll();
void SmoothInit(uint8_t);
void SmoothProcess(uint8_t, struct SmoothSettings*);
uint8_t GetSmoothCurrent(uint8_t);


void FireBL();
uint8_t GiveTargetBLFire(uint8_t);
uint8_t GiveSpeedBLFire(uint8_t);
uint16_t GiveWaitBLFire(uint8_t);

void RandomAllBL();
uint8_t GiveTargetBLRandAll(uint8_t);
uint8_t GiveSpeedBLRandAll(uint8_t);
uint16_t GiveWaitBLRandAll(uint8_t);

void RandomEachBL();
uint8_t GiveTargetBLRandEach(uint8_t);
uint8_t GiveSpeedBLRandEach(uint8_t);
uint16_t GiveWaitBLRandEach(uint8_t);

void ShiftBL();
uint8_t GiveTargetBLShift(uint8_t);
uint8_t GiveSpeedBLShift(uint8_t);
uint16_t GiveWaitBLShift(uint8_t);

void BinarySecondsBL();
void BinaryChangeEffect(uint8_t);
uint8_t GiveTargetBLBinary(uint8_t);
uint8_t GiveSpeedBLBinary(uint8_t);
uint16_t GiveWaitBLBinary(uint8_t);


#endif //BACKLIGHT_H