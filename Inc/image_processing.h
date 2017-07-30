#ifndef _IMAGE_PROESSING_H_
#define _IMAGE_PROESSING_H_ 

#include <math.h>
#include <stdlib.h>
#include "stm32f7xx_hal.h"
#include "main.h"
#include "dcmi.h"
#include "ov2640.h"
#include "sdram.h"
#include "wifi.h"

#define MAX_HIS_LENGTH 256
#define IMAGE_HEIGHT 120
#define IMAGE_WIDTH 160
#define IMAGE_SIZE ((IMAGE_HEIGHT) * (IMAGE_WIDTH))

#define BLACK 0
#define WHITE 255

#define MAX_LEN 1000

typedef struct 
{
	uint16_t nLabel;	//标号
	uint16_t nRow;		//游程所在行号
	uint16_t nStart;	//起始点列号
	uint16_t nEnd;		//终止点列号
}RunUnit;

typedef struct 
{
	RunUnit data[MAX_LEN];
	int16_t last;
}RunLength;

typedef struct 
{
	/*nMarkValue1 > nMarkValue2*/
	uint16_t nMarkValue1;	//标记值1
	uint16_t nMarkValue2;	//标记值2
}EqualMarkUnit;	

typedef struct 
{
	EqualMarkUnit data[MAX_LEN];
	int16_t last;
}EqualMark;

typedef struct 
{
	uint16_t label[MAX_LEN];
	int16_t last;
}Equals;

extern float his[MAX_HIS_LENGTH];

/*------------Exported functions-----------*/
void Img_Process(void);

/*------------Private functions-----------*/
static float Get_Histogram(uint8_t **image, float *his);
static uint8_t Osu_Threshold(float *his, float avgValue);
static void Gray_To_BW(uint8_t **image);
static void Run_Label(uint8_t **image);
static void Equal_Process(uint16_t *equal, uint16_t nValue1, uint16_t nValue2);
static void Label_Center(uint8_t **image);
static void Mid_Filter(uint8_t **image);

#endif
