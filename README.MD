# CN-Reliable-UDP
Reliable UDP with Selective Repeat implementation,for CS F303 Computer Networks

## Group:  
Keshav Beriwal - 2017B4A71301H  
Aman Badjate - 2017B3A70559H  
Prakhar Suryavansh  - 2017B4A71017H  
Garvit Soni - 2017B3A70458H 
Kshitij Verma - 2017B1A71145H 

# How to compile:
For compiling server, type the command `gcc -pthread -o server server.c`
For compiling client, type the command `gcc -pthread -o client client.c`

# How to run:
Open two terminals.  
- On one terminal, type the command `./server <server_port>` eg `./server 5000`
- On another termincal, type the command `./client <source_filename> <destination_filename> <ip> <client_port> <server_port> ` eg `./client readme.txt new_readme.txt 127.0.0.1 5005 5000 `

These two are the server and client terminals respectively.The client will send the file `<source_filename>` to server. The file will be saved as `<destination_filename>` once the transfer is successful.

# Libraries
The following C libraries are required:
```
stdlib
stdio
sys/types
sys/socket
netinet/in
netdb
strings
unistd
sys/time
```

# Network Simulation

Using the _netem_ tool,we can simulate conditions like packet loss,packet delays and packet corruption.We can insert rules in our Ubuntu terminal to simulate the conditions.

The _lo_ interface refers to _localhost_

**To view all the rules:**
`sudo tc qdisc show dev lo`

**To delete all the rules for an interface:**
`sudo tc qdisc del dev lo root`

**To simulate packet loss :**
`sudo tc qdisc add dev lo root netem loss 0.1%`
This causes 1/10th of a percent (i.e 1 out of 1000) packets to be randomly dropped.The _lo_ can be changed to interface on which you want to test.

**To simulate packet reordering :**
`sudo tc qdisc add dev lo root netem delay 40ms reorder 25%`
This causes 25% of packets to be sent immediately, others will be delayed by 40ms.

**To corrupt packets**
`sudo tc qdisc add dev lo root netem corrupt 0.1%`

**To add fixed delay to all packets**
`sudotc qdisc add dev lo root netem delay 100ms`

# Changes Made in Part II

Depending upon the needs, few changes have been made in the protocol keeping in mind the redundancy and complexity of the algorithm. The changes made reduces the complexity and increases the efficiency of the algorithm, handling all types of network conditions like package delay, package loss and package corruption. The changes made are listed below:

1.Changes in message format: Instead of using the flag bits (SYN, ACK, EAK, RST, TCS and NULL), we have defined an enumerator variable frametype in the header of the frame. The frametype will denote the type of frame. The frametype can take following possible values, EMPTY, ACK, DATA and FT_REQ. The function of SYN is fulfilled by FT_REQ. Less bits reduces the complexity of the program, making it more readable.

2.Reducing timeout value: The timeout value is kept as 10 millisecond. Less timeout would mean that packets are sent more frequently thereby reducing the overall time to transfer complete file.

3.No need of checksum: As the UDP protocol takes care of checksum and package corruption, there is no need to implement it separately. UDP ensures that the packets are not corrupted when sent from client to server or vice versa. 

4.Separate header for acknowledgement: As packets sent from client to server will have 256 bytes of DATA + space taken by the header variables,  we have created separate structure for acknowledgement. The acknowledgement frame will need only two fields, sequence number and the frametype. This way, the server need not send the entire packet (256 bytes of DATA + frame header) to the client as acknowledgement when the client only needs the frame type and acknowledgement number of the package it received. As the NACK denotes negative acknowledgement that is sent by the server in case it didn’t receive a packet, we have decided to not send any NACK to the client. Not sending any acknowledgement for any packet would indirectly mean sending NACK and client will know which packages didn’t reach the server when the timeout occurs and the client will re-transmit only those lost packages.
