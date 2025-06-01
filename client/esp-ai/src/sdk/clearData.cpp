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
#include "clearData.h"

void ESP_AI::clearData()
{ 
    JSONVar loc_data = get_local_all_data();
    String loc_wifi_name = loc_data["wifi_name"];
    if (loc_wifi_name == "")
    {
        // 在配网页面调用清除就清除全部配网数据
        set_local_data("wifi_name5", "");
        set_local_data("wifi_name4", "");
        set_local_data("wifi_name3", "");
        set_local_data("wifi_name2", "");
        set_local_data("wifi_pwd5", "");
        set_local_data("wifi_pwd4", "");
        set_local_data("wifi_pwd3", "");
        set_local_data("wifi_pwd2", "");
    }
    else
    {
        set_local_data("wifi_name5", loc_data["wifi_name4"]);
        set_local_data("wifi_name4", loc_data["wifi_name3"]);
        set_local_data("wifi_name3", loc_data["wifi_name2"]);
        set_local_data("wifi_name2", loc_data["wifi_name"]);
        set_local_data("wifi_pwd5", loc_data["wifi_pwd4"]);
        set_local_data("wifi_pwd4", loc_data["wifi_pwd3"]);
        set_local_data("wifi_pwd3", loc_data["wifi_pwd2"]);
        set_local_data("wifi_pwd2", loc_data["wifi_pwd"]);
    }
    set_local_data("wifi_name", "");
    set_local_data("wifi_pwd", "");
    set_local_data("api_key", "");
    set_local_data("ext1", "");
    set_local_data("ext2", "");
    set_local_data("ext3", "");
    set_local_data("ext4", "");
    set_local_data("ext5", "");
    set_local_data("ext6", "");
    set_local_data("ext7", ""); 
}
