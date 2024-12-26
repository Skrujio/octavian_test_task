for windows execute:
	external/SDL_ttf/external/Get-GitModules.ps1

for unix execute:
	external/SDL_ttf/external/download.sh

then execute:
	cmake -S . -B build
	cmake --build build
