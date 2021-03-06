#include "image_processing.h"

float his[MAX_HIS_LENGTH];

RunLength *runList;
EqualMark *markList;
Equals *equal;

HoughLine *lines;
uint16_t *houghAcc; //霍夫变换后图像在参数空间的部分灰度梯度直方图

uint32_t *sobelBuff; //soble 图

const int8_t sobelGx[3][3] = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};
const int8_t sobelGy[3][3] = {{1, 2, 1}, {0, 0, 0}, {-1, -2, -1}};

void Img_Process(void)
{
    uint16_t i;

    YUV2Gray((YUV_Format *)ov2640_FRAME_BUFFER, (__IO uint8_t **)ov2640_GRAY_BUFFER, OV2640_IMG_HEIGHT, OV2640_IMG_WIDTH);
    Mid_Filter((uint8_t **)ov2640_GRAY_BUFFER);
    // Gray_To_BW((uint8_t **)ov2640_GRAY_BUFFER);
    // Run_Label((uint8_t **)ov2640_GRAY_BUFFER);
    // Label_Center((uint8_t **)ov2640_GRAY_BUFFER);
    //Hough_Line((uint8_t **)ov2640_GRAY_BUFFER);

    // /* WIFI Img Send */
    while (recv[0] != '.')
    {
        WIFI_Transparent_SendData(testCMD_Start, 2);
        HAL_UART_Receive(&huart1, (uint8_t *)recv, 1, 1);
    }
    recv[0] = 0;
    for (i = 0; i < OV2640_IMG_HEIGHT / 15; i++)
    {
        WIFI_Transparent_SendData((uint8_t *)(ov2640_GRAY_BUFFER[i * 15]), IMAGE_WIDTH * 15);
        HAL_UART_Receive(&huart1, (uint8_t *)recv, 1, 0xffffffff);
    }

    /* UART Img Send */
    // HAL_UART_Transmit(&huart1, testCMD_Start, 2, 0xffffffff);
    // for (i = 0; i < OV2640_IMG_HEIGHT; i++)
    // {
    //     HAL_UART_Transmit(&huart1, (uint8_t *)ov2640_GRAY_BUFFER[i], OV2640_IMG_WIDTH, 0xffffffff);
    // }

    // HAL_UART_Transmit(&huart1, testCMD_End, 2, 0xffffffff);
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
    uint8_t threshold = 0;
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
            if (image[i][j] > threshold)
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

    uint16_t cnt = 1;
    uint16_t cntLabel = 0;

    uint16_t idxLabel = 1;
    uint16_t thisRow = 0;
    int16_t firstRunOnCur = 0;
    int16_t firstRunOnPre = 0;
    int16_t lastRunOnPre = -1;

    /*初始化行程记录表*/
    runList->last = -1;
    markList->last = -1;

    for (i = 0; i < IMAGE_HEIGHT; ++i)
    {
        if (image[i][0] == WHITE)
        {
            if (runList->last < MAX_LEN - 1)
            {
                ++cntLabel;
                runList->data[++(runList->last)].nLabel = cntLabel;
                runList->data[runList->last].nRow = i;
                runList->data[runList->last].nStart = 0;
            }
        }
        for (j = 1; j < IMAGE_WIDTH; ++j)
        {
            if (image[i][j - 1] == BLACK && image[i][j] == WHITE)
            {
                if (runList->last < MAX_LEN - 1)
                {
                    ++cntLabel;
                    runList->data[++(runList->last)].nLabel = cntLabel;
                    runList->data[runList->last].nRow = i;
                    runList->data[runList->last].nStart = j;
                }
            }
            else if (image[i][j - 1] == WHITE && image[i][j] == BLACK)
            {
                runList->data[runList->last].nEnd = j - 1;
            }
        }
        if (image[i][IMAGE_WIDTH - 1] == WHITE)
        {
            runList->data[runList->last].nEnd = IMAGE_WIDTH - 1;
        }
    }

    for (i = 0; i < runList->data[runList->last].nLabel; ++i)
    {
        if (runList->data[i].nRow != thisRow)
        {
            thisRow = runList->data[i].nRow;
            firstRunOnPre = firstRunOnCur;
            lastRunOnPre = i - 1;
            firstRunOnCur = i;
        }
        for (j = firstRunOnPre; j <= lastRunOnPre; ++j)
        {
            if (runList->data[i].nStart <= runList->data[j].nEnd + 1 &&
                runList->data[i].nEnd >= runList->data[j].nStart - 1 &&
                runList->data[i].nRow == runList->data[j].nRow + 1)
            {
                if (runList->data[i].nLabel == 0)
                {
                    runList->data[i].nLabel = runList->data[j].nLabel;
                }
                else if (runList->data[i].nLabel != runList->data[j].nLabel)
                {
                    if (markList->last < MAX_LEN - 1)
                    {
                        if (runList->data[i].nLabel > runList->data[j].nLabel)
                        {
                            markList->data[++(markList->last)].nMarkValue1 = runList->data[i].nLabel;
                            markList->data[markList->last].nMarkValue2 = runList->data[j].nLabel;
                        }
                        else
                        {
                            markList->data[++(markList->last)].nMarkValue1 = runList->data[j].nLabel;
                            markList->data[markList->last].nMarkValue2 = runList->data[i].nLabel;
                        }
                    }
                }
            }
        }
        if (runList->data[i].nLabel == 0)
        {
            runList->data[i].nLabel = idxLabel++;
        }
    }

    equal->last = runList->data[runList->last].nLabel;
    for (i = 0; i < equal->last + 1; ++i)
    {
        equal->label[i] = 0;
    }

    for (i = 0; i < markList->last + 1; ++i)
    {
        Equal_Process(equal->label, markList->data[i].nMarkValue1, markList->data[i].nMarkValue2);
    }

    for (i = 1; i < equal->last + 1; ++i)
    {
        if (equal->label[i] == 0)
        {
            equal->label[i] = cnt;
            ++cnt;
        }
        else
        {
            equal->label[i] = equal->label[equal->label[i]];
        }
    }

    for (i = 0; i < runList->last + 1; ++i)
    {
        runList->data[i].nLabel = equal->label[runList->data[i].nLabel];
        for (j = runList->data[i].nStart; j < runList->data[i].nEnd + 1; ++j)
        {
            image[runList->data[i].nRow][j] = runList->data[i].nLabel;
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
    float sumRow, sumCol, area;
    uint8_t level = 256 / (runList->data[runList->last].nLabel);

    for (k = 1; k < runList->data[runList->last].nLabel; ++k)
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
        image[(uint16_t)(sumRow / area)][(uint16_t)(sumCol / area)] = WHITE;
    }
}

/*中值滤波器*/
void Mid_Filter(uint8_t **image)
{
    uint16_t i, j, k, l;
    uint8_t temp[9]; //用于存放中心元素及其8邻域元素
    uint8_t t;       //用于排序中的临时变量

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

/*用梯度方向和霍夫变换提取直线*/
void Hough_Line(uint8_t **image)
{
    uint8_t lineCnt;    //备选直线个数
    __IO uint32_t i, j; //for 循环计数
    int32_t theta, rho;
    int32_t x_acc, y_acc;         //x方向梯度 y方向梯度
    int32_t acc_index, acc_value; //灰度梯度直方图索引、值
    HoughLine tmpLine0, tmpLine1; //merge直线的中间量

    int32_t thetaDiff_tmp;
    int32_t thetaDiff; //两直线的偏差值
    float sinMean, cosMean;
    uint32_t magnitude;

    int32_t img_label_tmp;

    for (i = 0; i < HOUGH_THETA_SIZE * HOUGH_ROH_SIZE; i++)
    {
        houghAcc[i] = 0;
    }
    /**用sobel提取梯度，并依据梯度进行霍夫变换 */
    for (i = 1; i < OV2640_IMG_HEIGHT - 1; i++)
    {
        for (j = 1; j < OV2640_IMG_WIDTH - 1; j++)
        {
            // if (image[i][j] == WHITE) //二值化后的黑线
            // {
            /*Sobel Start*/
            x_acc = sobelGx[0][0] * image[i - 1][j - 1] + sobelGx[0][1] * image[i - 1][j] + sobelGx[0][2] * image[i - 1][j + 1] + sobelGx[1][0] * image[i][j - 1] + sobelGx[1][1] * image[i][j] + sobelGx[1][2] * image[i][j + 1] + sobelGx[2][0] * image[i + 1][j - 1] + sobelGx[2][1] * image[i + 1][j] + sobelGx[2][2] * image[i + 1][j + 1];
            y_acc = sobelGy[0][0] * image[i - 1][j - 1] + sobelGy[0][1] * image[i - 1][j] + sobelGy[0][2] * image[i - 1][j + 1] + sobelGy[1][0] * image[i][j - 1] + sobelGy[1][1] * image[i][j] + sobelGy[1][2] * image[i][j + 1] + sobelGy[2][0] * image[i + 1][j - 1] + sobelGy[2][1] * image[i + 1][j] + sobelGy[2][2] * image[i + 1][j + 1];

            /*Sobel End*/
            // if (fast_roundf(fast_sqrtf((x_acc * x_acc) + (y_acc * y_acc))) > SOBEL_THRESH)
            //     ((uint8_t **)sobelBuff)[i][j] = WHITE;
            // else
            //     ((uint8_t **)sobelBuff)[i][j] = image[i][j];
            //原点在图像左上角，roh，theta分度都为1
            theta = fast_roundf(fast_atan2f(y_acc, x_acc) * 57.295780f) % 180; // * (180 / PI)
            if (theta < 0)
                theta += 180;
            rho = fast_roundf((j * cos_table[theta]) + (i * sin_table[theta])) + IMAGE_DIAG_LEN; //偏移为正值，方便作为数组索引，后边再变回去
            acc_index = (rho * HOUGH_THETA_SIZE) + theta + 1;                                    // add offset --- 后边8邻域最大值要去掉最外边一圈，所以在这加个1，往后移一位

            acc_value = houghAcc[acc_index] + fast_roundf(fast_sqrtf((x_acc * x_acc) + (y_acc * y_acc)));
            houghAcc[acc_index] = acc_value;
            // }
        }
    }

    lineCnt = 0;
    /**用邻域最大值（8邻域）法取参数空间中可能的点*/
    for (i = 1; i < HOUGH_ROH_SIZE - 1; i++)
    {
        uint16_t *row_ptr = houghAcc + (HOUGH_THETA_SIZE * i);

        for (j = 1; j < HOUGH_THETA_SIZE - 1; j++)
        {
            if ((row_ptr[j] >= HOUGH_SOBEL_MERGE_THRESH) && (row_ptr[j] >= row_ptr[j - HOUGH_THETA_SIZE - 1]) && (row_ptr[j] >= row_ptr[j - HOUGH_THETA_SIZE]) && (row_ptr[j] >= row_ptr[j - HOUGH_THETA_SIZE + 1]) && (row_ptr[j] >= row_ptr[j - 1]) && (row_ptr[j] >= row_ptr[j + 1]) && (row_ptr[j] >= row_ptr[j + HOUGH_THETA_SIZE - 1]) && (row_ptr[j] >= row_ptr[j + HOUGH_THETA_SIZE]) && (row_ptr[j] >= row_ptr[j + HOUGH_THETA_SIZE + 1]))
            {
                lines[lineCnt].magnitude = row_ptr[j];
                lines[lineCnt].theta = j - 1; // remove offset
                lines[lineCnt].rho = i - IMAGE_DIAG_LEN;
                lines[lineCnt].mergeFlag = HOUGH_LINE_MERGE_RAW;
                lineCnt++;
            }
            row_ptr[j] = 0;
        }
    }

    /**合并同一条直线上的点 */
    for (i = 0; i < lineCnt; i++)
    { // Merge overlapping.
        if (lines[i].mergeFlag == HOUGH_LINE_MERGE_RAW)
        {
            for (j = i + 1; j < lineCnt; j++)
            {
                if (lines[j].mergeFlag == HOUGH_LINE_MERGE_RAW)
                {
                    tmpLine0.rho = lines[i].rho;
                    tmpLine1.rho = lines[j].rho;
                    tmpLine0.theta = lines[i].theta;
                    tmpLine1.theta = lines[j].theta;
                    tmpLine0.magnitude = lines[i].magnitude;
                    tmpLine1.magnitude = lines[j].magnitude;

                    if (tmpLine0.rho < 0)
                    {
                        tmpLine0.rho = -tmpLine0.rho;
                        tmpLine0.theta += 180;
                    }

                    if (tmpLine1.rho < 0)
                    {
                        tmpLine1.rho = -tmpLine1.rho;
                        tmpLine1.theta += 180;
                    }

                    thetaDiff_tmp = abs(tmpLine0.theta - tmpLine1.theta);
                    thetaDiff = (thetaDiff_tmp >= 180) ? (360 - thetaDiff_tmp) : thetaDiff_tmp;

                    if (thetaDiff < HOUGH_THETA_DIFF_MARGIN && abs(tmpLine0.rho - tmpLine1.rho) < HOUGH_ROH_DIFF_MARGIN)
                    {
                        magnitude = tmpLine0.magnitude + tmpLine1.magnitude;
                        sinMean = ((sin_table[tmpLine0.theta] * tmpLine0.magnitude) + (sin_table[tmpLine1.theta] * tmpLine1.magnitude)) / magnitude;
                        cosMean = ((cos_table[tmpLine0.theta] * tmpLine0.magnitude) + (cos_table[tmpLine1.theta] * tmpLine1.magnitude)) / magnitude;

                        lines[i].theta = fast_roundf(fast_atan2f(sinMean, cosMean) * 57.295780f) % 360; // * (180 / PI)
                        if (lines[i].theta < 0)
                            lines[i].theta += 360;
                        lines[i].rho = fast_roundf(((tmpLine0.rho * tmpLine0.magnitude) + (tmpLine1.rho * tmpLine1.magnitude)) / magnitude);
                        lines[i].magnitude = magnitude / 2;

                        if (lines[i].theta >= 180)
                        {
                            lines[i].rho = -lines[i].rho;
                            lines[i].theta -= 180;
                        }

                        lines[i].mergeFlag = HOUGH_LINE_MERGE_MERGED;
                        lines[j].mergeFlag = HOUGH_LINE_MERGE_BE_MERGED;
                    }
                }
            }
        }
    }

    for (i = 0; i < IMAGE_HEIGHT; i++)
    {
        for (j = 0; j < IMAGE_WIDTH; j++)
        {
            ((uint8_t **)sobelBuff)[i][j] = image[i][j];
        }
        // des[i][j] = 0x00;
    }

    /*mark Line*/
    for (i = 0; i < lineCnt; i++)
    {
        if (lines[i].mergeFlag == HOUGH_LINE_MERGE_MERGED)
        {
            // for (j = 0; j < IMAGE_WIDTH; j++)
            // {
            //     img_label_tmp = fast_roundf((lines[i].rho - (j * cos_table[lines[i].theta])) / sin_table[lines[i].theta]);
            //     if (img_label_tmp > 0 && img_label_tmp < IMAGE_HEIGHT)
            //     {
            //         ((uint8_t **)sobelBuff)[img_label_tmp][j] = WHITE;
            //     }
            // }

            for (j = 0; j < IMAGE_HEIGHT; j++)
            {
                img_label_tmp = fast_roundf((lines[i].rho - (j * sin_table[lines[i].theta])) / cos_table[lines[i].theta]);
                if (img_label_tmp > 0 && img_label_tmp < IMAGE_WIDTH)
                {
                    ((uint8_t **)sobelBuff)[j][img_label_tmp] = WHITE;
                }
            }
        }
    }
}
