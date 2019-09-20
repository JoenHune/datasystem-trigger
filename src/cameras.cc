#include "st/stm32f10x_tim.h"

#include <cmath>
#include <stdio.h>

#include "cameras.h"

uint32_t exposure_time[TOTAL_CAM][TOTAL_KIND] = {0};
int32_t predict_error[TOTAL_CAM][CAMERA_FREQUENCE] = {0};
uint8_t index[TOTAL_CAM] = {0};

// 计算触发信号的高电平时间(即上升沿时间)
uint32_t trigger_time(uint32_t expect_exposure_time)
{
    return static_cast<uint32_t>(TRIGGER_PERIOD - expect_exposure_time / 2.f);
}

// 记录上一帧图像的曝光时间
void record_exposure_time(uint32_t exposure_time[], uint32_t last_time)
{
    // 上上帧的曝光时间
    exposure_time[LLAST] = exposure_time[LAST];
    // 上一帧曝光时间的预测误差
    exposure_time[ERR] = abs(exposure_time[LAST] - last_time);
    // 上一帧的曝光时间
    exposure_time[LAST] = last_time;

    // 误差累计
    // index在触发信号定时器的update中断中迭代
    // 同时update中断里还将对应的predict_error置为-1来表示默认丢帧
    // 详见update_predict_error_index()
    if (exposure_time == &exposure_time[CAM1])
    {
        predict_error[CAM1][index[CAM1]] = exposure_time[ERR];
    }
    else
    {
        predict_error[CAM2][index[CAM2]] = exposure_time[ERR];
    }
}

// 预测下一帧图像的曝光时间
void calculate_trigger_time(uint32_t exposure_time[], uint32_t min, uint32_t max)
{
    // 策略：预测下次曝光时间和上次曝光时间相同
    exposure_time[EXPECT] = exposure_time[LAST];

    // 设置预测曝光时间的上下限
    // 其中：
    // 时间下限：相机对曝光触发信号有最低高电平需求时间100us
    // 时间上限：当两帧曝光时间差距过大时，一个周期内无法容纳两次曝光，故下一次曝光的提前时间需要做出让步：1/2 * LLAST + C0 + LAST + C0 + 1/2 * EXPECT <= PERIOD
    //  （ 如10Hz触发时，最大曝光周期只允许100ms，而假设LLAST的对齐误差为0，当LLAST=10ms，LAST=70ms，当LAST完成曝光时，   ）
    //  （ 知当前周期已剩下100+5-70=35ms，所以下一帧最多只能提前35ms-C0曝光，其中C0为常数，表示相机两帧曝光之间需要的空闲时间 ）
    //  （ 即无法完成不短于70ms的曝光的对齐，只能尽可能缩小对齐误差                                                    ）
    auto max_addvance_time = static_cast<uint32_t>((2.f * (max - 2.f * C0 - exposure_time[LAST] - exposure_time[LLAST])));
    limit<uint32_t>(exposure_time[EXPECT], min, max_addvance_time);

    // 限幅的意义：
    // 如果实际曝光时间在限幅范围内，则理论上可以对齐下一帧的中心曝光时刻，若实际曝光时间长于(或短于，但短于的情况不会发生)限幅范围，则无法对齐
}

// 根据前一帧和前两帧图像的曝光时间计算下一帧图像的触发时间并完成相应设置
void update_camera_trigger_time(CAM_ID camera, TIM_TypeDef *detect_timer, TIM_TypeDef *trigger_timer, uint8_t trigger_channel)
{
    auto time = exposure_time[camera];

    // 记录上一帧(刚结束的一帧)和上上帧图像的曝光时间
    record_exposure_time(time, static_cast<uint32_t>(TIM_GetCounter(detect_timer) * FACTOR_DETECTOR_CNT_TO_US));

    // 预测下一帧图像的曝光时间
    calculate_trigger_time(time, MIN_EXPOSURE_TIME_US, MAX_EXPOSURE_TIME_US);

    // 量纲转换
    // 和生成触发信号的定时器TIM3的配置设置有关系
    // 因为TIM3配置成了PWM2模式，即定时器counter计数到compare时才出现高电平
    // 因此实际上需要设置的compare是compare = period - factor * 需要提前的时间，其中factor是us到定时器counter的时基变换因子
    time[EXPECT] = trigger_time(FACTOR_US_TO_TRIGGER_CNT * (time[EXPECT]));

    // 设置触发信号
    switch (trigger_channel)
    {
    case TIM_Channel_1:
        TIM_SetCompare1(trigger_timer, time[EXPECT]);
        break;

    case TIM_Channel_2:
        TIM_SetCompare2(trigger_timer, time[EXPECT]);
        break;

    case TIM_Channel_3:
        TIM_SetCompare3(trigger_timer, time[EXPECT]);
        break;

    case TIM_Channel_4:
        TIM_SetCompare4(trigger_timer, time[EXPECT]);
        break;

    default:
        break;
    }

    // 重制曝光时间输入捕获的定时器的counter，便于下一次计时
    TIM_SetCounter(detect_timer, 0);
}

void update_predict_error_index()
{
    for (auto camera = 0; camera < TOTAL_CAM; camera++)
    {
        index[camera]++;

        if (index[camera] >= CAMERA_FREQUENCE)
        {
            index[camera] = 0;
        }

        // 先默认所有帧都会丢，当收到下一帧的上升沿时，正常计算曝光时间误差
        predict_error[camera][index[camera]] = LOST;
    }
}

// 计算前若干个包的预测误差
float avg_error(CAM_ID camera, uint8_t number)
{
    float error = 0;
    for (uint8_t i = 0; i < number; i++)
    {
        if (predict_error[camera][ERR] != LOST)
            error += predict_error[camera][ERR];
    }
    error /= number;

    return error;
}

// 计算前若干个包的预测误差
float avg_error(CAM_ID camera)
{
    return avg_error(camera, CAMERA_FREQUENCE);
}

// 调试信息buffer
uint8_t message[USART3_TX_BUFFER_SIZE] = {0};

// 输出已记录的某个相机的所有误差，其中-1代表对应帧丢帧
void message_show_all_errors(CAM_ID camera)
{
    uint8_t length = sprintf(reinterpret_cast<char *>(message),
                             "e0=%7ld(us) e1=%7ld(us) e2=%7ld(us) e3=%7ld(us) e4=%7ld(us) e5=%7ld(us) e6=%7ld(us) e7=%7ld(us) e8=%7ld(us) e9=%7ld \n",
                             predict_error[camera][0],
                             predict_error[camera][1],
                             predict_error[camera][2],
                             predict_error[camera][3],
                             predict_error[camera][4],
                             predict_error[camera][5],
                             predict_error[camera][6],
                             predict_error[camera][7],
                             predict_error[camera][8],
                             predict_error[camera][9]);

    usart3_printblock(message, length);
}

// 输出某个相机的丢帧情况，用one-hot法描述
void message_show_frame_id(CAM_ID camera)
{
    uint32_t code = 0;

    for (uint8_t i = 0; i < CAMERA_FREQUENCE; i++)
    {
        if (predict_error[camera][i] == LOST)
        {
            code &= (0x01 << i);
        }
    }

    uint8_t length = sprintf(reinterpret_cast<char *>(message),
                             "camera: %d, state: %4X \n",
                             camera,
                             code);

    usart3_printblock(message, length);
}