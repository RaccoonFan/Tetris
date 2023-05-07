#include"Tetromino.h"

void Tetromino::setBlocks(int type) {
	switch (type)
	{
	case 0: this->m_tetromino = "..X...X...X...X."; break;
	case 1: this->m_tetromino = "..X..XX..X......"; break;
	case 2: this->m_tetromino = "..X..XX...X....."; break;
	case 3: this->m_tetromino = ".....XX..XX....."; break;
	case 4: this->m_tetromino = ".X...X...XX....."; break;
	case 5: this->m_tetromino = "..X...X..XX....."; break;
	case 6: this->m_tetromino = ".X...XX...X....."; break;
	default:
		break;
	}
}
Tetromino::Tetromino(int type) :m_type(type)
{
	this->setBlocks(type);
	this->m_rotation = 0;
}

int Tetromino::getRotation()
{
	while (this->m_rotation < 0)
		this->m_rotation += 4;
	return this->m_rotation;
}
int Tetromino::getBlockIndex(int px, int py, int nRotation)
{
	int pi = 0;
	switch (nRotation % 4)
	{
	case 0:	// 0 degrees
		pi = 4*py + px;
		break;
	case 1:	// 90 degrees
		pi = 12 + py - 4*px;
		break;
	case 2:	// 180 degrees
		pi = 15 - 4*py - px;
		break;
	case 3:	// 270 degrees
		pi = 3 - py + 4*px;
		break;
	default:
		break;
	}
	return pi;
}
char Tetromino::getBlock(int px, int py)
{
	return this->m_tetromino[getBlockIndex(px, py, getRotation())];
}

char Tetromino::getBlock(int px, int py, int nRotation)
{
	while (nRotation < 0)
	{
		nRotation += 4;
	}
	return this->m_tetromino[getBlockIndex(px, py, nRotation)];
}

void Tetromino::changeType(int type)
{
	this->m_type = type;
	this->setBlocks(type);
}