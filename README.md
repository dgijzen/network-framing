# network-framing
School excersize for networking. Take in an input file (hexadecimal) with a four character address (own address), followed by a constant stream of input.
This stream consists of dest-source-length of message - message- and checksum. 

In short the program: reads the frames, calculates the checksum and outputs the destination 
address source length and message (in alphabetic / numeric).
