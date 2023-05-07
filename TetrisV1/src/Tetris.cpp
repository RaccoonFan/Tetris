#include"Tetris.h"
#include"Tetromino.h"
#include<memory>
#include<ctime>

Tetris::Tetris()
{
	srand(time(NULL));
	memset(this->m_key, false, sizeof this->m_key);
	this->pField = new unsigned char[m_fieldWidth * m_fieldHeight];
	for (int x = 0; x < m_fieldWidth; x++)
		for (int y = 0; y < m_fieldHeight; y++)
			pField[y*m_fieldWidth + x] = (x == 0 || x == m_fieldWidth - 1 || y == m_fieldHeight - 1) ? 9 : 0;
}

Tetris::~Tetris()
{
	delete[] pField;
}

Tetris* Tetris::getTetrisInstance()
{
	if (ins == nullptr)
	{
		mutex.lock();
		if (ins == nullptr)
		{
			ins = new Tetris();
		}
		mutex.unlock();
	}
	return ins;
}

//Update button status
void Tetris::keyUpdate()
{
	for (int k = 0; k < 5; k++)								// Right Left Down Z
		m_key[k] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28ZX"[k]))) != 0;
}

bool Tetris::DoesPieceFit(Tetromino* teromino, int nRotation, int nPosX, int nPosY)
{
	// All Field cells >0 are occupied
	for (int px = 0; px < 4; px++)
		for (int py = 0; py < 4; py++)
		{	
			if (nPosX + px >= 0 && nPosX + px < m_fieldWidth && nPosY + py >= 0 && nPosY + py < m_fieldHeight)
			{
				int fi = (nPosY + py) * m_fieldWidth + (nPosX + px);
				// In Bounds so do collision check
				if (teromino->getBlock(px, py, nRotation) != '.' && pField[fi] != 0)
					return false; // fail on first hit
			}
		}

	return true;
}
//Start game
void Tetris::startTetris()
{
	char *pScreen = new char[m_screenWidth*m_screenHeight];
	for (int i = 0; i < m_screenHeight * m_screenWidth; i++)
		pScreen[i] = ' ';

	HANDLE	hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

	// Game Logic
	Tetromino* currentTetromino = new Tetromino(rand() % 7);
	int nCurrentX = m_fieldWidth / 2 - 2;
	int nCurrentY = 0;
	int nSpeed = 20;
	int nSpeedCount = 0;
	bool bForceDown = false;
	bool bRotateHoldZ = true;
	bool bRotateHoldX = true;
	int nPieceCount = 0;
	int nScore = 0;
	std::vector<int> vLines;
	bool bGameOver = false;

	while (!bGameOver) // Main Loop
	{
		// Timing 
		std::this_thread::sleep_for(std::chrono::milliseconds(50)); // Small Step = 1 Game Tick
		nSpeedCount++;
		bForceDown = (nSpeedCount == nSpeed);

		// Input 
		keyUpdate();

		// Game Logic 

		// Handle player movement
		nCurrentX += (m_key[0] && DoesPieceFit(currentTetromino, currentTetromino->m_rotation, nCurrentX + 1, nCurrentY)) ? 1 : 0;
		nCurrentX -= (m_key[1] && DoesPieceFit(currentTetromino, currentTetromino->m_rotation, nCurrentX - 1, nCurrentY)) ? 1 : 0;
		nCurrentY += (m_key[2] && DoesPieceFit(currentTetromino, currentTetromino->m_rotation, nCurrentX, nCurrentY + 1)) ? 1 : 0;

		// Rotate, but latch to stop wild spinning
		if (m_key[3])
		{
			currentTetromino->m_rotation += (bRotateHoldZ && DoesPieceFit(currentTetromino, currentTetromino->m_rotation + 1, nCurrentX, nCurrentY)) ? 1 : 0;
			bRotateHoldZ = false;
		}
		else
			bRotateHoldZ = true;

		if (m_key[4])
		{
			currentTetromino->m_rotation -= (bRotateHoldX && DoesPieceFit(currentTetromino, currentTetromino->m_rotation - 1, nCurrentX, nCurrentY)) ? 1 : 0;
			bRotateHoldX = false;
		}
		else
			bRotateHoldX = true;

		// Force the piece down the playfield if it's time
		if (bForceDown)
		{
			// Update difficulty every 50 pieces
			nSpeedCount = 0;
			nPieceCount++;
			if (nPieceCount % 50 == 0 && nSpeed >= 10)
				nSpeed--;

			// Test if piece can be moved down
			if (DoesPieceFit(currentTetromino, currentTetromino->m_rotation, nCurrentX, nCurrentY + 1))
				nCurrentY++; // It can, so do it!
			else
			{
				// It can't! Lock the piece in place
				for (int px = 0; px < 4; px++)
					for (int py = 0; py < 4; py++)
						if (currentTetromino->getBlock(px, py) != '.')
							pField[(nCurrentY + py) * m_fieldWidth + (nCurrentX + px)] = currentTetromino->m_type + 1;

				// Check for lines
				for (int py = 0; py < 4; py++)
					if (nCurrentY + py < m_fieldHeight - 1)
					{
						bool bLine = true;
						for (int px = 1; px < m_fieldWidth - 1; px++)
							bLine &= (pField[(nCurrentY + py) * m_fieldWidth + px]) != 0;

						if (bLine)
						{
							// Remove Line, set to =
							for (int px = 1; px < m_fieldWidth - 1; px++)
								pField[(nCurrentY + py) * m_fieldWidth + px] = 8;
							vLines.push_back(nCurrentY + py);
						}
					}

				nScore += 25;
				if (!vLines.empty())	nScore += (1 << vLines.size()) * 100;

				// Pick New Piece
				nCurrentX = m_fieldWidth / 2 - 2;
				nCurrentY = 0;
				currentTetromino->changeType(rand() % 7);

				// If piece does not fit straight away, game over!
				bGameOver = !DoesPieceFit(currentTetromino, currentTetromino->m_rotation, nCurrentX, nCurrentY);
			}
		}

		// Display ======================

		// Draw Field
		for (int x = 0; x < m_fieldWidth; x++)
			for (int y = 0; y < m_fieldHeight; y++)
				pScreen[(y + 2)*m_screenWidth + (x + 2)] = " ABCDEFG=#"[pField[y*m_fieldWidth + x]];

		// Draw Current Piece
		for (int px = 0; px < 4; px++)
			for (int py = 0; py < 4; py++)
				if (currentTetromino->getBlock(px, py) != '.')
					pScreen[(nCurrentY + py + 2)*m_screenWidth + (nCurrentX + px + 2)] = currentTetromino->m_type + 65;

		// Draw Score
		sprintf_s(&pScreen[2 * m_screenWidth + m_fieldWidth + 6], 16, "SCORE: %8d", nScore);

		// Animate Line Completion
		if (!vLines.empty())
		{
			// Display Frame (cheekily to draw lines)
			WriteConsoleOutputCharacter(hConsole, pScreen, m_screenWidth * m_screenHeight, { 0,0 }, &dwBytesWritten);
			std::this_thread::sleep_for(std::chrono::milliseconds(400)); // Delay a bit

			for (auto &v : vLines)
				for (int px = 1; px < m_fieldWidth - 1; px++)
				{
					for (int py = v; py > 0; py--)
						pField[py * m_fieldWidth + px] = pField[(py - 1) * m_fieldWidth + px];
					pField[px] = 0;
				}

			vLines.clear();
		}

		// Display Frame
		WriteConsoleOutputCharacter(hConsole, pScreen, m_screenWidth * m_screenHeight, { 0,0 }, &dwBytesWritten);
	}
	
	// Over
	delete[] pScreen;
	CloseHandle(hConsole);
	std::cout << "Game Over!! Score:" << nScore << std::endl;
	system("pause");

}