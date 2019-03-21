#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include <thread>
#include <vector>
#include <ctime>

using namespace std;

// Override base class with your custom functionality
class Tetris : public olc::PixelGameEngine
{
public:
	Tetris()
	{
		sAppName = "Tetris";
	}

private:
	//Game Assests
	olc::Sprite *tetrominoBlockColors;
	//olc::Sprite *tetromino;
	olc::Sprite *background;
	olc::Sprite *blockColor;
	
	olc::Sprite 
		*Green = new olc::Sprite(16, 16),
		*Red = new olc::Sprite(16, 16),
		*Yellow = new olc::Sprite(16, 16),
		*Cyan = new olc::Sprite(16, 16),
		*Blue = new olc::Sprite(16, 16),
		*Purple = new olc::Sprite(16, 16),
		*Orange = new olc::Sprite(16, 16);
	
	olc::Sprite *Blank = new olc::Sprite(16, 16);
	olc::Sprite *oldBlockColors;

	//Tetromino Assets
	int colorSelector = 0;
	int nCurrentPiece = 0;

	//Game Assets
	wstring tetromino[7];
	int nFieldWidth = 12;
	int nFieldHeight = 20;
	unsigned char *pField = nullptr;

	olc::Sprite *screen = new olc::Sprite[ScreenWidth() * ScreenHeight()];

	//Game Variables
	int nSpeed = 20;
	int nSpeedCounter = 0;
	bool bForceDown = false;
	int nPieceCount = 0;
	int nScore = 0;

	int nCurrentX = nFieldWidth / 4;		//nFieldWidth / 3
	int nCurrentY = 0;
	int nCurrentRotation = 0;

	int nTempCurrentX = 0;
	int nTempCurrentY = 0;

	bool bGameOver = false;

	vector<int> vLines;

	char piece;

public:
	bool OnUserCreate() override
	{
		//GENERATE SEED
		srand(time(NULL));

		//DRAW BACKGROUND & BORDER
		for (int px = 0; px < ScreenWidth(); px++)
			for (int py = 0; py < ScreenHeight(); py++)
				Draw(px, py, olc::Pixel(112, 128, 144));

		//create playing field
		pField = new unsigned char[nFieldWidth*nFieldHeight];
		for (int x = 0; x < nFieldWidth; x++)
			for (int y = 0; y < nFieldHeight; y++)
				pField[y*nFieldWidth + x] = (x == 0 || x == nFieldWidth - 1 || y == nFieldHeight - 1) ? 9 : 0;

		for (int i = 0; i < nFieldWidth * nFieldHeight; i++) screen[i] = olc::Sprite(16, 16);

		background = new olc::Sprite("tetrisBorder.png");
		DrawSprite(32, 16, background, 0);



		//INITIALIZE TETROMINO PALLETE
		tetrominoBlockColors = new olc::Sprite("tetrisBlocks.png");

		olc::Sprite *blocks[7] = { Green, Red, Yellow, Cyan, Blue, Purple, Orange };

		for (int i = 0; i < 7; i++)
		{
			for (int x = 16 * colorSelector; x < 16 + colorSelector * 16; x++)
			{
				for (int y = 0; y < 16; y++)
				{
					olc::Pixel p = tetrominoBlockColors->GetPixel(x, y);

					blocks[i]->SetPixel((x - (16 * colorSelector)), y, p);
				}
			}
			colorSelector++;
		}

		//SELECT COLOR FOR TETROMINO
		colorSelector = rand() % 7;

		//CREATE BLOCK COLOR FOR NEW TETROMINO
		blockColor = new olc::Sprite(16, 16);

		for (int x = 16 * colorSelector; x < 16 + colorSelector * 16; x++)
			for (int y = 0; y < 16; y++)
			{
				olc::Pixel p = tetrominoBlockColors->GetPixel(x, y);

				blockColor->SetPixel((x - (16 * colorSelector)), y, p);
			}

		//create assets
		tetromino[0].append(L"..X.");
		tetromino[0].append(L"..X.");
		tetromino[0].append(L"..X.");
		tetromino[0].append(L"..X.");

		tetromino[1].append(L"..X.");
		tetromino[1].append(L".XX.");
		tetromino[1].append(L".X..");
		tetromino[1].append(L"....");

		tetromino[2].append(L".X..");
		tetromino[2].append(L".XX.");
		tetromino[2].append(L"..X.");
		tetromino[2].append(L"....");

		tetromino[3].append(L"....");
		tetromino[3].append(L".XX.");
		tetromino[3].append(L".XX.");
		tetromino[3].append(L"....");

		tetromino[4].append(L"..X.");
		tetromino[4].append(L".XX.");
		tetromino[4].append(L"..X.");
		tetromino[4].append(L"....");

		tetromino[5].append(L"....");
		tetromino[5].append(L".XX.");
		tetromino[5].append(L"..X.");
		tetromino[5].append(L"..X.");

		tetromino[6].append(L"....");
		tetromino[6].append(L".XX.");
		tetromino[6].append(L".X..");
		tetromino[6].append(L".X..");
		//end of assets

		// Choose First Piece
		nCurrentPiece = rand() % 7;
		nCurrentRotation = 0;
		nCurrentX = nFieldWidth / 4;
		nCurrentY = -1;
		
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		if (!bGameOver) {
			//GAME TIMING
			this_thread::sleep_for(50ms); //Game Tick
			nSpeedCounter++;
			bForceDown = (nSpeedCounter == nSpeed);

			//USER INPUT
								//<----Move---->
			nCurrentX -= (GetKey(olc::Key::LEFT).bHeld) && (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX - 1, nCurrentY)) ? 1 : 0;
			nCurrentX += (GetKey(olc::Key::RIGHT).bHeld) && (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX + 1, nCurrentY)) ? 1 : 0;
			nCurrentY += (GetKey(olc::Key::DOWN).bHeld) && (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1)) ? 1 : 0;
			nCurrentRotation += (GetKey(olc::Key::SPACE).bPressed) && (DoesPieceFit(nCurrentPiece, nCurrentRotation + 1, nCurrentX, nCurrentY)) ? 1 : 0;

			//GAME LOGIC
			if (bForceDown)
			{
				//Animate Line completion
				if (!vLines.empty())
				{
					this_thread::sleep_for(75ms);
					// Remove Line and move pieces down
					for (auto &v : vLines)
						for (int px = 1; px < nFieldWidth - 1; px++)
						{
							for (int py = v; py > 0; py--)
								pField[py * nFieldWidth + px] = pField[(py - 1) * nFieldWidth + px];
							pField[px] = 0;
						}

					vLines.clear();
				}

				if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1) && vLines.empty())

					nCurrentY++; // Do it
				else
				{
					// Lock Current Piece into Field
					for (int px = 0; px < 4; px++)
						for (int py = 0; py < 4; py++)
							if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] != L'.') {
								piece = colorSelector + 49;
								pField[(nCurrentY + py) * nFieldWidth + (nCurrentX + px)] = piece;
							}

					//Iterate Game Difficulty
					if (nPieceCount % 10 == 0)
						if (nSpeed >= 10) nSpeed--;

					// Have Rows been Created
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
					//Create Line Multiplier
					nScore += 25;
					if (!vLines.empty()) nScore += (1 << vLines.size()) * 100;



					// Choose next Piece
					nCurrentX = nFieldWidth / 4;
					nCurrentY = -1;
					nCurrentRotation = 0;
					newTetromino();

					// if piece does not fit
					bGameOver = !DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY);
				}
				nSpeedCounter = 0;
			}

			//RENDEROUTPUT
			FillRect(0, 0, ScreenWidth(), ScreenHeight(), olc::Pixel(112, 128, 144));

			//DRAW FIELD
			DrawSprite(32, 16, background, 0);

			//Draw Old Pieces
			int transition = 10;

			for (int x = 0; x < nFieldWidth; x++) {
				for (int y = 0; y < nFieldHeight; y++) {
					if (isdigit(pField[y*nFieldWidth + x])) {
						transition = pField[y*nFieldWidth + x];
						//determine what color the blocks should be
						if (transition == 49)
							oldBlockColors = Green;
						else if (transition == 50)
							oldBlockColors = Red;
						else if (transition == 51)
							oldBlockColors = Yellow;
						else if (transition == 52)
							oldBlockColors = Cyan;
						else if (transition == 53)
							oldBlockColors = Blue;
						else if (transition == 54)
							oldBlockColors = Purple;
						else if (transition == 55)
							oldBlockColors = Orange;

						DrawSprite((2 + x) * 16, (2 + y) * 16, oldBlockColors, 0);
					}
				}
			}

			//Draw Current Piece
			if (vLines.empty())
			{
				for (int px = 0; px < 4; px++)
					for (int py = 0; py < 4; py++)
						if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] == L'X')
							DrawSprite((2 + nCurrentX + px) * 16, (2 + nCurrentY + py) * 16, blockColor, 0);
			}





			// Draw Score
			DrawString(nFieldWidth + 22, 2, "Score: " + to_string(nScore), olc::WHITE);

		}
		//-----------TESTING PURPOSES ONLY-------------------\\
		//DrawString(nFieldWidth + 200, 2, "Tranisition: " + to_string(transition), olc::WHITE);
		//DrawString(nFieldWidth + 200, 200, "Piece: " + to_string(piece), olc::WHITE);

		else
		{
			//PAUSE GAME

			//GAME OVER

			//Render Output
			FillRect(0, 0, ScreenWidth(), ScreenHeight(), olc::Pixel(112, 128, 144));
			DrawString(50, 80, "Game Over! Your Score is " + to_string(nScore), olc::WHITE);
			DrawString(50, 160, "Press Z to Continue Escape to Quit", olc::WHITE);

			if (GetKey(olc::Key::Z).bPressed)
			{
				pField = new unsigned char[nFieldWidth*nFieldHeight]; // Create play field buffer
				for (int x = 0; x < nFieldWidth; x++)
					for (int y = 0; y < nFieldHeight; y++)
						pField[y*nFieldWidth + x] = (x == 0 || x == nFieldWidth - 1 || y == nFieldHeight - 1) ? 9 : 0;


				nCurrentPiece = 1;
				nCurrentRotation = 0;
				nCurrentX = nFieldWidth / 3;
				nCurrentY = -1;



				nSpeed = 20;
				nSpeedCounter = 0;
				bForceDown = false;
				nPieceCount = 0;
				nScore = 0;
				bGameOver = false;
			}




			if (GetKey(olc::Key::ESCAPE).bPressed)
				return false;
		}
	}


	//CREATE TETROMINOS
	void newTetromino()
	{
		colorSelector = rand() % 7;

		//CREATE BLOCK COLOR FOR NEW TETROMINO
		blockColor = new olc::Sprite(16, 16);

		for (int x = 16 * colorSelector; x < 16 + colorSelector * 16; x++)
			for (int y = 0; y < 16; y++)
			{
				olc::Pixel p = tetrominoBlockColors->GetPixel(x, y);

				blockColor->SetPixel((x - (16 * colorSelector)), y, p);
			}

		//SELECT NEW TETROMINO
		nCurrentPiece = rand() % 7;
	}

	//CHECK ROTATION
	int Rotate(int px, int py, int r)
	{
		switch (r % 4)
		{
		case 0: return py * 4 + px;				//0 degrees
		case 1: return 12 + py - (px * 4);		//90 degrees
		case 2: return 15 - (py * 4) - px;		//180 degrees
		case 3: return 3 - py + (px * 4);		//270 degrees
		}
		return 0;
	}

	//DOES PIECE FIT
	bool DoesPieceFit(int nTetromino, int nRotation, int nPosX, int nPosY)
	{
		for (int px = 0; px < 4; px++)
			for (int py = 0; py < 4; py++)
			{
				//Get index into piece
				int pi = Rotate(px, py, nRotation);

				//Get index into field
				int fi = (nPosY + py) * nFieldWidth + (nPosX + px);

				if (nPosX + px >= 0 && nPosX + px < nFieldWidth)
					if (nPosY + py >= 0 && nPosY + py < nFieldHeight)
					{
						if (tetromino[nTetromino][pi] == L'X' && pField[fi] != 0)
							return false; //fail on first hit
					}
			}

		return true;
	}
};

int main()
{

	Tetris demo;
	if (demo.Construct(512, 480, 2, 2))
		demo.Start();

	return 0;
}