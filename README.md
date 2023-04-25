# CS214Proj3

## Test Plan
  
  For our testing play we intent on testing the following properties of our program
  
  1. Our program should detect invalid messages and respond with the appropriate message with the INVL header
    -if the message has the correct number of fields but not enough bytes it will send "INVL|17|NOT ENOUGH BYTES|"
    -if the number of fields was incorrect but we have enough bytes it will send "INVL|18|NOT ENOUGH FIELDS|"
    -if the message header not valid it will send "INVL|23|MESSAGE HEADER INVALID|"
    -if the total message length is less than 9 bytes (the minimum size a message can be) it will send "INVL|21|MESSAGE TOO SHORT|"
    -if the message length field is greater than 255 it will send "INVL|17|MESSAGE TOO LONG|"
  2. Our program should detect win, lose, and draw gamestates and end the games appropriately
  3. Our program should handle cases when either player sends a RSGN message appropriately
  4. Our program should be able to handle DRAW requests appropriately
  5. Our program should be able to support concurrent games
  6. Our program should maintain a list of player names and only allow players to join with unique names that aren't on the list

  In order to verify that each of these properties is met by our program we will run each of the following test cases: 
  
  ### Property 1
  
  test1.) Send client that result in player X winning 
  
  test2.) Send client "PLAY" message with a name that is too long 
  
  test3.) Send a client message with the appropriate number of "|" but with a number of bits different than the number specified in the second message field
  
  test4.) Send a client messge with too few "|" for the type of message being sent
  
  test5.) Send a client message with too many "|" for the type of message being sent
  
  ### Property 2
  
  test6.) Make valid moves from both players that ends in "X" winning 
  
  test7.) Make valid moves from both players that ends in "O" winning
  
  test8.) Make valid moves from both players that ends in a draw
 
  ### Property 3 
  
  test9.) Send client message from player 1 resigning 
  
  test10.) Send client message from player 2 resigning
  
  ### Property 4
  
  test11.) Send client message suggesting a draw from player 1
             -> Have player 2 reject the draw
             
  test12.) Send client message suggesting a draw from player 2
             -> Have player 1 accept the draw
  
  
  ### Property 5
  
  test13.)
  
  ### Property 6
  
  test14.)
  
  
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

  1.) test1.txt test winning case
  2.) test2.txt: test PLAY message with name that is too long

## Proof of code working properly
  1.) After running test case 1 I observed the perform the corresponding moves requested appropriately eventually ending in a "OVER|5|X Wins|" message which was the expected result
  2.) After running test case 2 I observed the server sent a "INVL|14|NAME TOO LONG|" message which was expected

