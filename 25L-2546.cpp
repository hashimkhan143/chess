#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include <cctype>
#include <cmath>

using namespace std;
using namespace sf;
const int SIZE = 8;
char board[SIZE][SIZE];
bool whiteTurn = true;
bool gameOver = false;

void initializeBoard();
void printBoard();
bool isValidMove(int sx, int sy, int dx, int dy);
bool isValidPawnMove(int sx, int sy, int dx, int dy, char piece);
bool isValidBishopMove(int sx, int sy, int dx, int dy, char piece);
bool isValidRookMove(int sx, int sy, int dx, int dy, char piece);
bool isValidKnightMove(int sx, int sy, int dx, int dy, char piece);
bool isValidQueenMove(int sx, int sy, int dx, int dy, char piece);
bool isValidKingMove(int sx, int sy, int dx, int dy, char piece);
void makeMove(int sx, int sy, int dx, int dy);

//pawn promotion
void handlePawnPromotion(int r, int c);

//stalemate
bool isStalemate(bool whiteKing);

bool isInsideBoard(int r, int c) { return r >= 0 && r < SIZE && c >= 0 && c < SIZE; }

//check and checkmate
bool canPieceAttackSquare(int sx, int sy, int dx, int dy); // low-level attack test
bool isSquareAttacked(int r, int c, bool byWhite);
bool isKingInCheck(bool whiteKing);
bool wouldBeInCheckAfterMove(int sx, int sy, int dx, int dy);
bool canAnyMoveSaveKing(bool whiteKing);
bool isCheckmate(bool whiteKing);

const int tilesize = 100;
const int boardsize = 8;
const float PIECE_SCALE = 0.78f;
Font aerial;
Texture whiteRookTexture, whiteKnightTexture, whiteBishopTexture,
whiteQueenTexture, whiteKingTexture, whitePawnTexture;
Texture blackRookTexture, blackKnightTexture, blackBishopTexture,
blackQueenTexture, blackKingTexture, blackPawnTexture;

Sprite pieceSprites[SIZE][SIZE];//represents drwable image on the screen

Texture& textureForPiece(char p) //for texture of piece which is required
{
    switch (p) {
    case 'R': return whiteRookTexture;
    case 'N': return whiteKnightTexture;
    case 'B': return whiteBishopTexture;
    case 'Q': return whiteQueenTexture;
    case 'K': return whiteKingTexture;
    case 'P': return whitePawnTexture;
    case 'r': return blackRookTexture;
    case 'n': return blackKnightTexture;
    case 'b': return blackBishopTexture;
    case 'q': return blackQueenTexture;
    case 'k': return blackKingTexture;
    case 'p': return blackPawnTexture;
    default:
        return blackPawnTexture; //if none of the case matches then return black pawn 
    }
}

void updateSpritesFromBoard() {
    for (int r = 0; r < SIZE; ++r) 
    {
        for (int c = 0; c < SIZE; ++c) 
        {
            if (board[r][c] != ' ') 
            {
                pieceSprites[r][c].setTexture(textureForPiece(board[r][c]));
                FloatRect bounds = pieceSprites[r][c].getLocalBounds();//gets width and height of piece in bounds
                pieceSprites[r][c].setOrigin(bounds.width / 2.f, bounds.height / 2.f);// center origin for nicer dragging and placement
                pieceSprites[r][c].setPosition(c * tilesize + tilesize / 2.f, r * tilesize + tilesize / 2.f);//position in center 
                pieceSprites[r][c].setScale(PIECE_SCALE, PIECE_SCALE);//shrinks piece upto 78% of original
            }
            else 
            {
                //for moving unused sprite off_screen
                pieceSprites[r][c].setPosition(-1000.f, -1000.f);
            }
        }
    }
}
int main()
{
    //initialize chess board logical state
    initializeBoard();

    //for loading textures
    if (!whiteRookTexture.loadFromFile("pieces/white-rook.png")) cout << "Failed loading white-rook\n";
    if (!whiteKnightTexture.loadFromFile("pieces/white-knight.png")) cout << "Failed loading white-knight\n";
    if (!whiteBishopTexture.loadFromFile("pieces/white-bishop.png")) cout << "Failed loading white-bishop\n";
    if (!whiteQueenTexture.loadFromFile("pieces/white-queen.png")) cout << "Failed loading white-queen\n";
    if (!whiteKingTexture.loadFromFile("pieces/white-king.png")) cout << "Failed loading white-king\n";
    if (!whitePawnTexture.loadFromFile("pieces/white-pawn.png")) cout << "Failed loading white-pawn\n";

    if (!blackRookTexture.loadFromFile("pieces/black-rook.png")) cout << "Failed loading black-rook\n";
    if (!blackKnightTexture.loadFromFile("pieces/black-knight.png")) cout << "Failed loading black-knight\n";
    if (!blackBishopTexture.loadFromFile("pieces/black-bishop.png")) cout << "Failed loading black-bishop\n";
    if (!blackQueenTexture.loadFromFile("pieces/black-queen.png")) cout << "Failed loading black-queen\n";
    if (!blackKingTexture.loadFromFile("pieces/black-king.png")) cout << "Failed loading black-king\n";
    if (!blackPawnTexture.loadFromFile("pieces/black-pawn.png")) cout << "Failed loading black-pawn\n";

    //create window
    RenderWindow window(VideoMode(tilesize * boardsize, tilesize * boardsize), "Chess board - Drag & Drop");

    initializeBoard();
    updateSpritesFromBoard();

    //dragging state
    bool isDragging = false;
    int dragFromR = -1, dragFromC = -1;
    Vector2f dragOffset(0.f, 0.f); // offset of mouse inside sprite to keep cursor relative
    Sprite draggingSprite;         // a copy of sprite while dragging

    while (window.isOpen())
    {
        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed)
                window.close();

            if (gameOver) continue;

            // Start dragging
            if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left)
            {
                int mouseX = event.mouseButton.x;
                int mouseY = event.mouseButton.y;
                int col = mouseX / tilesize;
                int row = mouseY / tilesize;
                cout << "From Square: " << char('A' + col) << 8 - row << " (Row " << (row + 1) << ", Col " << (col + 1) << ")" << endl;

                if (isInsideBoard(row, col) && board[row][col] != ' ')
                {
                    bool pieceIsWhite = isupper(board[row][col]);
                    if ((pieceIsWhite && whiteTurn) || (!pieceIsWhite && !whiteTurn))
                    {
                        isDragging = true;
                        dragFromR = row;
                        dragFromC = col;
                        draggingSprite = pieceSprites[row][col];
                        Vector2f spritePos = draggingSprite.getPosition();
                        dragOffset.x = (float)mouseX - spritePos.x;
                        dragOffset.y = (float)mouseY - spritePos.y;
                    }
                }
            }

            // Release dragging
            if (event.type == Event::MouseButtonReleased && event.mouseButton.button == Mouse::Left)
            {
                if (isDragging)
                {
                    int mouseX = event.mouseButton.x;
                    int mouseY = event.mouseButton.y;
                    int toCol = mouseX / tilesize;
                    int toRow = mouseY / tilesize;
                    cout << "TO Square: " << char('A' + toCol) << 8 - toRow << " (Row " << (toRow + 1) << ", Col " << (toCol + 1) << ")" << endl;

                    if (isInsideBoard(toRow, toCol) && isValidMove(dragFromR, dragFromC, toRow, toCol))
                    {
                        makeMove(dragFromR, dragFromC, toRow, toCol);
                        handlePawnPromotion(toRow, toCol);

                        bool opponentIsWhite = !whiteTurn;
                        bool opponentInCheck = isKingInCheck(opponentIsWhite);
                        bool mate = isCheckmate(opponentIsWhite);
                        bool stalemate = isStalemate(opponentIsWhite);

                        updateSpritesFromBoard();

                        if (mate)
                        {
                            cout << (opponentIsWhite ? "White" : "Black") << " is CHECKMATED!\n";
                            gameOver = true;
                        }
                        else if (stalemate)
                        {
                            cout << "STALEMATE! Game is a draw.\n";
                            gameOver = true;
                        }
                        else if (opponentInCheck)
                        {
                            cout << (opponentIsWhite ? "White" : "Black") << " is in CHECK!\n";
                        }

                        whiteTurn = !whiteTurn;
                    }

                    isDragging = false;
                }
            }
        }

        // drawing
        window.clear();

        Color lightSquare(238, 238, 210);
        Color darkSquare(118, 150, 86);

        //highlighting modification: determine tile hover color
        int hoverRow = -1, hoverCol = -1;
        if (isDragging)
        {
            Vector2i mousePos = Mouse::getPosition(window);
            hoverCol = mousePos.x / tilesize;
            hoverRow = mousePos.y / tilesize;
        }

        for (int row = 0; row < boardsize; ++row)
        {
            for (int col = 0; col < boardsize; ++col)
            {
                RectangleShape square(Vector2f(tilesize, tilesize));
                square.setPosition(col * tilesize, row * tilesize);
                if ((row + col) % 2 == 0) square.setFillColor(lightSquare);
                else square.setFillColor(darkSquare);

                // Highlight the hovered tile green/red
                if (isDragging && row == hoverRow && col == hoverCol)
                {
                    if (isInsideBoard(row, col) && isValidMove(dragFromR, dragFromC, row, col))
                        square.setFillColor(Color(100, 255, 100)); // green for valid
                    else
                        square.setFillColor(Color(255, 100, 100)); // red for invalid
                }

                window.draw(square);
            }
        }

        // draw pieces
        for (int r = 0; r < SIZE; ++r)
        {
            for (int c = 0; c < SIZE; ++c)
            {
                if (board[r][c] != ' ')
                {
                    if (isDragging && r == dragFromR && c == dragFromC) continue;
                    pieceSprites[r][c].setPosition((float)(c * tilesize + tilesize / 2.f), (float)(r * tilesize + tilesize / 2.f));
                    window.draw(pieceSprites[r][c]);
                }
            }
        }

        // draw dragging sprite
        if (isDragging)
        {
            Vector2i mpos = Mouse::getPosition(window);
            draggingSprite.setPosition((float)mpos.x - dragOffset.x, (float)mpos.y - dragOffset.y);
            window.draw(draggingSprite);
        }

        window.display();
    }

    return 0;
}


void initializeBoard()
{
    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            board[i][j] = ' ';
        }
    }
    // for white pieces (uppercase)
    for (int i = 0; i < SIZE; i++)
    {
        board[6][i] = 'P';
    }
    board[7][0] = 'R'; board[7][1] = 'N'; board[7][2] = 'B'; board[7][3] = 'Q';
    board[7][4] = 'K'; board[7][5] = 'B'; board[7][6] = 'N'; board[7][7] = 'R';
    // for black pieces (lowercase)
    for (int i = 0; i < SIZE; i++)
    {
        board[1][i] = 'p';
    }
    board[0][0] = 'r'; board[0][1] = 'n'; board[0][2] = 'b'; board[0][3] = 'q';
    board[0][4] = 'k'; board[0][5] = 'b'; board[0][6] = 'n'; board[0][7] = 'r';
}


bool isValidMove(int sx, int sy, int dx, int dy)
{
    if (!isInsideBoard(sx, sy) || !isInsideBoard(dx, dy)) return false;
    if (sx == dx && sy == dy) return false; // no movement

    char piece = board[sx][sy];
    if (piece == ' ') return false; // no piece selected

    // destination cannot be occupied by same-color piece
    if (board[dx][dy] != ' ' && (isupper(piece) == isupper(board[dx][dy]))) return false;

    char lower = tolower(piece);
    bool valid = false;
    switch (lower) {
    case 'p': valid = isValidPawnMove(sx, sy, dx, dy, piece); break;
    case 'r': valid = isValidRookMove(sx, sy, dx, dy, piece); break;
    case 'n': valid = isValidKnightMove(sx, sy, dx, dy, piece); break;
    case 'b': valid = isValidBishopMove(sx, sy, dx, dy, piece); break;
    case 'q': valid = isValidQueenMove(sx, sy, dx, dy, piece); break;
    case 'k': valid = isValidKingMove(sx, sy, dx, dy, piece); break;
    default: return false;
    }

    if (!valid) return false;

    //prevent moves that leave your own king in check
    if (wouldBeInCheckAfterMove(sx, sy, dx, dy)) return false;

    return true;
}

bool isValidPawnMove(int sx, int sy, int dx, int dy, char piece)
{
    int direction = (isupper(piece) ? -1 : +1);  // White moves up (row--), black moves down (row++)

    // Simple 1-step forward
    if (dx == sx + direction && dy == sy && board[dx][dy] == ' ')
        return true;

    // Initial 2-step forward
    bool isInitialWhite = (piece == 'P' && sx == 6);
    bool isInitialBlack = (piece == 'p' && sx == 1);
    if ((isInitialWhite || isInitialBlack) &&
        dx == sx + 2 * direction &&
        dy == sy &&
        board[sx + direction][sy] == ' ' &&
        board[dx][dy] == ' ')
    {
        return true;
    }

    //one step diagonally captures only if target occupied by opponent
    if (dx == sx + direction && abs(dy - sy) == 1) 
    {
        if (board[dx][dy] != ' ' && isupper(piece) != isupper(board[dx][dy]))
            return true;
    }

    return false;
}

bool isValidRookMove(int sx, int sy, int dx, int dy, char piece)
{
    // must move in same row or same column
    if (sx != dx && sy != dy) return false;

    int stepX = (dx > sx) ? 1 : (dx < sx) ? -1 : 0;
    int stepY = (dy > sy) ? 1 : (dy < sy) ? -1 : 0;

    int x = sx + stepX;
    int y = sy + stepY;
    while (x != dx || y != dy) {
        if (board[x][y] != ' ') return false; // blocked
        x += stepX;
        y += stepY;
    }

    // destination handled in outer check (cannot capture same color)
    return true;
}

bool isValidKnightMove(int sx, int sy, int dx, int dy, char piece)
{
    int dxAbs = abs(dx - sx);
    int dyAbs = abs(dy - sy);
    if ((dxAbs == 2 && dyAbs == 1) || (dxAbs == 1 && dyAbs == 2)) {
        // destination color check done earlier
        return true;
    }
    return false;
}

bool isValidBishopMove(int sx, int sy, int dx, int dy, char piece) {
    if (abs(dx - sx) != abs(dy - sy)) return false;

    int stepX = (dx > sx) ? 1 : -1;
    int stepY = (dy > sy) ? 1 : -1;
    int x = sx + stepX;
    int y = sy + stepY;
    while (x != dx && y != dy) {
        if (board[x][y] != ' ') return false;
        x += stepX;
        y += stepY;
    }

    return true;
}

bool isValidQueenMove(int sx, int sy, int dx, int dy, char piece)
{
    // queen = rook OR bishop
    if (sx == dx || sy == dy) return isValidRookMove(sx, sy, dx, dy, piece);
    if (abs(dx - sx) == abs(dy - sy)) return isValidBishopMove(sx, sy, dx, dy, piece);
    return false;
}

bool isValidKingMove(int sx, int sy, int dx, int dy, char piece)
{
    int dxAbs = abs(dx - sx);
    int dyAbs = abs(dy - sy);
    if (dxAbs <= 1 && dyAbs <= 1) {
        // cannot capture same color (already checked)
        return true;
    }
    return false;
}

void makeMove(int sx, int sy, int dx, int dy)
{
    // Basic move: move piece, capture implicitly handled by overwrite
    board[dx][dy] = board[sx][sy];
    board[sx][sy] = ' ';
}


// low-level: determines whether piece at (sx,sy) could move to (dx,dy)
// using piece-specific movement rules (bypasses king-in-check prevention).
bool canPieceAttackSquare(int sx, int sy, int dx, int dy)
{
    if (!isInsideBoard(sx, sy) || !isInsideBoard(dx, dy)) return false;
    if (sx == dx && sy == dy) return false;
    char piece = board[sx][sy];
    if (piece == ' ') return false;

    // For pawns, use attack-only logic (pawns attack diagonally regardless of target occupancy)
    char lower = tolower(piece);
    switch (lower) {
    case 'p': {
        int direction = (isupper(piece) ? -1 : +1);
        if (dx == sx + direction && abs(dy - sy) == 1) return true;
        return false;
    }
    case 'r': return isValidRookMove(sx, sy, dx, dy, piece);
    case 'n': return isValidKnightMove(sx, sy, dx, dy, piece);
    case 'b': return isValidBishopMove(sx, sy, dx, dy, piece);
    case 'q': return isValidQueenMove(sx, sy, dx, dy, piece);
    case 'k': return isValidKingMove(sx, sy, dx, dy, piece);
    default: return false;
    }
}

// Is square (r,c) attacked by any piece of color byWhite?
bool isSquareAttacked(int r, int c, bool byWhite)
{
    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            char p = board[i][j];
            if (p == ' ') continue;
            if (isupper(p) != byWhite) continue; // only consider pieces of color 'byWhite'
            // Use piece-specific attack test (won't recurse into check-testing)
            if (canPieceAttackSquare(i, j, r, c)) return true;
        }
    }
    return false;
}

// Is the given color's king currently in check?
bool isKingInCheck(bool whiteKing)
{
    // find the king
    for (int r = 0; r < SIZE; ++r) {
        for (int c = 0; c < SIZE; ++c) {
            char p = board[r][c];
            if (whiteKing && p == 'K') {
                // is (r,c) attacked by BLACK?
                return isSquareAttacked(r, c, false);
            }
            if (!whiteKing && p == 'k') {
                // is (r,c) attacked by WHITE?
                return isSquareAttacked(r, c, true);
            }
        }
    }
    // king not found (shouldn't happen in normal play) treat as not in check
    return false;
}

// Simulate the move and test whether the mover's king would be in check afterwards
bool wouldBeInCheckAfterMove(int sx, int sy, int dx, int dy)
{
    char savedFrom = board[sx][sy];
    char savedTo = board[dx][dy];

    // perform move
    board[dx][dy] = savedFrom;
    board[sx][sy] = ' ';

    // which color's king are we checking? same color as savedFrom
    bool moverIsWhite = isupper(savedFrom);
    bool stillInCheck = isKingInCheck(moverIsWhite);

    // revert
    board[sx][sy] = savedFrom;
    board[dx][dy] = savedTo;

    return stillInCheck;
}

// Try all legal moves for 'whiteKing' color; if any move avoids check, return true
// Note: despite the name, this function simply returns true if there exists any legal move
// for the given color (i.e., any move that is valid and doesn't leave own king in check).
bool canAnyMoveSaveKing(bool whiteKing)
{
    for (int sx = 0; sx < SIZE; ++sx) {
        for (int sy = 0; sy < SIZE; ++sy) {
            char p = board[sx][sy];
            if (p == ' ') continue;
            if (isupper(p) != whiteKing) continue; // only this color's pieces

            for (int dx = 0; dx < SIZE; ++dx) {
                for (int dy = 0; dy < SIZE; ++dy) {
                    // We must test legal moves that don't leave own king in check
                    if (!isValidMove(sx, sy, dx, dy)) continue;
                    // If move is valid, and would not leave king in check, then we can save
                    // (isValidMove already checks wouldBeInCheckAfterMove, so if valid -> safe)
                    return true;
                }
            }
        }
    }
    return false;
}

bool isCheckmate(bool whiteKing)
{
    if (!isKingInCheck(whiteKing)) return false;
    if (canAnyMoveSaveKing(whiteKing)) return false;
    return true;
}
// Console-menu based promotion: if pawn has reached last rank, ask user and replace pawn.
void handlePawnPromotion(int r, int c)
{
    if (!isInsideBoard(r, c)) return;
    char p = board[r][c];
    if (p != 'P' && p != 'p') return; // not a pawn

    // White pawn promotes when reaching row 0
    if (p == 'P' && r != 0) return;
    // Black pawn promotes when reaching row 7
    if (p == 'p' && r != 7) return;

    // Prompt user in console
    cout << "\nPawn Promotion! (console menu)\n";
    cout << "Choose piece:\n";
    cout << "1. Queen\n";
    cout << "2. Rook\n";
    cout << "3. Bishop\n";
    cout << "4. Knight\n";
    cout << "Enter choice (1-4): ";

    int choice = 0;
    while (true) {
        if (!(cin >> choice)) {
            // clear error and ignore rest of line
            cin.clear();
            string skip;
            getline(cin, skip);
            cout << "Invalid input. Enter a number 1-4: ";
            continue;
        }
        if (choice >= 1 && choice <= 4) break;
        cout << "Please enter a valid choice (1-4): ";
    }

    bool isWhite = (p == 'P');
    char newPiece = 'Q';
    switch (choice) {
    case 1: newPiece = (isWhite ? 'Q' : 'q'); break;
    case 2: newPiece = (isWhite ? 'R' : 'r'); break;
    case 3: newPiece = (isWhite ? 'B' : 'b'); break;
    case 4: newPiece = (isWhite ? 'K' : 'k'); break;
    default: newPiece = (isWhite ? 'Q' : 'q'); break;
    }

    board[r][c] = newPiece;
    cout << "Pawn promoted to " << newPiece << "\n";
}
// Stalemate: side to move is NOT in check, but has no legal moves.
bool isStalemate(bool whiteKing)
{
    // If side is in check, it's not stalemate
    if (isKingInCheck(whiteKing)) return false;

    // If there's any legal move for that side, it's not stalemate
    if (canAnyMoveSaveKing(whiteKing)) return false;

    // Not in check and no legal moves => stalemate
    return true;
}