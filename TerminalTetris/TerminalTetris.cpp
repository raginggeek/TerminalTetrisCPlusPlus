// TerminalTetris.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <thread>
#include <vector>
using namespace std;

#include <Windows.h>

wstring tetromino[7]; //shape container
int nFieldWidth = 12; //width of game area
int nFieldHeight = 18; //height of game area
unsigned char* pField = nullptr; //dynamically allocated play space

int nScreenWidth = 120;  //Console Width;
int nScreenHeight = 30; //Console Height;
int nFieldXOffSet = 2; //offset of play area in screen buffer width
int nFieldYOffset = 2; //offset of play area in screen buffer height

int Rotate(int px, int py, int r)
{
	switch (r % 4)
	{
	case 0: return py * 4 + px;        //rotate   0 degrees
	case 1: return 12 + py - (px * 4); //rotate  90 degrees
	case 2: return 15 - (py * 4) - px; //rotate 180 degrees
	case 3: return 3 - py + (px * 4);  //rotate 270 degrees
	}
	return 0;
}

bool DoesPieceFit(int nTetromino, int nRotation, int nPosX, int nPosY)
{
	//iterate over entirety of a piece(first by width
	for (int px = 0; px < 4; px++)
	{

		//then by height
		for (int py = 0; py < 4; py++)
		{
			//Get index into piece
			int pi = Rotate(px, py, nRotation);

			//Get index into field
			int fi = (nPosY + py) * nFieldWidth + (nPosX + px);

			//bounds checking width
			if (nPosX + px >= 0 && nPosX + px < nFieldWidth)
			{
				//bounds checking height
				if (nPosY + py >= 0 && nPosY + py < nFieldHeight)
				{
					//collision detection
					if (tetromino[nTetromino][pi] == L'X' && pField[fi] != 0)
					{
						return false; //fail on first collision
					}
				}
			}
		}
	}
	return true;
}

int main()
{
	// Create assets
	tetromino[0].append(L"..X.");
	tetromino[0].append(L"..X.");
	tetromino[0].append(L"..X.");
	tetromino[0].append(L"..X.");

	tetromino[1].append(L"....");
	tetromino[1].append(L".XX.");
	tetromino[1].append(L".XX.");
	tetromino[1].append(L"....");

	tetromino[2].append(L".X..");
	tetromino[2].append(L".XX.");
	tetromino[2].append(L"..X.");
	tetromino[2].append(L"....");

	tetromino[3].append(L"..X.");
	tetromino[3].append(L".XX.");
	tetromino[3].append(L".X..");
	tetromino[3].append(L"....");

	tetromino[4].append(L".XX.");
	tetromino[4].append(L"..X.");
	tetromino[4].append(L"..X.");
	tetromino[4].append(L"....");

	tetromino[5].append(L".XX.");
	tetromino[5].append(L".X..");
	tetromino[5].append(L".X..");
	tetromino[5].append(L"....");

	tetromino[6].append(L"..X.");
	tetromino[6].append(L".XX.");
	tetromino[6].append(L"..X.");
	tetromino[6].append(L"....");

	//Init the play space
	pField = new unsigned char[nFieldWidth * nFieldHeight]; //create a allocated play space
	for (int x = 0; x < nFieldWidth; x++)
	{
		for (int y = 0; y < nFieldHeight; y++)
		{
			pField[y * nFieldWidth + x] = (x == 0 || x == nFieldWidth - 1 || y == nFieldHeight - 1) ? 9 : 0;
		}
	}

	//Create the Screen Buffer
	wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];
	for (int i = 0; i < nScreenWidth * nScreenHeight; i++)
	{
		screen[i] = L' ';
	}
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

	//Game State
	bool bGameOver = false;
	int nCurrentPiece = rand() % 7;
	int nCurrentRotation = 0;
	int nCurrentX = nFieldWidth / 2;
	int nCurrentY = 0;

	bool bKey[4];
	bool bRotateHeld = false;

	int nSpeed = 20;
	int nSpeedCounter = 0;
	bool bForceDown = false;
	int nPieceCount = 0;
	int nScore = 0;

	vector<int> vLines;

	while (!bGameOver)
	{
		// GAME TIMING ==========================================
		this_thread::sleep_for(50ms); // Game Tick Rate
		nSpeedCounter++;
		bForceDown = (nSpeedCounter == nSpeed);

		// INPUT ================================================
		for (int k = 0; k < 4; k++)
		{

			/*
			 * Controls are:
			 * \x27 Right Arrow
			 * \x25 Left Arrow
			 * \x28 Down Arrow
			 * Z to Rotate
			 * 
			 */
			bKey[k] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28Z"[k]))) != 0; 
		}

		// GAME LOGIC ===========================================
		//if right arrow and piece fits move x 1 to the right
		nCurrentX += (bKey[0] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX + 1, nCurrentY)) ? 1 : 0;
		//if left arrow and piece fits move x 1 to the left(minus)
		nCurrentX -= (bKey[1] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX - 1, nCurrentY)) ? 1 : 0;
		//if down arrow and piece fits move y 1 down the screen
		nCurrentY += (bKey[2] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1)) ? 1 : 0;
		if (bKey[3])
		{
			//if piece fits and user not holding the button rotate piece
			nCurrentRotation += (!bRotateHeld && DoesPieceFit(nCurrentPiece, nCurrentRotation + 1, nCurrentX, nCurrentY)) ? 1 : 0;
			bRotateHeld = true;
		} else
		{
			bRotateHeld = false;
		}

		if (bForceDown)
		{
			if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1))
			{
				nCurrentY++; //piece fits so push it down
			} else
			{
				// Lock the current piece into the field
				for (int px = 0; px < 4; px++)
				{
					for (int py = 0; py < 4; py++)
					{
						if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] == L'X')
						{
							pField[(nCurrentY + py) * nFieldWidth + (nCurrentX + px)] = nCurrentPiece + 1;
						}
					}
				}
				nPieceCount++;
				if (nPieceCount % 10 == 0)
				{
					if (nSpeed >= 10)
					{
						nSpeed--;
					}
				}
				
				// Check for lines(based on last piece locked
				for (int py = 0; py < 4; py++)
				{
					//quick boundary check
					if (nCurrentY + py < nFieldHeight - 1)
					{
						//assume we have a line and prove otherwise via gaps.
						bool bLine = true;
						//scanning the whole length of the row
						for (int px = 1; px < nFieldWidth - 1; px++)
						{
							bLine &= (pField[(nCurrentY + py) * nFieldWidth + px]) != 0;
						}
						if (bLine)
						{
							for (int px = 1; px < nFieldWidth - 1; px++)
							{
								pField[(nCurrentY + py) * nFieldWidth + px] = 8;
							}
							vLines.push_back(nCurrentY + py);
						}
					}
				}
				nScore += 25;
				if (!vLines.empty()) {
					nScore += (1 << vLines.size()) * 100;
				}
				
				// Choose next piece
				nCurrentX = nFieldWidth / 2;
				nCurrentY = 0;
				nCurrentRotation = 0;
				nCurrentPiece = rand() % 7;
				
				// If piece does not fit lose!
				bGameOver = !DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY);
				
			}
			bForceDown = false;
			nSpeedCounter = 0;
		}
		

		

		// RENDER OUTPUT ========================================

		//Draw Field
		for (int x = 0; x < nFieldWidth; x++)
		{
			for (int y = 0; y < nFieldHeight; y++)
			{
				screen[(y + nFieldYOffset) * nScreenWidth + (x + nFieldXOffSet)] = L" ABCDEFG=#"[pField[y * nFieldWidth + x]];
			}
		}

		//Draw active piece
		for (int px = 0; px < 4; px++)
		{
			for (int py = 0; py < 4; py++ )
			{
				if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] == L'X')
				{
					screen[(nCurrentY + py + nFieldYOffset) * nScreenWidth + (nCurrentX + px + nFieldXOffSet)] = nCurrentPiece + 65;
				}
			}
		}

		//Draw Score
		swprintf_s(&screen[2 * nScreenWidth + nFieldWidth + 6], 16, L"SCORE: %8d", nScore);

		//Line Cleanup Logic
		if (!vLines.empty())
		{
			//Display Frame
			WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
			this_thread::sleep_for(400ms); //delay render a bit	to let the user soak in the reward
			for (auto &v : vLines)
			{
				for (int px = 1; px < nFieldWidth - 1; px++)
				{
					for (int py = v; py > 0; py--)
					{
						pField[py * nFieldWidth + px] = pField[(py - 1) * nFieldWidth + px];
					}
					pField[px] = 0;
				}
			}
			vLines.clear();
		}
		
		
		//Display Frame
		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
		
	}

	//Game Over
	CloseHandle(hConsole);
	cout << "Game Over!! Score: " << nScore << endl;
	system("pause");

	//exit
	return 0;
}
