#include<thread>
#include<chrono>
#include<Windows.h>
#include"Tetris.h"

Tetris* Tetris::ins = nullptr;
std::mutex Tetris::mutex;

int main()
{
	Tetris* tetris = Tetris::getTetrisInstance();
	tetris->startTetris();
	delete tetris;
	return 0;
}