/* *****************************************************************************
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022 All rights reserved.
 * eagle licensed under the Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 * http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR
 * PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Author: queyanwen
 * Create: 2022-06-23
 * Description: initialize and app enter
 * **************************************************************************** */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include "config.h"
#include "log.h"
#include "common.h"
#include "eaglecore.h"

#define MAIN_THREAD_LOOP_INTERVAL 5
static int g_keepMainRunning;

static void PrintUsage(const char *args[])
{
    printf("Usage: %s < config_file_name > \n", args[0]);
}

static int BaseInit(void)
{
    int ret = SUCCESS;
    ret = InitConfig();
    if (ret != SUCCESS) {
        return ret;
    }

    ret = InitLogger();
    if (ret != SUCCESS) {
        return ret;
    }

    ret = InitEagleSystem();
    if (ret != SUCCESS) {
        return ret;
    }
    // todo 其他必要的初始化
    return SUCCESS;
}

static void ClearEnv(void)
{
    // todo：必要的环境清理动作
    ClearLogger();
}

static void SignalHandler(int none)
{
    g_keepMainRunning = FALSE;
}

static void SetupSignal(void)
{
    // regist signal handler
    (void)signal(SIGINT, SignalHandler);
    (void)signal(SIGUSR1, SignalHandler);
    (void)signal(SIGUSR2, SignalHandler);
    (void)signal(SIGTERM, SignalHandler);
    (void)signal(SIGKILL, SignalHandler);
}

int main(int argc, const char *args[])
{
    int ret;
    /* if (argc < APP_ARGC) {
        PrintUsage(args);
        return -1;
    } */ // todo 增加启动参数，比如指定配置文件路径

    ret = BaseInit();
    if (ret != SUCCESS) {
        Logger(ERROR, MD_NM_MAN, "BaseInit failed. ret:%d", ret);
        exit(-1);
    }
    ret = StartEagleSystem();
    if (ret != SUCCESS) {
        Logger(ERROR, MD_NM_MAN, "StartEagleSystem failed. ret:%d", ret);
        exit(-1);
    }

    SetupSignal();
    g_keepMainRunning = TRUE;
    unsigned int count = 0;
    while (g_keepMainRunning) {
        sleep(MAIN_THREAD_LOOP_INTERVAL);
        count++;
        if (count % (GetTimerCfg()->cfgUpdataInterval / MAIN_THREAD_LOOP_INTERVAL) == 0) {
            CheckAndUpdateConfig();
        }
        if (count % (GetTimerCfg()->policyUpdateInterval / MAIN_THREAD_LOOP_INTERVAL) == 0) {
            CheckAndUpdatePolicy();
        }
    }
    StopEagleSystem();
    ClearEnv();
    return 0;
}
