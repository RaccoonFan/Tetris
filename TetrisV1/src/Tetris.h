#pragma once
#include <iostream>
#include<thread>
#include<chrono>
#include<Windows.h>
#include<vector>
#include<mutex>
#include"Tetromino.h"

class Tetris
{
	static Tetris* ins;
	const int m_screenWidth = 80;
	const int m_screenHeight = 30;
	const int m_fieldWidth = 12;
	const int m_fieldHeight = 18;
	unsigned char *pField = nullptr;
	bool m_key[5];

	Tetris();
	Tetris(const Tetris& other);
	Tetris& operator=(const Tetris& other);
	void keyUpdate();
	bool DoesPieceFit(Tetromino* teromino, int nRottion, int nPosX, int nPosY);

public:
	static std::mutex mutex;
	static Tetris* getTetrisInstance();
	void startTetris();
	~Tetris();
};