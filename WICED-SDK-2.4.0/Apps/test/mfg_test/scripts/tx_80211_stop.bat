echo Transmit 802.11 : Stop

set comport=99

..\src\wl\exe\wl --serial %comport% pkteng_stop tx
 