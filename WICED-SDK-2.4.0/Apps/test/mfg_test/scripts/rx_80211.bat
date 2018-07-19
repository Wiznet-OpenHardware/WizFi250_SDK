echo Receive 802.11  

set comport=99

..\src\wl\exe\wl --serial %comport% down
..\src\wl\exe\wl --serial %comport% mpc 0
..\src\wl\exe\wl --serial %comport% country ALL
..\src\wl\exe\wl --serial %comport% scansuppress 1
..\src\wl\exe\wl --serial %comport% channel 1
..\src\wl\exe\wl --serial %comport% bi 65535
..\src\wl\exe\wl --serial %comport% up
@echo sleep 3 seconds 
@ping -n 3 127.0.0.1 > nul
..\src\wl\exe\wl --serial %comport% counters
