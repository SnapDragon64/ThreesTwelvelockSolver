This is an ugly but workable program to help people get the Twelvelock
achievement in Threes.  (It also works for Sixlock.)  Its success rate is
only about 4%, but even in principle I doubt much more than 10% is
possible.  The problem is that all you can do is set up seven of the
"12"s, waste a few moves, and hope you get lucky with a "+" card spawn
for the last "12".

Hopefully most of the program is self-explanatory.  You just type in
whatever info it needs to see the game state, and it tells you what moves
to make.  You can Undo to the previous board state if you accidentally
tell it something wrong.  However, if you make a mistake and swipe the
wrong Threes direction, you'll have to restart the program.  Sorry!

Starting the program from the beginning of the game guarantees its deck
counts will be accurate.  The other way to guarantee the deck's state is
correct is to start the program after making 12N+2 moves, NOT counting "+"
card spawns.  If the deck is inaccurate, it's not that big a deal, but it
will probably hurt the program's predictions slightly.

The program tells you how happy it is with the current game state.  It's
not the most meaningful number in the world, but 50% is average, 30% or
under means the current state probably can't be recovered, and 60-70%
means you're heading towards a configuration that lets you wait for a
lucky "+" spawn to win.

Note that the algorithm I use will happily try to set up a checkboard of
"12"s before ever creating a "96" card, which is kind of pointless (since
a "96" is required before "+"s can be "12"s).  I could probably have made
the heuristics smarter, working in two phases, but oh well.  So if you
play using the program from the start, you'll probably waste some time
making a "12" checkerboard before a "96" is made.  But you'll know you
have a 4% chance of victory.

My recommended, more efficient, approach to grind for the achievement is
to start a Threes game, then play 12N+2 moves (not counting "+" spawns)
yourself until you've created a "96" (or the board starts to slip out of
control).  At that point, start the program and let it try to set up the
"12" checkerboard.  If it fails and the pretty "12" pattern starts to
disintegrate, you should probably just give up and start again.

If you're paying attention, you'll probably notice moves at which you're
one "+" spawn away from winning.  A "+" spawns with a 1/21 chance, and (as
long as "96" is the highest card on the board) has a 50-50 shot of being a
"12".  So unless you're lucky, you'll see these moves a lot.  Good luck! :)
