
.chapter A History of Alice Development

The following history of Alice will help you understand the various ifdefs
and directories better.

ALICE was begun in 1994.  It was developed on a very simple Unix (A System
III variant on the Onyx computer) but the main target platforms were a
special education computer called the ICON and the IBM PC.   And I do
mean the IBM PC, often with only 256 kilobytes of ram.

The ICON was an 80186 box running a variant of the {link
http://www.qnx.com}{ltext QNX} operating system.  QNX still exists.  It's
a real-time OS with many similarities to Unix.  The ICON had a graphical
screen and a trackball.   At the time, IBM PC computers did not typically
have a mouse.

The ICON was used at Ontario high schools, and they were the first customer
for ALICE.   The trackball was used by having it simply emulate a keypad,
streaming out arrow keys as the ball moved.

The version for the ICON, and for the initial PC, all had to play a lot of
memory games.  Some of this is still found in the code.  A program could
have at one time only a 64K data segment, though another 64K could be
used for stack.  However, games allowed the use of multiple segments,
and one version of ALICE used two 64K segments.   This was called the
"hybrid" model.  Fortunately all this is long in the past.

On the IBM PC, it was also possible to have a "large" model program that
used 32 bit pointers.  One still had only 640K of ram, but it was more than
the 128K the regular version could access.   Integers in all versions of
Alice at that time were only 16 bits.    On the PC, two versions were
released.  One could use only 128K of ram -- 64K for program and certain
tyeps of data, and 64K for the user's data and other information.  This
version was small and tight and as far as users were concerned used 16
bit pointers and 16 bit ints.  It could run on a PC with only 256K of ram,
which at the time was not that uncommon in high schools.

The other used the large model.  It could handle larger programs but itself
was bigger and a small bit slower.  Of course, today's processors are so
fast compared to those machines that both versions are blindingly fast today.

A version of ALICE was also made for QNX on an PC, though it was never
marketed.   The version for unix was only used for development, it was
never provided to users.

A version of ALICE was also made for a Siemens 8086 based MS-DOS (but not
PC-DOS) PC.  That machine is long dead.

Since ports to the ICON, QNX and the Siemens computer will never happen
again, I have been sometimes removing deadwood code in this area.  ALICE
is full of #IFDEFs from all the ports, and it makes the code too hairy.

.topic Turbo

Turbo Pascal from Borland won the pascal wars for serious users, though ALICE
was still very good for education and debugging.  Most of the extensions to
Pascal from Turbo Pascal were added to the large model version of ALICE.

.topic Final PC Version

The last PC release, 1.3.1 (large) still runs fine today under DOS, and in
a DOS window under Windows.  It handles colour and even can handle large
screens (ie. 50 lines) with the right settings.  It even does CGA graphics
by switching screens (ie. no windows.)

.topic Atari ST

In 1986 ALICE was ported to the Atari ST.  This version was a full GUI port
of the program, using the GEM operating environment.  The Atari ST port
was way ahead of its time, and allowed absolute beginners to write GUI
windowing system programs that were just a few lines rather than the
typical multi-page "hello world" programs you see in C or C++.   While
some of the changes for the GUI version were incorporated in the master
source tree, in many cases whole new versions of some components were written.

Unfortunately, that source is not currently found on my unix disks, and only
exists on an Atari ST hard drive I have (and which I hope still works) as
well as perhaps some old tapes and Atari ST floppies.  If somebody is really
keen on porting Alice to Windows or Mac or X, this Atari ST source would be
the place to start, so somebody would have to come down to Sunnyvale and see
if we could extract it.

On the plus side the Atari had 32 bit integers and pointers, paving the way
to the code working on modern machines.

.topic ALICE Basic

An ALICE Basic was also written for the ICON.  It was never ported to the PC.
The source for it is also lost, probably out on some old backup tapes. (QIC
cartridges)  ALICE Basic was a great basic, but frankly there's not a lot
of point in ressurecting it today, there are lots of fancy Basics out there.

In fact, I've been told that the Microsoft people who designed Visual Basic
"borrowed" many of the best features from Alice Pascal.  They certainly were in
ALICE long before Visual Basic came on the scene.

.topic APIN and APRUN

ALICE had some companion programs.  APIN is a parser that takes plain text
Pascal files and converts them to an ALICE ".ap" file.   This allows importing
other Pascal programs.  Just about any standard Pascal will import, as will
many programs from Turbo Pascal in the mid-80s.  (I have no idea about
more recent Turbo features.)

I've even modified ALICE so that if you give it a ".pas" on the command line
it will run apin (if it's in the $PATH) on it first.   ALICE can save
programs as text or in its own internal (syntax tree) format.

APRUN was a stand alone interpreter.  This could take a saved program and
run it without the editor.  It was meant to allow people to distribute
ALICE-only programs.   Today there are readily available free Pascal
compilers, so the need to run an ALICE program outside the editor seems
moot.    All efforts to support APRUN are abandoned.

.topic Help System

The Help system on Unix worked from a tree of help files, one file per help
screne.  On the PC, all files were compressed into one big helpfile, with
special huffman encoding.   There was a fancy system to build the file and
make it very small.  It helped ALICE fit on two 360K floppies.  (Yup, 360K
ones!)  And a simple version fit on a single floppy.   However, for now it
seems the tree of files is easier to work with an maintain, if bulkier and
slower.  Slow in 1985 is fast today.

.topic Demo version

When ALICE was a commercial product, it had a demo mode that allowed you
to take users on a tour, on keystroke at a time, with commentary.  That
still might be useful but I've decided it's not that useful and code is
slowly being pulled.

.topic Programs

A variety of Pascal programs came with ALICE.  Some were samples. Some
Only work on IBM PCs or ICONs or other computers because of graphics or
special OS calls.  So don't expect them all to work.


.topic Modern ALICE for Linux

I pulled out the source and compiled it for Linux and was amazed that it
actually worked with just a few tweaks.  So I've been cleaning it up a bit
for release to see if others want to play with it.   It's amazing how much
has changed in compilers and C libraries since the simple compilers that
16 bit integer ALICE was compiled for.  But we wrote it very portably, and
that helped.

One thing I had to change was the curses stuff, which has evolved quite a
bit.  We wrote our own curses for the IBM PC, and even sold it, to port
ALICE there.  Ditto the other platforms.   Modern curses uses 16 bit
input keys and 32 bit screen characters with attributes.  A few odd
window refresh bugs are still present.  ALICE had various strange items to
deal with curses bugs and they are probably causing more bugs.

So far it's mostly working using 32 bit ints instead of 16, which is
good to see.  The ST port helped there.

There's still DOS legacy code, including various "CProc" code to call C
functions and OS calls that makes no sense on Unix, but a lot of DOS demo
programs needed them.    In addition, many of the Turbo Pascal compatability
was done by using a Turbo Pascal library, turbolib.ap, to save space.
This would ideally be integrated today.

The old systems didn't complain if you wrote on a constant string, but
modern systems do.  A few bugs still exist due to this.

The old save files used 16 bit words and they still do.  Integer constants
are saved as strings so there may be no need to upgrade this, though the
code in theory can handle it.

The macro file (ap.init) needs updating to modern keyboards and modern
KEY_ definitions in curses.

The other thing that is highly needed is mouse support.  Curses has mouse
Xterm support, and that could be put in to both the many keyboard loop, and
of course to the pop-up menus and help screens, plus some functions for
the user.

.topic Using ALICE today

ALICE still has merit as a programming education tool, and a quick
prototype and debug environment if Pascal is your passion.   It can be
used over terminals or in an xterm, and the PC version still works on
PCs just fine.   The debugger remains up to speed in features in many ways
even though it is 15 years later -- ALICE had an IDE that was way ahead
of its time.

.subsect Syntax Directed Editors

The other issue is whether Syntax Directed Editors are a good thing or
not.  It's hard to say.  I think they are good for teaching.  If I were
doing it today, I would do it differently.  Back then ALICE had to be small
and fast on a slow machine.

Today I would make it work more like a typical text editor, expanding
productions when needed, but allowing random editing with re-parsing on
the fly.  If I were doing it at all, which I am not.
