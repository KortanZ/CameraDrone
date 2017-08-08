/* Includes ------------------------------------------------------------------*/
#include "ov2640.h"
#include "ov2640_para_table.h"
#include "imgData.h"

/*Private Function*/
static void ov2640_Component_Init(uint16_t deviceAddr, uint32_t resolution);
static void ov2640_IO_Write(uint8_t addr, uint8_t reg, uint8_t value);
static uint8_t ov2640_IO_Read(uint8_t addr, uint8_t reg);
static void ov2640_Delay(uint32_t delay);
static void ov2640_IO_Error(uint8_t addr);
static void Img_Init(void);
/******************/

/*Public Var*/
uint32_t currentResolution;

uint8_t *ov2640_FRAME_BUFFER;
uint32_t *ov2640_GRAY_BUFFER;

uint8_t testCMD_Start[] = {0x01, 0xFE};
uint8_t testCMD_End[] = {0xFE, 0x01};

/**
  * @brief  Initializes the OV2640 CAMERA component.
  * @param  deviceAddr: Device address on communication Bus.
  * @param  resolution: Camera resolution
  * @retval None
  */
void ov2640_Init(uint32_t resolution)
{
    if (ov2640_ReadID(OV2640_ADDR) == OV2640_ID)
    {
        /* Camera Init */
        ov2640_Component_Init(OV2640_ADDR, resolution);
    }

    currentResolution = resolution;
    Img_Init();
}

void Img_Init(void)
{
    uint16_t i;

    ov2640_FRAME_BUFFER = (uint8_t *)SDRAM_Malloc((uint32_t)OV2640_IMG_HEIGHT * OV2640_IMG_WIDTH * 2);
    ov2640_GRAY_BUFFER = (uint32_t *)SDRAM_Malloc((uint32_t)OV2640_IMG_HEIGHT * 4);

    for (i = 0; i < OV2640_IMG_HEIGHT; i++)
    {
        ov2640_GRAY_BUFFER[i] = (uint32_t)ov2640_GRAY_BUFFER + OV2640_IMG_HEIGHT * 4 + (OV2640_IMG_WIDTH) * i;
    }

    SDRAM_Malloc(OV2640_IMG_HEIGHT * (OV2640_IMG_WIDTH));

    runList = (RunLength *)SDRAM_Malloc(sizeof(RunLength));
    markList = (EqualMark *)SDRAM_Malloc(sizeof(EqualMark));
    equal = (Equals *)SDRAM_Malloc(sizeof(Equals));

    lines = (HoughLine *)SDRAM_Malloc(sizeof(HoughLine) * HOUGH_LINE_SIZE);
    houghAcc = (uint16_t *)SDRAM_Malloc(sizeof(uint16_t) * HOUGH_THETA_SIZE * HOUGH_ROH_SIZE);

    sobelBuff = (uint32_t *)SDRAM_Malloc((uint32_t)OV2640_IMG_HEIGHT * 4);
    for (i = 0; i < OV2640_IMG_HEIGHT; i++)
    {
        sobelBuff[i] = (uint32_t)sobelBuff + OV2640_IMG_HEIGHT * 4 + (OV2640_IMG_WIDTH) * i;
    }
		SDRAM_Malloc(OV2640_IMG_HEIGHT * (OV2640_IMG_WIDTH));

}

/**
  * @brief  Configures the OV2640 camera feature.
  * @param  deviceAddr: Device address on communication Bus.
  * @param  feature: Camera feature to be configured
  * @param  value: value to be configured
  * @param  brightness_value: Brightness value to be configured
  * @retval None
  */
void ov2640_Config(uint16_t deviceAddr, uint32_t feature, uint32_t value, uint32_t brightnessValue)
{
    uint8_t value1, value2;

    switch (feature)
    {
    case ov2640_BLACK_WHITE:
    {
        ov2640_IO_Write(deviceAddr, 0xff, 0x00);
        ov2640_IO_Write(deviceAddr, 0x7c, 0x00);
        ov2640_IO_Write(deviceAddr, 0x7d, value);
        ov2640_IO_Write(deviceAddr, 0x7c, 0x05);
        ov2640_IO_Write(deviceAddr, 0x7d, 0x80);
        ov2640_IO_Write(deviceAddr, 0x7d, 0x80);
        break;
    }
    case ov2640_CONTRAST_BRIGHTNESS:
    {
        value1 = (uint8_t)(value);
        value2 = (uint8_t)(value >> 8);
        ov2640_IO_Write(deviceAddr, 0xff, 0x00);
        ov2640_IO_Write(deviceAddr, 0x7c, 0x00);
        ov2640_IO_Write(deviceAddr, 0x7d, 0x04);
        ov2640_IO_Write(deviceAddr, 0x7c, 0x07);
        ov2640_IO_Write(deviceAddr, 0x7d, brightnessValue);
        ov2640_IO_Write(deviceAddr, 0x7d, value1);
        ov2640_IO_Write(deviceAddr, 0x7d, value2);
        ov2640_IO_Write(deviceAddr, 0x7d, 0x06);
        break;
    }
    case ov2640_COLOR_EFFECT:
    {
        value1 = (uint8_t)(value);
        value2 = (uint8_t)(value >> 8);
        ov2640_IO_Write(deviceAddr, 0xff, 0x00);
        ov2640_IO_Write(deviceAddr, 0x7c, 0x00);
        ov2640_IO_Write(deviceAddr, 0x7d, 0x18);
        ov2640_IO_Write(deviceAddr, 0x7c, 0x05);
        ov2640_IO_Write(deviceAddr, 0x7d, value1);
        ov2640_IO_Write(deviceAddr, 0x7d, value2);
        break;
    }
    default:
    {
        break;
    }
    }
}

/**
  * @brief  Starts the Camera capture in continuous mode.
  * @param  buff: pointer to the Camera output buffer
  */
void ov2640_ContinuousStart(uint8_t *buff)
{
    /* Start the Camera capture */
    HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_CONTINUOUS, (uint32_t)buff, GetSize(currentResolution));
}

/**
  * @brief  Starts the Camera capture in snapshot mode.
  * @param  buff: pointer to the Camera output buffer
  */
void ov2640_SnapshotStart(uint8_t *buff)
{
    /* Start the Camera capture */
    HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_SNAPSHOT, (uint32_t)buff, GetSize(currentResolution));
}

/**
  * @brief  Read the OV2640 Camera identity.
  * @param  deviceAddr: Device address on communication Bus.
  * @retval the OV2640 ID
  */
uint16_t ov2640_ReadID(uint16_t deviceAddr)
{
    /* Prepare the sensor to read the Camera ID */
    ov2640_IO_Write(deviceAddr, OV2640_DSP_RA_DLMT, 0x01);

    /* Get the camera ID */
    return (ov2640_IO_Read(deviceAddr, OV2640_SENSOR_PIDH));
}

/**
  * @brief  Get the capture size.
  * @param  resolution: the current resolution.
  * @retval cpature size
  */
uint32_t GetSize(uint32_t resolution)
{
    uint32_t size = 0;

    /* Get capture size */
    switch (resolution)
    {
    case ov2640_R160x120:
    {
        size = 0x2580;
        break;
    }
    case ov2640_R320x240:
    {
        size = 0x9600;
        break;
    }
    case ov2640_R400x300:
    {
        size = 0xEA60;
        break;
    }
    case ov2640_R640x480:
    {
        size = 0x25800;
        break;
    }
    default:
    {
        break;
    }
    }

    return size;
}

void ov2640_SetYUV(void)
{
    ov2640_IO_Write(OV2640_ADDR, 0xff, 0x00);
    ov2640_IO_Write(OV2640_ADDR, 0xda, 0x00);

    ov2640_IO_Write(OV2640_ADDR, 0xff, 0x01);
    ov2640_IO_Write(OV2640_ADDR, 0x11, 0x00);
    ov2640_IO_Write(OV2640_ADDR, 0x2a, 0x00);
    ov2640_IO_Write(OV2640_ADDR, 0x2b, 0x00);
}

/********************************************Autom*********************************************/
//设置图像输出窗口(传感器)(0层)
//sx,sy,起始地址
//width,height:宽度(对应:horizontal)和高度(对应:vertical)
void OV2640_Window_Set(uint16_t sx, uint16_t sy, uint16_t width, uint16_t height)
{
    uint16_t endx;
    uint16_t endy;
    uint8_t temp;
    endx = sx + width / 2; //V*2
    endy = sy + height / 2;

    ov2640_IO_Write(OV2640_ADDR, 0XFF, 0X01);
    temp = ov2640_IO_Read(OV2640_ADDR, 0X03); //读取Vref之前的值
    temp &= 0XF0;
    temp |= ((endy & 0X03) << 2) | (sy & 0X03);
    ov2640_IO_Write(OV2640_ADDR, 0X03, temp);      //设置Vref的start和end的最低2位
    ov2640_IO_Write(OV2640_ADDR, 0X19, sy >> 2);   //设置Vref的start高8位
    ov2640_IO_Write(OV2640_ADDR, 0X1A, endy >> 2); //设置Vref的end的高8位

    temp = ov2640_IO_Read(OV2640_ADDR, 0X32); //读取Href之前的值
    temp &= 0XC0;
    temp |= ((endx & 0X07) << 3) | (sx & 0X07);
    ov2640_IO_Write(OV2640_ADDR, 0X32, temp);      //设置Href的start和end的最低3位
    ov2640_IO_Write(OV2640_ADDR, 0X17, sx >> 3);   //设置Href的start高8位
    ov2640_IO_Write(OV2640_ADDR, 0X18, endx >> 3); //设置Href的end的高8位
}

//该函数设置图像尺寸大小,也就是所选格式的输出分辨率(DSP)(1层)
//UXGA:1600*1200,SVGA:800*600,CIF:352*288
//width,height:图像宽度和图像高度
//返回值:0,设置成功
//    其他,设置失败
uint8_t OV2640_ImageSize_Set(uint16_t width, uint16_t height)
{
    uint8_t temp;
    ov2640_IO_Write(OV2640_ADDR, 0XFF, 0X00);
    ov2640_IO_Write(OV2640_ADDR, 0XE0, 0X04);
    ov2640_IO_Write(OV2640_ADDR, 0XC0, (width) >> 3 & 0XFF);  //设置HSIZE的10:3位
    ov2640_IO_Write(OV2640_ADDR, 0XC1, (height) >> 3 & 0XFF); //设置VSIZE的10:3位
    temp = (width & 0X07) << 3;
    temp |= height & 0X07;
    temp |= (width >> 4) & 0X80;
    ov2640_IO_Write(OV2640_ADDR, 0X8C, temp);
    ov2640_IO_Write(OV2640_ADDR, 0XE0, 0X00);
    return 0;
}

//(DSP) (2层)
//设置图像开窗大小
//由:OV2640_ImageSize_Set确定传感器输出分辨率从大小.
//该函数则在这个范围上面进行开窗,用于OV2640_OutSize_Set的输出
//注意:本函数的宽度和高度,必须大于等于OV2640_OutSize_Set函数的宽度和高度
//     OV2640_OutSize_Set设置的宽度和高度,根据本函数设置的宽度和高度,由DSP
//     自动计算缩放比例,输出给外部设备.
//width,height:宽度(对应:horizontal)和高度(对应:vertical),width和height必须是4的倍数
//返回值:0,设置成功
//    其他,设置失败
uint8_t OV2640_ImageWin_Set(uint16_t offx, uint16_t offy, uint16_t width, uint16_t height)
{
    uint16_t hsize;
    uint16_t vsize;
    uint8_t temp;
    if (width % 4)
        return 1;
    if (height % 4)
        return 2;
    hsize = width / 4;
    vsize = height / 4;
    ov2640_IO_Write(OV2640_ADDR, 0XFF, 0X00);
    ov2640_IO_Write(OV2640_ADDR, 0XE0, 0X04);
    ov2640_IO_Write(OV2640_ADDR, 0X51, hsize & 0XFF); //设置H_SIZE的低八位
    ov2640_IO_Write(OV2640_ADDR, 0X52, vsize & 0XFF); //设置V_SIZE的低八位
    ov2640_IO_Write(OV2640_ADDR, 0X53, offx & 0XFF);  //设置offx的低八位
    ov2640_IO_Write(OV2640_ADDR, 0X54, offy & 0XFF);  //设置offy的低八位
    temp = (vsize >> 1) & 0X80;
    temp |= (offy >> 4) & 0X70;
    temp |= (hsize >> 5) & 0X08;
    temp |= (offx >> 8) & 0X07;
    ov2640_IO_Write(OV2640_ADDR, 0X55, temp);                //设置H_SIZE/V_SIZE/OFFX,OFFY的高位
    ov2640_IO_Write(OV2640_ADDR, 0X57, (hsize >> 2) & 0X80); //设置H_SIZE/V_SIZE/OFFX,OFFY的高位
    ov2640_IO_Write(OV2640_ADDR, 0XE0, 0X00);
    return 0;
}

//(DSP) (3层)-------最终输出大小
//设置图像输出大小
//OV2640输出图像的大小(分辨率),完全由改函数确定
//width,height:宽度(对应:horizontal)和高度(对应:vertical),width和height必须是4的倍数
//返回值:0,设置成功
//    其他,设置失败
uint8_t OV2640_ImgOut_Set(uint16_t width, uint16_t height)
{
    uint16_t outh;
    uint16_t outw;
    uint8_t temp;
    if (width % 4)
        return 1;
    if (height % 4)
        return 2;
    outw = width / 4;
    outh = height / 4;
    ov2640_IO_Write(OV2640_ADDR, 0XFF, 0X00);
    ov2640_IO_Write(OV2640_ADDR, 0XE0, 0X04);
    ov2640_IO_Write(OV2640_ADDR, 0X5A, outw & 0XFF); //设置OUTW的低八位
    ov2640_IO_Write(OV2640_ADDR, 0X5B, outh & 0XFF); //设置OUTH的低八位
    temp = (outw >> 8) & 0X03;
    temp |= (outh >> 6) & 0X04;
    ov2640_IO_Write(OV2640_ADDR, 0X5C, temp); //设置OUTH/OUTW的高位
    ov2640_IO_Write(OV2640_ADDR, 0XE0, 0X00);
    return 0;
}

/**********************************************************************************************/
void OV2640_AutoExposure(uint8_t level)
{
    uint32_t index;
    switch (level)
    {
    case 0:
        for (index = 0; index < (sizeof(OV2640_AUTOEXPOSURE_LEVEL0) / 2); index++)
        {
            ov2640_IO_Write(OV2640_ADDR, OV2640_AUTOEXPOSURE_LEVEL0[index][0], OV2640_AUTOEXPOSURE_LEVEL0[index][1]);
            ov2640_Delay(1);
        }
        break;
    case 1:
        for (index = 0; index < (sizeof(OV2640_AUTOEXPOSURE_LEVEL1) / 2); index++)
        {
            ov2640_IO_Write(OV2640_ADDR, OV2640_AUTOEXPOSURE_LEVEL1[index][0], OV2640_AUTOEXPOSURE_LEVEL1[index][1]);
            ov2640_Delay(1);
        }
        break;
    case 2:
        for (index = 0; index < (sizeof(OV2640_AUTOEXPOSURE_LEVEL2) / 2); index++)
        {
            ov2640_IO_Write(OV2640_ADDR, OV2640_AUTOEXPOSURE_LEVEL2[index][0], OV2640_AUTOEXPOSURE_LEVEL2[index][1]);
            ov2640_Delay(1);
        }
        break;
    case 3:
        for (index = 0; index < (sizeof(OV2640_AUTOEXPOSURE_LEVEL3) / 2); index++)
        {
            ov2640_IO_Write(OV2640_ADDR, OV2640_AUTOEXPOSURE_LEVEL3[index][0], OV2640_AUTOEXPOSURE_LEVEL3[index][1]);
            ov2640_Delay(1);
        }
        break;
    case 4:
        for (index = 0; index < (sizeof(OV2640_AUTOEXPOSURE_LEVEL4) / 2); index++)
        {
            ov2640_IO_Write(OV2640_ADDR, OV2640_AUTOEXPOSURE_LEVEL4[index][0], OV2640_AUTOEXPOSURE_LEVEL4[index][1]);
            ov2640_Delay(1);
        }
        break;
    default:
        for (index = 0; index < (sizeof(OV2640_AUTOEXPOSURE_LEVEL0) / 2); index++)
        {
            ov2640_IO_Write(OV2640_ADDR, OV2640_AUTOEXPOSURE_LEVEL0[index][0], OV2640_AUTOEXPOSURE_LEVEL0[index][1]);
            ov2640_Delay(1);
        }
        break;
    }
}

void YUV2Gray(__IO YUV_Format *src, __IO uint8_t **des, uint16_t row, uint16_t col)
{
    uint16_t i, j;
    __IO uint8_t pixTemp;

    for (i = 0; i < row; i++)
    {
        for (j = 0; j < col; j++)
        {
            des[i][j] = src[col * i + j].pixType[0];
            // des[i][j] = imgTest[i][j];
        }
        // des[i][j] = 0x00;
    }
}

void YUV2Blue(__IO YUV_Format *src, __IO uint8_t **des, uint16_t row, uint16_t col)
{
    uint16_t i, j;
    __IO uint8_t pixTemp;

    for (i = 0; i < row; i++)
    {
        for (j = 0; j < col; j++)
        {
            if ((col * i + j) % 2 == 0)
            {
                des[i][j] = src[col * i + j].pixType[1];
            }
            else
            {
                des[i][j] = src[(col * i + j) - 1].pixType[1];
            }
            
            // des[i][j] = WHITE;
        }
        // des[i][j] = 0x00;
    }
}

void YUV2Red(__IO YUV_Format *src, __IO uint8_t **des, uint16_t row, uint16_t col)
{
    uint16_t i, j;
    __IO uint8_t pixTemp;

    for (i = 0; i < row; i++)
    {
        for (j = 0; j < col; j++)
        {
            if ((col * i + j) % 2 == 1)
            {
                des[i][j] = src[col * i + j].pixType[1];
            }
            else
            {
                des[i][j] = src[(col * i + j) + 1].pixType[1];
            }
            
            // des[i][j] = WHITE;
        }
        // des[i][j] = 0x00;
    }
}

/******************************************************************************
                            Static Functions
*******************************************************************************/

static void ov2640_Component_Init(uint16_t deviceAddr, uint32_t resolution)
{
    uint32_t index;

    /* Initialize I2C */
    //  ov2640_IO_Init();    //Initialized in main()

    /* Prepare the camera to be configured */
    ov2640_IO_Write(deviceAddr, OV2640_DSP_RA_DLMT, 0x01);
    ov2640_IO_Write(deviceAddr, OV2640_SENSOR_COM7, 0x80);
    ov2640_Delay(200);

    /* Initialize OV2640 */
    switch (resolution)
    {
    case ov2640_R160x120:
    {
        for (index = 0; index < (sizeof(OV2640_QQVGA) / 2); index++)
        {
            ov2640_IO_Write(deviceAddr, OV2640_QQVGA[index][0], OV2640_QQVGA[index][1]);
            ov2640_Delay(1);
        }
        break;
    }
    case ov2640_R320x240:
    {
        for (index = 0; index < (sizeof(OV2640_QVGA) / 2); index++)
        {
            ov2640_IO_Write(deviceAddr, OV2640_QVGA[index][0], OV2640_QVGA[index][1]);
            ov2640_Delay(1);
        }
        break;
    }
    case ov2640_R400x300:
    {
        for (index = 0; index < (sizeof(OV2640_CIF_400X300) / 2); index++)
        {
            ov2640_IO_Write(deviceAddr, OV2640_CIF_400X300[index][0], OV2640_CIF_400X300[index][1]);
            ov2640_Delay(2);
        }
        break;
    }
    case ov2640_R480x272:
    {
        for (index = 0; index < (sizeof(OV2640_480x272) / 2); index++)
        {
            ov2640_IO_Write(deviceAddr, OV2640_480x272[index][0], OV2640_480x272[index][1]);
            ov2640_Delay(2);
        }
        break;
    }
    case ov2640_R640x480:
    {
        for (index = 0; index < (sizeof(OV2640_VGA) / 2); index++)
        {
            ov2640_IO_Write(deviceAddr, OV2640_VGA[index][0], OV2640_VGA[index][1]);
            ov2640_Delay(2);
        }
        break;
    }
    default:
    {
        break;
    }
    }
}

/**
  * @brief  Writes a single data.
  * @param  addr: I2C address
  * @param  reg: reg address 
  * @param  value: Data to be written
  */
static void ov2640_IO_Write(uint8_t addr, uint8_t reg, uint8_t value)
{
    HAL_StatusTypeDef status = HAL_OK;

    status = HAL_I2C_Mem_Write(&hi2c2, addr, (uint16_t)reg, I2C_MEMADD_SIZE_8BIT, &value, 1, I2C_TIMEOUT);

    /* Check the communication status */
    if (status != HAL_OK)
    {
        /* I2C error occured */
        ov2640_IO_Error(addr);
    }
}

/**
  * @brief  Reads a single data.
  * @param  addr: I2C address
  * @param  reg: reg address 
  * @retval Data to be read
  */
static uint8_t ov2640_IO_Read(uint8_t addr, uint8_t reg)
{
    HAL_StatusTypeDef status = HAL_OK;
    uint8_t value = 0;

    status = HAL_I2C_Mem_Read(&hi2c2, addr, reg, I2C_MEMADD_SIZE_8BIT, &value, 1, I2C_TIMEOUT);

    /* Check the communication status */
    if (status != HAL_OK)
    {
        /* Execute user timeout callback */
        ov2640_IO_Error(addr);
    }

    return value;
}

/**
  * @brief  Manages error callback by re-initializing I2C.
  * @param  addr: I2C Address
  */
static void ov2640_IO_Error(uint8_t addr)
{
    /* De-initialize the IOE comunication BUS */
    HAL_I2C_DeInit(&hi2c2);

    /* Re-Initiaize the IOE comunication BUS */
    MX_I2C2_Init();
}

static void ov2640_Delay(uint32_t Delay)
{
    HAL_Delay(Delay);
}

/****END OF FILE****/
