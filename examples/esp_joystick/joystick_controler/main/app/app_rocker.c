/*
 * SPDX-FileCopyrightText: 2023-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
#include "app_rocker.h"

adc_oneshot_unit_handle_t game_mode_adc_handle = NULL;

adc_cali_handle_t left_hotas1_adc_chan_handle = NULL;
bool do_left_hotas1_adc_chan = false;

adc_cali_handle_t left_hotas2_adc_chan_handle = NULL;
bool do_left_hotas2_adc_chan = false;

adc_cali_handle_t right_hotas1_adc_chan_handle = NULL;
bool do_right_hotas1_adc_chan = false;

adc_cali_handle_t right_hotas2_adc_chan_handle = NULL;
bool do_right_hotas2_adc_chan = false;

static int g_left_hotas1_adc_raw[2][10];
static int g_left_hotas2_adc_raw[2][10];
static int g_right_hotas1_adc_raw[2][10];
static int g_right_hotas2_adc_raw[2][10];

static bool example_adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle)
{
    adc_cali_handle_t handle = NULL;
    esp_err_t ret = ESP_FAIL;
    bool calibrated = false;

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    if (!calibrated) {
        ESP_LOGI(ROCKER_TAG, "calibration scheme version is %s", "Curve Fitting");
        adc_cali_curve_fitting_config_t cali_config = {
            .unit_id = unit,
            // .chan = channel,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_curve_fitting(&cali_config, &handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }
#endif

#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    if (!calibrated) {
        ESP_LOGI(ROCKER_TAG, "calibration scheme version is %s", "Line Fitting");
        adc_cali_line_fitting_config_t cali_config = {
            .unit_id = unit,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_line_fitting(&cali_config, &handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }
#endif

    *out_handle = handle;
    if (ret == ESP_OK) {
        ESP_LOGI(ROCKER_TAG, "Calibration Success");
    } else if (ret == ESP_ERR_NOT_SUPPORTED || !calibrated) {
        ESP_LOGW(ROCKER_TAG, "eFuse not burnt, skip software calibration");
    } else {
        ESP_LOGE(ROCKER_TAG, "Invalid arg or no memory");
    }

    return calibrated;
}

esp_err_t rocker_adc_init(void)
{
    adc_oneshot_unit_init_cfg_t rocker_adc_init_config = {
        .unit_id = ADC_UNIT_2,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&rocker_adc_init_config, &game_mode_adc_handle));

    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = EXAMPLE_ADC_ATTEN,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(game_mode_adc_handle, LEFT_HOTAS1_ADC_CHAN, &config));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(game_mode_adc_handle, LEFT_HOTAS2_ADC_CHAN, &config));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(game_mode_adc_handle, RIGHT_HOTAS1_ADC_CHAN, &config));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(game_mode_adc_handle, RIGHT_HOTAS2_ADC_CHAN, &config));

    do_left_hotas1_adc_chan = example_adc_calibration_init(ADC_UNIT_2, LEFT_HOTAS1_ADC_CHAN, EXAMPLE_ADC_ATTEN, &left_hotas1_adc_chan_handle);
    do_left_hotas2_adc_chan = example_adc_calibration_init(ADC_UNIT_2, LEFT_HOTAS2_ADC_CHAN, EXAMPLE_ADC_ATTEN, &left_hotas2_adc_chan_handle);
    do_right_hotas1_adc_chan = example_adc_calibration_init(ADC_UNIT_2, RIGHT_HOTAS1_ADC_CHAN, EXAMPLE_ADC_ATTEN, &right_hotas1_adc_chan_handle);
    do_right_hotas2_adc_chan = example_adc_calibration_init(ADC_UNIT_2, RIGHT_HOTAS2_ADC_CHAN, EXAMPLE_ADC_ATTEN, &right_hotas2_adc_chan_handle);
    ESP_LOGI(ROCKER_TAG, "rocker adc init OK.");
    return ESP_OK;
}

void get_rocker_adc_value(uint16_t rocker_value[4])
{
    adc_oneshot_read(game_mode_adc_handle, LEFT_HOTAS1_ADC_CHAN, &g_left_hotas1_adc_raw[0][0]);
    adc_oneshot_read(game_mode_adc_handle, LEFT_HOTAS2_ADC_CHAN, &g_left_hotas2_adc_raw[0][0]);
    adc_oneshot_read(game_mode_adc_handle, RIGHT_HOTAS1_ADC_CHAN, &g_right_hotas1_adc_raw[0][0]);
    adc_oneshot_read(game_mode_adc_handle, RIGHT_HOTAS2_ADC_CHAN, &g_right_hotas2_adc_raw[0][0]);
    if (do_left_hotas1_adc_chan && do_left_hotas2_adc_chan && do_right_hotas1_adc_chan && do_right_hotas2_adc_chan) {
        rocker_value[0] = g_left_hotas1_adc_raw[0][0];
        rocker_value[1] = g_left_hotas2_adc_raw[0][0];
        rocker_value[2] = g_right_hotas1_adc_raw[0][0];
        rocker_value[3] = g_right_hotas2_adc_raw[0][0];
    }
}