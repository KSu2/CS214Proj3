# CS214Proj3

Group Members: Kevin Su (ks1507), Ghautham Sambabu (gs878)

## Test Plan
  
  For our testing play we intent on testing the following properties of our program
  
  1. Our program should detect invalid messages and respond with the appropriate message with the INVL header
 
   - if the message has the correct number of fields but not enough bytes it will send "INVL|17|NOT ENOUGH BYTES|"
   - if the number of fields was incorrect but we have enough bytes it will send "INVL|18|NOT ENOUGH FIELDS|"
   - if the message header not valid it will send "INVL|23|MESSAGE HEADER INVALID|"
   - if the total message length is less than 9 bytes (the minimum size a message can be) it will send "INVL|21|MESSAGE TOO SHORT|"
   - if the message length field is greater than 255 it will send "INVL|17|MESSAGE TOO LONG|"
    
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
             -> Have player 2 reject the draw. Afterwards, the game should continue with player1 having the next move
             
  test12.) Send client message suggesting a draw from player 2
             -> Have player 1 accept the draw. Afterwards, the game should end with the end status being a Draw.
  
  
  ### Property 5
  
  test13.) Connect to server with two clients three times. This should result in the server thread printing the message "NEW GAME STARTED" times.
  
  ### Property 6
  
  test14.) Connect to server with message "PLAY|2|A|" from client. After this connect to server again with message "PLAY|2|A|" from client. The server should send message "INVL|20|NAME ALREADY IN USE|".
  
  For testing purposes we will write all messages that should be sent to clients to the clients to STDOUT_FILENO instead. We will also use additional print statements to debug, such as printing that status of the board and the message that was received from the sock.
  
## How to run code

  Build necessary executables:
  ```
  make ttts
  make ttt
  ```
  ttts - Server and game
  ttt - client which will take input from STDIN

  Start server:
  ```
  ./ttts <port-number>
  ```

  Start client: 
  ```
  ./ttt <domain-name> <port-number>
  ```

  Test_client will read line by line from the corresponding test <file-name> and send the messages to the server. After each message it will wait for a response from the server before sending the next message.

  1.) test1.txt test winning case
  2.) test2.txt: test PLAY message with name that is too long

## Proof of code passing properties
  
  1. After running test case 1 I observed the perform the corresponding moves requested appropriately eventually ending in a "OVER|5|X Wins|" message which was the expected result, thus passing this test
  
  2. After running test case 2, I observed the server sent a "INVL|14|NAME TOO LONG|" message, which was expected, thus passing this test
  
  3. After running test case 3, I observed the server sent a "INVL|21|MESSAGE TOO SHORT|" message, which was expected, thus passing this test
  
  4. After running test case 4, I observed the server sent a "INVL|18|NOT ENOUGH FIELDS|" message, which was expected, thus passing this test
  
  5. After running test case 5, I observed the server sent a "INVL|23|MESSAGE HEADER INVALID|" message, which was expected, thus passing this test
  
  6. After running test case 6, The game ran smoothly, resulting in a X win and a message from the server stating that "X wins" therefore, it passed this test
  
  7. After running test case 7, The game ran smoothly, resulting in an O win and a message from the server stating that "O wins" therefore, it passed this test
  
  8. After running test case 8, The game ran smoothly, resulting in a draw and a message from the server stating that "Draw" therefore, it passed this test
  
  9. After running test case 9, The game ended with a resignation from player 1 as the server sent out a message to player 1 "OVER|20|L|Player 1 has resigned|", thus it passed this test
  
  10. After running test case 10, The game ended with a resignation from player 2 as the server sent out a message to player 1 "OVER|27|W|Player 2 has resigned|", thus it passed this test 
  
  11. After running test case 11, I observed a message from the server sent to player 2 "DRAW|2|S" and once player 2 rejected the draw, it sent out a message to Player 1 "DRAW|2|S", thus continuing the game, passing this test
  
  12. After running test case 12, I observed a message from the server sent to player 1 "DRAW|2|S" and once player 1 accepted the draw, it sent out a message to both players "OVER|2|D", thus ending the game, passing this test 
  
  13. After running test case 13, I observed the server wrote in the terminal that "NEW GAME STARTED" three times, thus passing this case
  
  14. After running test case 14, I observed the server wrote to the client, that try to sign up as "A" although it was already in use, a message which was "INVL|20|NAME ALREADY IN USE|", thus passing this test

