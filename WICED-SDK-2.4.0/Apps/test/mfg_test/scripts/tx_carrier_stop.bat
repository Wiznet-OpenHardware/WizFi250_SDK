echo Transmit Carrier : Stop  

set comport=99

..\src\wl\exe\wl --serial %comport% fqacurcy 0  
..\src\wl\exe\wl --serial %comport% down
..\src\wl\exe\wl --serial %comport% up