(sleep 1; for i in `seq 1 30`; do echo "msg 0 0 Packet: $i"; sleep 0.1 ;echo "msg 0 1 Packet: $i"; sleep 0.1; done; sleep 5; echo "exit") | ./tritontalk -r 2 -s 2 

