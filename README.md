Alice Pascal is a syntax directed programming editor. It is a teaching and
learning tool. Alice features smart auto completion, auto indenting, interpreted
mode where you can type statements and run them immediately, and it also has a
debugger.

Alice Pascal was written for MS-DOS and Atari ST computers. The goal of this
project is to make a port to Haiku, a more modern operating system. Here you
will find the MS-DOS sources ported to run on UNIX systems (anything reasonably
POSIX compatible should do, including Linux), as well as the original Atari ST
port sourcecode. Our plan is to use this code, which has a graphical user
interface support, to build a portable version of Alice Personal Pascal.

The UNIX sources comes from Brad Templetons homepage:
http://www.templetons.com/brad/alice.html

The Atari ST sources are not available from that page. They were not completely
cleaned up nad will need some work before they are turned into something useful
(even for Atari users).


This repository also includes the MAD Library found here:
http://indigo.ie/~odonnllb/mad.html
This is meant to help with getting things to run in the beginning. We will
likely drop it and replace it with some more usual UI toolkit once things
are compiling (IUP is a likely candidate, but if you do a GUI with something
else before I do, let me know!).
