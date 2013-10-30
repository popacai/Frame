(sleep 0.5; for i in `seq 1 10`; do echo "msg 0 0 Packet: $i"; sleep 0.1; done; sleep 5; echo "exit") | ./tritontalk -r 1 -s 1 
