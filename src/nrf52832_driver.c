#include "nrf52832_driver.h"
#include "common.h"
#include <zephyr/kernel.h>

#include <zephyr/logging/log.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>

LOG_MODULE_REGISTER(nrf52, LOG_LEVEL_DBG);

static struct bt_le_adv_param *adv_param = BT_LE_ADV_PARAM(
	    (BT_LE_ADV_OPT_CONNECTABLE |
	    BT_LE_ADV_OPT_USE_IDENTITY),                                                                // Connectable advertising and use identity address
	    800,                                                                                        // Min Advertising Interval 500ms (800*0.625ms)
	    801,                                                                                        // Max Advertising Interval 500.625ms (801*0.625ms)
	    NULL);                                                                                      // Set to NULL for undirected advertising

#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

static const struct bt_data ad[] = {
	/* Set the advertising flags */
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	/* Set the advertising packet data  */
	BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
};

static const struct bt_data sd[] = {
    BT_DATA_BYTES(BT_DATA_UUID128_ALL, CW_ADS_BT_UUID_VAL), 
};

static int receive_data(const void *buf, uint16_t len) {
	if (len == 17U) {
		uint8_t temp_date[17];

		memcpy(temp_date, buf, len);

		if(ds3231_time_write(temp_date)) {
			return -1;
		} else {
			ds3231_time_read();
			st7735_screen_write();
			return 0;
		}
	}
}

static ssize_t calibrate_time(struct bt_conn *conn, const struct bt_gatt_attr *attr, 
		const void *buf, uint16_t len, uint16_t offset, uint8_t flags) {

	receive_data(buf, len);

	return len;
}

static ssize_t customize_font(struct bt_conn *conn, const struct bt_gatt_attr *attr, 
		const void *buf, uint16_t len, uint16_t offset, uint8_t flags) {

	return len;
}

/* Create and add the MY LBS service to the Bluetooth LE stack */
BT_GATT_SERVICE_DEFINE(cube_watch_sevice, 
		/* primary service */
		BT_GATT_PRIMARY_SERVICE(CW_CTS_BT_UUID),
		/* calibration time service */
		BT_GATT_CHARACTERISTIC(CW_CTC_BT_UUID, 
				BT_GATT_CHRC_WRITE,
				BT_GATT_PERM_WRITE, 
				NULL, 
				calibrate_time, 
				NULL),
		BT_GATT_PRIMARY_SERVICE(CW_FCS_BT_UUID),
		BT_GATT_CHARACTERISTIC(CW_FCC_BT_UUID, 
				BT_GATT_CHRC_WRITE,
				BT_GATT_PERM_WRITE, 
				NULL, 
				customize_font, 
				NULL),
);

/*
 * @brief if connected
 */
static void on_connected(struct bt_conn *conn, uint8_t err) {
	if (err) {
		LOG_ERR("Connection failed (err %u)", err);
		return;
	}
    write_screen_thread_suspend();
    LOG_DBG("Connection succeed!");
}

/*
 * @brief if disconnected
 */
static void on_disconnected(struct bt_conn *conn, uint8_t reason) {
	LOG_DBG("Disconnected (reason %u)", reason);
	ds3231_time_read();
	st7735_screen_write();
    write_screen_thread_resume();
}

/*
 * @brief connection callbacks
 */
struct bt_conn_cb connection_callbacks = {
	.connected = on_connected,
	.disconnected = on_disconnected,
};

/*
 * @brief nrf52832 init func
 *
 * @retval 0 succeed
 * @retval -1 failed
 */
int nrf52832_init(void) {
    /* Enable the Bluetooth LE stack */
    if(bt_enable(NULL)) {
        return -1;
    }

    bt_conn_cb_register(&connection_callbacks);

    /* Start advertising */
	if (bt_le_adv_start(adv_param, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd))) {
		return -1;
	}

    return 0;
}
