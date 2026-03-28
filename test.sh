echo "cmd=LIST_INTERFACES" | nc -U /var/run/netagent.sock
echo "cmd=STATUS" | nc -U /var/run/netagent.sock
echo "cmd=CONNECT iface=wlan0 ssid=Christian24_2.4G psk=Kanokchon99" | nc -U /var/run/netagent.sock
#echo "cmd=CONNECT iface=em0" | nc -U /var/run/netagent.sock

# timeout test (wait 5 second and get error)
# nc -U /var/run/netagent.sock
