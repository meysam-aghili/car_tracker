# car_tracker

[home mosquitto broker server]
create mosquitto docker container in wsl.
map windows ip/port to wsl ip/port using following command in powershell : netsh interface portproxy add v4tov4 listenport=1883 listenaddress=0.0.0.0 connectport=1883 connectaddress=172.20.150.76
add firewall rule for inbound port 1883
port forewarding on home router in port 1883
if the isp uses nat for your public ip(if public ip in router differs from actual public ip) you must buy static ip from isp.
register in no-ip ddns and set the settings in router.


[broker metrics dashboard]
setup mosquitto-exporter to connect to the broker and publish data on port 9234
in prometheus.yml set the job for store data into prometheus
connect grafana to prometheus and create dashboard