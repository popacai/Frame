(sleep 0.5; for i in `seq 1 280`; do echo "msg 0 1 Packet: $i"; sleep 0.1; echo "msg 0 0 Packet: $i"; done; sleep 5; echo "exit") | ./tritontalk -r 2 -s 2
#(sleep 0.5; for i in `seq 1 250`; do echo "msg 0 0 Packet: $i"; done; sleep 5; echo "exit") | ./tritontalk -r 1 -s 1 -c 0.9
