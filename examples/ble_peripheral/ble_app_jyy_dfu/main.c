/**
 * Copyright (c) 2015 - 2018, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/**
 * @brief Blinky Sample Application main file.
 *
 * This file contains the source code for a sample server application using the LED Button service.
 */

#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "app_error.h"
#include "ble.h"
#include "ble_err.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "ble_advdata.h"
#include "ble_conn_params.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "boards.h"
#include "app_timer.h"
#include "app_button.h"
#include "ble_lbs.h"
#include "nrf_ble_gatt.h"
#include "nrf_ble_qwr.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_delay.h"
#include "nrf_drv_timer.h"
#include "nrf_drv_saadc.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"


#include "ff.h"
#include "diskio_blkdev.h"
#include "nrf_block_dev_sdc.h"

#include "app_timer.h"
#include "app_scheduler.h"


//DFU

#include "nrf_dfu_ble_svci_bond_sharing.h"
#include "nrf_svci_async_handler.h"
#include "nrf_svci_async_function.h"
#include "ble_dfu.h"
#include "peer_manager.h"
#include "fds.h"
#include "ble_conn_state.h"
#define  SEC_PARAM_BOND  1
/**< Perform bonding. */
#define  SEC_PARAM_MITM  0
/**< Man In The Middle protection not required. */
#define  SEC_PARAM_LESC  0
/**< LE Secure Connections not enabled. */
#define  SEC_PARAM_KEYPRESS  0
/**< Keypress notifications not enabled. */
#define  SEC_PARAM_IO_CAPABILITIES  BLE_GAP_IO_CAPS_NONE
/**< No I/O capabilities. */
#define  SEC_PARAM_OOB  0
/**< Out Of Band data not available. */
#define  SEC_PARAM_MIN_KEY_SIZE  7
/**< Minimum encryption key size. */
#define  SEC_PARAM_MAX_KEY_SIZE  16
/**< Maximum encryption key size. */



#define FILE_NAME   "NORDIC.TXT"
#define TEST_STRING "Impedence collection starts.\r\n"

#define SDC_SCK_PIN       3///< SDC serial clock (SCK) pin.ARDUINO_13_PIN 低变高
#define SDC_MOSI_PIN      2///< SDC serial data in (DI) pin.ARDUINO_11_PIN 高变低
#define SDC_MISO_PIN      28///< SDC serial data out (DO) pin.ARDUINO_12_PIN 高变低
#define SDC_CS_PIN        29///< SDC chip select (CS) pin. ARDUINO_10_PIN 高变低


#define ADVERTISING_LED                 BSP_BOARD_LED_0                         /**< Is on when device is advertising. */
#define CONNECTED_LED                   BSP_BOARD_LED_1                         /**< Is on when device has connected. */
#define LEDBUTTON_LED                   BSP_BOARD_LED_2                         /**< LED to be toggled with the help of the LED Button Service. */
#define LEDBUTTON_BUTTON                BSP_BUTTON_0                            /**< Button that will trigger the notification event with the LED Button Service */

#define DEVICE_NAME                     "ImpedanceJYY15"                        /**< Name of device. Will be included in the advertising data. */

#define APP_BLE_OBSERVER_PRIO           3                                       /**< Application's BLE observer priority. You shouldn't need to modify this value. */
#define APP_BLE_CONN_CFG_TAG            1                                       /**< A tag identifying the SoftDevice BLE configuration. */

#define APP_ADV_INTERVAL                6400                                      /**< The advertising interval (in units of 0.625 ms; this value corresponds to 40 ms). */
#define APP_ADV_DURATION                BLE_GAP_ADV_TIMEOUT_GENERAL_UNLIMITED   /**< The advertising time-out (in units of seconds). When set to 0, we will never time out. */


#define MIN_CONN_INTERVAL               MSEC_TO_UNITS(20, UNIT_1_25_MS)        /**8< Minimum acceptable connection interval . */
#define MAX_CONN_INTERVAL               MSEC_TO_UNITS(20, UNIT_1_25_MS)        /**12< Maximum acceptable connection interval . */
#define SLAVE_LATENCY                   90                                       /**< Slave latency. */
#define CONN_SUP_TIMEOUT                MSEC_TO_UNITS(6000,UNIT_10_MS)         /**< Connection supervisory time-out (4 seconds). */

//#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(20000)                  /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (15 seconds). */
//#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(5000)    /**< Time between each call to sd_ble_gap_conn_param_update after the first call (5 seconds). */
#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(5000)                       /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(30000)     
#define MAX_CONN_PARAMS_UPDATE_COUNT    3                                       /**< Number of attempts before giving up the connection parameter negotiation. */

#define BUTTON_DETECTION_DELAY          APP_TIMER_TICKS(50)                     /**< Delay from a GPIOTE event until a button is reported as pushed (in number of timer ticks). */

#define DEAD_BEEF                       0xDEADBEEF                              /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */


BLE_LBS_DEF(m_lbs);                                                             /**< LED Button Service instance. */
NRF_BLE_GATT_DEF(m_gatt);                                                       /**< GATT module instance. */
NRF_BLE_QWR_DEF(m_qwr); 

/**< Context for the Queued Write module.*/
static const nrf_drv_timer_t m_timer = NRF_DRV_TIMER_INSTANCE(1);

static uint16_t m_conn_handle = BLE_CONN_HANDLE_INVALID;                        /**< Handle of the current connection. */

static uint8_t m_adv_handle = BLE_GAP_ADV_SET_HANDLE_NOT_SET;                   /**< Advertising handle used to identify an advertising set. */
static uint8_t m_enc_advdata[BLE_GAP_ADV_SET_DATA_SIZE_MAX];                    /**< Buffer for storing an encoded advertising set. */
static uint8_t m_enc_scan_response_data[BLE_GAP_ADV_SET_DATA_SIZE_MAX];         /**< Buffer for storing an encoded scan data. */


//static FIL file1;
/**@brief Struct that contains pointers to the encoded advertising data. */
 static void advertising_start(bool erase_bonds);

 /**@brief Function for handling Peer Manager events.
 *
 * @param[in] p_evt Peer Manager event.
 */
 static void pm_evt_handler(pm_evt_t const * p_evt)
 {
 ret_code_t err_code;

 switch (p_evt->evt_id)
    {
     case PM_EVT_BONDED_PEER_CONNECTED:
     {
     NRF_LOG_INFO("Connected to a previously bonded device.");
     } break;

     case PM_EVT_CONN_SEC_SUCCEEDED:
     {
     NRF_LOG_INFO("Connection secured: role: %d, conn_handle: 0x%x,procedure: %d.",
     ble_conn_state_role(p_evt->conn_handle),
     p_evt->conn_handle,
     p_evt->params.conn_sec_succeeded.procedure);
     } break;

     case PM_EVT_CONN_SEC_FAILED:
     {
     /* Often, when securing fails, it shouldn't be restarted, for security reasons.
     * Other times, it can be restarted directly.
     * Sometimes it can be restarted, but only after changing some Security Parameters.
     * Sometimes, it cannot be restarted until the link is disconnected and reconnected.
     * Sometimes it is impossible, to secure the link, or the peer device does not support it.
     * How to handle this error is highly application dependent. */
     } break;

     case PM_EVT_CONN_SEC_CONFIG_REQ:
     {
     // Reject pairing request from an already bonded peer.
     pm_conn_sec_config_t conn_sec_config = {.allow_repairing = false};
     pm_conn_sec_config_reply(p_evt->conn_handle, &conn_sec_config);
     } break;

     case PM_EVT_STORAGE_FULL:
     {
     // Run garbage collection on the flash.
     err_code = fds_gc();
     if (err_code == FDS_ERR_NO_SPACE_IN_QUEUES)
     {
     // Retry.
     }
     else
     {
     APP_ERROR_CHECK(err_code);
     }
     } break;

     case PM_EVT_PEERS_DELETE_SUCCEEDED:
     {
     advertising_start(false);
     } break;

     case PM_EVT_PEER_DATA_UPDATE_FAILED:
     {
     //Assert.
     APP_ERROR_CHECK(p_evt->params.peer_data_update_failed.error);
     } break;

     case PM_EVT_PEER_DELETE_FAILED:
     {
     //Assert.
     APP_ERROR_CHECK(p_evt->params.peer_delete_failed.error);
     } break;

     case PM_EVT_PEERS_DELETE_FAILED:
     {
     //Assert.
     APP_ERROR_CHECK(p_evt->params.peers_delete_failed_evt.error);
     } break;

     case PM_EVT_ERROR_UNEXPECTED:
     {
     //Assert.
     APP_ERROR_CHECK(p_evt->params.error_unexpected.error);
     } break;

     case PM_EVT_CONN_SEC_START:
     case PM_EVT_PEER_DATA_UPDATE_SUCCEEDED:
     case PM_EVT_PEER_DELETE_SUCCEEDED:
     case PM_EVT_LOCAL_DB_CACHE_APPLIED:
     case PM_EVT_LOCAL_DB_CACHE_APPLY_FAILED:
     // This can happen when the local DB has changed.
     case PM_EVT_SERVICE_CHANGED_IND_SENT:
     case PM_EVT_SERVICE_CHANGED_IND_CONFIRMED:
     default:
     break;
    }
}

static void peer_manager_init()
{
     ble_gap_sec_params_t sec_param;
     ret_code_t err_code;

     err_code = pm_init();
     APP_ERROR_CHECK(err_code);

     memset(&sec_param, 0, sizeof(ble_gap_sec_params_t));

     // Security parameters to be used for all security procedures.
     sec_param.bond = SEC_PARAM_BOND;
     sec_param.mitm = SEC_PARAM_MITM;
     sec_param.lesc = SEC_PARAM_LESC;
     sec_param.keypress = SEC_PARAM_KEYPRESS;
     sec_param.io_caps = SEC_PARAM_IO_CAPABILITIES;
     sec_param.oob = SEC_PARAM_OOB;
     sec_param.min_key_size = SEC_PARAM_MIN_KEY_SIZE;
     sec_param.max_key_size = SEC_PARAM_MAX_KEY_SIZE;
     sec_param.kdist_own.enc = 1;
     sec_param.kdist_own.id = 1;
     sec_param.kdist_peer.enc = 1;
     sec_param.kdist_peer.id = 1;

     err_code = pm_sec_params_set(&sec_param);
     APP_ERROR_CHECK(err_code);

     err_code = pm_register(pm_evt_handler);
     APP_ERROR_CHECK(err_code);
 }

 /** @brief Clear bonding information from persistent storage.
 */
 static void delete_bonds(void)
 {
 ret_code_t err_code;

 NRF_LOG_INFO("Erase bonds!");

 err_code = pm_peers_delete();
 APP_ERROR_CHECK(err_code);
 }
 
 /**@brief Function for starting advertising.
 */
 static void advertising_start(bool erase_bonds)
 {
     if (erase_bonds == true)
     {
     delete_bonds();
     //Advertising is started by PM_EVT_PEERS_DELETE_SUCCEEDED event.
     }
     else
     {
     //uint32_t err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
		 uint32_t err_code = sd_ble_gap_adv_start(m_adv_handle, APP_BLE_CONN_CFG_TAG);
     APP_ERROR_CHECK(err_code);

     NRF_LOG_DEBUG("advertising is started");
     }

     // uint32_t err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
     // APP_ERROR_CHECK(err_code);
 }
 
 
/**@brief Struct that contains pointers to the encoded advertising data. */
static ble_gap_adv_data_t m_adv_data =
    {
        .adv_data =
            {
                .p_data = m_enc_advdata,
                .len = BLE_GAP_ADV_SET_DATA_SIZE_MAX},
        .scan_rsp_data =
            {
                .p_data = m_enc_scan_response_data,
                .len = BLE_GAP_ADV_SET_DATA_SIZE_MAX

            }};

/**@brief Function for assert macro callback.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num    Line number of the failing ASSERT call.
 * @param[in] p_file_name File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t *p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}

/**@brief Function for the LEDs initialization.
 *
 * @details Initializes all LEDs used by the application.
 */

/**@brief Function for the LEDs initialization.
 *
 * @details Initializes all LEDs used by the application.
 */
static void leds_init(void)
{
    bsp_board_init(BSP_INIT_LEDS);
}


/**@brief Function for the Timer initialization.
 *
 * @details Initializes the timer module.
 */



/**@brief Function for the GAP initialization.
 *
 * @details This function sets up all the necessary GAP (Generic Access Profile) parameters of the
 *          device including the device name, appearance, and the preferred connection parameters.
 */
static void gap_params_init(void)
{
    ret_code_t              err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *)DEVICE_NAME,
                                          strlen(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing the GATT module.
 */
static void gatt_init(void)
{
    ret_code_t err_code = nrf_ble_gatt_init(&m_gatt, NULL);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing the Advertising functionality.
 *
 * @details Encodes the required advertising data and passes it to the stack.
 *          Also builds a structure to be passed to the stack when starting advertising.
 */
static void advertising_init(void)
{
    ret_code_t    err_code;
    ble_advdata_t advdata;
    ble_advdata_t srdata;

    ble_uuid_t adv_uuids[] = {{LBS_UUID_SERVICE, m_lbs.uuid_type}};

    // Build and set advertising data.
    memset(&advdata, 0, sizeof(advdata));

    advdata.name_type          = BLE_ADVDATA_FULL_NAME;
    advdata.include_appearance = true;
    advdata.flags              = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;


    memset(&srdata, 0, sizeof(srdata));
    srdata.uuids_complete.uuid_cnt = sizeof(adv_uuids) / sizeof(adv_uuids[0]);
    srdata.uuids_complete.p_uuids  = adv_uuids;

    err_code = ble_advdata_encode(&advdata, m_adv_data.adv_data.p_data, &m_adv_data.adv_data.len);
    APP_ERROR_CHECK(err_code);

    err_code = ble_advdata_encode(&srdata, m_adv_data.scan_rsp_data.p_data, &m_adv_data.scan_rsp_data.len);
    APP_ERROR_CHECK(err_code);

    ble_gap_adv_params_t adv_params;

    // Set advertising parameters.
    memset(&adv_params, 0, sizeof(adv_params));

    adv_params.primary_phy     = BLE_GAP_PHY_1MBPS;
    adv_params.duration        = APP_ADV_DURATION;
    adv_params.properties.type = BLE_GAP_ADV_TYPE_CONNECTABLE_SCANNABLE_UNDIRECTED;
    adv_params.p_peer_addr     = NULL;
    adv_params.filter_policy   = BLE_GAP_ADV_FP_ANY;
    adv_params.interval        = APP_ADV_INTERVAL;

    err_code = sd_ble_gap_adv_set_configure(&m_adv_handle, &m_adv_data, &adv_params);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling Queued Write Module errors.
 *
 * @details A pointer to this function will be passed to each service which may need to inform the
 *          application about an error.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void nrf_qwr_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for handling write events to the LED characteristic.
 *
 * @param[in] p_lbs     Instance of LED Button Service to which the write applies.
 * @param[in] led_state Written/desired state of the LED.
 */
static FIL file;
static void led_write_handler(uint16_t conn_handle, ble_lbs_t * p_lbs, uint8_t led_state)
{
    if (led_state)
    {
			/**/
      bsp_board_led_on(LEDBUTTON_LED);
      NRF_LOG_INFO("Received LED ON!");
    }
    else
    {
        bsp_board_led_off(LEDBUTTON_LED);
        NRF_LOG_INFO("Received LED OFF!");
    }
}

//DFU
 static void ble_dfu_evt_handler(ble_dfu_buttonless_evt_type_t event)
 {
 switch (event)
 {
 case BLE_DFU_EVT_BOOTLOADER_ENTER_PREPARE:
 NRF_LOG_INFO("Device is preparing to enter bootloader mode.");
 // YOUR_JOB: Disconnect all bonded devices that currently are connected.
 // This is required to receive a service changed indication
 // on bootup after a successful (or aborted) Device Firmware Update.
 break;

 case BLE_DFU_EVT_BOOTLOADER_ENTER:
 // YOUR_JOB: Write app-specific unwritten data to FLASH, control finalization of this
 // by delaying reset by reporting false in app_shutdown_handler
 NRF_LOG_INFO("Device will enter bootloader mode.");
 break;

 case BLE_DFU_EVT_BOOTLOADER_ENTER_FAILED:
 NRF_LOG_ERROR("Request to enter bootloader mode failed asynchroneously.");
 // YOUR_JOB: Take corrective measures to resolve the issue
 // like calling APP_ERROR_CHECK to reset the device.
 break;

 case BLE_DFU_EVT_RESPONSE_SEND_ERROR:
 NRF_LOG_ERROR("Request to send a response to client failed.");
 // YOUR_JOB: Take corrective measures to resolve the issue
 // like calling APP_ERROR_CHECK to reset the device.
 APP_ERROR_CHECK(false);
 break;

 default:
 NRF_LOG_ERROR("Unknown event from ble_dfu_buttonless.");
 break;
 }
 }
 /**@brief Handler for shutdown preparation.
 *
 * @details During shutdown procedures, this function will be called at a 1 second interval
 * untill the function returns true. When the function returns true, it means that the
 * app is ready to reset to DFU mode.
 * @param[in] event Power manager event.
 * @retval True if shutdown is allowed by this power manager handler, otherwise false.
 */
 static bool app_shutdown_handler(nrf_pwr_mgmt_evt_t event)
 {
 switch (event)
 {
 case NRF_PWR_MGMT_EVT_PREPARE_DFU:
 NRF_LOG_INFO("Power management wants to reset to DFU mode.");
 // YOUR_JOB: Get ready to reset into DFU mode
 //
 // If you aren't finished with any ongoing tasks, return "false" to
 // signal to the system that reset is impossible at this stage.
 //
 // Here is an example using a variable to delay resetting the device.
 //
 // if (!m_ready_for_reset)
 // {
 // return false;
 // }
 // else
 //{
 //
 // // Device ready to enter
 // uint32_t err_code;
 // err_code = sd_softdevice_disable();
 // APP_ERROR_CHECK(err_code);
 // err_code = app_timer_stop_all();
 // APP_ERROR_CHECK(err_code);
 //}
 break;

 default:
 // YOUR_JOB: Implement any of the other events available from the power management module:
 // -NRF_PWR_MGMT_EVT_PREPARE_SYSOFF
 // -NRF_PWR_MGMT_EVT_PREPARE_WAKEUP
 // -NRF_PWR_MGMT_EVT_PREPARE_RESET
 return true;
 }

 NRF_LOG_INFO("Power management allowed to reset to DFU mode.");
 return true;
}

//lint -esym(528, m_app_shutdown_handler)
/**@brief Register application shutdown handler with priority 0.
 */
NRF_PWR_MGMT_HANDLER_REGISTER(app_shutdown_handler, 0);



static void time_write_handler(uint16_t conn_handle, ble_lbs_t * p_lbs, uint8_t const* str, uint8_t length)
{		

}

/**@brief Function for initializing services that will be used by the application.
 */
static void services_init(void)
{
    ret_code_t         err_code;
    ble_lbs_init_t     init     = {0};
    nrf_ble_qwr_init_t qwr_init = {0};
		ble_dfu_buttonless_init_t dfus_init = {0};
		
    // Initialize Queued Write Module.
    qwr_init.error_handler = nrf_qwr_error_handler;

    err_code = nrf_ble_qwr_init(&m_qwr, &qwr_init);
    APP_ERROR_CHECK(err_code);

    // Initialize LBS.
    init.led_write_handler = led_write_handler;
		init.time_write_handler = time_write_handler;

    err_code = ble_lbs_init(&m_lbs, &init);
    APP_ERROR_CHECK(err_code);
		
		// Initialize the async SVCI interface to bootloader.
		err_code = ble_dfu_buttonless_async_svci_init();
    APP_ERROR_CHECK(err_code);

    dfus_init.evt_handler = ble_dfu_evt_handler;
    err_code = ble_dfu_buttonless_init(&dfus_init);
    APP_ERROR_CHECK(err_code); 
}


/**@brief Function for handling the Connection Parameters Module.
 *
 * @details This function will be called for all events in the Connection Parameters Module that
 *          are passed to the application.
 *
 * @note All this function does is to disconnect. This could have been done by simply
 *       setting the disconnect_on_fail config parameter, but instead we use the event
 *       handler mechanism to demonstrate its use.
 *
 * @param[in] p_evt  Event received from the Connection Parameters Module.
 */
static void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
    ret_code_t err_code;

    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
}


/**@brief Function for handling a Connection Parameters error.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for initializing the Connection Parameters module.
 */
static void conn_params_init(void)
{
    ret_code_t             err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail             = false;
    cp_init.evt_handler                    = on_conn_params_evt;
    cp_init.error_handler                  = conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}




/**@brief Function for handling BLE events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 * @param[in]   p_context   Unused.
 */
uint8_t con_control=0;
bool flag_connect = false;
bool temp_connect = false;
static void ble_evt_handler(ble_evt_t const * p_ble_evt, void * p_context)
{
    ret_code_t err_code;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            NRF_LOG_INFO("Connected");
				    flag_connect = true;
						temp_connect = true;
            bsp_board_led_on(CONNECTED_LED);
            bsp_board_led_off(ADVERTISING_LED);
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            err_code = nrf_ble_qwr_conn_handle_assign(&m_qwr, m_conn_handle);
            APP_ERROR_CHECK(err_code);
            err_code = app_button_enable();
						con_control = 1;
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            NRF_LOG_INFO("Disconnected");
						flag_connect = false;
						temp_connect = true;
						FRESULT ff_result;
						ff_result=f_sync(&file);
				    if (ff_result != FR_OK)
							{
									NRF_LOG_INFO("f_sync dis failed\r\n.");
							}
							else
							{
									NRF_LOG_INFO("f_sync dis success!\r\n.");
							}
				
            bsp_board_led_off(CONNECTED_LED);
            m_conn_handle = BLE_CONN_HANDLE_INVALID;
            err_code = app_button_disable();
            APP_ERROR_CHECK(err_code);
            advertising_start(false);
            break;

        case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
            // Pairing not supported
            err_code = sd_ble_gap_sec_params_reply(m_conn_handle,
                                                   BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP,
                                                   NULL,
                                                   NULL);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
        {
            NRF_LOG_DEBUG("PHY update request.");
            ble_gap_phys_t const phys =
            {
                .rx_phys = BLE_GAP_PHY_AUTO,
                .tx_phys = BLE_GAP_PHY_AUTO,
            };
            err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
            APP_ERROR_CHECK(err_code);
        } break;

        case BLE_GATTS_EVT_SYS_ATTR_MISSING:
            // No system attributes have been stored.
            err_code = sd_ble_gatts_sys_attr_set(m_conn_handle, NULL, 0, 0);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTC_EVT_TIMEOUT:
            // Disconnect on GATT Client timeout event.
            NRF_LOG_DEBUG("GATT Client Timeout.");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_TIMEOUT:
            // Disconnect on GATT Server timeout event.
            NRF_LOG_DEBUG("GATT Server Timeout.");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        default:
            // No implementation needed.
            break;
    }
}


/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void)
{
    ret_code_t err_code;

    err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);

    // Configure the BLE stack using the default settings.
    // Fetch the start address of the application RAM.
    uint32_t ram_start = 0;
    err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    APP_ERROR_CHECK(err_code);

    // Enable BLE stack.
    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);

    // Register a handler for BLE events.
    NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);
}


/**@brief Function for handling events from the button handler module.
 *
 * @param[in] pin_no        The pin that the event applies to.
 * @param[in] button_action The button action (press/release).
 */
static void button_event_handler(uint8_t pin_no, uint8_t button_action)
{
    ret_code_t err_code;

    switch (pin_no)
    {
        case LEDBUTTON_BUTTON:
            NRF_LOG_INFO("Send button state change.");
            //err_code = ble_lbs_on_button_change(m_conn_handle, &m_lbs, button_action);
            if (err_code != NRF_SUCCESS &&
                err_code != BLE_ERROR_INVALID_CONN_HANDLE &&
                err_code != NRF_ERROR_INVALID_STATE &&
                err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING)
            {
                APP_ERROR_CHECK(err_code);
            }
            break;

        default:
            APP_ERROR_HANDLER(pin_no);
            break;
    }
}


/**@brief Function for initializing the button handler module.
 */
static void buttons_init(void)
{
    ret_code_t err_code;

    //The array must be static because a pointer to it will be saved in the button handler module.
    static app_button_cfg_t buttons[] =
    {
        {LEDBUTTON_BUTTON, false, BUTTON_PULL, button_event_handler}
    };

    err_code = app_button_init(buttons, ARRAY_SIZE(buttons),
                               BUTTON_DETECTION_DELAY);
    APP_ERROR_CHECK(err_code);
}


static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}


/**@brief Function for initializing power management.
 */
static void power_management_init(void)
{
    ret_code_t err_code;
    err_code = nrf_pwr_mgmt_init();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling the idle state (main loop).
 *
 * @details If there is no pending log operation, then sleep until next the next event occurs.
 */
static void idle_state_handle(void)
{
    if (NRF_LOG_PROCESS() == false)
    {
        nrf_pwr_mgmt_run();
    }
}




/**
 * @brief  SDC block device definition
 * */
NRF_BLOCK_DEV_SDC_DEFINE(
        m_block_dev_sdc,
        NRF_BLOCK_DEV_SDC_CONFIG(
                SDC_SECTOR_SIZE,
                APP_SDCARD_CONFIG(SDC_MOSI_PIN, SDC_MISO_PIN, SDC_SCK_PIN, SDC_CS_PIN)
         ),
         NFR_BLOCK_DEV_INFO_CONFIG("Nordic", "SDC", "1.00")
);

/**
 * @brief Function for demonstrating FAFTS usage.
 */


int counttx=0;
uint8_t  DataRead[20];



volatile uint8_t state = 1;

void saadc_callback(nrf_drv_saadc_evt_t const * p_event)
{
}

void saadc_init(void)
{
    ret_code_t err_code;
    nrf_saadc_channel_config_t channel_config =
            NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_AIN3);
    err_code = nrf_drv_saadc_init(NULL, saadc_callback);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_saadc_channel_init(0, &channel_config);
    APP_ERROR_CHECK(err_code);

}

static void thi_monitor_handler(void)
{
		nrf_saadc_value_t  saadc_val=0;
		nrf_drv_saadc_sample_convert(0,&saadc_val);
		if(counttx<10){
			DataRead[2*counttx]=(saadc_val>>8);
			DataRead[2*counttx+1]=saadc_val;
			counttx++;
		}
		if(counttx>=10)
		{
			if(m_conn_handle!=BLE_CONN_HANDLE_INVALID)
			{
				ble_lbs_on_button_change1(m_conn_handle, &m_lbs, DataRead);
			}
			counttx=0;
		}

}

//定时器超时中断操作
static void thi_monitor_timeout_handler(nrf_timer_event_t event_type,void * p_context)
{
	    switch (event_type)
    {
        case NRF_TIMER_EVENT_COMPARE2:
					thi_monitor_handler();
            break;
        default:
            //Do nothing.
            break;
    }

}

static void timers_init1(void)
{
	  uint32_t err_code;
    nrf_drv_timer_config_t timer_cfg = NRF_DRV_TIMER_DEFAULT_CONFIG;
    timer_cfg.bit_width = NRF_TIMER_BIT_WIDTH_32;
    err_code = nrf_drv_timer_init(&m_timer, &timer_cfg, thi_monitor_timeout_handler);
    APP_ERROR_CHECK(err_code);
}

static void application_timers_start(void)
{
    uint32_t ticks = nrf_drv_timer_ms_to_ticks(&m_timer, 2);
    nrf_drv_timer_extended_compare(&m_timer,
                                   NRF_TIMER_CC_CHANNEL2,
                                   ticks,
                                   NRF_TIMER_SHORT_COMPARE2_CLEAR_MASK,
                                   true);
    nrf_drv_timer_enable(&m_timer);
}


static void timers_init(void)
{
    // Initialize timer module, making it use the scheduler
    ret_code_t err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);

}


/**@brief Function for application main entry.
 */
int main(void)
{
    log_init();
    leds_init();
		
    timers_init();
    buttons_init();
    power_management_init();
    ble_stack_init();
		peer_manager_init();
    gap_params_init();
    gatt_init();
	
		nrf_gpio_cfg_output(6);//负载开关引脚
		nrf_gpio_pin_clear(6);
		
	  saadc_init();
		
		timers_init1();
		//application_timers_start();
	
    services_init();
    advertising_init();
    conn_params_init();

    // Start execution.
    NRF_LOG_INFO("Blinky example started.");
    advertising_start(false);

    NRF_LOG_INFO("FATFS example started.");
		NRF_LOG_FLUSH();	  

    for (;;)
    {
        if (flag_connect == true && temp_connect == true)
        {
					nrf_gpio_pin_set(6);
					application_timers_start();
          temp_connect = false;
        }
        else if (flag_connect == false && temp_connect == true)
        {
					nrf_gpio_pin_clear(6);
          nrf_drv_timer_disable(&m_timer);
          temp_connect = false;
        }
      idle_state_handle();
    }
}


/**
 * @}
 */
