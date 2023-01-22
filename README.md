=============================================

KrampusHack 2022 Hellscape

=============================================

Task: For Mokkan

Mokkan Sunday, December 11, 2022 at 9:46 PM
My wishlist:

1. I like building, management, and systems that interact with each other.
2. Space, robots, aliens!
3. A game where violence isn't front-and-center.
4. Red is my favorite color.
5. Music!

You don't have to use all of these ideas :)

=============================================

How I tried to achieve the rules:

1. You're a robot that is interacting with it's environment to save his life
2. After being attacked by an Aliens, a spaceship containing robots have crashed into a hellish kind of planet. You're
   the only surviving one 
3. You have to help your robot escape from the planet, avoiding various corrosives and hot eruptions, and reach the
   safety module that have correctly landed somewhere. You have to search for it, explore the map
3. Red
4. Music :-)

=============================================

What I achieved:

A nice fluid simulation, it's red and it has musics !
I didn't had enough time to use it in a real game, and it's a bit slow too. 
Being ill for a few days ruined my time.

Special thanks to Matthias Müller, Physics research head at nvidia for his papers here:
https://matthias-research.github.io/pages/tenMinutePhysics/

=============================================

Keys:

* Mouse to move the block inside the smoke
* Key F1-F2 => showSmoke or not
* Key F3-F4 => showPressure or not (can be used in conjonction of showSmoke)
* Key F5-F6 => showPaint psychedelic coloring
* Edit the json and restart the demo to apply more tweaks

=============================================

Options: 

* Configurable log level, example: ./Hellscape.exe -V DEBUG 
* Editable fluid_state.json

==============================================

Build: 

* Need gcc and allegro5 installed 
* Makefile works on Linux, Windows
* get into Makefile directory and type make

git clone git@github.com:gullradriel/HellScape.git

cd HellScape

make

==============================================

Variables in json:

{
	"numIters":	12,       # precision of the interpolation, the lower the worst. Best range from 8 to 80

	"density":	10000.0,  # density of the fluid. not really what I was expecting

	"dt":	0.5 ,         # fixed delta time. range from 0.1 to 0.5

	"gravity":	0.0,      # unused in the demo, but it's what it's saying. Gravity.

	"overRelaxation":	1.9,            # fluid behavior, the lower the calmer. range from 1.0 to 1.9

	"fluid_production_percentage": 0.3, # percentage of the left screen used to generate fluid

	"cScale": 12.0        # scale of the fluid ( WIDTH/cScale,HEIGHT/cScale) range from 4 to 24, by steps of 2   

}
