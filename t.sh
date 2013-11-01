#(sleep 0.5; for i in `seq 1 100`; do echo "msg 0 1 Packet: $i"; sleep 0.1; echo "msg 0 0 Packet: $i"; done; sleep 600; echo "exit") | ./tritontalk -r 2 -s 2 
#(sleep 0.5; for i in `seq 1 10000`; do echo "msg 0 1 Packet: $i"; done; sleep 600; echo "exit") | ./tritontalk -r 2 -s 2 -c 0.2 -d 0.2
(sleep 0.5; for i in `seq 1 300`; do echo "msg 0 0 Packet: $i"; sleep 0.05;done; sleep 600; echo "exit") | ./tritontalk -r 1 -s 1 -c 0.9 > output > output_c
