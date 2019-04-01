#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>
#include <misc/printk.h>
#include <settings/settings.h>
#include <zephyr.h>

// Includes for the GATT Services
#include "../include/gatt/hog.h"
#include "../include/gatt/bas.h"

/**
 * @brief Container for the advertisement information. This shows to potential
 * clients that this hardware provides the listed services
 *
 */
static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA_BYTES(BT_DATA_UUID16_ALL, 0x12, 0x18,       /* HID Service */
                                      0x0F, 0x18)       /* Battery Service */
};

/**
 * @brief Connected callback
 * 
 * @param conn The connection that called this callback
 * @param err Error integer
 */
static void connected(struct bt_conn *conn, u8_t err)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	if (err) {
		printk("Failed to connect to %s (%u)\n", addr, err);
		return;
	}

	printk("Connected %s\n", addr);

	if (bt_conn_security(conn, BT_SECURITY_MEDIUM)) {
		printk("Failed to set security\n");
	}
}

/**
 * @brief Disconnected callback
 * 
 * @param conn The connection that called this callback
 * @param reason A reason? integer
 */
static void disconnected(struct bt_conn *conn, u8_t reason)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	printk("Disconnected from %s (reason %u)\n", addr, reason);
}


/**
 * @brief Security Changed callback
 * 
 * @param conn The connection that called this callback
 * @param level What level the security has been changed to
 */
static void security_changed(struct bt_conn *conn, bt_security_t level)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	printk("Security changed: %s level %u\n", addr, level);
}


/**
 * @brief Container for the callback functions for various BT events
 * 
 */
static struct bt_conn_cb conn_callbacks = {
	.connected = connected,
	.disconnected = disconnected,
	.security_changed = security_changed,
};

/**
 * @brief This is the callback function that is called by bt_enable. It sets up
 * the entire BT stack
 *
 * @param err An integer error code
 */
static void bt_ready(int err) {
  if (err) {
    printk("Bluetooth init failed (err %d)\n", err);
    return;
  }
  printk("Bluetooth initialized\n");

  // Now we have to initialize the GATT Serivces
  // Specifically, we have to initialize the BAS (Battery Service) and HOG (HID
  // over GATT Service)
  hog_init();
  bas_init();

  // Load the BT configuration settings from flash, old encryption keys, etc.
  // This allows us to maintain connections through power cycles
  if (IS_ENABLED(CONFIG_SETTINGS)) {
    settings_load();
  }

  // Start the BT device's advertisement with the advertisement data
  err = bt_le_adv_start(BT_LE_ADV_CONN_NAME, ad, ARRAY_SIZE(ad), NULL, 0);

  // Check to see if advertisement sucessfully started
  if (err) {
    printk("Advertising failed to start (err %d)\n", err);
    return;
  }

  printk("Advertising successfully started\n");
}

/**
 * @brief This function is called when a new user connects and handles the generation of a passkey
 * 
 * @param conn The connection object
 * @param passkey The passkey to pair
 */
static void auth_passkey_display(struct bt_conn *conn, unsigned int passkey)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	printk("Passkey for %s: %06u\n", addr, passkey);
}

/**
 * @brief This function is called when a user cancels the pairing process
 * 
 * @param conn The connection object
 */
static void auth_cancel(struct bt_conn *conn)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	printk("Pairing cancelled: %s\n", addr);
}

/**
 * @brief Function to run once the pairing is complete
 * 
 * @param conn The connection object
 * @param bonded Whether or not the pairing is bonded
 */
static void pairing_complete(struct bt_conn *conn, bool bonded)
{
  printf("Pairing is complete\n");
}

/**
 * @brief The struct to hold the authentication callbacks
 * 
 */
static struct bt_conn_auth_cb auth_cb_display = {
	.passkey_display = auth_passkey_display,
	.passkey_entry = NULL,
	.cancel = auth_cancel,
  .pairing_complete = pairing_complete,
};

void main(void) {
  // Variable to hold the error state
  int err;

  // Enables the bluetooth and calls the bt_ready callback we provided
  err = bt_enable(bt_ready);

  // Check to see if bluetooth enabled
  if (err) {
    printk("Bluetooth init failed (err %d)\n", err);
    return;
  }

  // Attach the callbacks to the BT events
  bt_conn_cb_register(&conn_callbacks);
  bt_conn_auth_cb_register(&auth_cb_display);
}