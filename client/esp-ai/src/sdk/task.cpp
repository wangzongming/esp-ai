/*
 * MIT License
 *
 * Copyright (c) 2025-至今 小明IO
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * @author 小明IO
 * @email  1746809408@qq.com
 * @github https://github.com/wangzongming/esp-ai
 * @websit https://espai.fun
 */
#include "task.h"

void ESP_AI::delAllTask()
{ 

    if (wakeup_task_handle != NULL)
    {
        vTaskDelete(wakeup_task_handle);
        wakeup_task_handle = NULL; // 防止悬空指针
        DEBUG_PRINTLN(debug, F("[TASK] -> wakeup_task_handle 已删除。"));
    }
    if (sensor_task_handle != NULL)
    {
        vTaskDelete(sensor_task_handle);
        sensor_task_handle = NULL; // 防止悬空指针
        DEBUG_PRINTLN(debug, F("[TASK] -> sensor_task_handle 已删除。"));
    }
    if (on_wakeup_task_handle != NULL)
    {
        vTaskDelete(on_wakeup_task_handle);
        on_wakeup_task_handle = NULL; // 防止悬空指针
        DEBUG_PRINTLN(debug, F("[TASK] -> on_wakeup_task_handle 已删除。"));
    }
    if (get_position_task_handle != NULL)
    {
        vTaskDelete(get_position_task_handle);
        get_position_task_handle = NULL; // 防止悬空指针
        DEBUG_PRINTLN(debug, F("[TASK] -> get_position_task_handle 已删除。"));
    }
    if (send_audio_task_handle != NULL)
    {
        vTaskDelete(send_audio_task_handle);
        send_audio_task_handle = NULL; // 防止悬空指针
        DEBUG_PRINTLN(debug, F("[TASK] -> send_audio_task_handle 已删除。"));
    }
    if (volume_listener_task_handle != NULL)
    {
        vTaskDelete(volume_listener_task_handle);
        volume_listener_task_handle = NULL; // 防止悬空指针
        DEBUG_PRINTLN(debug, F("[TASK] -> volume_listener_task_handle 已删除。"));
    }
}


void ESP_AI::suspendAllTask()
{
    if (wakeup_task_handle != NULL)
    {
        vTaskSuspend(wakeup_task_handle);
        DEBUG_PRINTLN(debug, F("[TASK] -> wakeup_task_handle 已挂起。"));
    }
    if (sensor_task_handle != NULL)
    {
        vTaskSuspend(sensor_task_handle);
        DEBUG_PRINTLN(debug, F("[TASK] -> sensor_task_handle 已挂起。"));
    }
    if (on_wakeup_task_handle != NULL)
    {
        vTaskSuspend(on_wakeup_task_handle);
        DEBUG_PRINTLN(debug, F("[TASK] -> on_wakeup_task_handle 已挂起。"));
    }
    if (get_position_task_handle != NULL)
    {
        vTaskSuspend(get_position_task_handle);
        DEBUG_PRINTLN(debug, F("[TASK] -> get_position_task_handle 已挂起。"));
    }
    if (send_audio_task_handle != NULL)
    {
        vTaskSuspend(send_audio_task_handle);
        DEBUG_PRINTLN(debug, F("[TASK] -> send_audio_task_handle 已挂起。"));
    }
    if (volume_listener_task_handle != NULL)
    {
        vTaskSuspend(volume_listener_task_handle);
        DEBUG_PRINTLN(debug, F("[TASK] -> volume_listener_task_handle 已挂起。"));
    } 
}

void ESP_AI::resumeAllTask()
{
    if (wakeup_task_handle != NULL)
    {
        vTaskResume(wakeup_task_handle);
        DEBUG_PRINTLN(debug, F("[TASK] -> wakeup_task_handle 已恢复"));
    }
    if (sensor_task_handle != NULL)
    {
        vTaskResume(sensor_task_handle);
        DEBUG_PRINTLN(debug, F("[TASK] -> sensor_task_handle 已恢复"));
    }
    if (on_wakeup_task_handle != NULL)
    {
        vTaskResume(on_wakeup_task_handle);
        DEBUG_PRINTLN(debug, F("[TASK] -> on_wakeup_task_handle 已恢复"));
    }
    if (get_position_task_handle != NULL)
    {
        vTaskResume(get_position_task_handle);
        DEBUG_PRINTLN(debug, F("[TASK] -> get_position_task_handle 已恢复"));
    }
    if (send_audio_task_handle != NULL)
    {
        vTaskResume(send_audio_task_handle);
        DEBUG_PRINTLN(debug, F("[TASK] -> send_audio_task_handle 已恢复"));
    }
    if (volume_listener_task_handle != NULL)
    {
        vTaskResume(volume_listener_task_handle);
        DEBUG_PRINTLN(debug, F("[TASK] -> volume_listener_task_handle 已恢复"));
    }
}