LIB_FILES="src/Texture.cpp src/ShaderProgram.cpp src/Game.cpp"
LIB_OPTIONS="-lSD2 -lGLEW -lGL -ldl -lfPIC -shared -o game.so -g"
GAME_OPTIONS="-lSD2 -lGLEW -lGL -ldl -o game -g"
GAME_FILES="src/main.cpp"
LIB_INCLUDE="-I/home/epic/Development/libs"

g++ $LIB_FILES -std=c++11 $LIB_INCLUDE -lSDL2 -lGLEW -lGL -ldl -fPIC -shared -o game.so -g

g++ $GAME_FILES -std=c++11 $LIB_INCLUDE -lSDL2 -lGLEW -lGL -ldl -o game -g
