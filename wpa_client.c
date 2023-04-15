#if 0
$CC -O1  ~/wpa_client.c -o wpa_client.bin -lpthread -lwpa_client -I.

wpa_supplicant -P /run/wpa-supp.pid -g /var/run/wpa_supplicant-global -Dnl80211 -dd

wpa_cli -g/var/run/wpa_supplicant-global

STATUS
INTERFACE_LIST
INTERFACE_REMOVE wlan0
INTERFACE_ADD wlan0
IFNAME=wlan0 PING

INTERFACE_REMOVE ap0
IFNAME=wlan0 DRIVER interface_remove ap0
IFNAME=wlan0 DRIVER interface_create ap0
INTERFACE_ADD ap0
IFNAME=ap0 PING

IFNAME=wlan0 REMOVE_NETWORK all
IFNAME=ap0 REMOVE_NETWORK all
IFNAME=ap0 ADD_NETWORK
IFNAME=ap0 SET_NETWORK 0 ssid "f084889dc9e8422c87150db4ad1594ed"
IFNAME=ap0 SET_NETWORK 0 mode 2
IFNAME=ap0 SET_NETWORK 0 frequency 2418
IFNAME=ap0 SET_NETWORK 0 key_mgmt WPA-PSK
IFNAME=ap0 SET_NETWORK 0 proto WPA2
IFNAME=ap0 SET_NETWORK 0 psk "verystrongpass"
IFNAME=ap0 ENABLE_NETWORK 0
IFNAME=ap0 DISABLE_NETWORK 0
IFNAME=ap0 REMOVE_NETWORK all
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <assert.h>
#include <errno.h>
#include <sys/select.h>

#include "wpa_ctrl.h"

int fds_wait(fd_set * rfds, int * fds, uint32_t fds_len, uint32_t ms)
{
  assert (rfds && fds && fds_len && ms);

  FD_ZERO(rfds);

  int nfds=-1;

  for(unsigned i = 0; i < fds_len; i++) {
    assert(fds[i] > 0);
    FD_SET(fds[i], rfds);
    if (fds[i] > nfds) {
      nfds = fds[i];
    }
  }
  assert(nfds >= 1);
  struct timeval timeout = {ms / 1000, (ms % 1000) * 1000};
  return select(nfds + 1, rfds, NULL, NULL, &timeout);
}

static void * wpa_async_task(void * arg)
{
  (void) arg;
  struct wpa_ctrl * ctl = (struct wpa_ctrl *) arg;
  int fd = wpa_ctrl_get_fd(ctl);
  assert(fd > 0);

  char reply [4096];
  size_t reply_len;

  while(1) {
      fd_set rfds = {0};
      fds_wait(&rfds, &fd, 1, UINT32_MAX);

      reply_len = sizeof(reply) - 1;
      reply [0] = 0;
      int ret = wpa_ctrl_recv(ctl, reply, &reply_len);
      if (reply_len && *reply) {
        char * cmd = strstr(reply, "IFNAME=");
        char * _ifname="\0";
        if (cmd && *cmd) {
          cmd += sizeof("IFNAME=") - 1;
          _ifname=cmd;
          while(*cmd && *cmd != ' ') cmd++;
          *cmd++ = 0;
          printf("RxAsync<<%s<< %s\n", _ifname, cmd);
        } else {
          printf("RxAsync<< %s\n", reply);
        }
      }
  }

  pthread_exit(0);
}



static struct wpa_ctrl *wpa_ctrl_handle = NULL;
static struct wpa_ctrl *wpa_mon_handle = NULL;
const char * ifname = "\0";

int main(void)
{
  wpa_ctrl_handle = wpa_ctrl_open("/var/run/wpa_supplicant-global");
  assert(wpa_ctrl_handle);
  wpa_mon_handle = wpa_ctrl_open("/var/run/wpa_supplicant-global");
  assert(wpa_mon_handle);
  assert(0 == wpa_ctrl_attach(wpa_mon_handle));

  pthread_t pth;
  pthread_create(&pth, NULL, wpa_async_task, wpa_mon_handle);

  char cmdline[128] = {0};
  do {
    unsigned cmd_len=strlen(cmdline);
    if (cmd_len > 2 && cmdline && *cmdline && cmdline[0] != '\r' && cmdline[0] != '\n') {
      if (0 == strncmp(cmdline, "EXIT", sizeof("EXIT") - 1)) {
        printf("Exit Loop");
        break;
      }
      if (cmdline[cmd_len - 1] == '\n') {
        cmdline[cmd_len - 1] = 0;
        cmd_len--;
      }

      char * cmd = strstr(cmdline, "IFNAME=");
      ifname="\0";
      if (cmd && *cmd) {
        cmd += sizeof("IFNAME=") - 1;
        ifname=cmd;
        while(*cmd && *cmd != ' ') cmd++;
        *cmd++ = 0;
        printf("TX>>%s>> %s\n", ifname, cmd);
        cmd[-1] = ' ';
      } else {
        cmd = cmdline;
        printf("TX>> %s\n", cmdline);
      }

      char reply[1024];
      *reply = 0;
      size_t rb = read(wpa_ctrl_get_fd(wpa_ctrl_handle), reply, sizeof(reply));
      rb = rb;

      *reply = 0;
      size_t reply_len = sizeof(reply);
      int ret =  wpa_ctrl_request(wpa_ctrl_handle, cmdline, cmd_len, reply, &reply_len, NULL);
      assert(0 == ret);
      reply[reply_len] = 0;
      if (ifname && *ifname && cmd > cmdline) {
        cmd[-1] = 0;
        printf("RX<<%s<< %s\n", ifname, reply);
      } else {
        printf("RX<< %s\n", reply);
      }

    }
    fputs("> ", stdout);
    *cmdline = 0;
  } while (fgets(cmdline, sizeof(cmdline) - 1, stdin));

  putc('\n', stdout);
  pthread_cancel(pth);
  pthread_join(pth, 0);
  wpa_ctrl_close(wpa_ctrl_handle);
  wpa_ctrl_handle = NULL;
  wpa_ctrl_detach(wpa_mon_handle);
  wpa_ctrl_close(wpa_mon_handle);
  wpa_mon_handle = NULL;
  return 0;
}















# if 0


#include "control/wpa/client.h"
#include "control/tools/tools.h"
#include "control/wpa/scan_list.h"
#include "control/wifi_manager_control.h"
#include "manager/wifi_manager_common.h"
#include "data/wifi_manager_data.h"
#include "utils/utils_string.h"

// ----------------------------------------------------------------------------
// Local Definitions

#define WPA_BUFFER_SIZE 16384

// ----------------------------------------------------------------------------
// Local Variable Definitions

static struct wpa_ctrl *wpa_ctrl_handle = NULL;
static struct wpa_ctrl *wpa_mon_handle = NULL;
static int wpa_mon_fd = CONTROL_NO_FD;
static const char * ifname = "";

// ----------------------------------------------------------------------------
// Local Function Declarations

// ----------------------------------------------------------------------------
// Local Function Definitions

// ----------------------------------------------------------------------------
// Exported Function Definitions

int wpa_client_get_event_fd(void)
{
    if (!wpa_mon_handle) {
        return CONTROL_NO_FD;
    }
    return wpa_mon_fd;
}


bool wpa_client_session_open(void)
{
    if (NULL
        == (wpa_ctrl_handle = wpa_ctrl_open("/var/run/wpa_supplicant-global"))) {
        L_ERROR("Failed to call wpa_ctrl_open ctrl\n");
    } else if (NULL
               == (wpa_mon_handle =
                           wpa_ctrl_open("/var/run/wpa_supplicant-global"))) {
        L_ERROR("Failed to call wpa_ctrl_open ctrl\n");
    } else if (0 != wpa_ctrl_attach(wpa_mon_handle)) {
        L_ERROR("Failed to call wpa_ctrl_attach ctrl\n");
    } else if (-1 == (wpa_mon_fd = wpa_ctrl_get_fd(wpa_mon_handle))) {
        L_ERROR("Failed to call wpa_ctrl_attach ctrl\n");
    } else {
        return true;
    }

    wpa_client_session_close();

    return false;
}

void Setup0(void) {

bool wpa_set_ifacename(const  char * iface);

        char reply[32];
        size_t reply_len;

        reply_len = sizeof(reply) - 1;
        *reply=0;
        ifname="\0";
        wpa_client_wpa_client(reply, &reply_len, "interface_add wlan0");
        wpa_client_wpa_client(reply, &reply_len, "SCAN");
        getchar();
        ifname="wlan0";
        wpa_client_wpa_client(reply, &reply_len, "SCAN");
        getchar();
        wpa_client_wpa_client(reply, &reply_len, "SCAN_RESULTS");
        getchar();
        wpa_client_wpa_client(reply, &reply_len, "ping");

        getchar();


        reply_len = sizeof(reply) - 1;
        *reply=0;
        ifname="wlan0";
        wpa_client_wpa_client(reply, &reply_len, "SCAN");

        getchar();

        reply_len = sizeof(reply) - 1;
        ifname="wlan0";
        wpa_client_wpa_client(reply, &reply_len, "SCAN_RESULTS");


        *reply=0;
        reply_len = sizeof(reply) - 1;
        ifname="wlan0";
        wpa_client_wpa_client(reply, &reply_len, "ping");
}

bool wpa_client_session_close(void)
{
    if (wpa_ctrl_handle) {
        wpa_ctrl_close(wpa_ctrl_handle);
        wpa_ctrl_handle = NULL;
    }

    if (wpa_mon_handle) {
        wpa_ctrl_detach(wpa_mon_handle);
        wpa_ctrl_close(wpa_mon_handle);
        wpa_mon_handle = NULL;
        wpa_mon_fd = CONTROL_NO_FD;
    }

    return true;
}

bool wpa_set_ifacename(const  char * iface)
{
  ifname = iface;
  return true;
}
bool wpa_client_send_sync_cmd(char *reply, size_t *reply_len, const char *cmd,
                              size_t len)
{
    if (NULL == wpa_ctrl_handle) {
        L_ERROR("Null pointer wpa_ctrl_handle\n");
        return false;
    }

    char buff[512] = {0};
    if(ifname && *ifname) {
        snprintf(buff, sizeof(buff) - 1, "IFNAME=%s %s", ifname, cmd);
        cmd = buff;
        len = strlen(buff);
    }

    printf("TX>>%s>> %s\n", ifname, cmd);

    char tmpb[1024];
    size_t rb = read(wpa_ctrl_get_fd(wpa_ctrl_handle), tmpb, sizeof(tmpb));
rb=rb;

    int ret =
            wpa_ctrl_request(wpa_ctrl_handle, cmd, len, reply, reply_len, NULL);

    if (ret != 0) {
        L_ERROR("wpa_ctrl_request failed cmd<%s> ret<%d>\n", cmd, ret);
    } else {
        reply[*reply_len] = 0;
    printf("RX>>%s>> %s\n", ifname, reply);
    }

    return (0 == ret);
}

bool wpa_client_recv_async_data(char *data, size_t *data_len)
{
    if (NULL == wpa_mon_handle) {
        L_ERROR("Null pointer wpa_mon_handle\n");
        return false;
    }

    return (0 == wpa_ctrl_recv(wpa_mon_handle, data, data_len));
}

bool wpa_client_station_add(uint8_t *id, const uint8_t *ssid, bool hidden_ssid,
                            const char *mac, const uint8_t *passphrase,
                            enum wifi_encryption_type encryption_type,
                            uint8_t priority)
{
    char reply[4096];
    /* Worst case is all of the bytes needed to be encoded as \xab or \033
     * i.e. 4 * 32 */
    char ssid_encoded[4 * WIFI_MANAGER_SSID_MAX_LENGTH + 1];
    size_t reply_len;

    if (!id || !ssid || !mac || !passphrase) {
        L_ERROR("Invalid argument\n");
        return false;
    }

    if (!*ssid || !*passphrase) {
        L_ERROR("Invalid argument\n");
        return false;
    }

    if (NULL == wpa_ctrl_handle) {
        L_ERROR("Null pointer wpa_ctrl_handle\n");
        return false;
    }

    /* Add network so that an id is reserved */
    reply_len = sizeof(reply) - 1;
    if (!wpa_client_wpa_client(reply, &reply_len, "ADD_NETWORK")) {
        return false;
    }

    if (!isdigit(*reply)) {
        return false;
    }

    *id = (uint8_t)atoi(reply);

    /* Set the SSID */
    reply_len = sizeof(reply) - 1;
    tools_ssid_encode(ssid_encoded, sizeof(ssid_encoded), ssid,
                      utils_strnlen_s((const char*)ssid, WIFI_MANAGER_SSID_MAX_LENGTH));
    /* P prefix prepares wpa_supplicant for \x encoded SSIDs */
    if (!wpa_client_wpa_client(reply, &reply_len, "SET_NETWORK %d ssid P\"%s\"", *id,
                           ssid_encoded)) {
        return false;
    }

    if (strcmp(reply, "OK\n")) {
        L_ERROR("Wpa: SET_NETWORK ssid failed\n");
        return false;
    }

    /* Set hidden SSID support */
    if (hidden_ssid) {
        reply_len = sizeof(reply) - 1;
        if (!wpa_client_wpa_client(reply, &reply_len, "SET_NETWORK %d scan_ssid 1",
                               *id)) {
            return false;
        }

        if (strcmp(reply, "OK\n")) {
            L_ERROR("Wpa: SET_NETWORK scan_ssid failed\n");
            return false;
        }
    }

    /* Set the BSSID */
    if (0 < utils_strnlen_s(mac, WIFI_MANAGER_BSSID_MAX_LENGTH)) {
        reply_len = sizeof(reply) - 1;
        if (!wpa_client_wpa_client(reply, &reply_len, "SET_NETWORK %d bssid %s",
                               *id, mac)) {
            return false;
        }

        if (strcmp(reply, "OK\n")) {
            L_ERROR("Wpa: BSSID failed\n");
            return false;
        }
    }

    /* Set the passphrase */
    reply_len = sizeof(reply) - 1;
    if (!wpa_client_wpa_client(reply, &reply_len, "SET_NETWORK %d psk \"%s\"", *id,
                           passphrase)) {
        return false;
    }

    if (strcmp(reply, "OK\n")) {
        L_ERROR("Wpa: SET_NETWORK psk failed\n");
        return false;
    }

    /* Ignore, protocol will restrict encryption types */
    (void)encryption_type;

    /* Set supported protocols */
    reply_len = sizeof(reply) - 1;
    if (!wpa_client_wpa_client(reply, &reply_len, "SET_NETWORK %d proto %s", *id,
                           data_get_supported_protocols())) {
        return false;
    }

    if (strcmp(reply, "OK\n")) {
        L_ERROR("Wpa: SET_NETWORK proto failed\n");
        return false;
    }

    /* Set preferred priority */
    reply_len = sizeof(reply) - 1;
    if (!wpa_client_wpa_client(reply, &reply_len, "SET_NETWORK %d priority %hhu",
                           *id, priority)) {
        return false;
    }

    if (strcmp(reply, "OK\n")) {
        L_ERROR("Wpa: SET_NETWORK priority failed\n");
        return false;
    }

    return true;
}

bool wpa_client_station_remove(uint8_t id)
{
    if (NULL == wpa_ctrl_handle) {
        L_ERROR("Null pointer wpa_ctrl_handle\n");
        return false;
    }

    char reply[4096];
    size_t reply_len;

    reply_len = sizeof(reply) - 1;
    if (!wpa_client_wpa_client(reply, &reply_len, "DISABLE_NETWORK %d", id)) {
        return false;
    }

    // No need to check if DISABLE_NETWORK returns OK or FAIL.

    reply_len = sizeof(reply) - 1;
    if (!wpa_client_wpa_client(reply, &reply_len, "REMOVE_NETWORK %d", id)) {
        return false;
    }

    // No need to check if REMOVE_NETWORK returns OK or FAIL.

    return true;
}

bool wpa_client_station_remove_all(void)
{
    if (NULL == wpa_ctrl_handle) {
        L_ERROR("Null pointer wpa_ctrl_handle\n");
        return false;
    }

    char reply[4096];
    size_t reply_len;

    for (size_t id = 0; id < 256; id++) {
        reply_len = sizeof(reply) - 1;
        (void)wpa_client_wpa_client(reply, &reply_len, "REMOVE_NETWORK %ld", id);
    }

    return false;
}

bool wpa_client_station_connect(uint8_t id, bool disable_others)
{
    if (NULL == wpa_ctrl_handle) {
        L_ERROR("Null pointer wpa_ctrl_handle\n");
        return false;
    }

    char reply[4096];
    size_t reply_len;

    if (disable_others) {
        reply_len = sizeof(reply) - 1;
        if (!wpa_client_wpa_client(reply, &reply_len, "SELECT_NETWORK %d", id)) {
            return false;
        }

        if (strcmp(reply, "OK\n")) {
            L_ERROR("Wpa: SELECT_NETWORK failed\n");
            return false;
        }
    }

    reply_len = sizeof(reply) - 1;
    if (!wpa_client_wpa_client(reply, &reply_len, "ENABLE_NETWORK %d", id)) {
        return false;
    }

    if (strcmp(reply, "OK\n")) {
        L_ERROR("Wpa: ENABLE_NETWORK failed\n");
        return false;
    }

    return true;
}

bool wpa_client_station_disconnect(void)
{
    if (NULL == wpa_ctrl_handle) {
        L_ERROR("Null pointer wpa_ctrl_handle\n");
        return false;
    }

    uint8_t id = UINT8_MAX;
    if (WNET_AP_LINK_STATUS_CONNECTED
        != wpa_client_station_connect_status(&id)) {
        Log(ALOG_WARNING, __FILE__, __LINE__, "Wifi already disconnected\n");
        return true;
    }

    Log(ALOG_INFO, __FILE__, __LINE__, "Disconnect wifi from station id(%d)\n",
        id);

    char reply[4096];
    size_t reply_len;

    reply_len = sizeof(reply) - 1;
    if (!wpa_client_wpa_client(reply, &reply_len, "DISABLE_NETWORK %d", id)) {
        return false;
    }

    return true;
}

bool wpa_client_station_update_priority(uint8_t id, uint8_t priority)
{
    char reply[4096];
    size_t reply_len;

    reply_len = sizeof(reply) - 1;
    if (!wpa_client_wpa_client(reply, &reply_len, "SET_NETWORK %d priority %hhu",
                           id, priority)) {
        return false;
    }

    if (strcmp(reply, "OK\n")) {
        L_ERROR("Wpa: SET_NETWORK priority failed\n");
        return false;
    }

    return true;
}

bool wpa_client_connection_get_status(struct wnet_link_data *buf)
{
    char reply[512];
    size_t reply_len;
    /* Allocate a few extra bytes (8) for SSID as the keyword function is not
     * with respect to maximum length */
    char keyword_v[WIFI_MANAGER_SSID_MAX_LENGTH * 4 + 1 + 8];
    size_t keyword_l;
    uint8_t ssid_decoded[WIFI_MANAGER_SSID_MAX_LENGTH];
    size_t ssid_decoded_len;

    if (!buf) {
        return false;
    }

    if (NULL == wpa_ctrl_handle) {
        L_INFO("wpa_session not open\n");
        return false;
    }

    /* Init the buffer data */
    bzero(buf, sizeof(struct wnet_link_data));
    buf->wsta_buf.id = 0xFF;

    reply_len = sizeof(reply) - 1;
    if (!wpa_client_wpa_client(reply, &reply_len, "STATUS")) {
        return false;
    }

    keyword_l = sizeof(keyword_v) - 1;
    keyword_v[0] = 0;

    if (!tools_keyword_value(reply, "wpa_state", '=', keyword_v, &keyword_l)) {
        L_ERROR("wpa_state not found\n");
        return false;
    }

    if (!strcmp(keyword_v, "INACTIVE")) {
        buf->link_status = WNET_AP_LINK_STATUS_DISCONNECTED;
    } else if (!strcmp(keyword_v, "DISCONNECTED")) {
        buf->link_status = WNET_AP_LINK_STATUS_DISCONNECTED;
    } else if (!strcmp(keyword_v, "4WAY_HANDSHAKE")) {
        buf->link_status = WNET_AP_LINK_STATUS_CONNECTING;
    } else if (!strcmp(keyword_v, "SCANNING")) {
        buf->link_status = WNET_AP_LINK_STATUS_BUSY;
    } else if (!strcmp(keyword_v, "ASSOCIATING")) {
        buf->link_status = WNET_AP_LINK_STATUS_CONNECTING;
    } else if (!strcmp(keyword_v, "ASSOCIATED")) {
        buf->link_status = WNET_AP_LINK_STATUS_CONNECTING;
    } else if (!strcmp(keyword_v, "COMPLETED")) {
        buf->link_status = WNET_AP_LINK_STATUS_CONNECTED;

        keyword_l = sizeof(keyword_v) - 1;
        keyword_v[0] = 0;
        if (tools_keyword_value(reply, "id", '=', keyword_v, &keyword_l)) {
            buf->wsta_buf.id = (uint8_t)atoi(keyword_v);
        }

        keyword_l = sizeof(keyword_v) - 1;
        keyword_v[0] = 0;
        if (tools_keyword_value(reply, "\nssid", '=', keyword_v, &keyword_l)) {
            ssid_decoded_len = tools_ssid_decode(ssid_decoded,
                                                 WIFI_MANAGER_SSID_MAX_LENGTH,
                                                 keyword_v);
            COPY_SSID_LEN(buf->wsta_buf.ssid, ssid_decoded, ssid_decoded_len);
        }

        keyword_l = sizeof(keyword_v) - 1;
        keyword_v[0] = 0;
        if (tools_keyword_value(reply, "bssid", '=', keyword_v, &keyword_l)) {
            utils_strncpy_s(buf->wsta_buf.bssid, sizeof(buf->wsta_buf.bssid),
                            keyword_v, keyword_l);
        }

        keyword_l = sizeof(keyword_v) - 1;
        keyword_v[0] = 0;
        if (tools_keyword_value(reply, "freq", '=', keyword_v, &keyword_l)) {
            buf->wsta_buf.frequency = (uint16_t)atoi(keyword_v);
        }

        buf->wsta_buf.flags = 0;
        keyword_l = sizeof(keyword_v) - 1;
        keyword_v[0] = 0;
        if (tools_keyword_value(reply, "group_cipher", '=', keyword_v,
                                &keyword_l)) {
            buf->wsta_buf.flags |= wpa_scan_key_mgt_str_to_u32(keyword_v);
        }

        keyword_l = sizeof(keyword_v) - 1;
        keyword_v[0] = 0;
        if (tools_keyword_value(reply, "key_mgmt", '=', keyword_v,
                                &keyword_l)) {
            buf->wsta_buf.flags |= wpa_scan_key_mgt_str_to_u32(keyword_v);
        }

    } else {
        L_ERROR("Unknown wpa_state <%s>\n", keyword_v);
        buf->link_status = WNET_AP_LINK_STATUS_UNKNOWN;
    }

    return true;
}

enum wnet_ap_link_status wpa_client_station_connect_status(uint8_t *id)
{
    struct wnet_link_data wnet_data;

    bzero(&wnet_data, sizeof(wnet_data));

    wnet_data.wsta_buf.id = 0xFF;
    if (!wpa_client_connection_get_status(&wnet_data)) {
        return WNET_AP_LINK_STATUS_UNKNOWN;
    }

    *id = wnet_data.wsta_buf.id;

    return wnet_data.link_status;
}

bool wpa_client_service_ping(void)
{
    char reply[32];
    size_t reply_len;

    reply_len = sizeof(reply) - 1;

    return wpa_client_wpa_client(reply, &reply_len, "PING")
           && (0 == strcmp(reply, "PONG\n"));
}

bool wpa_client_scan_start(void)
{
    char reply[32];
    size_t reply_len;

    reply_len = sizeof(reply) - 1;

    if (!wpa_client_wpa_client(reply, &reply_len, "SCAN")) {
        return false;
    }

    if (strcmp(reply, "OK\n")) {
        L_ERROR("Wpa: SCAN failed reply<%s>\n", reply);
        return false;
    }

    return true;
}

bool wpa_client_get_scan_result(char *reply, size_t *reply_len)
{
    return wpa_client_wpa_client(reply, reply_len, "SCAN_RESULTS");
}

bool wpa_client_wpa_client(char *reply, size_t *reply_len, const char *cmd_fmt, ...)
{
    char cmd[4096];
    cmd[0] = 0;
    va_list args;
    va_start(args, cmd_fmt);
    size_t len = utils_vsnprintf(cmd, sizeof(cmd), cmd_fmt, args);
    va_end(args);

    reply[0] = 0;

    return wpa_client_send_sync_cmd(reply, reply_len, cmd, len);
}

bool wpa_client_set_debug_level(const char * level)
{
    char reply[32];
    size_t reply_len;

    reply_len = sizeof(reply) - 1;
    if (!wpa_client_wpa_client(reply, &reply_len, "LOG_LEVEL %s", level)) {
      L_ERROR("Set wpa_supplicant debug level to %s failed", level);
      return false;
    } else if (0 != strcmp(reply, "OK\n")) {
      L_ERROR("Set wpa_supplicant debug level to %s failed, reply %s",  level, reply);
      return false;
    } else {
      return true;
    }
}
#endif
