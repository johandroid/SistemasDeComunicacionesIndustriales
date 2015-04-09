# configura eth1 via dhcp
sudo dhclient eth1

# configurem wlan0 per a compartir internet
sudo iptables --table nat --append POSTROUTING --out-interface wlan0 -j MASQUERADE
sudo iptables --append FORWARD --in-interface eth1 -j ACCEPT
echo 1 > /proc/sys/net/ipv4/ip_forward

# configurem l'hora i gw

ssh root@192.168.7.2 'route del default'
ssh root@192.168.7.2 'route add default gw 192.168.7.1'
# ssh root@192.168.7.2 'nameserver 8.8.8.8'
ssh root@192.168.7.2 'ntpdate -b -s -u es.pool.ntp.org'
