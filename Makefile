all:
	rm -f run
	g++ main.cpp imgui.cpp imgui_draw.cpp imgui_widgets.cpp imgui-SFML.cpp imgui_demo.cpp -DGL_SILENCE_DEPRECATION -std=c++17 -O3 -g -I ~/imgui/ -I ~/imgui-sfml/ -I ~/raknet/RakNet/Source/ -L ~/raknet/RakNet/build/Lib/LibStatic/ -lRakNetLibStatic -lsfml-graphics -lsfml-window -lsfml-system -framework OpenGL -o run
