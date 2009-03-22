Justin White

I wrote this program for CS-436, Computer Graphics, at Bridgewater State College.
I've heard the professor used it at lease the next few semesters as an example.
Cool!

I've now been porting it to use SDL and SDL_ttf instead of GLUT.
This means we have 2.5 dependencies:
	SDL: headers, libs, DLL
	SDL_ttf: headers, libs, DLL
	Freetype: DLL

I use Code::Blocks and set global compiler and linker options to search
for headers and libs for SDL and SDL_ttf.

I built my own SDL_ttf lib and DLL with a CodeBlocks project which I should
submit to the SDL_ttf devs. Maybe this will help them start making MinGW
binary releases, or at least enable CodeBlocks users to build their own SDL_ttf.
The DLLs I put with the build target (.exe).

I include SDL_main_win32.c and #define NO_STDIO_REDIRECT so SDL doesn't make
the stdout.txt and stderr.txt files. Makes it easier to do simple printf-type
debugging when you can see output on the console.

OK, Bye.

