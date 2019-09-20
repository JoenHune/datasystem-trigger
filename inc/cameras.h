#pragma once

#include "configuration.h"

#include "basic/common.h"

typedef enum
{
    LLAST = 0, // 上上次曝光时间
    LAST,      // 上次曝光时间(即刚结束的一帧的曝光时间)
    EXPECT,    // 预测下次的曝光时间
    ERR,       // 上次预测的误差
    TOTAL_KIND // 总数(此项一定要放在最后)
} EXPOSURE_TIME_KIND;

typedef enum
{
    CAM1 = 0,
    CAM2,
    CAM3,
    CAM4,
    TOTAL_CAM // 总数(此项一定要放在最后)
} CAM_ID;

// 标识位，表示对应帧图像出现丢帧
static const int16_t LOST = -1;

// extern uint16_t exposure_time[TOTAL_CAM][TOTAL_KIND];
// extern int16_t predict_error[TOTAL_CAM][CAMERA_FREQUENCE];
// extern uint8_t index[TOTAL_CAM];

// 根据前一帧和前两帧图像的曝光时间计算下一帧图像的触发时间并完成相应设置
void update_camera_trigger_time(CAM_ID camera, TIM_TypeDef *detect_timer, TIM_TypeDef *trigger_timer, uint8_t trigger_channel);

// 用于记录是否丢帧
void update_predict_error_index();

// 计算前若干个包的预测误差
float avg_error(CAM_ID camera);
float avg_error(CAM_ID camera, uint8_t number);

// 输出已记录的所有误差
void message_show_all_errors(CAM_ID camera);