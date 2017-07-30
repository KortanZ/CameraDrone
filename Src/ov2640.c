/* Includes ------------------------------------------------------------------*/
#include "ov2640.h"
#include "ov2640_para_table.h"

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
//uint8_t testCMD_Start[] = {0x01, 0xFE, 0x00};
//uint8_t testCMD_End[] = {0xFE, 0x01, 0x00};
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
        ov2640_GRAY_BUFFER[i] = (uint32_t)ov2640_GRAY_BUFFER + OV2640_IMG_HEIGHT * 4 + (OV2640_IMG_WIDTH)*i;
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
    }
    break;
    case ov2640_R320x240:
    {
        size = 0x9600;
    }
    break;
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
}

void YUV2Gray(__IO YUV_Format *src, __IO uint8_t **des, uint16_t row, uint16_t col)
{
    uint16_t i, j;
    __IO uint8_t pixTemp;

    for (i = 0; i < row; i++)
    {
        for (j = 0; j < col; j++)
        {
            des[i][j] = src[col * i + j].Y;
            // des[i][j] = i + 1;
        }
//        des[i][j] = 0x00;
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
