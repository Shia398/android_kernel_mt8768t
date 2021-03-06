// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2019 MediaTek Inc.
 */

#include "mclk.h"

struct MCLK_PINCTRL_NAMES
	mclk_pinctrl_list[IMGSENSOR_SENSOR_IDX_MAX_NUM][MCLK_STATE_MAX_NUM] = {
	{{"cam0_mclk_off"}, {"cam0_mclk_on"} },
	{{"cam1_mclk_off"}, {"cam1_mclk_on"} },
	{{"cam2_mclk_off"}, {"cam2_mclk_on"} },
	{{"cam3_mclk_off"}, {"cam3_mclk_on"} },
	{{"cam4_mclk_off"}, {"cam4_mclk_on"} },
};


static struct mclk mclk_instance;
static enum IMGSENSOR_RETURN mclk_release(void *pinstance)
{
	int i;
	struct mclk *pinst = (struct mclk *)pinstance;

	for (i = IMGSENSOR_SENSOR_IDX_MIN_NUM;
	    i < IMGSENSOR_SENSOR_IDX_MAX_NUM;
	    i++) {
		if (pinst->ppinctrl_state[i][MCLK_STATE_DISABLE] != NULL &&
			!IS_ERR(pinst->ppinctrl_state[i][MCLK_STATE_DISABLE])) {
			mutex_lock(&pinctrl_mutex);
			pinctrl_select_state(
			    pinst->ppinctrl,
			    pinst->ppinctrl_state[i][MCLK_STATE_DISABLE]);
			mutex_unlock(&pinctrl_mutex);
		}

	}
	return IMGSENSOR_RETURN_SUCCESS;

}
static enum IMGSENSOR_RETURN mclk_init(void *pinstance)
{
	struct mclk *pinst = (struct mclk *)pinstance;
	struct platform_device *pplatform_dev = gpimgsensor_hw_platform_device;
	int i;
	enum   IMGSENSOR_RETURN ret           = IMGSENSOR_RETURN_SUCCESS;

	pinst->ppinctrl = devm_pinctrl_get(&pplatform_dev->dev);
	if (IS_ERR(pinst->ppinctrl)) {
		PK_DBG("%s : Cannot find camera pinctrl!\n", __func__);
		return IMGSENSOR_RETURN_ERROR;
	}

	for (i = IMGSENSOR_SENSOR_IDX_MIN_NUM;
	    i < IMGSENSOR_SENSOR_IDX_MAX_NUM;
	    i++) {


		if (mclk_pinctrl_list[i][MCLK_STATE_DISABLE].ppinctrl_names)
			pinst->ppinctrl_state[i][MCLK_STATE_DISABLE] =
			    pinctrl_lookup_state(pinst->ppinctrl,
		    mclk_pinctrl_list[i][MCLK_STATE_DISABLE].ppinctrl_names);

		if (pinst->ppinctrl_state[i][MCLK_STATE_DISABLE] != NULL &&
			!IS_ERR(pinst->ppinctrl_state[i][MCLK_STATE_DISABLE])) {
			mutex_lock(&pinctrl_mutex);
			pinctrl_select_state(
			    pinst->ppinctrl,
			    pinst->ppinctrl_state[i][MCLK_STATE_DISABLE]);
			mutex_unlock(&pinctrl_mutex);
		} else {
			PK_DBG(
			    "%s : pinctrl err, %s\n",
			    __func__,
		    mclk_pinctrl_list[i][MCLK_STATE_DISABLE].ppinctrl_names);

			ret = IMGSENSOR_RETURN_ERROR;
		}


		if (mclk_pinctrl_list[i][MCLK_STATE_ENABLE].ppinctrl_names)
			pinst->ppinctrl_state[i][MCLK_STATE_ENABLE] =
				pinctrl_lookup_state(pinst->ppinctrl,
			mclk_pinctrl_list[i][MCLK_STATE_ENABLE].ppinctrl_names);

		if (pinst->ppinctrl_state[i][MCLK_STATE_ENABLE] == NULL ||
			IS_ERR(pinst->ppinctrl_state[i][MCLK_STATE_ENABLE])) {

			PK_DBG("%s : pinctrl err, %s\n", __func__,
			mclk_pinctrl_list[i][MCLK_STATE_ENABLE].ppinctrl_names);

			ret = IMGSENSOR_RETURN_ERROR;
		}

	}

	return ret;
}

static enum IMGSENSOR_RETURN mclk_set(
	void *pinstance,
	enum IMGSENSOR_SENSOR_IDX   sensor_idx,
	enum IMGSENSOR_HW_PIN       pin,
	enum IMGSENSOR_HW_PIN_STATE pin_state)
{
	struct mclk *pinst = (struct mclk *)pinstance;
	struct pinctrl_state *ppinctrl_state;
	enum   IMGSENSOR_RETURN ret = IMGSENSOR_RETURN_SUCCESS;
	enum MCLK_STATE state_index = MCLK_STATE_DISABLE;

	/*PK_DBG("%s : sensor_idx %d mclk_set pinctrl, PinIdx %d, Val %d\n",
	 *__func__, sensor_idx, pin, pin_state);
	 */

	if (pin_state < IMGSENSOR_HW_PIN_STATE_LEVEL_0 ||
	   pin_state > IMGSENSOR_HW_PIN_STATE_LEVEL_HIGH) {
		ret = IMGSENSOR_RETURN_ERROR;
	} else {
		state_index = (pin_state > IMGSENSOR_HW_PIN_STATE_LEVEL_0)
		    ? MCLK_STATE_ENABLE : MCLK_STATE_DISABLE;

		ppinctrl_state = pinst->ppinctrl_state[sensor_idx][state_index];
		mutex_lock(&pinctrl_mutex);
		if (ppinctrl_state != NULL && !IS_ERR(ppinctrl_state))
			pinctrl_select_state(pinst->ppinctrl, ppinctrl_state);
		else
			PK_DBG(
			    "%s : sensor_idx %d fail to set pinctrl, PinIdx %d, Val %d\n",
			    __func__,
			    sensor_idx,
			    pin,
			    pin_state);
		mutex_unlock(&pinctrl_mutex);
	}
	return ret;
}

static struct IMGSENSOR_HW_DEVICE device = {
	.pinstance = (void *)&mclk_instance,
	.init      = mclk_init,
	.set       = mclk_set,
	.release   = mclk_release,
	.id        = IMGSENSOR_HW_ID_MCLK
};

enum IMGSENSOR_RETURN imgsensor_hw_mclk_open(
	struct IMGSENSOR_HW_DEVICE **pdevice)
{
	*pdevice = &device;
	return IMGSENSOR_RETURN_SUCCESS;
}

