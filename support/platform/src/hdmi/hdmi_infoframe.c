/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "hdf_log.h"
#include "hdmi_core.h"
#include "securec.h"

#define HDF_LOG_TAG hdmi_infoframe_c

#define HDMI_IEEE_OUI_1_4_1ST 0x03
#define HDMI_IEEE_OUI_1_4_2ND 0x0C
#define HDMI_IEEE_OUI_1_4_3RD 0x00

static void HdmiInfoframeFillCheckSum(uint8_t *data, uint32_t len)
{
    uint32_t i;
    uint8_t checkSum = 0;

    for (i = 0; i < len; i++) {
        checkSum += data[i];
    }
    if (checkSum > 0) {
        /*
         * The checksum shall be calculated such that a byte-wide sum of all three bytes of the Packet Header and
         * all valid bytes of the InfoFrame Packet contents(determined by InfoFrame_length), plus the checksum itself,
         * equals zero.
         */
        data[3] = HDMI_INFOFRAME_CHECKSUM - checkSum;
    }
}

static void HdmiInfoframeFillHeader(struct HdmiInfoframeHeader *header, uint8_t *data, uint32_t len)
{
    if (len < HDMI_INFOFRAME_PACKET_HEADER_LEN) {
        HDF_LOGE("len = %d, val is too small.", len);
        return;
    }
    data[0] = header->type;
    data[1] = header->verNum;
    data[2] = header->len;
}

static int32_t HdmiInfoframePacketVsEncoding(union HdmiInfoframeInfo *infoframe, uint8_t *data, uint32_t len)
{
    uint32_t lenght;
    struct HdmiVs14VsifContent *vsifContent = NULL;
    struct HdmiVsUserVsifContent *userContent = NULL;
    struct HdmiVsInfoframe *vs = &(infoframe->vs);

    lenght = HDMI_INFOFRAME_PACKET_HEADER_LEN + vs->len;
    if (len < lenght) {
        HDF_LOGE("len = %d, val is too small.", len);
        return HDF_ERR_INVALID_PARAM;
    }
    if (memset_s(data, len, 0, len) != EOK) {
        HDF_LOGE("memset_s fail.");
        return HDF_ERR_IO;
    }

    HdmiInfoframeFillHeader(&(infoframe->header), data, len);
    if (vs->vsifContent.vsif.oui == HDMI_IEEE_OUI_1_4) {
        data[4] = HDMI_IEEE_OUI_1_4_1ST;
        data[5] = HDMI_IEEE_OUI_1_4_2ND;
        data[6] = HDMI_IEEE_OUI_1_4_3RD;
        vsifContent = &(vs->vsifContent.vsif);
        userContent = &(vs->vsifContent.userVsif);
        data[7] = (vsifContent->format & HDMI_VENDOR_1_4_FORMAT_MARK) << HDMI_VENDOR_1_4_FORMAT_SHIFT;
        if (vsifContent->format == HDMI_VS_VIDEO_FORMAT_4K) {
            data[8] = vsifContent->vic;
            return HDF_SUCCESS;
        } else if (vsifContent->format == HDMI_VS_VIDEO_FORMAT_3D) {
            data[8] = (vsifContent->_3dStruct & HDMI_VENDOR_3D_STRUCTURE_MARK) << HDMI_VENDOR_3D_STRUCTURE_SHIFT;
        }
        data[9] = (vsifContent->_3dExtData & HDMI_VENDOR_3D_EXT_DATA_MARK) << HDMI_VENDOR_3D_EXT_DATA_SHIFT;
        if (vsifContent->_3dMetaPresent == false) {
            if (userContent->len == 0 || (userContent->len + lenght) > len) {
                return HDF_SUCCESS;
            }
            if (memcpy_s(&data[lenght], (len - lenght), userContent->data, userContent->len) != EOK) {
                HDF_LOGE("memcpy_s fail.");
                return HDF_ERR_IO;
            }
            lenght += userContent->len;
        }
    }
    HdmiInfoframeFillCheckSum(data, lenght);
    return HDF_SUCCESS;
}

static int32_t HdmiInfoframePacketAviEncoding(union HdmiInfoframeInfo *infoframe, uint8_t *data, uint32_t len)
{
    uint32_t lenght;
    uint8_t *buff = data;
    struct HdmiAviInfoframe *avi = &(infoframe->avi);

    lenght = HDMI_INFOFRAME_PACKET_HEADER_LEN + avi->len;
    if (len < lenght) {
        HDF_LOGE("len = %d, val is too small.", len);
        return HDF_ERR_INVALID_PARAM;
    }
    if (memset_s(buff, len, 0, len) != EOK) {
        HDF_LOGE("memset_s fail.");
        return HDF_ERR_IO;
    }

    HdmiInfoframeFillHeader(&(infoframe->header), data, len);
    buff += HDMI_INFOFRAME_PACKET_HEADER_LEN;
    /* PB1 */
    buff[0] |= (avi->colorSpace & HDMI_AVI_COLOR_SPACE_MARK) << HDMI_AVI_COLOR_SPACE_SHIFT;
    buff[0] |= (avi->scanMode & HDMI_AVI_SCAN_MODE_MARK);
    if (avi->activeFormatInformationPresent == true) {
        buff[0] |= (1 << HDMI_AVI_ACTIVE_INFORMATION_SHIFT);
    }
    if (avi->horizBarInfoPresent == true) {
        buff[0] |= (1 << HDMI_AVI_HORIZONTAL_BAR_SHIFT);
    }
    if (avi->vertBarInfoPresent == true) {
        buff[0] |= (1 << HDMI_AVI_VERTICAL_BAR_SHIFT);
    }
    /* PB2 */
    buff[1] |= (avi->colorimetry & HDMI_AVI_COLORIMETRY_MARK) << HDMI_AVI_COLORIMETRY_SHIFT;
    buff[1] |= (avi->pictureAspect & HDMI_AVI_PICTURE_ASPECT_RATE_MARK) << HDMI_AVI_PICTURE_ASPECT_RATE_SHIFT;
    buff[1] |= (avi->activeAspect & HDMI_AVI_ACTIVE_FORMAT_ASPECT_RATE_MARK);
    /* PB3 */
    buff[2] |= (avi->extColorimetry & HDMI_AVI_EXT_COLORIMETRY_MARK) << HDMI_AVI_EXT_COLORIMETRY_SHIFT;
    buff[2] |= (avi->range & HDMI_AVI_EXT_QUANTIZATION_RANGE_MARK) << HDMI_AVI_EXT_QUANTIZATION_RANGE_SHIFT;
    buff[2] |= (avi->nups & HDMI_AVI_NUPS_RANGE_MARK);
    if (avi->itc == true) {
        buff[2] |= (1 << HDMI_AVI_IT_CONTENT_SHIFT);
    }
    /* PB4 */
    buff[3] = avi->vic;
    /* PB5 */
    buff[4] |= (avi->yccRange & HDMI_AVI_YCC_QUANTIZATION_RANGE_MARK) << HDMI_AVI_YCC_QUANTIZATION_RANGE_SHIFT;
    buff[4] |= (avi->itcType & HDMI_AVI_IT_CONTENT_TYPE_MARK) << HDMI_AVI_IT_CONTENT_TYPE_SHIFT;
    buff[4] |= (avi->pixelRepetitionFactor & HDMI_AVI_PIXEL_REPETION_FACTOR_MARK);
    /* PB6 */
    buff[5] = (uint8_t)(avi->topBar & HDMI_AVI_BAR_MODE_MARK);
    /* PB7 */
    buff[6] = (uint8_t)((avi->topBar >> HDMI_AVI_BAR_MODE_SHIFT) & HDMI_AVI_BAR_MODE_MARK);
    /* PB8 */
    buff[7] = (uint8_t)(avi->bottomBar & HDMI_AVI_BAR_MODE_MARK);
    /* PB9 */
    buff[8] = (uint8_t)((avi->bottomBar >> HDMI_AVI_BAR_MODE_SHIFT) & HDMI_AVI_BAR_MODE_MARK);
    /* PB10 */
    buff[9] = (uint8_t)(avi->leftBar & HDMI_AVI_BAR_MODE_MARK);
    /* PB11 */
    buff[10] = (uint8_t)((avi->leftBar >> HDMI_AVI_BAR_MODE_SHIFT) & HDMI_AVI_BAR_MODE_MARK);
    /* PB12 */
    buff[11] = (uint8_t)(avi->rightBar & HDMI_AVI_BAR_MODE_MARK);
    /* PB13 */
    buff[12] = (uint8_t)((avi->rightBar >> HDMI_AVI_BAR_MODE_SHIFT) & HDMI_AVI_BAR_MODE_MARK);
    HdmiInfoframeFillCheckSum(data, lenght);
    return HDF_SUCCESS;
}

static int32_t HdmiInfoframePacketSpdEncoding(union HdmiInfoframeInfo *infoframe, uint8_t *data, uint32_t len)
{
    uint32_t lenght;
    uint8_t *buff = data;
    struct HdmiSpdInfoframe *spd = &(infoframe->spd);

    lenght = HDMI_INFOFRAME_PACKET_HEADER_LEN + spd->len;
    if (len < lenght) {
        HDF_LOGE("len = %d, val is too small.", len);
        return HDF_ERR_INVALID_PARAM;
    }
    if (memset_s(buff, len, 0, len) != EOK) {
        HDF_LOGE("memset_s fail.");
        return HDF_ERR_IO;
    }

    buff += HDMI_INFOFRAME_PACKET_HEADER_LEN;
    /* PB1~PB8 */
    if (memcpy_s(buff, (len - HDMI_INFOFRAME_PACKET_HEADER_LEN), spd->vendorName, sizeof(spd->vendorName)) != EOK) {
        HDF_LOGE("memcpy_s fail.");
        return HDF_ERR_IO;
    }
    buff += HDMI_SPD_VENDOR_NAME_LEN;
    /* PB9~PB24 */
    if (memcpy_s(buff, (len - HDMI_INFOFRAME_PACKET_HEADER_LEN - HDMI_SPD_VENDOR_NAME_LEN),
        spd->productDescription, sizeof(spd->productDescription)) != EOK) {
        HDF_LOGE("memcpy_s fail.");
        return HDF_ERR_IO;
    }
    buff += HDMI_SPD_PRODUCT_DESCRIPTION_LEN;
    /* PB25 */
    buff[0] = spd->sdi;
    HdmiInfoframeFillCheckSum(data, lenght);
    return HDF_SUCCESS;
}

static int32_t HdmiInfoframePacketAudioEncoding(union HdmiInfoframeInfo *infoframe, uint8_t *data, uint32_t len)
{
    uint32_t lenght;
    uint8_t *buff = data;
    struct HdmiAudioInfoframe *audio = &(infoframe->audio);

    lenght = HDMI_INFOFRAME_PACKET_HEADER_LEN + audio->len;
    if (len < lenght) {
        HDF_LOGE("len = %d, val is too small.", len);
        return HDF_ERR_INVALID_PARAM;
    }
    if (memset_s(buff, len, 0, len) != EOK) {
        HDF_LOGE("memset_s fail.");
        return HDF_ERR_IO;
    }

    HdmiInfoframeFillHeader(&(infoframe->header), data, len);
    buff += HDMI_INFOFRAME_PACKET_HEADER_LEN;
    /* PB1 */
    buff[0] |= (audio->codingType & HDMI_AUDIO_CODING_TYPE_MARK) << HDMI_AUDIO_CODING_TYPE_SHIFT;
    buff[0] |= (audio->channelCount & HDMI_AUDIO_CHANNEL_COUNT_MARK);
    /* PB2 */
    buff[1] |= (audio->sampleFreq & HDMI_AUDIO_SAMPLE_FREQUENCY_MARK) << HDMI_AUDIO_SAMPLE_FREQUENCY_SHIFT;
    buff[1] |= (audio->sampleSize & HDMI_AUDIO_SAMPLE_SIZE_MARK);
    /* PB3 */
    buff[2] |= (audio->codingExtType & HDMI_AUDIO_CXT_MARK);
    /* PB4 */
    buff[3] |= audio->channelAllocation;
    /* PB5 */
    buff[4] |= (audio->levelShiftValue & HDMI_AUDIO_LEVEL_SHIFT_VALUE_MARK) << HDMI_AUDIO_LEVEL_SHIFT_VALUE_SHIFT;
    buff[4] |= (audio->playBackLevel & HDMI_AUDIO_LEF_PLAYBACK_LEVEL_MARK);
    if (audio->dmInh == true) {
        buff[4] |= (1 << HDMI_AUDIO_DM_INH_SHIFT);
    }
    HdmiInfoframeFillCheckSum(data, lenght);
    return HDF_SUCCESS;
}

static int32_t HdmiInfoframePacketDrmEncoding(union HdmiInfoframeInfo *infoframe, uint8_t *data, uint32_t len)
{
    uint32_t lenght;
    uint8_t *buff = data;
    struct HdmiDrmInfoframe *drm = &(infoframe->drm);
    struct HdmiStaticMetadataDescriptor1st *des = &(drm->des.type1);

    lenght = HDMI_INFOFRAME_PACKET_HEADER_LEN + drm->len;
    if (len < lenght) {
        HDF_LOGE("len = %d, val is too small.", len);
        return HDF_ERR_INVALID_PARAM;
    }
    if (memset_s(buff, len, 0, len) != EOK) {
        HDF_LOGE("memset_s fail.");
        return HDF_ERR_IO;
    }

    HdmiInfoframeFillHeader(&(infoframe->header), data, len);
    buff += HDMI_INFOFRAME_PACKET_HEADER_LEN;
    /* PB1 */
    buff[0] = drm->eotfType;
    /* PB2 */
    buff[1] = drm->metadataType;
    /* PB3 */
    buff[2] = (uint8_t)(des->displayPrimaries0X & HDMI_DRM_METADATA_MARK);
    /* PB4 */
    buff[3] = (uint8_t)((des->displayPrimaries0X >> HDMI_DRM_METADATA_SHIFT) & HDMI_DRM_METADATA_MARK);
    /* PB5 */
    buff[4] = (uint8_t)(des->displayPrimaries0Y & HDMI_DRM_METADATA_MARK);
    /* PB6 */
    buff[5] = (uint8_t)((des->displayPrimaries0Y & HDMI_DRM_METADATA_MARK) & HDMI_DRM_METADATA_MARK);
    /* PB7 */
    buff[6] = (uint8_t)(des->displayPrimaries1X & HDMI_DRM_METADATA_MARK);
    /* PB8 */
    buff[7] = (uint8_t)((des->displayPrimaries1X & HDMI_DRM_METADATA_MARK) & HDMI_DRM_METADATA_MARK);
    /* PB9 */
    buff[8] = (uint8_t)(des->displayPrimaries1Y & HDMI_DRM_METADATA_MARK);
    /* PB10 */
    buff[9] = (uint8_t)((des->displayPrimaries1Y & HDMI_DRM_METADATA_MARK) & HDMI_DRM_METADATA_MARK);
    /* PB11 */
    buff[10] = (uint8_t)(des->displayPrimaries2X & HDMI_DRM_METADATA_MARK);
    /* PB12 */
    buff[11] = (uint8_t)((des->displayPrimaries2X & HDMI_DRM_METADATA_MARK) & HDMI_DRM_METADATA_MARK);
    /* PB13 */
    buff[12] = (uint8_t)(des->displayPrimaries2Y & HDMI_DRM_METADATA_MARK);
    /* PB14 */
    buff[13] = (uint8_t)((des->displayPrimaries2Y & HDMI_DRM_METADATA_MARK) & HDMI_DRM_METADATA_MARK);
    /* PB15 */
    buff[14] = (uint8_t)(des->whitePointX & HDMI_DRM_METADATA_MARK);
    /* PB16 */
    buff[15] = (uint8_t)((des->whitePointX & HDMI_DRM_METADATA_MARK) & HDMI_DRM_METADATA_MARK);
    /* PB17 */
    buff[16] = (uint8_t)(des->whitePointY & HDMI_DRM_METADATA_MARK);
    /* PB18 */
    buff[17] = (uint8_t)((des->whitePointY & HDMI_DRM_METADATA_MARK) & HDMI_DRM_METADATA_MARK);
    /* PB19 */
    buff[18] = (uint8_t)(des->maxDisplayMasteringLuminance & HDMI_DRM_METADATA_MARK);
    /* PB20 */
    buff[19] = (uint8_t)((des->maxDisplayMasteringLuminance & HDMI_DRM_METADATA_MARK) & HDMI_DRM_METADATA_MARK);
    /* PB21 */
    buff[20] = (uint8_t)(des->minDisplayMasteringLuminance & HDMI_DRM_METADATA_MARK);
    /* PB22 */
    buff[21] = (uint8_t)((des->minDisplayMasteringLuminance & HDMI_DRM_METADATA_MARK) & HDMI_DRM_METADATA_MARK);
    /* PB23 */
    buff[22] = (uint8_t)(des->maxContentLightLevel & HDMI_DRM_METADATA_MARK);
    /* PB24 */
    buff[23] = (uint8_t)((des->maxContentLightLevel & HDMI_DRM_METADATA_MARK) & HDMI_DRM_METADATA_MARK);
    /* PB25 */
    buff[24] = (uint8_t)(des->maxFrameAverageLightLevel & HDMI_DRM_METADATA_MARK);
    /* PB26 */
    buff[25] = (uint8_t)((des->maxFrameAverageLightLevel & HDMI_DRM_METADATA_MARK) & HDMI_DRM_METADATA_MARK);
    HdmiInfoframeFillCheckSum(data, lenght);
    return HDF_SUCCESS;
}

static int32_t HdmiInfoframePacketEncoding(union HdmiInfoframeInfo *infoframe,
    enum HdmiPacketType type, uint8_t *data, uint32_t len)
{
    int32_t ret;

    if (infoframe == NULL || data == NULL) {
        HDF_LOGE("input param is invalid.");
        return HDF_ERR_INVALID_PARAM;
    }

    switch (type) {
        case HDMI_INFOFRAME_PACKET_TYPE_VS:
            ret = HdmiInfoframePacketVsEncoding(infoframe, data, len);
            break;
        case HDMI_INFOFRAME_PACKET_TYPE_AVI:
            ret = HdmiInfoframePacketAviEncoding(infoframe, data, len);
            break;
        case HDMI_INFOFRAME_PACKET_TYPE_SPD:
            ret = HdmiInfoframePacketSpdEncoding(infoframe, data, len);
            break;
        case HDMI_INFOFRAME_PACKET_TYPE_AUDIO:
            ret = HdmiInfoframePacketAudioEncoding(infoframe, data, len);
            break;
        case HDMI_INFOFRAME_PACKET_TYPE_DRM:
            ret = HdmiInfoframePacketDrmEncoding(infoframe, data, len);
            break;
        default:
            HDF_LOGD("type %d not support.", type);
            ret = HDF_ERR_NOT_SUPPORT;
    }
    return ret;
}

static int32_t HdmiInfoframeSend(struct HdmiInfoframe *frame, union HdmiInfoframeInfo *infoframe)
{
    uint8_t buffer[HDMI_INFOFRAME_LEN] = {0};
    struct HdmiCntlr *cntlr = NULL;
    int32_t ret;

    if (frame == NULL || frame->priv == NULL || infoframe == NULL) {
        HDF_LOGE("HdmiInfoframeSend: input param is invalid.");
        return HDF_ERR_INVALID_PARAM;
    }
    cntlr = (struct HdmiCntlr *)frame->priv;
    if (cntlr->ops == NULL || cntlr->ops->infoframeSend == NULL || cntlr->ops->infoframeEnable == NULL) {
        HDF_LOGD("HdmiInfoframeSend not support.");
        return HDF_ERR_NOT_SUPPORT;
    }

    ret = HdmiInfoframePacketEncoding(infoframe, infoframe->header.type, buffer, HDMI_INFOFRAME_LEN);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("encding infoframe %d fail", infoframe->header.type);
        return ret;
    }

    HdmiCntlrLock(cntlr);
    cntlr->ops->infoframeEnable(cntlr, infoframe->header.type, false);
    ret = cntlr->ops->infoframeSend(cntlr, infoframe->header.type, buffer, HDMI_INFOFRAME_LEN);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("send infoframe %d fail", infoframe->header.type);
        HdmiCntlrUnlock(cntlr);
        return ret;
    }
    cntlr->ops->infoframeEnable(cntlr, infoframe->header.type, true);
    HdmiCntlrUnlock(cntlr);
    return HDF_SUCCESS;
}

static void HdmiFillAviHdrInfoframe(struct HdmiAviInfoframe *avi,
    struct HdmiVideoAttr *videoAttr, struct HdmiHdrAttr *hdrAttr, struct HdmiCommonAttr *commAttr)
{
    switch (hdrAttr->mode) {
        case HDMI_HDR_MODE_CEA_861_3:
        case HDMI_HDR_MODE_CEA_861_3_AUTHEN:
            avi->colorimetry = videoAttr->colorimetry;
            avi->extColorimetry = videoAttr->extColorimetry;
            avi->colorSpace = commAttr->colorSpace;
            if (hdrAttr->mode == HDMI_HDR_MODE_CEA_861_3_AUTHEN) {
                avi->colorSpace = HDMI_COLOR_SPACE_YCBCR422;
            }
            avi->range = videoAttr->quantization;
            avi->yccRange = videoAttr->yccQuantization;
            break;
        case HDMI_HDR_MODE_DOLBY_NORMAL:
            avi->colorSpace = HDMI_COLOR_SPACE_YCBCR422;
            avi->colorimetry = (videoAttr->xvycc == true) ? HDMI_COLORIMETRY_EXTENDED : videoAttr->colorimetry;
            avi->extColorimetry = videoAttr->extColorimetry;
            avi->range = videoAttr->quantization;
            avi->yccRange = HDMI_YCC_QUANTIZATION_RANGE_FULL;
            break;
        case HDMI_HDR_MODE_DOLBY_TUNNELING:
            avi->colorSpace = HDMI_COLOR_SPACE_RGB;
            avi->colorimetry = (videoAttr->xvycc == true) ? HDMI_COLORIMETRY_EXTENDED : videoAttr->colorimetry;
            avi->extColorimetry = videoAttr->extColorimetry;
            avi->range = HDMI_QUANTIZATION_RANGE_FULL;
            avi->yccRange = videoAttr->yccQuantization;
            break;
        default:
            avi->colorSpace = commAttr->colorSpace;
            avi->colorimetry = (videoAttr->xvycc == true) ? HDMI_COLORIMETRY_EXTENDED : videoAttr->colorimetry;
            avi->extColorimetry = videoAttr->extColorimetry;
            avi->range = videoAttr->quantization;
            avi->yccRange = videoAttr->yccQuantization;
            break;
    }
}

static void HdmiFillAviInfoframeVersion(struct HdmiAviInfoframe *avi)
{
    /*
     * see hdmi spec2.0 10.1.
     * The Y2 and VIC7 bits are simply set to zero in a Version 2 AVI InfoFrame and might not be decoded by
     * some Sinks. A Version 3 AVI InfoFrame shall be used and the Version field set to 0x03 (indicating that
     * the Sink shall decode the additional most-significant bits) whenever either of the most-significant bits
     * Y2 or VIC7 are set to '1'. If both Y2 and VIC7 are set to '0', then a Version 2 AVI InfoFrame shall be used
     * and the Version field shall be set to 0x02 (indicating that the Sink does not have to decode the additional
     * most-significant bits).
     */
    avi->verNum = HDMI_AVI_VERSION2;
    if (((uint32_t)avi->colorSpace & HDMI_AVI_Y2_MASK) > 0 ||
        (avi->vic > HDMI_VIC_5120X2160P100_64_27)) {
        avi->verNum = HDMI_AVI_VERSION3;
    } else if (avi->colorimetry == HDMI_COLORIMETRY_EXTENDED &&
               avi->extColorimetry == HDMI_EXTENDED_COLORIMETRY_ADDITIONAL) {
        /*
         * (C1,C0) is (1,1) and (EC2,EC1,EC0) is (1,1,1), version shall be 4.
         * All fields of the Version 4 AVI InfoFrame are the same as Version 3 AVI InfoFrame,
         * except for the InfoFrame Version Number, Length of AVI InfoFrame, and additional Data Byte 14.
         */
        avi->verNum = HDMI_AVI_VERSION4;
        (avi->len)++;
    }
}

static void HdmiFillAviInfoframe(struct HdmiAviInfoframe *avi,
    struct HdmiVideoAttr *videoAttr, struct HdmiHdrAttr *hdrAttr, struct HdmiCommonAttr *commAttr)
{
    bool enable3d = true;

    if (memset_s(avi, sizeof(struct HdmiAviInfoframe), 0, sizeof(struct HdmiAviInfoframe)) != EOK) {
        HDF_LOGE("fill vsif, memset_s fail.");
        return;
    }
    avi->type = HDMI_INFOFRAME_PACKET_TYPE_AVI;
    avi->len = HDMI_AVI_INFOFRAME_LEN;

    avi->activeFormatInformationPresent = true;
    avi->colorSpace = commAttr->colorSpace;
    avi->activeAspect = videoAttr->activeAspect;
    avi->pictureAspect = videoAttr->aspect;
    avi->colorimetry = (videoAttr->xvycc == true) ? HDMI_COLORIMETRY_EXTENDED : videoAttr->colorimetry;
    avi->nups = videoAttr->nups;

    avi->range = videoAttr->quantization;
    avi->extColorimetry = videoAttr->extColorimetry;
    if (videoAttr->_3dStruct >= HDMI_VS_VIDEO_3D_BUTT) {
        enable3d = false;
    }
    avi->vic = HdmiCommonGetVic(videoAttr->timing, videoAttr->aspect, enable3d);
    avi->pixelRepetitionFactor = (uint8_t)videoAttr->pixelRepeat;
    avi->yccRange = videoAttr->yccQuantization;
    HdmiFillAviHdrInfoframe(avi, videoAttr, hdrAttr, commAttr);
    HdmiFillAviInfoframeVersion(avi);
}

int32_t HdmiAviInfoframeSend(struct HdmiInfoframe *frame, bool enable)
{
    struct HdmiCntlr *cntlr = NULL;
    union HdmiInfoframeInfo infoframe = {0};

    if (frame == NULL || frame->priv == NULL) {
        HDF_LOGE("HdmiAviInfoframeSend: input param is invalid.");
        return HDF_ERR_INVALID_PARAM;
    }
    cntlr = (struct HdmiCntlr *)frame->priv;
    if (cntlr->ops == NULL || cntlr->ops->infoframeEnable == NULL || cntlr->ops->infoframeSend == NULL) {
        HDF_LOGD("HdmiAviInfoframeSend not support.");
        return HDF_ERR_NOT_SUPPORT;
    }

    if (enable == false) {
        cntlr->ops->infoframeEnable(cntlr, HDMI_INFOFRAME_PACKET_TYPE_AVI, false);
        return HDF_SUCCESS;
    }
    HdmiFillAviInfoframe(&(frame->avi), &(cntlr->attr.videoAttr), &(cntlr->attr.hdrAttr), &(cntlr->attr.commAttr));
    infoframe.avi = frame->avi;
    return HdmiInfoframeSend(frame, &infoframe);
}

void HdmiFillAudioInfoframe(struct HdmiAudioInfoframe *audio, struct HdmiAudioAttr *audioAttr)
{
    if (memset_s(audio, sizeof(struct HdmiAudioInfoframe), 0, sizeof(struct HdmiAudioInfoframe)) != EOK) {
        HDF_LOGE("fill vsif, memset_s fail.");
        return;
    }
    audio->type = HDMI_INFOFRAME_PACKET_TYPE_AUDIO;
    audio->len = HDMI_AUDIO_INFOFRAME_LEN;
    audio->verNum = HDMI_AUDIO_INFOFRAME_VERSION;

    /* fill channels. */
    if (audioAttr->ifType != HDMI_AUDIO_IF_TYPE_I2S) {
        HDF_LOGI("audio channel refer to stream.");
    } else {
        HDF_LOGI("audio channel %u \n", audioAttr->channels);
        audio->channelCount = (audioAttr->channels >= 2) ? (audioAttr->channels - 1) : 0;
    }

    /* fill coding type. */
    if (audioAttr->codingType == HDMI_AUDIO_CODING_TYPE_AC3 ||
        audioAttr->codingType == HDMI_AUDIO_CODING_TYPE_DTS ||
        audioAttr->codingType == HDMI_AUDIO_CODING_TYPE_EAC3 ||
        audioAttr->codingType == HDMI_AUDIO_CODING_TYPE_DTS_HD) {
        audio->codingType = audioAttr->codingType;
    } else {
        audio->codingType = HDMI_AUDIO_CODING_TYPE_STREAM;
    }

    /* fill CA field. see CEA-861-D table 20. */
    switch (audioAttr->channels) {
        case HDMI_AUDIO_FORMAT_CHANNEL_3:
            audio->channelAllocation = 0x01; /* 1 channel */
            break;
        case HDMI_AUDIO_FORMAT_CHANNEL_6:
            audio->channelAllocation = 0x0b; /* 3, 4 channel */
            break;
        case HDMI_AUDIO_FORMAT_CHANNEL_8:
            audio->channelAllocation = 0x13; /* 1, 2, 5 channel */
            break;
        default:
            audio->channelAllocation = 0x00; /* 0 channel */
            break;
    }
}

int32_t HdmiAudioInfoframeSend(struct HdmiInfoframe *frame, bool enable)
{
    struct HdmiCntlr *cntlr = NULL;
    union HdmiInfoframeInfo infoframe = {0};

    if (frame == NULL || frame->priv == NULL) {
        HDF_LOGE("HdmiAudioInfoframeSend: input param is invalid.");
        return HDF_ERR_INVALID_PARAM;
    }
    cntlr = (struct HdmiCntlr *)frame->priv;
    if (cntlr->ops == NULL || cntlr->ops->infoframeEnable == NULL) {
        HDF_LOGD("HdmiAudioInfoframeSend not support.");
        return HDF_ERR_NOT_SUPPORT;
    }

    if (enable == false) {
        HdmiCntlrLock(cntlr);
        cntlr->ops->infoframeEnable(cntlr, HDMI_INFOFRAME_PACKET_TYPE_AUDIO, false);
        HdmiCntlrUnlock(cntlr);
        return HDF_SUCCESS;
    }
    HdmiFillAudioInfoframe(&(frame->audio), &(cntlr->attr.audioAttr));
    infoframe.audio = frame->audio;
    return HdmiInfoframeSend(frame, &infoframe);
}

static void HdmiFillDrmInfoframe(struct HdmiDrmInfoframe *drm, struct HdmiHdrAttr *HdrAttr)
{
    if (memset_s(drm, sizeof(struct HdmiDrmInfoframe), 0, sizeof(struct HdmiDrmInfoframe)) != EOK) {
        HDF_LOGE("fill vsif, memset_s fail.");
        return;
    }
    drm->type = HDMI_INFOFRAME_PACKET_TYPE_DRM;
    drm->len = HDMI_DRM_INFOFRAME_LEN;
    drm->verNum = HDMI_DRM_INFOFRAME_VERSION;
    drm->eotfType = HdrAttr->eotfType;
    drm->metadataType = HdrAttr->metadataType;
    drm->des = HdrAttr->descriptor;
}

int32_t HdmiDrmInfoframeSend(struct HdmiInfoframe *frame, bool enable)
{
    struct HdmiCntlr *cntlr = NULL;
    union HdmiInfoframeInfo infoframe = {0};

    if (frame == NULL || frame->priv == NULL) {
        HDF_LOGE("HdmiDrmInfoframeSend: input param is invalid.");
        return HDF_ERR_INVALID_PARAM;
    }
    cntlr = (struct HdmiCntlr *)frame->priv;
    if (cntlr->ops == NULL || cntlr->ops->infoframeEnable == NULL) {
        HDF_LOGD("HdmiDrmInfoframeSend not support.");
        return HDF_ERR_NOT_SUPPORT;
    }

    if (enable == false) {
        cntlr->ops->infoframeEnable(cntlr, HDMI_INFOFRAME_PACKET_TYPE_DRM, false);
        return HDF_SUCCESS;
    }
    HdmiFillDrmInfoframe(&(frame->drm), &(cntlr->attr.hdrAttr));
    infoframe.drm = frame->drm;
    return HdmiInfoframeSend(frame, &infoframe);
}

static uint8_t HdmiGetVsifLength(struct HdmiVs14VsifContent *_14Vsif, bool dolbyEnable, bool hdrSupport)
{
    uint8_t length = 0x07;

    if (hdrSupport == true) {
        if (dolbyEnable == true) {
            length = 0x18;
        } else if (_14Vsif->format == HDMI_VS_VIDEO_FORMAT_4K) {
            length = 0x05;
        } else if (_14Vsif->format == HDMI_VS_VIDEO_FORMAT_3D) {
            length = 0x07;
        } else {
            length = 0x04;
        }
    }
    return length;
}

static void HdmiFill14Vsif(struct HdmiVsInfoframe *vs, struct HdmiVideoAttr *videoAttr)
{
    struct HdmiVideo4kInfo *_4kInfo = NULL;
    struct HdmiVs14VsifContent *vsif = &(vs->vsifContent.vsif);
    enum HdmiVic vic;
    uint32_t cnt;

    vsif->oui = HDMI_IEEE_OUI_1_4;
    vic = HdmiCommonGetVic(videoAttr->timing, videoAttr->aspect, false);
    if ((vic == HDMI_VIC_3840X2160P24_16_9 || vic == HDMI_VIC_3840X2160P25_16_9 ||
        vic == HDMI_VIC_3840X2160P30_16_9 || vic == HDMI_VIC_4096X2160P24_256_135) &&
        videoAttr->_3dStruct == HDMI_VS_VIDEO_3D_BUTT) {
        vsif->format = HDMI_VS_VIDEO_FORMAT_4K;
        for (cnt = 0; cnt <= HDMI_VIDEO_4K_CODES_MAX; cnt++) {
            _4kInfo = HdmiCommonGetVideo4kInfo(cnt);
            if (_4kInfo != NULL && _4kInfo->timing == videoAttr->timing) {
                vsif->vic = _4kInfo->_4kVic;
                break;
            }
        }
    } else if (videoAttr->_3dStruct < HDMI_VS_VIDEO_3D_BUTT) {  // common 3D
        vsif->format = HDMI_VS_VIDEO_FORMAT_3D;
        vsif->_3dStruct = videoAttr->_3dStruct;
    } else {
        vsif->format = HDMI_VS_VIDEO_FORMAT_NULL;
        vsif->_3dStruct = videoAttr->_3dStruct;
    }
}

static void HdmiFillVsInfoframe(struct HdmiInfoframe *frame, struct HdmiVideoAttr *videoAttr,
    bool dolbyEnable, bool hdrSupport)
{
    struct HdmiVsInfoframe *vs = &(frame->vs);
    int32_t ret;

    ret = memset_s(vs, sizeof(struct HdmiVsInfoframe), 0, sizeof(struct HdmiVsInfoframe));
    if (ret != EOK) {
        HDF_LOGE("fill vsif, memset_s fail.");
        return;
    }
    vs->type = HDMI_INFOFRAME_PACKET_TYPE_VS;
    vs->verNum = HDMI_VSIF_VERSION;
    HdmiFill14Vsif(vs, videoAttr);
    vs->len = HdmiGetVsifLength(&(vs->vsifContent.vsif), dolbyEnable, hdrSupport);
    /* fill user vendor data */
    vs->vsifContent.userVsif.len = frame->userVsif.len;
    ret = memcpy_s(vs->vsifContent.userVsif.data, HDMI_VENDOR_USER_DATA_MAX_LEN,
        frame->userVsif.data, frame->userVsif.len);
    if (ret != EOK) {
        HDF_LOGE("fill vsif, memcpy_s fail.");
    }
}

int32_t HdmiVsInfoframeSend(struct HdmiInfoframe *frame, bool enable, bool dolbyEnable)
{
    struct HdmiCntlr *cntlr = NULL;
    union HdmiInfoframeInfo infoframe = {0};

    if (frame == NULL || frame->priv == NULL) {
        HDF_LOGE("HdmiVsInfoframeSend: input param is invalid.");
        return HDF_ERR_INVALID_PARAM;
    }
    cntlr = (struct HdmiCntlr *)frame->priv;
    if (cntlr->ops == NULL || cntlr->ops->infoframeEnable == NULL) {
        HDF_LOGD("HdmiVsInfoframeSend not support.");
        return HDF_ERR_NOT_SUPPORT;
    }

    if (enable == false) {
        cntlr->ops->infoframeEnable(cntlr, HDMI_INFOFRAME_PACKET_TYPE_VS, false);
        return HDF_SUCCESS;
    }
    HdmiFillVsInfoframe(frame, &(cntlr->attr.videoAttr), dolbyEnable, frame->hdrSupport);
    infoframe.vs = frame->vs;
    return HdmiInfoframeSend(frame, &infoframe);
}

static void HdmiFillSpdInfoframe(struct HdmiSpdInfoframe *spd, char *vendorName,
    char *productName, enum HdmiSpdSdi sdi)
{
    uint32_t len, length;

    if (memset_s(spd, sizeof(struct HdmiSpdInfoframe), 0, sizeof(struct HdmiSpdInfoframe)) != EOK) {
        HDF_LOGE("fill spd infoframe, memset_s fail.");
        return;
    }
    spd->type = HDMI_INFOFRAME_PACKET_TYPE_SPD;
    spd->len = HDMI_SPD_INFOFRAME_LEN;
    spd->verNum = HDMI_SPD_VERSION;
    spd->sdi = sdi;

    len = (uint32_t)strlen(vendorName);
    length = (uint32_t)sizeof(spd->vendorName);
    length = (length > len) ? len : length;
    if (memcpy_s(spd->vendorName, length, vendorName, length) != EOK) {
        HDF_LOGE("fill spd infoframe vendor name, memcpy_s fail.");
    }

    len = (uint32_t)strlen(productName);
    length = (uint32_t)sizeof(spd->productDescription);
    length = (length > len) ? len : length;
    if (memcpy_s(spd->productDescription, length, productName, length) != EOK) {
        HDF_LOGE("fill spd infoframe product name, memcpy_s fail.");
    }
}

int32_t HdmiSpdInfoframeSend(struct HdmiInfoframe *frame, bool enable,
    char *vendorName, char *productName, enum HdmiSpdSdi sdi)
{
    struct HdmiCntlr *cntlr = NULL;
    union HdmiInfoframeInfo infoframe = {0};

    if (frame == NULL || frame->priv == NULL) {
        HDF_LOGE("HdmiSpdInfoframeSend: input param is invalid.");
        return HDF_ERR_INVALID_PARAM;
    }
    cntlr = (struct HdmiCntlr *)frame->priv;
    if (cntlr->ops == NULL || cntlr->ops->infoframeEnable == NULL) {
        HDF_LOGD("HdmiSpdInfoframeSend not support.");
        return HDF_ERR_NOT_SUPPORT;
    }

    if (enable == false) {
        cntlr->ops->infoframeEnable(cntlr, HDMI_INFOFRAME_PACKET_TYPE_SPD, false);
        return HDF_SUCCESS;
    }
    HdmiFillSpdInfoframe(&(frame->spd), vendorName, productName, sdi);
    infoframe.spd = frame->spd;
    return HdmiInfoframeSend(frame, &infoframe);
}

int32_t HdmiInfoframeGetInfo(struct HdmiInfoframe *frame, enum HdmiPacketType type,
    union HdmiInfoframeInfo *infoframe)
{
    if (frame == NULL || infoframe == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }

    switch (type) {
        case HDMI_INFOFRAME_PACKET_TYPE_VS:
            infoframe->vs = frame->vs;
            break;
        case HDMI_INFOFRAME_PACKET_TYPE_AVI:
            infoframe->avi = frame->avi;
            break;
        case HDMI_INFOFRAME_PACKET_TYPE_AUDIO:
            infoframe->audio = frame->audio;
            break;
        case HDMI_INFOFRAME_PACKET_TYPE_DRM:
            infoframe->drm = frame->drm;
            break;
        default:
            HDF_LOGD("infoframe %d not support get", type);
            return HDF_ERR_INVALID_PARAM;
    }
    return HDF_SUCCESS;
}

int32_t HdmiInfoframeSetInfo(struct HdmiInfoframe *frame, enum HdmiPacketType type,
    union HdmiInfoframeInfo *infoframe)
{
    if (frame == NULL || infoframe == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }

    switch (type) {
        case HDMI_INFOFRAME_PACKET_TYPE_VS:
            frame->vs = infoframe->vs;
            break;
        case HDMI_INFOFRAME_PACKET_TYPE_AVI:
            frame->avi = infoframe->avi;
            break;
        case HDMI_INFOFRAME_PACKET_TYPE_AUDIO:
            frame->audio = infoframe->audio;
            break;
        case HDMI_INFOFRAME_PACKET_TYPE_DRM:
            frame->drm = infoframe->drm;
            break;
        default:
            HDF_LOGD("infoframe %d not support set", type);
            return HDF_ERR_INVALID_PARAM;
    }
    return HDF_SUCCESS;
}