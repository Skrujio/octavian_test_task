for windows execute:
	external/SDL_ttf-main/external/Get-GitModules.ps1

for unix execute:
	external/SDL_ttf-main/external/download.sh

then execute:
	cmake -S . -B build
	cmake --build build