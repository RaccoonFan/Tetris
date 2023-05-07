#pragma once
#include<string>

class Tetromino
{
	std::string m_tetromino;
	int getBlockIndex(int px, int py, int nRotation);
	int getRotation();
	void setBlocks(int type);
public:
	Tetromino(int type);
	char getBlock(int px, int py);
	char getBlock(int px, int py, int nRotation);
	void changeType(int type);
	int m_rotation;
	int m_type;
};