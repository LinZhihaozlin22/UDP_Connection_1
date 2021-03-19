# UDP_Connection_1
Client using customized protocol on top of UDP protocol for sending information to the server.
(One client connects to one server.)


Assumption: 
1. size of payloads of all packets will be 255 bytes. 
2. If client receive a reject packet, it will print the rejected error and then keep sending the rest packets to server. (ack_timer for that packet will also stop after receiving reject response.)


---- Compile ---

 1. Open terminal, and open Assignment_1 directory using: "cd Assignment_1" 
    (may vary depend on the location of your files)
 2. then compile server file, enter: "gcc server.c -o server"
 3. then compile client file, enter: "gcc client.c -o client"

To use other port number: open "server.c" and "client.c" files and change the value of 'PORT' located at line 29 in "server.c" and line 31 in "client.c". Recompile if needed.


------ Run ----

 1. After compiling, run server using "./server". 
   (if success, a message "Listening for client messages..." will show)

 2. then open a new bash and enter the same directory. after that, run client using "./client"


  Initial test case of client is as following:
    -First 5 packets are all correct

    -Then it will send another 5 packets emulating one correct packet and four error packets
      Listed by sending order:
  	1. packet 0 - simulate Case-3 error (missing end of packet identifier)
  	2. packet 1 - simulate Case-2 error (length mismatch)
  	3. packet 3 - simulate Case-1 error (out-of-sequence)
  	4. packet 2 - simulate ACK (correct packet)
  	5. packet 2 - simulate Case-4 error (duplicated packet)


To test ack_timer: run client using "./client" without running server (skip step 1 and do step 2)






------------ Additional Test Cases ------------

There are 4 additional test case files simulating 4 different errors
  "case1.c" simulates CASE-1 error
  "case2.c" simulates CASE-2 error
  "case3.c" simulates CASE-3 error
  "case4.c" simulates CASE-4 error

---- Compile ----

  1. Open terminal, and open reject_case directory using: "cd Assignment_1/reject_case" 
     (may vary depend on the location of your files)
  2. then to compile each case file using:
      case-1: "gcc case1.c -o case1"
      case-2: "gcc case2.c -o case2"
      case-3: "gcc case3.c -o case3"
      case-4: "gcc case4.c -o case4"

To use other port number: open "server.c" and the test files and change the value of 'PORT' located at line 29 in "server.c" and line 31 in all test files. Recompile if needed.



---- Run ----

 1. After compiling, run server using the steps shown above

 2. then open a new bash and enter 'reject_case directory' and run different test files using:
      case-1: "./case1"
      case-2: "./case2"
      case-3: "./case3"
      case-4: "./case4"
