#include "image_processing.h"

float his[MAX_HIS_LENGTH];

static RunLength runList;
static EqualMark markList;

void Img_Process(void)
{
    uint16_t i;

    YUV2Gray((YUV_Format *)ov2640_FRAME_BUFFER, (__IO uint8_t **)ov2640_GRAY_BUFFER, OV2640_IMG_HEIGHT, OV2640_IMG_WIDTH);
    Mid_Filter((uint8_t **)ov2640_GRAY_BUFFER);
    Gray_To_BW((uint8_t **)ov2640_GRAY_BUFFER);
    Run_Label((uint8_t **)ov2640_GRAY_BUFFER);
    Label_Center((uint8_t **)ov2640_GRAY_BUFFER);

    HAL_UART_Transmit(&huart1, testCMD_Start, 2, 0xffffffff);
    for (i = 0; i < OV2640_IMG_HEIGHT; i++)
    {
        HAL_UART_Transmit(&huart1, (uint8_t *)ov2640_GRAY_BUFFER[i], OV2640_IMG_WIDTH, 0xffffffff);
    }

    HAL_UART_Transmit(&huart1, testCMD_End, 2, 0xffffffff);
}

/*求图像的统计直方图，并返回图像平均灰度*/
static float Get_Histogram(uint8_t **image, float *his)
{
    uint16_t i, j;
    float avgValue = 0;

    for (i = 0; i < IMAGE_HEIGHT; ++i)
    {
        for (j = 0; j < IMAGE_WIDTH; ++j)
        {
            ++his[image[i][j]];
        }
    }

    /*****************归一化*********************/
    for (i = 0; i < MAX_HIS_LENGTH; ++i)
    {
        his[i] /= (IMAGE_HEIGHT * IMAGE_WIDTH);
        avgValue += i * his[i];
    }

    return avgValue;
}

/*利用遍历法和最大类间方差求阈值*/
static uint8_t Osu_Threshold(float *his, float avgValue)
{
    uint8_t i;
    uint8_t threshold;
    float ut;
    float var, thisMaxVar = 0;
    float wk = 0, uk = 0;

    for (i = 0; i < MAX_HIS_LENGTH - 1; ++i)
    {
        wk += his[i];
        uk += i * his[i];

        ut = avgValue * wk - uk;
        var = ut * ut / (wk * (1 - wk));

        if (var > thisMaxVar)
        {
            thisMaxVar = var;
            threshold = i;
        }
    }

    return threshold;
}

/*利用osu法进行图像二值化*/
static void Gray_To_BW(uint8_t **image)
{
    uint16_t i, j;
    uint8_t threshold;
    float avgGray;

    avgGray = Get_Histogram((uint8_t **)ov2640_GRAY_BUFFER, his);
    threshold = Osu_Threshold(his, avgGray);

    for (i = 0; i < IMAGE_HEIGHT; ++i)
    {
        for (j = 0; j < IMAGE_WIDTH; ++j)
        {
            if (image[i][j] < threshold)
            {
                image[i][j] = BLACK;
            }
            else
            {
                image[i][j] = WHITE;
            }
        }
    }
}

/*利用行程标记法，标记图中的连通区域*/
static void Run_Label(uint8_t **image)
{
    uint16_t i, j;

    Equals equal;
    uint16_t cnt = 1;
    uint16_t cntLabel = 0;

    uint16_t idxLabel = 1;
    uint16_t thisRow = 0;
    int16_t firstRunOnCur = 0;
    int16_t firstRunOnPre = 0;
    int16_t lastRunOnPre = -1;

    /*初始化行程记录表*/
    runList.last = -1;
    markList.last = -1;

    for (i = 0; i < IMAGE_HEIGHT; ++i)
    {
        if (image[i][0] == WHITE)
        {
            ++cntLabel;
            runList.data[++(runList.last)].nLabel = cntLabel;
            runList.data[runList.last].nRow = i;
            runList.data[runList.last].nStart = 0;
        }
        for (j = 1; j < IMAGE_WIDTH; ++j)
        {
            if (image[i][j - 1] == BLACK && image[i][j] == WHITE)
            {
                ++cntLabel;
                runList.data[++(runList.last)].nLabel = cntLabel;
                runList.data[runList.last].nRow = i;
                runList.data[runList.last].nStart = j;
            }
            else if (image[i][j - 1] == WHITE && image[i][j] == BLACK)
            {
                runList.data[runList.last].nEnd = j - 1;
            }
        }
        if (image[i][IMAGE_WIDTH - 1] == WHITE)
        {
            runList.data[runList.last].nEnd = IMAGE_WIDTH - 1;
        }
    }

    for (i = 0; i < runList.data[runList.last].nLabel; ++i)
    {
        if (runList.data[i].nRow != thisRow)
        {
            thisRow = runList.data[i].nRow;
            firstRunOnPre = firstRunOnCur;
            lastRunOnPre = i - 1;
            firstRunOnCur = i;
        }
        for (j = firstRunOnPre; j <= lastRunOnPre; ++j)
        {
            if (runList.data[i].nStart <= runList.data[j].nEnd + 1 &&
                runList.data[i].nEnd >= runList.data[j].nStart - 1 &&
                runList.data[i].nRow == runList.data[j].nRow + 1)
            {
                if (runList.data[i].nLabel == 0)
                {
                    runList.data[i].nLabel = runList.data[j].nLabel;
                }
                else if (runList.data[i].nLabel != runList.data[j].nLabel)
                {
                    if (runList.data[i].nLabel > runList.data[j].nLabel)
                    {
                        markList.data[++(markList.last)].nMarkValue1 = runList.data[i].nLabel;
                        markList.data[markList.last].nMarkValue2 = runList.data[j].nLabel;
                    }
                    else
                    {
                        markList.data[++(markList.last)].nMarkValue1 = runList.data[j].nLabel;
                        markList.data[markList.last].nMarkValue2 = runList.data[i].nLabel;
                    }
                }
            }
        }
        if (runList.data[i].nLabel == 0)
        {
            runList.data[i].nLabel = idxLabel++;
        }
    }

    equal.last = runList.data[runList.last].nLabel;
    for (i = 0; i < equal.last + 1; ++i)
    {
        equal.label[i] = 0;
    }

    for (i = 0; i < markList.last + 1; ++i)
    {
        Equal_Process(equal.label, markList.data[i].nMarkValue1, markList.data[i].nMarkValue2);
    }

    for (i = 1; i < equal.last + 1; ++i)
    {
        if (equal.label[i] == 0)
        {
            equal.label[i] = cnt;
            ++cnt;
        }
        else
        {
            equal.label[i] = equal.label[equal.label[i]];
        }
    }

    for (i = 0; i < runList.last + 1; ++i)
    {
        runList.data[i].nLabel = equal.label[runList.data[i].nLabel];
        for (j = runList.data[i].nStart; j < runList.data[i].nEnd + 1; ++j)
        {
            image[runList.data[i].nRow][j] = runList.data[i].nLabel;
        }
    }
}

/*利用递归的方法处理等价对*/
static void Equal_Process(uint16_t *equal, uint16_t nValue1, uint16_t nValue2)
{
    if (equal[nValue1] == 0)
    {
        equal[nValue1] = nValue2;
        return;
    }
    if (equal[nValue1] == nValue2)
    {
        return;
    }
    if (equal[nValue1] > nValue2)
    {
        Equal_Process(equal, equal[nValue1], nValue2);
    }
    if (equal[nValue1] < nValue2)
    {
        Equal_Process(equal, nValue2, equal[nValue1]);
    }
}

/*标记出连通区域的中心*/
void Label_Center(uint8_t **image)
{
    uint16_t i, j, k;
    uint16_t sumRow, sumCol, area;
    uint8_t level = 256 / (runList.data[runList.last].nLabel);

    for (k = 1; k < runList.data[runList.last].nLabel; ++k)
    {
        sumRow = 0;
        sumCol = 0;
        area = 0;
        for (i = 0; i < IMAGE_HEIGHT; ++i)
        {
            for (j = 0; j < IMAGE_WIDTH; ++j)
            {
                if (image[i][j] == k)
                {
                    sumRow += i;
                    sumCol += j;
                    ++area;
                    image[i][j] = k * level;
                }
            }
        }
        image[sumRow / area][sumCol / area] = WHITE;
    }
}

/*中值滤波器*/
void Mid_Filter(uint8_t **image)
{
    uint16_t i, j, k, l;
    uint8_t temp[9];    //用于存放中心元素及其8邻域元素
    uint8_t t;          //用于排序中的临时变量

    for (i = 1; i < IMAGE_HEIGHT - 1; ++i)
    {
        for (j = 1; j < IMAGE_WIDTH - 1; ++j)
        {
            temp[0] = image[i - 1][j - 1];
            temp[1] = image[i - 1][j];
            temp[2] = image[i - 1][j + 1];
            temp[3] = image[i][j - 1];
            temp[4] = image[i][j];
            temp[5] = image[i][j + 1];
            temp[6] = image[i + 1][j - 1];
            temp[7] = image[i + 1][j];
            temp[8] = image[i + 1][j + 1];

            for (k = 0; k < 9 - 1; ++k)
            {
                for (l = 0; l < 9 - k - 1; ++l)
                {
                    if (temp[l] > temp[l + 1])
                    {
                        t = temp[l];
                        temp[l] = temp[l + 1];
                        temp[l + 1] = t;
                    }
                }
            }

            image[i][j] = temp[4];
        }
    }
}
