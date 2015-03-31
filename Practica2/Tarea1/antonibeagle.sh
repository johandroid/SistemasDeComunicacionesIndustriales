sudo dhclient eth1
sudo iptables --table nat --append POSTROUTING --out-interface wlan0 -j MASQUERADE
sudo iptables --append FORWARD --in-interface eth1 -j ACCEPT

ssh root@192.168.7.2 'ntpdate -b -s -u es.pool.ntp.org'
ssh root@192.168.7.2 'route del default'
ssh root@192.168.7.2 'route add default gw 192.168.7.2'
