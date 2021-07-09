/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "audio_sapm.h"
#include "osal_io.h"
#include "osal_time.h"
#include "osal_timer.h"

#define HDF_LOG_TAG audio_sapm

static void AudioSapmEnterSleep(uintptr_t para);

/* power up sequences */
static int32_t g_audioSapmPowerUpSeq[] = {
    [AUDIO_SAPM_PRE] = 0,             /* 0 is audio sapm power up sequences */
    [AUDIO_SAPM_SUPPLY] = 1,          /* 1 is audio sapm power up sequences */
    [AUDIO_SAPM_MICBIAS] = 2,         /* 2 is audio sapm power up sequences */
    [AUDIO_SAPM_AIF_IN] = 3,          /* 3 is audio sapm power up sequences */
    [AUDIO_SAPM_AIF_OUT] = 3,         /* 3 is audio sapm power up sequences */
    [AUDIO_SAPM_MIC] = 4,             /* 4 is audio sapm power up sequences */
    [AUDIO_SAPM_MUX] = 5,             /* 5 is audio sapm power up sequences */
    [AUDIO_SAPM_VIRT_MUX] = 5,        /* 5 is audio sapm power up sequences */
    [AUDIO_SAPM_VALUE_MUX] = 5,       /* 5 is audio sapm power up sequences */
    [AUDIO_SAPM_DAC] = 6,             /* 6 is audio sapm power up sequences */
    [AUDIO_SAPM_MIXER] = 7,           /* 7 is audio sapm power up sequences */
    [AUDIO_SAPM_MIXER_NAMED_CTRL] = 7, /* 7 is audio sapm power up sequences */
    [AUDIO_SAPM_PGA] = 8,             /* 8 is audio sapm power up sequences */
    [AUDIO_SAPM_ADC] = 9,             /* 9 is audio sapm power up sequences */
    [AUDIO_SAPM_OUT_DRV] = 10,        /* 10 is audio sapm power up sequences */
    [AUDIO_SAPM_HP] = 10,             /* 10 is audio sapm power up sequences */
    [AUDIO_SAPM_SPK] = 10,            /* 10 is audio sapm power up sequences */
    [AUDIO_SAPM_POST] = 11,           /* 11 is audio sapm power up sequences */
};

/* power down sequences */
static int32_t g_audioSapmPowerDownSeq[] = {
    [AUDIO_SAPM_PRE] = 0,             /* 0 is audio sapm power down sequences */
    [AUDIO_SAPM_ADC] = 1,             /* 1 is audio sapm power down sequences */
    [AUDIO_SAPM_HP] = 2,              /* 2 is audio sapm power down sequences */
    [AUDIO_SAPM_SPK] = 2,             /* 2 is audio sapm power down sequences */
    [AUDIO_SAPM_OUT_DRV] = 2,         /* 2 is audio sapm power down sequences */
    [AUDIO_SAPM_PGA] = 4,             /* 4 is audio sapm power down sequences */
    [AUDIO_SAPM_MIXER_NAMED_CTRL] = 5, /* 5 is audio sapm power down sequences */
    [AUDIO_SAPM_MIXER] = 5,           /* 5 is audio sapm power down sequences */
    [AUDIO_SAPM_DAC] = 6,             /* 6 is audio sapm power down sequences */
    [AUDIO_SAPM_MIC] = 7,             /* 7 is audio sapm power down sequences */
    [AUDIO_SAPM_MICBIAS] = 8,         /* 8 is audio sapm power down sequences */
    [AUDIO_SAPM_MUX] = 9,             /* 9 is audio sapm power down sequences */
    [AUDIO_SAPM_VIRT_MUX] = 9,        /* 9 is audio sapm power down sequences */
    [AUDIO_SAPM_VALUE_MUX] = 9,       /* 9 is audio sapm power down sequences */
    [AUDIO_SAPM_AIF_IN] = 10,         /* 10 is audio sapm power down sequences */
    [AUDIO_SAPM_AIF_OUT] = 10,        /* 10 is audio sapm power down sequences */
    [AUDIO_SAPM_SUPPLY] = 11,         /* 11 is audio sapm power down sequences */
    [AUDIO_SAPM_POST] = 12,           /* 12 is audio sapm power down sequences */
};

static int32_t g_audioSapmIsSleep = 0;
static int32_t g_audioSapmIsStandby = 0;
OSAL_DECLARE_TIMER(g_sleepTimer);

static int32_t ConnectedInputEndPoint(const struct AudioSapmComponent *cpt)
{
    struct AudioSapmpath *path = NULL;
    int32_t count = 0;
    int32_t endPointVal = 1;

    if (cpt == NULL) {
        ADM_LOG_ERR("input param cpt is NULL.");
        return HDF_FAILURE;
    }

    switch (cpt->sapmType) {
        case AUDIO_SAPM_DAC:
        case AUDIO_SAPM_AIF_IN:
        case AUDIO_SAPM_INPUT:
        case AUDIO_SAPM_MIC:
        case AUDIO_SAPM_LINE:
            return endPointVal;
        default:
            break;
    }

    DLIST_FOR_EACH_ENTRY(path, &cpt->sources, struct AudioSapmpath, listSink) {
        if ((path->source != NULL) && (path->connect == CONNECT_SINK_AND_SOURCE)) {
            count += ConnectedInputEndPoint(path->source);
        }
    }
    return count;
}

static int32_t ConnectedOutputEndPoint(const struct AudioSapmComponent *cpt)
{
    struct AudioSapmpath *path = NULL;
    int32_t count = 0;
    int32_t endPointVal = 1;

    if (cpt == NULL) {
        ADM_LOG_ERR("input param cpt is NULL.");
        return HDF_FAILURE;
    }

    switch (cpt->sapmType) {
        case AUDIO_SAPM_ADC:
        case AUDIO_SAPM_AIF_OUT:
        case AUDIO_SAPM_OUTPUT:
        case AUDIO_SAPM_HP:
        case AUDIO_SAPM_SPK:
        case AUDIO_SAPM_LINE:
            return endPointVal;
        default:
            break;
    }

    DLIST_FOR_EACH_ENTRY(path, &cpt->sinks, struct AudioSapmpath, listSource) {
        if ((path->sink != NULL) && (path->connect == CONNECT_SINK_AND_SOURCE)) {
            count += ConnectedOutputEndPoint(path->sink);
        }
    }
    return count;
}

static int32_t AudioSapmGenericCheckPower(const struct AudioSapmComponent *cpt)
{
    int32_t input;
    int32_t output;

    if (cpt == NULL) {
        ADM_LOG_ERR("input param cpt is NULL.");
        return HDF_FAILURE;
    }

    input = ConnectedInputEndPoint(cpt);
    if (input == HDF_FAILURE) {
        ADM_LOG_ERR("input endpoint fail!");
        return HDF_FAILURE;
    }
    output = ConnectedOutputEndPoint(cpt);
    if (output == HDF_FAILURE) {
        ADM_LOG_ERR("output endpoint fail!");
        return HDF_FAILURE;
    }

    if ((input == 0) || (output == 0)) {
        ADM_LOG_DEBUG("component is not in a complete path.");
        return SAPM_POWER_DOWN;
    }
    return SAPM_POWER_UP;
}

static int32_t AudioSapmAdcCheckPower(const struct AudioSapmComponent *cpt)
{
    int32_t input;

    if (cpt == NULL) {
        ADM_LOG_ERR("input param cpt is NULL.");
        return HDF_FAILURE;
    }

    if (cpt->active == 0) {
        input = AudioSapmGenericCheckPower(cpt);
    } else {
        input = ConnectedInputEndPoint(cpt);
    }
    if (input == HDF_FAILURE) {
        ADM_LOG_ERR("input endpoint fail!");
        return HDF_FAILURE;
    }
    return input;
}

static int AudioSapmDacCheckPower(const struct AudioSapmComponent *cpt)
{
    int32_t output;

    if (cpt == NULL) {
        ADM_LOG_ERR("input param cpt is NULL.");
        return HDF_FAILURE;
    }

    if (cpt->active == 0) {
        output = AudioSapmGenericCheckPower(cpt);
    } else {
        output = ConnectedOutputEndPoint(cpt);
    }
    if (output == HDF_FAILURE) {
        ADM_LOG_ERR("output endpoint fail!");
        return HDF_FAILURE;
    }
    return output;
}

static void AudioSampCheckPowerCallback(struct AudioSapmComponent *cpt)
{
    if (cpt == NULL) {
        ADM_LOG_ERR("input param cpt is NULL.");
        return;
    }

    switch (cpt->sapmType) {
        case AUDIO_SAPM_ANALOG_SWITCH:
        case AUDIO_SAPM_MIXER:
        case AUDIO_SAPM_MIXER_NAMED_CTRL:
            cpt->PowerCheck = AudioSapmGenericCheckPower;
            break;
        case AUDIO_SAPM_MUX:
        case AUDIO_SAPM_VIRT_MUX:
        case AUDIO_SAPM_VALUE_MUX:
            cpt->PowerCheck = AudioSapmGenericCheckPower;
            break;
        case AUDIO_SAPM_ADC:
        case AUDIO_SAPM_AIF_OUT:
            cpt->PowerCheck = AudioSapmAdcCheckPower;
            break;
        case AUDIO_SAPM_DAC:
        case AUDIO_SAPM_AIF_IN:
            cpt->PowerCheck = AudioSapmDacCheckPower;
            break;
        case AUDIO_SAPM_PGA:
        case AUDIO_SAPM_OUT_DRV:
        case AUDIO_SAPM_INPUT:
        case AUDIO_SAPM_OUTPUT:
        case AUDIO_SAPM_MICBIAS:
        case AUDIO_SAPM_SPK:
        case AUDIO_SAPM_HP:
        case AUDIO_SAPM_MIC:
        case AUDIO_SAPM_LINE:
            cpt->PowerCheck = AudioSapmGenericCheckPower;
            break;
        default:
            cpt->PowerCheck = AudioSapmGenericCheckPower;
            break;
    }

    return;
}

int32_t AudioSapmNewComponent(struct AudioCard *audioCard, const struct AudioSapmComponent *component)
{
    struct AudioSapmComponent *cpt = NULL;

    if ((audioCard == NULL) || (component == NULL)) {
        ADM_LOG_ERR("input params check error: audioCard=%p, component=%p.", audioCard, component);
        return HDF_FAILURE;
    }

    cpt = (struct AudioSapmComponent *)OsalMemCalloc(sizeof(struct AudioSapmComponent));
    if (cpt == NULL) {
        ADM_LOG_ERR("malloc cpt fail!");
        return HDF_FAILURE;
    }

    if (memcpy_s(cpt, sizeof(struct AudioSapmComponent), component, sizeof(struct AudioSapmComponent)) != EOK) {
        ADM_LOG_ERR("memcpy cpt fail!");
        OsalMemFree(cpt);
        return HDF_FAILURE;
    }

    cpt->componentName = (char *)OsalMemCalloc(strlen(component->componentName) + 1);
    if (cpt->componentName == NULL) {
        ADM_LOG_ERR("malloc cpt->componentName fail!");
        OsalMemFree(cpt);
        return HDF_FAILURE;
    }
    if (memcpy_s(cpt->componentName, strlen(component->componentName) + 1,
        component->componentName, strlen(component->componentName) + 1) != EOK) {
        ADM_LOG_ERR("memcpy cpt->componentName fail!");
        OsalMemFree(cpt->componentName);
        OsalMemFree(cpt);
        return HDF_FAILURE;
    }
    cpt->codec = audioCard->rtd->codec;
    cpt->kcontrolsNum = component->kcontrolsNum;
    cpt->active = 0;
    AudioSampCheckPowerCallback(cpt);
    cpt->PowerClockOp = NULL;

    DListHeadInit(&cpt->sources);
    DListHeadInit(&cpt->sinks);
    DListHeadInit(&cpt->list);
    DListHeadInit(&cpt->dirty);
    DListInsertHead(&cpt->list, &audioCard->components);

    cpt->connected = CONNECT_CODEC_PIN;

    return HDF_SUCCESS;
}

int32_t AudioSapmNewComponents(struct AudioCard *audioCard,
    const struct AudioSapmComponent *component, int32_t cptMaxNum)
{
    int32_t i;
    int32_t ret;

    if ((audioCard == NULL) || (component == NULL)) {
        ADM_LOG_ERR("input params check error: audioCard=%p, component=%p.", audioCard, component);
        return HDF_FAILURE;
    }

    for (i = 0; i < cptMaxNum; i++) {
        ret = AudioSapmNewComponent(audioCard, component);
        if (ret != HDF_SUCCESS) {
            ADM_LOG_ERR("AudioSapmNewComponent fail!");
            return HDF_FAILURE;
        }
        component++;
    }

    return HDF_SUCCESS;
}

static void MuxSetPathStatus(const struct AudioSapmComponent *cpt, struct AudioSapmpath *path,
    const struct AudioEnumKcontrol *enumKtl, int32_t i)
{
    int32_t ret;
    uint32_t val;
    int32_t item;
    uint32_t reg;
    uint32_t shift;

    if ((cpt == NULL) || (path == NULL) || (enumKtl == NULL)) {
        ADM_LOG_ERR("input MuxSet params check error: cpt=%p, path=%p, enumKtl=%p.", cpt, path, enumKtl);
        return;
    }

    shift = enumKtl->shiftLeft;
    ret = AudioCodecDeviceReadReg(cpt->codec, reg, &val);
    if (ret != HDF_SUCCESS) {
        ADM_LOG_ERR("MuxSet read reg fail!");
        return;
    }

    item = val >> shift;

    path->connect = UNCONNECT_SINK_AND_SOURCE;
    for (i = 0; i < enumKtl->max; i++) {
        if ((strcmp(path->name, enumKtl->texts[i]) == 0) && item == i) {
            path->connect = CONNECT_SINK_AND_SOURCE;
        }
    }
    return;
}

static void MuxValueSetPathStatus(const struct AudioSapmComponent *cpt, struct AudioSapmpath *path,
    const struct AudioEnumKcontrol *enumKtl, int32_t i)
{
    int32_t ret;
    uint32_t val;
    uint32_t item;
    uint32_t reg = 0;
    uint32_t shift;
    if ((cpt == NULL) || (path == NULL) || (enumKtl == NULL)) {
        ADM_LOG_ERR("input muxValueSet params check error: cpt=%p, path=%p, enumKtl=%p.", cpt, path, enumKtl);
        return;
    }

    shift = enumKtl->shiftLeft;

    ret = AudioCodecDeviceReadReg(cpt->codec, reg, &val);
    if (ret != HDF_SUCCESS) {
        ADM_LOG_ERR("muxValueSet read reg fail!");
        return;
    }

    val = val >> shift;
    for (item = 0; item < enumKtl->max; item++) {
        if (val == enumKtl->values[item])
            break;
    }

    path->connect = UNCONNECT_SINK_AND_SOURCE;
    for (i = 0; i < enumKtl->max; i++) {
        if ((strcmp(path->name, enumKtl->texts[i]) == 0) && item == i) {
            path->connect = CONNECT_SINK_AND_SOURCE;
        }
    }
    return;
}

static void MixerSetPathStatus(const struct AudioSapmComponent *cpt, struct AudioSapmpath *path,
    const struct AudioMixerControl *mixerCtrl)
{
    int32_t ret;
    uint32_t reg;
    uint32_t mask;
    uint32_t shift;
    uint32_t invert;
    uint32_t curValue;

    if ((cpt == NULL) || (path == NULL) || (mixerCtrl == NULL)) {
        ADM_LOG_ERR("input params check error: cpt=%p, path=%p, mixerCtrl=%p.", cpt, path, mixerCtrl);
        return;
    }

    reg = mixerCtrl->reg;
    shift = mixerCtrl->shift;
    mask = mixerCtrl->mask;
    invert = mixerCtrl->invert;

    ret = AudioCodecDeviceReadReg(cpt->codec, reg, &curValue);
    if (ret != HDF_SUCCESS) {
        ADM_LOG_ERR("read reg fail!");
        return;
    }

    curValue = (curValue >> shift) & mask;
    if ((invert && !curValue) || (!invert && curValue)) {
        path->connect = CONNECT_SINK_AND_SOURCE;
    } else {
        path->connect = UNCONNECT_SINK_AND_SOURCE;
    }

    return;
}

static int32_t AudioSapmSetPathStatus(struct AudioSapmComponent *cpt, struct AudioSapmpath *path, int32_t i)
{
    if ((cpt == NULL) || (path == NULL)) {
        ADM_LOG_ERR("input params check error: cpt=%p, path=%p.", cpt, path);
        return HDF_FAILURE;
    }
    switch (cpt->sapmType) {
        case AUDIO_SAPM_MIXER:
        case AUDIO_SAPM_ANALOG_SWITCH:
        case AUDIO_SAPM_MIXER_NAMED_CTRL: {
            MixerSetPathStatus(cpt, path, (struct AudioMixerControl *)cpt->kcontrolNews[i].privateValue);
        }
        break;
        case AUDIO_SAPM_MUX: {
            MuxSetPathStatus(cpt, path, (struct AudioEnumKcontrol *)cpt->kcontrolNews[i].privateValue, i);
        }
        break;
        case AUDIO_SAPM_VALUE_MUX: {
            MuxValueSetPathStatus(cpt, path, (struct AudioEnumKcontrol *)cpt->kcontrolNews[i].privateValue, i);
        }
        break;
        default: {
            path->connect = CONNECT_SINK_AND_SOURCE;
            break;
        }
    }

    return HDF_SUCCESS;
}

static int32_t AudioSapmConnectMixer(struct AudioCard *audioCard,
    struct AudioSapmComponent *source, struct AudioSapmComponent *sink,
    struct AudioSapmpath *path, const char *controlName)
{
    int i;

    if ((audioCard == NULL) || (source == NULL) || (sink == NULL) || (path == NULL) || (controlName == NULL)) {
        ADM_LOG_ERR("input params check error: audioCard=%p, source=%p, sink=%p, path=%p, controlName=%p.",
            audioCard, source, sink, path, controlName);
        return HDF_FAILURE;
    }

    for (i = 0; i < sink->kcontrolsNum; i++) {
        if (strcmp(controlName, sink->kcontrolNews[i].name) == 0) {
            path->name = (char *)OsalMemCalloc(strlen(sink->kcontrolNews[i].name) + 1);
            if (path->name == NULL) {
                ADM_LOG_ERR("malloc path->name fail!");
                return HDF_FAILURE;
            }
            if (memcpy_s(path->name, strlen(sink->kcontrolNews[i].name) + 1, sink->kcontrolNews[i].name,
                strlen(sink->kcontrolNews[i].name) + 1) != EOK) {
                OsalMemFree(path->name);
                ADM_LOG_ERR("memcpy cpt->componentName fail!");
                return HDF_FAILURE;
            }
            DListInsertHead(&path->list, &audioCard->paths);
            DListInsertHead(&path->listSink, &sink->sources);
            DListInsertHead(&path->listSource, &source->sinks);

            AudioSapmSetPathStatus(sink, path, i);

            return HDF_SUCCESS;
        }
    }

    return HDF_FAILURE;
}

static int32_t AudioSampStaticOrDynamicPath(struct AudioCard *audioCard,
    struct AudioSapmComponent *source, struct AudioSapmComponent *sink,
    struct AudioSapmpath *path, const struct AudioSapmRoute *route)
{
    int32_t ret;

    if ((audioCard == NULL) || (source == NULL) || (sink == NULL) || (path == NULL) || (route == NULL)) {
        ADM_LOG_ERR("input params check error: audioCard=%p, source=%p, sink=%p, path=%p, route=%p.",
            audioCard, source, sink, path, route);
        return HDF_FAILURE;
    }

    if (route->control == NULL) {
        DListInsertHead(&path->list, &audioCard->paths);
        DListInsertHead(&path->listSink, &sink->sources);
        DListInsertHead(&path->listSource, &source->sinks);
        path->connect = CONNECT_SINK_AND_SOURCE;
        return HDF_SUCCESS;
    }

    switch (sink->sapmType) {
        case AUDIO_SAPM_MUX:
        case AUDIO_SAPM_VIRT_MUX:
        case AUDIO_SAPM_VALUE_MUX:
            break;
        case AUDIO_SAPM_ANALOG_SWITCH:
        case AUDIO_SAPM_MIXER:
        case AUDIO_SAPM_MIXER_NAMED_CTRL:
        case AUDIO_SAPM_PGA:
        case AUDIO_SAPM_SPK:
            ret = AudioSapmConnectMixer(audioCard, source, sink, path, route->control);
            if (ret != HDF_SUCCESS) {
                ADM_LOG_ERR("connect mixer fail!");
                return HDF_FAILURE;
            }
            break;
        case AUDIO_SAPM_HP:
        case AUDIO_SAPM_MIC:
        case AUDIO_SAPM_LINE:
            DListInsertHead(&path->list, &audioCard->paths);
            DListInsertHead(&path->listSink, &sink->sources);
            DListInsertHead(&path->listSource, &source->sinks);
            path->connect = CONNECT_SINK_AND_SOURCE;
            break;
        default:
            DListInsertHead(&path->list, &audioCard->paths);
            DListInsertHead(&path->listSink, &sink->sources);
            DListInsertHead(&path->listSource, &source->sinks);
            path->connect = CONNECT_SINK_AND_SOURCE;
            break;
    }

    return HDF_SUCCESS;
}

static void AudioSampExtComponentsCheck(struct AudioSapmComponent *cptSource, struct AudioSapmComponent *cptSink)
{
    if ((cptSource == NULL) || (cptSink == NULL)) {
        ADM_LOG_ERR("input params check error: cptSource=%p, cptSink=%p.", cptSource, cptSink);
        return;
    }

    /* check for external components */
    if (cptSink->sapmType == AUDIO_SAPM_INPUT) {
        if (cptSource->sapmType == AUDIO_SAPM_MICBIAS || cptSource->sapmType == AUDIO_SAPM_MIC ||
            cptSource->sapmType == AUDIO_SAPM_LINE || cptSource->sapmType == AUDIO_SAPM_OUTPUT)
            cptSink->external = EXIST_EXTERNAL_WIDGET;
    }
    if (cptSource->sapmType == AUDIO_SAPM_OUTPUT) {
        if (cptSink->sapmType == AUDIO_SAPM_SPK || cptSink->sapmType == AUDIO_SAPM_HP ||
            cptSink->sapmType == AUDIO_SAPM_LINE || cptSink->sapmType == AUDIO_SAPM_INPUT)
            cptSource->external = EXIST_EXTERNAL_WIDGET;
    }

    return;
}
static int32_t AudioSapmAddRoute(struct AudioCard *audioCard, const struct AudioSapmRoute *route)
{
    struct AudioSapmpath *path = NULL;
    struct AudioSapmComponent *cptSource = NULL;
    struct AudioSapmComponent *cptSink = NULL;
    struct AudioSapmComponent *cpt = NULL;
    int32_t ret;

    if ((audioCard == NULL) || (route == NULL)) {
        ADM_LOG_ERR("input params check error: audioCard=%p, route=%p.", audioCard, route);
        return HDF_FAILURE;
    }

    DLIST_FOR_EACH_ENTRY(cpt, &audioCard->components, struct AudioSapmComponent, list) {
        if ((cptSource == NULL) && (strcmp(cpt->componentName, route->source) == 0)) {
            cptSource = cpt;
            continue;
        }
        if ((cptSink == NULL) && (strcmp(cpt->componentName, route->sink) == 0)) {
            cptSink = cpt;
        }
        if ((cptSource != NULL) && (cptSink != NULL)) {
            break;
        }
    }
    if ((cptSource == NULL) || (cptSink == NULL)) {
        ADM_LOG_ERR("find component fail!");
        return HDF_FAILURE;
    }

    path = (struct AudioSapmpath *)OsalMemCalloc(sizeof(struct AudioSapmpath));
    if (path == NULL) {
        ADM_LOG_ERR("malloc path fail!");
        return HDF_FAILURE;
    }
    path->source = cptSource;
    path->sink = cptSink;
    DListHeadInit(&path->list);
    DListHeadInit(&path->listSink);
    DListHeadInit(&path->listSource);

    /* check for external components */
    AudioSampExtComponentsCheck(cptSource, cptSink);

    ret = AudioSampStaticOrDynamicPath(audioCard, cptSource, cptSink, path, route);
    if (ret != HDF_SUCCESS) {
        OsalMemFree(path);
        ADM_LOG_ERR("static or dynamic path fail!");
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t AudioSapmAddRoutes(struct AudioCard *audioCard, const struct AudioSapmRoute *route, int32_t routeMaxNum)
{
    int32_t i;
    int32_t ret;

    if ((audioCard == NULL) || (route == NULL)) {
        ADM_LOG_ERR("input params check error: audioCard=%p, route=%p.", audioCard, route);
        return HDF_FAILURE;
    }

    for (i = 0; i < routeMaxNum; i++) {
        ret = AudioSapmAddRoute(audioCard, route);
        if (ret != HDF_SUCCESS) {
            ADM_LOG_ERR("AudioSapmAddRoute failed!");
            return HDF_FAILURE;
        }
        route++;
    }
    return HDF_SUCCESS;
}

int32_t AudioSapmNewMixerControls(struct AudioSapmComponent *cpt, struct AudioCard *audioCard)
{
    struct AudioSapmpath *path = NULL;
    int32_t i;

    if ((cpt == NULL) || (audioCard == NULL)) {
        ADM_LOG_ERR("input params check error: cpt=%p, audioCard=%p.", cpt, audioCard);
        return HDF_FAILURE;
    }

    for (i = 0; i < cpt->kcontrolsNum; i++) {
        DLIST_FOR_EACH_ENTRY(path, &cpt->sources, struct AudioSapmpath, listSink) {
            if (strcmp(path->name, cpt->kcontrolNews[i].name) != 0) {
                continue;
            }

            path->kcontrol = AudioAddControl(audioCard, &cpt->kcontrolNews[i]);
            if (path->kcontrol == NULL) {
                ADM_LOG_ERR("add control fail!");
                return HDF_FAILURE;
            }
            cpt->kcontrols[i] = path->kcontrol;
            DListInsertHead(&cpt->kcontrols[i]->list, &audioCard->controls);
        }
    }

    return HDF_SUCCESS;
}

int AudioSapmNewMuxControls(struct AudioSapmComponent *cpt, struct AudioCard *audioCard)
{
    struct AudioKcontrol *kctrl = NULL;

    if (cpt == NULL) {
        ADM_LOG_ERR("input param cpt is NULL.");
        return HDF_FAILURE;
    }

    if (cpt->kcontrolsNum != 1) {
        ADM_LOG_ERR("incorrect number of controls.");
        return HDF_FAILURE;
    }

    kctrl = AudioAddControl(audioCard, &cpt->kcontrolNews[0]);
    if (kctrl == NULL) {
        ADM_LOG_ERR("add control fail!");
        return HDF_FAILURE;
    }
    cpt->kcontrols[0] = kctrl;
    DListInsertHead(&cpt->kcontrols[0]->list, &audioCard->controls);

    return HDF_SUCCESS;
}

static void AudioSapmPowerSeqInsert(struct AudioSapmComponent *newCpt,
    struct DListHead *list, int8_t isPowerUp)
{
    struct AudioSapmComponent *cpt = NULL;
    int32_t *seq = {0};

    if (newCpt == NULL) {
        ADM_LOG_ERR("input param newCpt is NULL.");
        return;
    }

    if (isPowerUp) {
        seq = g_audioSapmPowerUpSeq;
    } else {
        seq = g_audioSapmPowerDownSeq;
    }

    DLIST_FOR_EACH_ENTRY(cpt, list, struct AudioSapmComponent, powerList) {
        if ((seq[newCpt->sapmType] - seq[cpt->sapmType]) < 0) {
            DListInsertTail(&newCpt->powerList, &cpt->powerList);
            return;
        }
    }
    DListInsertTail(&newCpt->powerList, list);

    ADM_LOG_DEBUG("[%s] success.", newCpt->componentName);
    return;
}

static void AudioSapmSetPower(struct AudioCard *audioCard, struct AudioSapmComponent *cpt,
    uint8_t power, struct DListHead *upList, struct DListHead *downList)
{
    struct AudioSapmpath *path = NULL;

    if (cpt == NULL) {
        ADM_LOG_ERR("input param cpt is NULL.");
        return;
    }

    DLIST_FOR_EACH_ENTRY(path, &cpt->sources, struct AudioSapmpath, listSink) {
        if (path->source != NULL) {
            if ((path->source->power != power) && path->connect) {
                if (DListIsEmpty(&path->source->dirty)) {
                    DListInsertTail(&path->source->dirty, &audioCard->sapmDirty);
                }
            }
        }
    }
    DLIST_FOR_EACH_ENTRY(path, &cpt->sinks, struct AudioSapmpath, listSource) {
        if (path->sink != NULL) {
            if ((path->sink->power != power) && path->connect) {
                if (DListIsEmpty(&path->sink->dirty)) {
                    DListInsertTail(&path->sink->dirty, &audioCard->sapmDirty);
                }
            }
        }
    }

    if (power) {
        AudioSapmPowerSeqInsert(cpt, upList, power);
    } else {
        AudioSapmPowerSeqInsert(cpt, downList, power);
    }

    cpt->power = power;
    return;
}

static void AudioSapmPowerUpSeqRun(struct DListHead *list)
{
    enum AudioDeviceType deviceType;
    struct AudioMixerControl mixerControl;
    void *device = NULL;
    struct AudioSapmComponent *cpt = NULL;
    int32_t ret;

    if (list == NULL) {
        ADM_LOG_ERR("input param list is NULL.");
        return;
    }

    DLIST_FOR_EACH_ENTRY(cpt, list, struct AudioSapmComponent, powerList) {
        if ((cpt->reg >= 0) && (cpt->power == SAPM_POWER_DOWN)) {
            cpt->power = SAPM_POWER_UP;
            mixerControl.reg = cpt->reg;
            mixerControl.mask = cpt->mask;
            mixerControl.shift = cpt->shift;
            if (cpt->codec != NULL && cpt->codec->devData != NULL) {
                deviceType = AUDIO_CODEC_DEVICE;
                device = cpt->codec;
            } else {
                deviceType = AUDIO_ACCESSORY_DEVICE;
                device = cpt->accessory;
            }
            ret = AudioUpdateRegBits(deviceType, device, &mixerControl, SAPM_POWER_UP);
            if (ret != HDF_SUCCESS) {
                ADM_LOG_ERR("update reg bits fail!");
                return;
            }
        }
    }
    return;
}

static void AudioSapmPowerDownSeqRun(struct DListHead *list)
{
    void *device = NULL;
    enum AudioDeviceType deviceType;
    struct AudioMixerControl mixerControl;
    struct AudioSapmComponent *cpt = NULL;
    int32_t ret;

    if (list == NULL) {
        ADM_LOG_ERR("input param list is NULL.");
        return;
    }

    DLIST_FOR_EACH_ENTRY(cpt, list, struct AudioSapmComponent, powerList) {
        if ((cpt->reg >= 0) && (cpt->power == SAPM_POWER_UP)) {
            cpt->power = SAPM_POWER_DOWN;
            mixerControl.mask = cpt->mask;
            mixerControl.reg = cpt->reg;
            mixerControl.shift = cpt->shift;
            if (cpt->codec != NULL && cpt->codec->devData != NULL) {
                deviceType = AUDIO_CODEC_DEVICE;
                device = cpt->codec;
            } else {
                deviceType = AUDIO_ACCESSORY_DEVICE;
                device = cpt->accessory;
            }
            ret = AudioUpdateRegBits(deviceType, device, &mixerControl, SAPM_POWER_DOWN);
            if (ret != HDF_SUCCESS) {
                ADM_LOG_ERR("update reg bits fail!");
                return;
            }
        }
    }

    return;
}

int32_t AudioSapmPowerComponents(struct AudioCard *audioCard)
{
    struct AudioSapmComponent *cpt = NULL;
    struct DListHead upList;
    struct DListHead downList;

    if (audioCard == NULL) {
        ADM_LOG_ERR("input param audioCard is NULL.");
        return HDF_FAILURE;
    }

    DListHeadInit(&upList);
    DListHeadInit(&downList);

    DLIST_FOR_EACH_ENTRY(cpt, &audioCard->sapmDirty, struct AudioSapmComponent, dirty) {
        cpt->newPower = cpt->PowerCheck(cpt);
        if (cpt->newPower == cpt->power) {
            continue;
        }

        if (g_audioSapmIsStandby && cpt->PowerClockOp != NULL) {
            cpt->PowerClockOp(cpt);
        }

        AudioSapmSetPower(audioCard, cpt, cpt->newPower, &upList, &downList);
    }

    DLIST_FOR_EACH_ENTRY(cpt, &audioCard->components, struct AudioSapmComponent, list) {
        DListRemove(&cpt->dirty);
        DListHeadInit(&cpt->dirty);
    }

    AudioSapmPowerDownSeqRun(&downList);
    AudioSapmPowerUpSeqRun(&upList);

    return HDF_SUCCESS;
}

static void ReadInitComponentPowerStatus(struct AudioSapmComponent *cpt)
{
    int32_t ret;
    uint32_t regVal;

    if (cpt == NULL) {
        ADM_LOG_ERR("input param cpt is NULL.");
        return;
    }

    if (cpt->reg >= 0) {
        ret = AudioCodecDeviceReadReg(cpt->codec, cpt->reg, &regVal);
        if (ret != HDF_SUCCESS) {
            ADM_LOG_ERR("read reg fail!");
            return;
        }
        regVal &= 1 << cpt->shift;

        if (cpt->invert) {
            regVal = !regVal;
        }

        if (regVal) {
            cpt->power = SAPM_POWER_UP;
        } else {
            cpt->power = SAPM_POWER_DOWN;
        }
    }

    return;
}

void AudioSapmSleep(const struct AudioCard *audioCard)
{
    if (audioCard == NULL) {
        ADM_LOG_ERR("input param audioCard is NULL.");
        return;
    }

    if (g_sleepTimer.realTimer != NULL) {
        OsalTimerDelete(&g_sleepTimer);
    }
    OsalTimerCreate(&g_sleepTimer, SAPM_POLL_TIME, AudioSapmEnterSleep, (uintptr_t)audioCard);
    OsalTimerStartLoop(&g_sleepTimer);
    AudioSapmRefreshTime(true);

    return;
}

int32_t AudioSapmNewControls(struct AudioCard *audioCard)
{
    struct AudioSapmComponent *cpt = NULL;
    int32_t ret;

    if (audioCard == NULL) {
        ADM_LOG_ERR("input param audioCard is NULL.");
        return HDF_FAILURE;
    }

    DLIST_FOR_EACH_ENTRY(cpt, &audioCard->components, struct AudioSapmComponent, list) {
        if (cpt->newCpt) {
            continue;
        }
        if (cpt->kcontrolsNum > 0) {
            cpt->kcontrols = OsalMemCalloc(sizeof(struct AudioKcontrol*) * cpt->kcontrolsNum);
            if (cpt->kcontrols == NULL) {
                ADM_LOG_ERR("malloc kcontrols fail!");
                return HDF_FAILURE;
            }
        }

        switch (cpt->sapmType) {
            case AUDIO_SAPM_ANALOG_SWITCH:
            case AUDIO_SAPM_MIXER:
            case AUDIO_SAPM_MIXER_NAMED_CTRL:
            case AUDIO_SAPM_SPK:
            case AUDIO_SAPM_PGA:
                ret = AudioSapmNewMixerControls(cpt, audioCard);
                break;
            case AUDIO_SAPM_MUX:
            case AUDIO_SAPM_VIRT_MUX:
            case AUDIO_SAPM_VALUE_MUX:
                ret =AudioSapmNewMuxControls(cpt, audioCard);
                break;
            default:
                ret = HDF_SUCCESS;
                break;
        }
        if (ret != HDF_SUCCESS) {
            OsalMemFree(cpt->kcontrols);
            ADM_LOG_ERR("sapm new mixer or mux controls fail!");
            return HDF_FAILURE;
        }

        ReadInitComponentPowerStatus(cpt);
        cpt->newCpt = 1;
        DListInsertTail(&cpt->dirty, &audioCard->sapmDirty);
    }

    ret = AudioSapmPowerComponents(audioCard);
    if (ret != HDF_SUCCESS) {
        ADM_LOG_ERR("sapm power component fail!");
        return HDF_FAILURE;
    }

    AudioSapmSleep(audioCard);
    return HDF_SUCCESS;
}

static int32_t MixerUpdatePowerStatus(struct AudioKcontrol *kcontrol, uint32_t pathStatus)
{
    struct AudioCard *audioCard = NULL;
    struct AudioSapmpath *path = NULL;
    int32_t ret;

    if (kcontrol == NULL || kcontrol->pri == NULL) {
        ADM_LOG_ERR("input param kcontrol is NULL.");
        return HDF_FAILURE;
    }
    audioCard = (struct AudioCard *)(kcontrol->pri);

    DLIST_FOR_EACH_ENTRY(path, &audioCard->paths, struct AudioSapmpath, list) {
        if (path->kcontrol != kcontrol) {
            continue;
        }
        if (path->sink == NULL || path->source == NULL) {
            ADM_LOG_ERR("get path sink or source fail!");
            return HDF_FAILURE;
        }
        if (path->sink->sapmType != AUDIO_SAPM_MIXER &&
            path->sink->sapmType != AUDIO_SAPM_MIXER_NAMED_CTRL &&
            path->sink->sapmType != AUDIO_SAPM_PGA &&
            path->sink->sapmType != AUDIO_SAPM_SPK &&
            path->sink->sapmType != AUDIO_SAPM_ANALOG_SWITCH) {
            ADM_LOG_DEBUG("no mixer device.");
            return HDF_DEV_ERR_NO_DEVICE;
        }
        path->connect = pathStatus;
        DListInsertTail(&path->source->dirty, &audioCard->sapmDirty);
        DListInsertTail(&path->sink->dirty, &audioCard->sapmDirty);
        break;
    }

    ret = AudioSapmPowerComponents(audioCard);
    if (ret != HDF_SUCCESS) {
        ADM_LOG_ERR("sapm power component fail!");
        return HDF_FAILURE;
    }

    DLIST_FOR_EACH_ENTRY(path, &audioCard->paths, struct AudioSapmpath, list) {
        ADM_LOG_DEBUG("path->sink->componentName = %s, path->source->componentName = %s, \
            path->connect = %d.",
            path->sink->componentName, path->source->componentName, path->connect);
    }
    return HDF_SUCCESS;
}

int32_t AudioSapmGetCtrlSw(struct AudioKcontrol *kcontrol, struct AudioCtrlElemValue *elemValue)
{
    int32_t ret;
    struct CodecDevice *codec = NULL;
    struct AudioMixerControl *mixerCtrl  = NULL;
    uint32_t curValue;

    if (kcontrol == NULL || kcontrol->privateValue <= 0 || elemValue == NULL) {
        ADM_LOG_ERR("input params: kcontrol is NULL or elemValue=%p.", elemValue);
        return HDF_ERR_INVALID_OBJECT;
    }

    codec = AudioKcontrolGetCodec(kcontrol);
    if (codec == NULL || codec->devData == NULL || codec->devData->Read == NULL) {
        ADM_LOG_ERR("codec device is NULL.");
        return HDF_FAILURE;
    }
    mixerCtrl = (struct AudioMixerControl *)kcontrol->privateValue;
    ret = codec->devData->Read(codec, mixerCtrl->reg, &curValue);
    if (ret != HDF_SUCCESS) {
        ADM_LOG_ERR("sapm Get Ctrl Read fail!");
        return HDF_FAILURE;
    }

    curValue = (curValue >> mixerCtrl->shift) & mixerCtrl->mask;
    if (curValue > mixerCtrl->max) {
        ADM_LOG_ERR("invalid curValue:%d.", curValue);
        return HDF_FAILURE;
    }

    if (mixerCtrl->invert) {
        curValue = mixerCtrl->max - curValue;
    }
    elemValue->value[0] = curValue;

    return HDF_SUCCESS;
}

int32_t AudioSapmPutCtrlSwSub(struct AudioKcontrol *kcontrol, struct AudioCtrlElemValue *elemValue,
    struct AudioMixerControl *mixerCtrl, int32_t value, uint32_t pathStatus)
{
    int32_t ret = HDF_FAILURE;
    void *device = NULL;
    enum AudioDeviceType deviceType;
    uint32_t curValue;
    struct CodecDevice *codec = AudioKcontrolGetCodec(kcontrol);
    struct AccessoryDevice *accessory = AudioKcontrolGetAccessory(kcontrol);

    if (kcontrol == NULL || elemValue == NULL || mixerCtrl == NULL) {
        ADM_LOG_ERR("parser is fail!");
        return HDF_FAILURE;
    }
    if (codec != NULL) {
        deviceType = AUDIO_CODEC_DEVICE;
        device = (void *)codec;
        ret = AudioCodecDeviceReadReg(codec, mixerCtrl->reg, &curValue);
    } else if (accessory != NULL) {
        deviceType = AUDIO_ACCESSORY_DEVICE;
        device = (void *)accessory;
        ret = AudioAccessoryDeviceReadReg(accessory, mixerCtrl->reg, &curValue);
    }
    if (ret != HDF_SUCCESS) {
        ADM_LOG_ERR("Device read fail!");
        return HDF_FAILURE;
    }
    curValue &= mixerCtrl->mask << mixerCtrl->shift;
    value = (value & mixerCtrl->mask) << mixerCtrl->shift;
    if ((curValue != value) || g_audioSapmIsSleep) {
        if (MixerUpdatePowerStatus(kcontrol, pathStatus) != HDF_SUCCESS) {
            ADM_LOG_ERR("update power status fail!");
            return HDF_FAILURE;
        }
        if (AudioUpdateRegBits(deviceType, device, mixerCtrl, elemValue->value[0]) != HDF_SUCCESS) {
            ADM_LOG_ERR("update reg bits fail!");
            return HDF_FAILURE;
        }
    }

    return HDF_SUCCESS;
}

/* 1.first user specify old component -- power down; 2.second user specify new component -- power up */
int32_t AudioSapmPutCtrlSw(struct AudioKcontrol *kcontrol, struct AudioCtrlElemValue *elemValue)
{
    uint32_t pathStatus;
    int32_t value;
    struct AudioMixerControl *mixerCtrl = NULL;

    if ((kcontrol == NULL) || (kcontrol->privateValue <= 0) || (elemValue == NULL)) {
        ADM_LOG_ERR("input params: kcontrol is NULL or elemValue=%p.", elemValue);
        return HDF_ERR_INVALID_OBJECT;
    }
    mixerCtrl = (struct AudioMixerControl *)kcontrol->privateValue;
    value = elemValue->value[0];
    if (value < mixerCtrl->min || value > mixerCtrl->max) {
        ADM_LOG_ERR("value is invalid.");
        return HDF_ERR_INVALID_OBJECT;
    }
    if (mixerCtrl->invert) {
        value = mixerCtrl->max - value;
    }
    if (value) {
        pathStatus = CONNECT_SINK_AND_SOURCE;
    } else {
        pathStatus = UNCONNECT_SINK_AND_SOURCE;
    }
    if (AudioSapmPutCtrlSwSub(kcontrol, elemValue, mixerCtrl, value, pathStatus) == HDF_FAILURE) {
        ADM_LOG_ERR("AudioSapmPutCtrlSwSub value is fail.");
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}


u64 AudioSapmRefreshTime(bool bRefresh)
{
    static u64 time = 0;

    if (bRefresh) {
        time = OsalGetSysTimeMs();
        g_audioSapmIsSleep = false;
    }
    return time;
}

static bool AudioSapmCheckTime(void)
{
    int64_t diffTime = OsalGetSysTimeMs() - AudioSapmRefreshTime(false);
    if (diffTime > SAPM_SLEEP_TIME) {
        return true;
    } else if (diffTime < 0) {
        AudioSapmRefreshTime(true);
    }
    return false;
}

static void AudioSapmEnterSleepSub(uintptr_t para, struct AudioSapmComponent *cpt)
{
    void *device = NULL;
    int i;
    enum AudioDeviceType deviceType;
    struct DListHead downList;
    struct AudioCard *audioCard = (struct AudioCard *)para;

    DListHeadInit(&downList);
    DLIST_FOR_EACH_ENTRY(cpt, &audioCard->components, struct AudioSapmComponent, list) {
        for (i = 0; (i < cpt->kcontrolsNum) && (cpt->kcontrols != NULL) && (cpt->kcontrols[i] != NULL); i++) {
            struct AudioMixerControl *mixerCtrl = (struct AudioMixerControl *)(cpt->kcontrols[i]->privateValue);
            if (mixerCtrl != NULL) {
                if (cpt->codec != NULL && cpt->codec->devData != NULL) {
                    deviceType = AUDIO_CODEC_DEVICE;
                    device = cpt->codec;
                } else {
                    deviceType = AUDIO_ACCESSORY_DEVICE;
                    device = cpt->accessory;
                }
                AudioUpdateRegBits(deviceType, device, mixerCtrl, SAPM_POWER_DOWN);
            }
        }
        ReadInitComponentPowerStatus(cpt);
        if (cpt->power == SAPM_POWER_UP) {
            AudioSapmPowerSeqInsert(cpt, &downList, SAPM_POWER_DOWN);
        }
    }
    AudioSapmPowerDownSeqRun(&downList);
    g_audioSapmIsSleep = true;
}

static void AudioSapmEnterSleep(uintptr_t para)
{
    struct AudioSapmComponent *cpt = NULL;
    struct AudioCard *audioCard = (struct AudioCard *)para;
    static bool bFirst = true;

    if ((g_audioSapmIsSleep == true) || (audioCard == NULL) || (!AudioSapmCheckTime())) {
        return;
    }

    if (bFirst) {
        bFirst = false;
        AudioSapmRefreshTime(true);
        return;
    }

    DLIST_FOR_EACH_ENTRY(cpt, &audioCard->components, struct AudioSapmComponent, list) {
        if (cpt->PowerClockOp != NULL) {
            if (g_audioSapmIsStandby == false) {
                cpt->PowerClockOp(cpt);
                g_audioSapmIsStandby = true;
                AudioSapmRefreshTime(true);
            }
        }
    }
    if (g_audioSapmIsStandby == true) {
        if (!AudioSapmCheckTime()) {
            return;
        }
        g_audioSapmIsStandby = false;
    }

    AudioSapmEnterSleepSub(para, cpt);
}
