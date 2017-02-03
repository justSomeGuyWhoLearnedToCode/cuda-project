/*
Tue 27 Dec 2016 01:27:53 PM EST 

Christian Aggeles

This assumes an 8-byte (64-bit) bandwidth for the data stream. 

Using a unicode as teh packet library gives a large array of lookup values for indexing 
the hash table that houses teh end results of the user input.
 
Enumeration of 64 bit values (4 unicode characters)


char 1: bits 0 -7


0) on/off - has the pad received a signal during this stream?
1) if no on 0, did it receive a signal in the packet 2 before?  
2) voice library - tonebank, tone (2 characters is 16^2 choices)
3) velocities - size of incident stroke, voltage level, voltage sublevel
4) effects - eq, compression, filter, chorus, etc.
5) sound source - left half: 0, right half: 1
6) change in x position of the transmitter 1 - yes, 0 - no
7) change in y position of the transmitter 1 - yes, 0 - no


Each ascii character has 8 positions per char (8 bits), and when interpreting the
command sent through the stream, the first character holds data regarding the decision to either change some parameter or let it remain static.

The second choice is a sort of hash table that further delineates which data is contained 
within the packet. Because the channel index has already been  

byte 2:

8) if no on 0 & 1, go one previous
9) if no on 0, 1, and 8, go one previous  
10) voice library - each pad has two sonic palettes (analog/digital::0/1)
11) velocity - divides the signal into two halves of the preset thresholds
12) if bit 4 in char1 is 1, is the effect from fx bank 1 or 2 
13) left/right half of half chosen by bit 5 in character 1
14) is the x position within 2 units from where it was previously 1 - yes, 0 - no 
15) is the y position within 2 units from where it was previously 1 - yes, 0 - no 


byte 3:

Further refinement of the previous choices

16) same as 0, 1, 8, & 9
17) "" 
18) within sonic palette, which half of voices to choose (first half: 0, second: 1)
19) again divides the level of refinement via velocity
20) first half of bank of fx: 0, second: 1
21) if more than 4 surfaces, then left half: 0, right: 1
22) if x is mroe than 2 units, is it more than 4
23) if y is more than 2 units, is it more than 4

The refinements in bits 24-63, but the bits in position 6 is used for the velocity and the 7th for the sound bank are used for velocity measurement. If the bit in 0 and 1 remains 0, a signed bit will accompany the signal and act as an indication of 128 cycles of silence. 

Of course, if the signed bit is true, the 
signal look up for the first and second bit will begin at 129 and continue to the past.

The refinements continue until an end value is reached. AFter that, the bit maintains a 0 value.

This technique produces a velocity threshold of 2^10 per signal.
 
The signal is then interpreted in a look-up table housed in the Data struct.


Give that I have designed the stream to be comprised of 8 metrics, the first ascii character will give the index of the subset.

128/8 = 16

For each metric, there are a minimum of 16 choices, and for those metrics having more than 16, the second, third and fourth character of the stream packet gives the compliment of what data may be missing.

1) Rhythms 

	i)
	ii)
	iii)
	iv)
	v)
	vi)
	vii)
	viii)



*/


