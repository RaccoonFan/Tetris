#include <iostream>
#include<thread>
#include<chrono>
#include<Windows.h>
#include<vector>

std::string tetromino[7];
int nFieldWidth = 12;
int nFieldHeight = 18;
unsigned char *pField = nullptr;

int nScreenWidth = 80;
int nScreenHeight = 30;

int Rotate(int px, int py, int r)
{
	while (r < 0) r += 4;
	switch (r % 4)
	{
	case 0: return py * 4 + px;
	case 1: return 12 + py - px * 4;
	case 2: return 15 - py * 4 - px;
	case 3: return 3 - py + px * 4;
	}
	return 0;
}
bool DoesPieceFit(int nTetromino, int nRotation, int nPosX, int nPosY)
{
	// All Field cells >0 are occupied
	for (int px = 0; px < 4; px++)
		for (int py = 0; py < 4; py++)
		{
			// Get index into piece
			int pi = Rotate(px, py, nRotation);

			// Get index into field
			int fi = (nPosY + py) * nFieldWidth + (nPosX + px);

			if (nPosX + px >= 0 && nPosX + px < nFieldWidth)
			{
				if (nPosY + py >= 0 && nPosY + py < nFieldHeight)
				{
					// In Bounds so do collision check
					if (tetromino[nTetromino][pi] != '.' && pField[fi] != 0)
						return false; // fail on first hit
				}
			}
		}

	return true;
}

int main()
{
	//Create assets
	tetromino[0].append("..X.");
	tetromino[0].append("..X.");
	tetromino[0].append("..X.");
	tetromino[0].append("..X.");

	tetromino[1].append("..X.");
	tetromino[1].append(".XX.");
	tetromino[1].append(".X..");
	tetromino[1].append("....");

	tetromino[2].append(".X..");
	tetromino[2].append(".XX.");
	tetromino[2].append("..X.");
	tetromino[2].append("....");

	tetromino[3].append("..X.");
	tetromino[3].append(".XXX");
	tetromino[3].append("....");
	tetromino[3].append("....");

	tetromino[4].append("....");
	tetromino[4].append(".XX.");
	tetromino[4].append("..X.");
	tetromino[4].append("..X.");
	
	tetromino[5].append("....");
	tetromino[5].append(".XX.");
	tetromino[5].append(".X..");
	tetromino[5].append(".X..");
	
	tetromino[6].append(".XX.");
	tetromino[6].append(".XX.");
	tetromino[6].append("....");
	tetromino[6].append("....");

	pField = new unsigned char[nFieldWidth*nFieldHeight];
	for (int x = 0; x < nFieldWidth; x++)
		for (int y = 0; y < nFieldHeight; y++)
			pField[y*nFieldWidth + x] = (x == 0 || x == nFieldWidth - 1 || y == nFieldHeight - 1) ? 9 : 0;

	char *pScreen = new char[nScreenWidth*nScreenHeight];
	for (int i = 0; i < nScreenWidth*nScreenHeight; i++) pScreen[i] = ' ';
	HANDLE	hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

	// Game Logic
	bool bKey[5];
	int nCurrentPiece = 0;
	int nCurrentRotation = 0;
	int nCurrentX = nFieldWidth / 2 - 2;
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
		// Timing =======================
		std::this_thread::sleep_for(std::chrono::milliseconds(50)); // Small Step = 1 Game Tick
		nSpeedCount++;
		bForceDown = (nSpeedCount == nSpeed);

		// Input ========================
		for (int k = 0; k < 5; k++)								// Right Left Down Z
			bKey[k] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28ZX"[k]))) != 0;

		// Game Logic ===================

		// Handle player movement
		nCurrentX += (bKey[0] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX + 1, nCurrentY)) ? 1 : 0;
		nCurrentX -= (bKey[1] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX - 1, nCurrentY)) ? 1 : 0;
		nCurrentY += (bKey[2] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1)) ? 1 : 0;

		// Rotate, but latch to stop wild spinning
		if (bKey[3])
		{
			nCurrentRotation += (bRotateHoldZ && DoesPieceFit(nCurrentPiece, nCurrentRotation + 1, nCurrentX, nCurrentY)) ? 1 : 0;
			bRotateHoldZ = false;
		}
		else
			bRotateHoldZ = true;

		if (bKey[4])
		{
			nCurrentRotation -= (bRotateHoldX && DoesPieceFit(nCurrentPiece, nCurrentRotation + 1, nCurrentX, nCurrentY)) ? 1 : 0;
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
			if (nPieceCount % 50 == 0)
				if (nSpeed >= 10) nSpeed--;

			// Test if piece can be moved down
			if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1))
				nCurrentY++; // It can, so do it!
			else
			{
				// It can't! Lock the piece in place
				for (int px = 0; px < 4; px++)
					for (int py = 0; py < 4; py++)
						if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] != '.')
							pField[(nCurrentY + py) * nFieldWidth + (nCurrentX + px)] = nCurrentPiece + 1;

				// Check for lines
				for (int py = 0; py < 4; py++)
					if (nCurrentY + py < nFieldHeight - 1)
					{
						bool bLine = true;
						for (int px = 1; px < nFieldWidth - 1; px++)
							bLine &= (pField[(nCurrentY + py) * nFieldWidth + px]) != 0;

						if (bLine)
						{
							// Remove Line, set to =
							for (int px = 1; px < nFieldWidth - 1; px++)
								pField[(nCurrentY + py) * nFieldWidth + px] = 8;
							vLines.push_back(nCurrentY + py);
						}
					}

				nScore += 25;
				if (!vLines.empty())	nScore += (1 << vLines.size()) * 100;

				// Pick New Piece
				nCurrentX = nFieldWidth / 2 - 2;
				nCurrentY = 0;
				nCurrentRotation = 0;
				nCurrentPiece = rand() % 7;

				// If piece does not fit straight away, game over!
				bGameOver = !DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY);
			}
		}

		// Display ======================

		// Draw Field
		for (int x = 0; x < nFieldWidth; x++)
			for (int y = 0; y < nFieldHeight; y++)
				pScreen[(y + 2)*nScreenWidth + (x + 2)] = " ABCDEFG=#"[pField[y*nFieldWidth + x]];

		// Draw Current Piece
		for (int px = 0; px < 4; px++)
			for (int py = 0; py < 4; py++)
				if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] != '.')
					pScreen[(nCurrentY + py + 2)*nScreenWidth + (nCurrentX + px + 2)] = nCurrentPiece + 65;

		// Draw Score
		sprintf_s(&pScreen[2 * nScreenWidth + nFieldWidth + 6], 16, "SCORE: %8d", nScore);

		// Animate Line Completion
		if (!vLines.empty())
		{
			// Display Frame (cheekily to draw lines)
			WriteConsoleOutputCharacter(hConsole, pScreen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
			std::this_thread::sleep_for(std::chrono::milliseconds(400)); // Delay a bit

			for (auto &v : vLines)
				for (int px = 1; px < nFieldWidth - 1; px++)
				{
					for (int py = v; py > 0; py--)
						pField[py * nFieldWidth + px] = pField[(py - 1) * nFieldWidth + px];
					pField[px] = 0;
				}

			vLines.clear();
		}

		// Display Frame
		WriteConsoleOutputCharacter(hConsole, pScreen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
	}

	// Over
	CloseHandle(hConsole);
	std::cout << "Game Over!! Score:" << nScore << std::endl;
	system("pause");

	return 0;
}