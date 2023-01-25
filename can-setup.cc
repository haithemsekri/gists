// g++ can_start_stop.cc -o can_start_stop.cc.bin -Wall -Wextra -pedantic -O2 -Wall -Wextra -fstack-protector-strong -D_FORTIFY_SOURCE=2 -Wshift-count-overflow -Wsizeof-array-argument -Wall -Wextra -Werror -Warray-bounds=2 -feliminate-unused-debug-types -Wformat -Wformat-security -Werror=format-security -O2 -ffile-prefix-map=$PWD=. -fdiagnostics-show-option -Wfloat-equal -Wlogical-op -Wundef -Wredundant-decls -Wshadow -Wstrict-overflow=2 -Wwrite-strings -Wpointer-arith -Wcast-qual -Wformat=2 -Wformat-truncation -Wmissing-include-dirs -Wswitch-enum -Wdisabled-optimization -Winline -Winvalid-pch -Wdouble-promotion -Wshadow -Wvector-operation-performance -Wnull-dereference -Wduplicated-cond -Wshift-overflow=2 -Wnull-dereference -Wduplicated-cond -Wcast-function-type  -Wmissing-field-initializers -Wunreachable-code -Wpointer-arith

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <net/if.h>
#include <linux/can.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/can/netlink.h>

#undef L_DEBUG
#undef L_ERROR
#undef L_FATAL
#define L_DEBUG printf
#define L_ERROR printf
#define L_FATAL(...) printf(__VA_ARGS__); exit(-1)

#define NLMSG_TAIL(nmsg) \
  ((struct rtattr *) (((uintptr_t) (nmsg)) + NLMSG_ALIGN((nmsg)->nlmsg_len)))

#define IF_UP 1
#define IF_DOWN 2

struct get_req {
  struct nlmsghdr n;
  struct rtgenmsg g;
};

struct set_req {
  struct nlmsghdr n;
  struct ifinfomsg i;
  char buf[1024];
};

struct req_info {
  uint8_t restart;
  uint8_t disable_autorestart;
  __u32 restart_ms;
  struct can_ctrlmode *ctrlmode;
  struct can_bittiming *bittiming;
};

static int addattr32(struct nlmsghdr *n, size_t maxlen, int type, __u32 data)
{
  int len = RTA_LENGTH(4);
  struct rtattr *rta;

  if (NLMSG_ALIGN(n->nlmsg_len) + len > maxlen) {
    L_ERROR(
      "addattr32: Error! max allowed bound %zu exceeded\n",
      maxlen);
    return -1;
  }

  rta = NLMSG_TAIL(n);
  rta->rta_type = type;
  rta->rta_len = len;
  memcpy(RTA_DATA(rta), &data, 4);
  n->nlmsg_len = NLMSG_ALIGN(n->nlmsg_len) + len;

  return 0;
}

static int addattr_l(struct nlmsghdr *n, size_t maxlen, int type,
         const void *data, int alen)
{
  int len = RTA_LENGTH(alen);
  struct rtattr *rta;

  if (NLMSG_ALIGN(n->nlmsg_len) + RTA_ALIGN(len) > maxlen) {
    L_ERROR(
      "addattr_l ERROR: message exceeded bound of %zu\n",
      maxlen);
    return -1;
  }

  rta = NLMSG_TAIL(n);
  rta->rta_type = type;
  rta->rta_len = len;
  memcpy(RTA_DATA(rta), data, alen);
  n->nlmsg_len = NLMSG_ALIGN(n->nlmsg_len) + RTA_ALIGN(len);

  return 0;
}

static int send_mod_request(int fd, struct nlmsghdr *n)
{
  int status;

  struct sockaddr_nl nladdr;
  memset(&nladdr, 0, sizeof(nladdr));

  struct nlmsghdr *h;

  struct iovec iov;
  memset(&iov, 0x00, sizeof(iov));
  iov.iov_base = (void *)n;
  iov.iov_len = n->nlmsg_len;

  struct msghdr msg;
  memset(&msg, 0x00, sizeof(msg));
  msg.msg_name = &nladdr;
  msg.msg_namelen = sizeof(nladdr);
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;

  char buf[16384];

  nladdr.nl_family = AF_NETLINK;
  nladdr.nl_pid = 0;
  nladdr.nl_groups = 0;

  n->nlmsg_seq = 0;
  n->nlmsg_flags |= NLM_F_ACK;

  status = sendmsg(fd, &msg, 0);

  if (status < 0) {
    L_ERROR("Cannot talk to rtnetlink\n");
    return -1;
  }

  iov.iov_base = buf;
  while (1) {
    iov.iov_len = sizeof(buf);
    status = recvmsg(fd, &msg, 0);
    for (h = (struct nlmsghdr *)buf; (size_t) status >= sizeof(*h);) {
      int len = h->nlmsg_len;
      int l = len - sizeof(*h);
      if (l < 0 || len > status) {
        if (msg.msg_flags & MSG_TRUNC) {
          L_ERROR("Truncated message\n");
          return -1;
        }
        L_ERROR("!!!malformed message: len=%d\n", len);
        return -1;
      }

      if (h->nlmsg_type == NLMSG_ERROR) {
        struct nlmsgerr *err =
            (struct nlmsgerr *)NLMSG_DATA(h);
        if ((size_t) l < sizeof(struct nlmsgerr)) {
          L_ERROR("ERROR truncated\n");
        } else {
          errno = -err->error;
          if (errno == 0)
            return 0;

          L_ERROR("RTNETLINK answers: (errno == %s)\n", strerror(errno));
        }
        return -1;
      }
      status -= NLMSG_ALIGN(len);
      h = (struct nlmsghdr *)((char *)h + NLMSG_ALIGN(len));
    }
  }

  return 0;
}

static int open_nl_sock(void)
{
  int fd;

  fd = socket(AF_NETLINK, SOCK_RAW|SOCK_CLOEXEC, NETLINK_ROUTE);
  if (fd < 0) {
    L_ERROR("Cannot open netlink socket\n");
    return -1;
  }

  int sndbuf = 32768;
  assert(0 == setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (void *)&sndbuf, sizeof(sndbuf)));

  int rcvbuf = 1048576;
  assert(0 == setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (void *)&rcvbuf, sizeof(rcvbuf)));

  int ackbuf = 1;
  assert(0 == setsockopt(fd, SOL_NETLINK, NETLINK_EXT_ACK, (void *)&ackbuf, sizeof(ackbuf)));

  struct sockaddr_nl local;
  memset(&local, 0, sizeof(local));
  local.nl_family = AF_NETLINK;
  local.nl_pid = 0;
  local.nl_groups = 0;

  if (bind(fd, (struct sockaddr *)&local, sizeof(local)) < 0) {
    L_ERROR("Cannot bind netlink socket\n");
    return -1;
  }

  unsigned int addr_len = sizeof(local);
  if (getsockname(fd, (struct sockaddr *)&local, &addr_len) < 0) {
    L_ERROR("Cannot getsockname\n");
    return -1;
  }
  if (addr_len != sizeof(local)) {
    L_ERROR("Wrong address length %u\n", addr_len);
    return -1;
  }
  if (local.nl_family != AF_NETLINK) {
    L_ERROR("Wrong address family %d\n", local.nl_family);
    return -1;
  }

  int strictChkBuf = 1;
  assert(0 == setsockopt(fd, SOL_NETLINK, NETLINK_GET_STRICT_CHK, &strictChkBuf, sizeof(strictChkBuf)));
  return fd;
}

static int do_set_nl_link(int fd, uint8_t if_state, const char *name,
        struct req_info *req_info)
{
  struct set_req req;
  memset(&req, 0, sizeof(req));

  req.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg));
  req.n.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
  req.n.nlmsg_type = RTM_NEWLINK;
  req.i.ifi_family = AF_UNSPEC;
  #define ARPHRD_NETROM 0
  req.i.ifi_type = ARPHRD_NETROM;
  req.i.ifi_index = if_nametoindex(name);
  if (req.i.ifi_index == 0) {
    L_ERROR("Cannot find device \"%s\"\n", name);
    return -1;
  }

  if (if_state) {
    switch (if_state) {
    case IF_DOWN:
      req.i.ifi_change |= IFF_UP;
      req.i.ifi_flags &= ~IFF_UP;
      break;
    case IF_UP:
      req.i.ifi_change |= IFF_UP;
      req.i.ifi_flags |= IFF_UP;
      break;
    default:
      L_ERROR("unknown state\n");
      return -1;
    }
  }

  if (req_info != NULL) {
    const char *type = "can";
    /* setup linkinfo section */
    struct rtattr *linkinfo = NLMSG_TAIL(&req.n);
    addattr_l(&req.n, sizeof(req), IFLA_LINKINFO, NULL, 0);
    addattr_l(&req.n, sizeof(req), IFLA_INFO_KIND, type,
        strlen(type));
    /* setup data section */
    struct rtattr *data = NLMSG_TAIL(&req.n);
    addattr_l(&req.n, sizeof(req), IFLA_INFO_DATA, NULL, 0);

    if (req_info->restart_ms > 0 || req_info->disable_autorestart)
      addattr32(&req.n, 1024, IFLA_CAN_RESTART_MS,
          req_info->restart_ms);

    if (req_info->restart)
      addattr32(&req.n, 1024, IFLA_CAN_RESTART, 1);

    if (req_info->bittiming != NULL) {
      addattr_l(&req.n, 1024, IFLA_CAN_BITTIMING,
          req_info->bittiming,
          sizeof(struct can_bittiming));
    }

    if (req_info->ctrlmode != NULL) {
      addattr_l(&req.n, 1024, IFLA_CAN_CTRLMODE,
          req_info->ctrlmode,
          sizeof(struct can_ctrlmode));
    }

    /* mark end of data section */
    data->rta_len = (uintptr_t)NLMSG_TAIL(&req.n) - (uintptr_t)data;

    /* mark end of link info section */
    linkinfo->rta_len =
        (uintptr_t)NLMSG_TAIL(&req.n) - (uintptr_t)linkinfo;
  }

  return send_mod_request(fd, &req.n);
}

#define CMD_UP      "--up"
#define CMD_DOWN    "--down"
#define CMD_BITRATE "--bitrate"

int main(int argc, char * argv [])
{
  if (argc < 3) {
    L_ERROR("Expected 3 arguments: %s iface options\n", argv[0]);
    return -1;
  }

  int bitrate;
  int sockfd =  open_nl_sock();
  int ret = -1;
  if (!strncmp(argv[2], CMD_UP, sizeof(CMD_UP) - 1)) {
    L_DEBUG("Setting %s up\n", argv[1]);
    ret = do_set_nl_link(sockfd, IF_UP, argv[1], NULL);
  } else if ((argc >= 4) && !strncmp(argv[2], CMD_BITRATE, sizeof(CMD_BITRATE) - 1) && (bitrate = atoi(argv[3]))) {
    struct req_info req_info;
    struct can_bittiming bt;
    memset(&req_info, 0x00, sizeof(req_info));
    memset(&bt, 0, sizeof(bt));
    bt.bitrate = bitrate;
    req_info.bittiming = &bt;
    L_DEBUG("Setting bitrate to (%u)\n", bt.bitrate);
    ret = do_set_nl_link(sockfd, 0, argv[1], &req_info);
  } else if (!strncmp(argv[2], CMD_DOWN, sizeof(CMD_DOWN) - 1)) {
    L_DEBUG("Setting %s down\n", argv[1]);
    ret = do_set_nl_link(sockfd, IF_DOWN, argv[1], NULL);
  } else {
    L_ERROR("Invalid options: %s\n", argv[2]);
  }

  if (sockfd > 0) {
    close(sockfd);
  }
  return ret;
}
