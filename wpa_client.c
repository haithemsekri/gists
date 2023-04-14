// $CC -O1  wpa_client.c -o wpa_client.bin -lpthread -lwpa_client
#if 0
STATUS
INTERFACE_LIST
INTERFACE_REMOVE wlan0
INTERFACE_ADD wlan0
IFNAME=wlan0 PING

INTERFACE_REMOVE bcm0
IFNAME=wlan0 DRIVER interface_remove bcm0
IFNAME=wlan0 DRIVER interface_create bcm0
INTERFACE_ADD bcm0
IFNAME=bcm0 PING

IFNAME=wlan0 REMOVE_NETWORK all
IFNAME=bcm0 REMOVE_NETWORK all
IFNAME=bcm0 ADD_NETWORK
IFNAME=bcm0 SET_NETWORK 0 ssid "f084889dc9e8422c87150db4ad1594ed"
IFNAME=bcm0 SET_NETWORK 0 mode 2
IFNAME=bcm0 SET_NETWORK 0 frequency 2428
IFNAME=bcm0 SET_NETWORK 0 key_mgmt WPA-PSK
IFNAME=bcm0 SET_NETWORK 0 proto WPA2
IFNAME=bcm0 SET_NETWORK 0 psk "testpwd1"
IFNAME=bcm0 ENABLE_NETWORK 0
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
      printf("RxAsync>> %s\n", reply);
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
      char reply[1024];

      *reply = 0;
      size_t rb = read(wpa_ctrl_get_fd(wpa_ctrl_handle), reply, sizeof(reply));
      rb = rb;
      printf("TX>>%s>> %s\n", ifname, cmdline);

      *reply = 0;
      size_t reply_len = sizeof(reply);
      int ret =  wpa_ctrl_request(wpa_ctrl_handle, cmdline, cmd_len, reply, &reply_len, NULL);
      assert(0 == ret);
      reply[reply_len] = 0;
      printf("RX>>%s>> %s\n", ifname, reply);
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

