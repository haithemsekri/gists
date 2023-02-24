

def get_iface_addrs(mac_addr):
    """
    Return a list of IPv4 addresses for a given MAC address.
    If there are no IP addresses assigned to the MAC address, it will return
    an empty list.
    Arguments:
    mac_addr -- A MAC address as a string, input format: "aa:bb:cc:dd:ee:ff"
    """
    with IPRoute() as ip2:
        links = ip2.link_lookup(address=mac_addr)
        if len(links) > 1:
            raise ValueError(f"found multiple links for MAC {mac_addr}")
        link_index = links[0] if links else None
        # Only get v4 addresses
        addresses = [addr.get_attrs('IFA_ADDRESS')
                     for addr in ip2.get_addr(family=socket.AF_INET)
                     if addr.get('index', None) == link_index]
        # flatten possibly nested list
        return list(itertools.chain.from_iterable(addresses))
		
def get_iface_info(ifname):
    """
    Given an interface name (e.g. 'eth1'), return a dictionary with the
    following keys:
    - ip_addr: Main IPv4 address, if set, or an empty string otherwise.
    - ip_addrs: List of valid IPv4 addresses. Can be an empty list.
    - mac_addr: MAC address
    All values are stored as strings.
    """
    def is_bridge(link_info_info):
        " Returns True if link_info_ is a bridge "
        return (link_info_info is not None) and \
                (link_info_info.get_attr('IFLA_INFO_KIND') == 'bridge')
    try:
        with IPRoute() as ipr:
            links = ipr.link_lookup(ifname=ifname)
            if len(links) == 0:
                raise LookupError("No interfaces known with name `{}'!"
                                  .format(ifname))
            link_info = ipr.get_links(links)[0]
            link_speed = get_link_speed(ifname)
    except IndexError:
        raise LookupError("Could not get links for interface `{}'"
                          .format(ifname))
    mac_addr = link_info.get_attr('IFLA_ADDRESS')
    ip_addrs = get_iface_addrs(mac_addr)
    return {
        'mac_addr': mac_addr,
        'ip_addr': ip_addrs[0] if ip_addrs else '',
        'ip_addrs': ip_addrs,
        'link_speed': link_speed,
        'bridge': is_bridge(link_info.get_attr('IFLA_LINKINFO')),
        'mtu': link_info.get_attr('IFLA_MTU'),
		

from pyroute2 import IPRoute
ip = IPRoute()
index = ip.link_lookup(ifname='vlan.5')[0]
ip.addr('add', index, address='192.168.10.4', mask=24)
ip.addr('del', index, address='192.168.10.4', mask=24)
ip.close()

from pyroute2 import IPRoute
ip = IPRoute()
ip.get_addr(label='vlan.5')[0].get_attr('IFA_ADDRESS')
ip.get_addr(label='vlan.5')[1].get_attr('IFA_ADDRESS')

from pyroute2 import IPRoute
import socket
import fcntl
import struct
import sy

ifname="vlan.5"
socket.inet_ntoa(fcntl.ioctl(socket.socket(socket.AF_INET, socket.SOCK_DGRAM), 35099, struct.pack('256s', ifname))[20:24])

from pyroute2 import IPRoute
ip = IPRoute()
info = [{'iface': x['index'], 'iface': x['index'], 'addr': x.get_attr('IFA_ADDRESS'), 'mask':  x['prefixlen']} for x in ip.get_addr()]

from pyroute2 import IPRoute
ip = IPRoute()
print([x.get_attr('IFLA_IFNAME') for x in ip.get_links()])

from pyroute2 import IPRoute
import json
import time

ifname=""
n_ip="192.168.5.60"
n_mask=24

with IPRoute() as ipr:
    links = ipr.link_lookup(ifname=ifname)
    if not links:
        raise ValueError(f"interface doesn't exists {ifname}")
    link_index = links[0] if links else None
    # Only get v4 addresses
    ip_addrs = [{'addr': addr.get_attr('IFA_ADDRESS'), 'mask':  addr['prefixlen']}
                    for addr in ipr.get_addr(family=socket.AF_INET)
                    if addr.get('index', None) == link_index]
    for ip_addr in ip_addrs:
        print(f"del {ip_addr}")
        ele = json.loads(json.dumps(ip_addr))
        ipr.addr('del', link_index, address=ele['addr'], mask=int(ele['mask']))
    time.sleep(5)
    print(f"add {n_ip}")
    ip.addr('add', link_index, address=n_ip, mask=n_mask)
    time.sleep(5)
    print(f"del {n_ip}")
    ip.addr('del', link_index, address=n_ip, mask=n_mask)
    for ip_addr in ip_addrs:
        print(f"del {ip_addr}")
        ele = json.loads(json.dumps(ip_addr))
        ipr.addr('add', link_index, address=ele['addr'], mask=int(ele['mask']))

json.loads(json.dumps(jsonStr))
ip_addr='{"addr": "192.168.5.3", "mask": 24}'
ele = json.loads(ip_addr)
address=ele['addr']
mask=int(ele['mask'])

info = [{'iface': x['index'], 'ifname': x.get_attr('IFLA_IFNAME'), 'addr': x.get_attr('IFA_ADDRESS'), 'mask':  x['prefixlen']} for x in ip.get_addr(family=socket.AF_INET)]

ipr=IPRoute()
link_index = ipr.link_lookup(ifname="vlan.5")[0]
info = [{'iface': x['index'], 'ifname': x.get_attr('IFLA_IFNAME'), 'addr': x.get_attr('IFA_ADDRESS'), 'mask':  x['prefixlen']} 
    for x in ip.get_addr(family=socket.AF_INET)
	if x.get('index', None) == link_index]


	

IFLA_EXT_MASK 

        return list(itertools.chain.from_iterable(addresses))
		
		
    ipr = IPRoute()
    if_link = ipr.link_lookup(ifname=interface)
    LOGGER.debug(f"ip_link down {interface}")
    ipr.link("set", index=if_link, state="down")
    yield
    LOGGER.debug(f"ip_link up {interface}")
    ipr.link("set", index=if_link, state="up")
