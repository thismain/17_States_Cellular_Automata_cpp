# 17_States_Cellular_Automata_cpp

For exploring 17 state cellular automata. Allows for panning and zooming and saving of clear, low filesize images, in png format. Each of 17 states has a unique color, and a state is defined as having a different ruleset for whether it persists to the next time step, or becomes dead, or black. And a ruleset dictates the 17 ranges of 8 neighboring cells that will cause the cell to die, and the 17 ranges which will induce a dead cell to be born into the given state. The main function of this program is to explore the effects produced by different, randomly selected rulesets.

For downloading and installing the rquired libraries SDL2, SDL_Image, and SDL_TTF,  on ubuntu:
sudo apt-get install: libsdl2-dev, libsdl2-image-dev, libsdl2-ttf-dev

To compile the program and run it:
g++ states2.cpp -w -lSDL2 -lSDL2_image -lSDL2_ttf -o states2 && ./states2
