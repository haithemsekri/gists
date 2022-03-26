--------------------------------------------------------------------- 

#https://en.wikibooks.org/wiki/QEMU/Monitor 

root@pve:~# lsusb -t 

/:  Bus 02.Port 1: Dev 1, Class=root_hub, Driver=xhci_hcd/10p, 10000M 

/:  Bus 01.Port 1: Dev 1, Class=root_hub, Driver=xhci_hcd/16p, 480M 

    |__ Port 6: Dev 4, If 0, Class=Human Interface Device, Driver=usbhid, 1.5M 

    |__ Port 6: Dev 4, If 1, Class=Human Interface Device, Driver=usbhid, 1.5M 

    |__ Port 7: Dev 29, If 0, Class=Mass Storage, Driver=usb-storage, 480M 

    |__ Port 9: Dev 2, If 0, Class=Vendor Specific Class, Driver=rtsx_usb, 480M 

    |__ Port 14: Dev 3, If 0, Class=Wireless, Driver=btusb, 12M 

    |__ Port 14: Dev 3, If 1, Class=Wireless, Driver=btusb, 12M 


qm list 

qm start 107 

qm guest cmd 107 network-get-interfaces | grep -B1 "ipv4" | grep "\"ip-address\" :" | grep -v "127.0.0.1" 

qm monitor 107 

device_add usb-host,hostbus=1,hostport=7,id=sundisk 

device_del sundisk 

pvesh create /nodes/pve/qemu/107/monitor --command "device_add usb-host,hostbus=1,hostport=7,id=sundisk" 

pvesh create /nodes/pve/qemu/107/monitor --command "info usb" 

pvesh create /nodes/pve/qemu/107/monitor --command "device_del sundisk" 

--------------------------------------------------------------------- 

qm shutdown 105 

qm shutdown 106 

qm shutdown 107 

qm shutdown 106 

qm list 

for vmid in 105 106 107 999 

do
  ret="$(qm start $vmid 2>&1)"
  echo ret:$ret 

  [[ -z "$ret" ]] && echo "$vmid" && break
done 

rm -rf /tmp/pve/vms/$vmid 

mkdir /tmp/pve/vms/$vmid 

echo "$vmid" 

sleep 10 

qm guest cmd $vmid network-get-interfaces | grep -B1 "ipv4" | grep "\"ip-address\" :" | grep -v "127.0.0.1" 

cd /sys/bus/usb/devices 

ls */idProduct | while read l; do dev="$(dirname $l)"; PidVid="$(cat $dev/idVendor):$(cat $dev/idProduct)";  [[ "$PidVid" == "0781:5581" ]] && echo $dev:$PidVid; done 
