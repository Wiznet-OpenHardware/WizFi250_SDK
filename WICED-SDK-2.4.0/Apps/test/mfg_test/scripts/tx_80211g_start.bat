echo Transmit 802.11g : Start  

set comport=99

..\src\wl\exe\wl --serial %comport% down
..\src\wl\exe\wl --serial %comport% country ALL
..\src\wl\exe\wl --serial %comport% band b
..\src\wl\exe\wl --serial %comport% chanspec -c 6 -b 2 -w 20 -s 0
..\src\wl\exe\wl --serial %comport% mpc 0
..\src\wl\exe\wl --serial %comport% ampdu 1
..\src\wl\exe\wl --serial %comport% bi 65000
..\src\wl\exe\wl --serial %comport% frameburst 1
..\src\wl\exe\wl --serial %comport% rateset 11b
..\src\wl\exe\wl --serial %comport% up
..\src\wl\exe\wl --serial %comport% txant 0
..\src\wl\exe\wl --serial %comport% antdiv 0
..\src\wl\exe\wl --serial %comport% nrate -r 54
..\src\wl\exe\wl --serial %comport% phy_watchdog 0
..\src\wl\exe\wl --serial %comport% disassoc 
..\src\wl\exe\wl --serial %comport% txpwr1 -1 
@echo sleep 3 seconds 
@ping -n 3 127.0.0.1 > nul
..\src\wl\exe\wl --serial %comport% pkteng_start 00:90:4c:aa:bb:cc tx 40 1000 0

