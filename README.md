# Morse
A simple program to train/practice morse code using a modified version of the Koch method.
The program picks a random character from the set that you've learned so far and plays the morse code for it out loud. Then, you type in the character. The main difference from the Koch method is that this program doesn't move on to the next character until you've completed the current one.

Note that the current version is fixed at 20 wpm (1 dit is 0.06 seconds long).
# Building
Compile with
`cc morse.c -o morse -lcurses -lm`
