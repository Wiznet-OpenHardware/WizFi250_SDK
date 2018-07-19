echo Transmit Carrier : Start

set comport=99

..\src\wl\exe\wl --serial %comport% down
..\src\wl\exe\wl --serial %comport% country ALL
..\src\wl\exe\wl --serial %comport% band b
..\src\wl\exe\wl --serial %comport% mpc 0
..\src\wl\exe\wl --serial %comport% up
..\src\wl\exe\wl --serial %comport% out 
..\src\wl\exe\wl --serial %comport% fqacurcy 6
