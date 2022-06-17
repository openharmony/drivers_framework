/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include <iostream>
#include <string>
#include <string_ex.h>
#include <hdf_io_service.h>
#include <idevmgr_hdi.h>
#include <iservmgr_hdi.h>
#include <osal_time.h>

#define HDF_LOG_TAG hdf_dbg

static constexpr uint32_t DATA_SIZE = 5000;
static constexpr uint32_t FUNC_IDX = 1;
static constexpr uint32_t SERVER_NAME_IDX = 3;
static constexpr uint32_t INTERFACE_DESC_IDX = 4;
static constexpr uint32_t CMD_ID_IDX = 5;
static constexpr uint32_t PARA_CNT_IDX = 6;
static constexpr uint32_t PARA_MULTIPLE = 2;
static constexpr uint32_t WAIT_TIME = 100;
static constexpr uint32_t HELP_INFO_PARA_CNT = 2;
static constexpr int32_t DBG_HDI_PARA_MIN_LEN = 7;
static constexpr int32_t DBG_HDI_SERVICE_LOAD_IDX = 2;
static constexpr int32_t QUERY_INFO_PARA_CNT = 3;
static constexpr const char *HELP_COMMENT =
    " hdf_dbg menu:  \n"
    " hdf_dbg -h   :display help information\n"
    " hdf_dbg -q   :query all service and device information\n"
    " hdf_dbg -q 0 :query service information of kernel space\n"
    " hdf_dbg -q 1 :query service information of user space\n"
    " hdf_dbg -q 2 :query device information of kernel space\n"
    " hdf_dbg -q 3 :query device information use space\n"
    " hdf_dbg -d   :debug hdi interface\n"
    "   detailed usage:\n"
    "   debug hdi interface, parameterType can be int or string now, for example:\n"
    "     hdf_dbg -d loadFlag serviceName interfaceToken cmd parameterCount parameterType parameterValue\n"
    "     detailed examples:\n"
    "       hdf_dbg -d 1 sample_driver_service hdf.test.sampele_service 1 2 int 100 int 200\n"
    "       hdf_dbg -d 0 sample_driver_service hdf.test.sampele_service 7 1 string foo\n";

using GetInfoFunc = void (*)();
static void GetAllServiceUserSpace();
static void GetAllServiceKernelSpace();
static void GetAllDeviceUserSpace();
static void GetAllDeviceKernelSpace();
static void GetAllInformation();

GetInfoFunc g_getInfoFuncs[] = {
    GetAllServiceKernelSpace,
    GetAllServiceUserSpace,
    GetAllDeviceKernelSpace,
    GetAllDeviceUserSpace,
    GetAllInformation,
};

using OHOS::MessageOption;
using OHOS::MessageParcel;
using OHOS::HDI::DeviceManager::V1_0::HdiDevHostInfo;
using OHOS::HDI::DeviceManager::V1_0::IDeviceManager;
using OHOS::HDI::ServiceManager::V1_0::HdiServiceInfo;
using OHOS::HDI::ServiceManager::V1_0::IServiceManager;
using std::cout;
using std::endl;

static void PrintHelp()
{
    cout << HELP_COMMENT;
}

static void PrintAllServiceInfoUser(const std::vector<HdiServiceInfo> &serviceInfos)
{
    uint32_t cnt = 0;
    cout << "display service info in user space, format:" << endl;
    cout << "servName:\t" << "devClass" << endl;
    for (auto &info : serviceInfos) {
        cout << info.serviceName << ":\t0x" << std::hex << info.devClass << endl;
        cnt++;
    }

    cout << "total " << std::dec << cnt << " services in user space" << endl;
}

static void PrintAllServiceInfoKernel(struct HdfSBuf *data, bool flag)
{
    uint32_t cnt = 0;
    cout << "display service info in kernel space, format:" << endl;
    cout << "servName:\t" << "devClass" << endl;
    while (flag) {
        const char *servName = HdfSbufReadString(data);
        if (servName == nullptr) {
            break;
        }
        uint16_t devClass;
        if (!HdfSbufReadUint16(data, &devClass)) {
            return;
        }
        cout << servName << ":\t0x" << std::hex << devClass << endl;
        cnt++;
    }

    cout << "total " << std::dec << cnt << " services in kernel space" << endl;
}

static void PrintALLDeviceInfoUser(const std::vector<HdiDevHostInfo> &deviceInfos)
{
    cout << "display device info in user space, format:" << endl;
    cout << "hostName:\t" << "hostId" << endl;
    cout << "deviceId" << endl;
    uint32_t hostCnt = 0;
    uint32_t devNodeCnt = 0;
    for (auto &info : deviceInfos) {
        cout << info.hostName << ":\t0x" << std::hex << info.hostId << endl;
        for (auto &id : info.devId) {
            cout << "0x" << std::hex << id << endl;
            devNodeCnt++;
        }
        hostCnt++;
    }

    cout << "total " << std::dec << hostCnt << " hosts, " << devNodeCnt << " devNodes in user space" << endl;
}

static int32_t PrintOneHostInfoKernel(struct HdfSBuf *data, uint32_t &devNodeCnt)
{
    const char *hostName = HdfSbufReadString(data);
    if (hostName == nullptr) {
        return HDF_FAILURE;
    }
    uint32_t hostId;
    if (!HdfSbufReadUint32(data, &hostId)) {
        cout << "PrintOneHostInfoKernel HdfSbufReadUint32 hostId failed" << endl;
        return HDF_FAILURE;
    }

    cout << hostName << ":\t0x" << std::hex << hostId << endl;
    uint32_t devCnt;
    if (!HdfSbufReadUint32(data, &devCnt)) {
        cout << "PrintOneHostInfoKernel HdfSbufReadUint32 devCnt failed" << endl;
        return HDF_FAILURE;
    }
    for (uint32_t i = 0; i < devCnt; i++) {
        uint32_t devId;
        if (!HdfSbufReadUint32(data, &devId)) {
            cout << "PrintOneHostInfoKernel HdfSbufReadUint32 devId failed" << endl;
            return HDF_FAILURE;
        }
        cout << "0x" << std::hex << devId << endl;
    }
    devNodeCnt += devCnt;
    return HDF_SUCCESS;
}
static void PrintAllDeviceInfoKernel(struct HdfSBuf *data, bool flag)
{
    uint32_t hostCnt = 0;
    uint32_t devNodeCnt = 0;
    cout << "display device info in kernel space, format:" << endl;
    cout << "hostName:" << '\t' << "hostId" << endl;
    cout << "deviceId" << endl;

    while (flag) {
        if (PrintOneHostInfoKernel(data, devNodeCnt) == HDF_FAILURE) {
            break;
        }
        hostCnt++;
    }

    cout << "total " << std::dec << hostCnt << " hosts, " << devNodeCnt << " devNodes in kernel space" << endl;
}

static int32_t ParseHdiParameter(int argc, char **argv, MessageParcel &data)
{
    int32_t paraCnt = atoi(argv[PARA_CNT_IDX]);
    if ((paraCnt * PARA_MULTIPLE) != (argc - PARA_CNT_IDX - 1)) {
        cout << "parameter count error, input: " << paraCnt << " real: " << (argc - PARA_CNT_IDX - 1) << endl;
        return HDF_FAILURE;
    }
    int32_t paraTypeIdx = PARA_CNT_IDX + 1;
    for (int i = 0; i < paraCnt; i++) {
        int32_t paraValueIdx = paraTypeIdx + 1;
        if (strcmp(argv[paraTypeIdx], "string") == 0) {
            data.WriteCString(argv[paraValueIdx]);
        } else if (strcmp(argv[paraTypeIdx], "int") == 0) {
            data.WriteInt32(atoi(argv[paraValueIdx]));
        } else {
            cout << "parameterType error:" << argv[paraTypeIdx] << endl;
            return HDF_FAILURE;
        }
        paraTypeIdx += PARA_MULTIPLE;
    }
    return HDF_SUCCESS;
}

static int32_t InjectDebugHdi(int argc, char **argv)
{
    if (argc < DBG_HDI_PARA_MIN_LEN) {
        PrintHelp();
        return HDF_FAILURE;
    }

    auto servmgr = IServiceManager::Get();
    auto devmgr = IDeviceManager::Get();
    int32_t loadFlag = atoi(argv[DBG_HDI_SERVICE_LOAD_IDX]);
    MessageParcel data;
    data.WriteInterfaceToken(OHOS::Str8ToStr16(argv[INTERFACE_DESC_IDX]));
    int32_t ret = ParseHdiParameter(argc, argv, data);
    if (ret != HDF_SUCCESS) {
        PrintHelp();
        return HDF_FAILURE;
    }
    if (loadFlag == 1) {
        devmgr->LoadDevice(argv[SERVER_NAME_IDX]);
        OsalMSleep(WAIT_TIME);
    }

    MessageParcel reply;
    MessageOption option;
    auto service = servmgr->GetService(argv[SERVER_NAME_IDX]);
    if (service == nullptr) {
        cout << "getService " << argv[SERVER_NAME_IDX] << " failed" << endl;
        goto END;
    }

    ret = service->SendRequest(atoi(argv[CMD_ID_IDX]), data, reply, option);
    cout << "call service " << argv[SERVER_NAME_IDX] << " hdi cmd:" << atoi(argv[CMD_ID_IDX]) << " return:" << ret
         << endl;
END:
    if (loadFlag == 1) {
        devmgr->UnloadDevice(argv[SERVER_NAME_IDX]);
    }
    return ret;
}

static void GetAllServiceUserSpace()
{
    auto servmgr = IServiceManager::Get();
    if (servmgr == nullptr) {
        cout << "GetAllServiceUserSpace get ServiceManager failed" << endl;
        return;
    }
    std::vector<HdiServiceInfo> serviceInfos;
    (void)servmgr->ListAllService(serviceInfos);

    PrintAllServiceInfoUser(serviceInfos);
}

static void GetAllServiceKernelSpace()
{
    struct HdfSBuf *data = HdfSbufObtain(DATA_SIZE);
    if (data == nullptr) {
        cout << "GetAllServiceKernelSpace HdfSbufObtain failed" << endl;
        return;
    }
    int32_t ret = HdfListAllService(data);
    OsalMSleep(WAIT_TIME);
    if (ret == HDF_SUCCESS) {
        PrintAllServiceInfoKernel(data, true);
    } else {
        PrintAllServiceInfoKernel(data, false);
    }

    HdfSbufRecycle(data);
}

static void GetAllDeviceUserSpace()
{
    auto devmgr = IDeviceManager::Get();
    if (devmgr == nullptr) {
        cout << "GetAllDeviceUserSpace get DeviceManager failed" << endl;
        return;
    }
    std::vector<HdiDevHostInfo> deviceInfos;
    (void)devmgr->ListAllDevice(deviceInfos);
    PrintALLDeviceInfoUser(deviceInfos);
}

static void GetAllDeviceKernelSpace()
{
    struct HdfSBuf *data = HdfSbufObtain(DATA_SIZE);
    if (data == nullptr) {
        cout << "GetAllDeviceKernelSpace HdfSbufObtain failed" << endl;
        return;
    }

    int32_t ret = HdfListAllDevice(data);
    OsalMSleep(WAIT_TIME);
    if (ret == HDF_SUCCESS) {
        PrintAllDeviceInfoKernel(data, true);
    } else {
        PrintAllDeviceInfoKernel(data, false);
    }
    HdfSbufRecycle(data);
}

static void GetAllInformation()
{
    GetAllServiceUserSpace();
    GetAllServiceKernelSpace();
    GetAllDeviceUserSpace();
    GetAllDeviceKernelSpace();
}

int main(int argc, char **argv)
{
    if (argc == 1 || (argc == HELP_INFO_PARA_CNT && strcmp(argv[FUNC_IDX], "-h") == 0)) {
        PrintHelp();
    } else if (argc == QUERY_INFO_PARA_CNT || argc == QUERY_INFO_PARA_CNT - 1) {
        if (strcmp(argv[FUNC_IDX], "-q") != 0) {
            PrintHelp();
            return HDF_FAILURE;
        }
        uint32_t funcCnt = sizeof(g_getInfoFuncs) / sizeof(g_getInfoFuncs[0]);
        if (argc == QUERY_INFO_PARA_CNT - 1) {
            g_getInfoFuncs[funcCnt - 1]();
            return HDF_SUCCESS;
        }
        uint32_t queryIdx = atoi(argv[QUERY_INFO_PARA_CNT - 1]);
        if (queryIdx < funcCnt - 1) {
            g_getInfoFuncs[queryIdx]();
        } else {
            PrintHelp();
            return HDF_FAILURE;
        }
    } else if (argc > QUERY_INFO_PARA_CNT) {
        if (strcmp(argv[FUNC_IDX], "-d") == 0) {
            return InjectDebugHdi(argc, argv);
        } else {
            PrintHelp();
            return HDF_FAILURE;
        }
    }

    return HDF_SUCCESS;
}
