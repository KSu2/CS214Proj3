# CS214Proj3

## Test Plan

## How to run code

Build necessary executables:
```
make ttts
make test_client
```

Start server:
```
./ttt <host-name> <port-number>
```

Start Test_client: 
```
./test_client <host-name> <port-number> <file-name>
```

Test_client will read line by line from the corresponding test <file-name> and send the messages to the server. After each message it will wait for a response from the server before sending the next message.

test1.txt: test winning case
test2.txt: test PLAY message with name that is too long

## Proof of code working properly
