simulesonrun : simuleson inputsimulesonseul inputsimuleson
	./simuleson inputsimuleson

simulesondebug : simuleson.cpp input
	g++ -std=c++14 -g -Wall -Wextra simuleson.cpp -o simulesondebug -lsndfile
	gdb simulesondebug

simuleson : simuleson.cpp Makefile
	g++ -std=c++14 -Os -march=native -ffast-math -Wall -Wextra simuleson.cpp -o simuleson -lsndfile

affichespectrerun : affichespectre inputaffichespectre
	feh `./affichespectre inputaffichespectre`

affichespectredebug : affichespectre.cpp input
	g++ -std=c++14 -g -Wall -Wextra affichespectre.cpp -o affichespectredebug
	gdb affichespectredebug

affichespectre : affichespectre.cpp Makefile
	g++ -std=c++14 -Os -march=native -ffast-math -Wall -Wextra affichespectre.cpp -o affichespectre

