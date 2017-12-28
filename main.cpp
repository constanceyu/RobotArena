//
//  main.cpp
//  Proj. 7
//
//  Created by Faith Yu on 26/5/2016.
//  Copyright © 2016 Faith Yu. All rights reserved.
//

// robots.cpp

#include <iostream>
#include <string>
#include <algorithm>
#include <cstdlib>
#include <cctype>
#include <ctime>
#include <cassert>
#include <type_traits>
#include <cassert>

using namespace std;

///////////////////////////////////////////////////////////////////////////
// Manifest constants
///////////////////////////////////////////////////////////////////////////

const int MAXROWS = 20;             // max number of rows in the arena
const int MAXCOLS = 20;             // max number of columns in the arena
const int MAXROBOTS = 100;          // max number of robots allowed
const int MAXCHANNELS = 3;          // max number of channels
const int INITIAL_HEALTH = 3;       // initial health of robots
const double WALL_DENSITY = 0.12;   // density of walls

const int NORTH = 0;
const int EAST  = 1;
const int SOUTH = 2;
const int WEST  = 3;
const int NUMDIRS = 4;

const int EMPTY = 0;
const int WALL  = 1;

///////////////////////////////////////////////////////////////////////////
// Type definitions
///////////////////////////////////////////////////////////////////////////

class Arena;  // This is needed to let the compiler know that Arena is a
// type name, since it's mentioned in the Robot declaration.

class Robot
{
public:
    // Constructor
    Robot(Arena* ap, int r, int c, int channel);
    
    // Accessors
    int  row() const;
    int  col() const;
    int  channel() const;
    bool isDead() const;
    
    // Mutators
    void forceMove(int dir);
    void move();
    
private:
    Arena* m_arena;
    int    m_row;
    int    m_col;
    int    m_channel;
    int    m_health;
};

class Player
{
public:
    // Constructor
    Player(Arena *ap, int r, int c);
    
    // Accessors
    int  row() const;
    int  col() const;
    bool isDead() const;
    
    // Mutators
    string stand();
    string move(int dir);
    void   setDead();
    
private:
    Arena* m_arena;
    int    m_row;
    int    m_col;
    bool   m_dead;
};

class Arena
{
public:
    // Constructor/destructor
    Arena(int nRows, int nCols);
    ~Arena();
    
    // Accessors
    int     rows() const;
    int     cols() const;
    Player* player() const;
    int     robotCount() const;
    int     getCellStatus(int r, int c) const;
    int     numberOfRobotsAt(int r, int c) const;
    void    display(string msg) const;
    
    // Mutators
    void   setCellStatus(int r, int c, int status);
    bool   addRobot(int r, int c, int channel);
    bool   addPlayer(int r, int c);
    string moveRobots(int channel, int dir);
    
private:
    int     m_grid[MAXROWS][MAXCOLS];
    int     m_rows;
    int     m_cols;
    Player* m_player;
    Robot*  m_robots[MAXROBOTS];
    int     m_nRobots;
    
    // Helper functions
    void checkPos(int r, int c) const;
};

class Game
{
public:
    // Constructor/destructor
    Game(int rows, int cols, int nRobots);
    ~Game();
    
    // Mutators
    void play();
    
private:
    Arena* m_arena;
    
    // Helper functions
    string takePlayerTurn();
    string takeRobotsTurn();
};

///////////////////////////////////////////////////////////////////////////
//  Auxiliary function declarations
///////////////////////////////////////////////////////////////////////////

int randInt(int lowest, int highest);
int smallest(int a, int b, int c, int d);
char intToChar(int a);
bool charToDir(char ch, int& dir);
bool attemptMove(const Arena& a, int dir, int& r, int& c);
bool recommendMove(const Arena& a, int r, int c, int& bestDir);
void clearScreen();

///////////////////////////////////////////////////////////////////////////
//  Robot implementation
///////////////////////////////////////////////////////////////////////////

Robot::Robot(Arena* ap, int r, int c, int channel)
{
    if (ap == nullptr)
    {
        cout << "***** A robot must be created in some Arena!" << endl;
        exit(1);
    }
    if (r < 1  ||  r > ap->rows()  ||  c < 1  ||  c > ap->cols())
    {
        cout << "***** Robot created with invalid coordinates (" << r << ","
        << c << ")!" << endl;
        exit(1);
    }
    if (channel < 1  ||  channel > MAXCHANNELS)
    {
        cout << "***** Robot created with invalid channel " << channel << endl;
        exit(1);
    }
    m_arena = ap;
    m_row = r;
    m_col = c;
    m_channel = channel;
    m_health = INITIAL_HEALTH;
}

int Robot::row() const
{
    return m_row;
}

int Robot::col() const
{
    return m_col;
}

int Robot::channel() const
{
    return m_channel;
}

bool Robot::isDead() const
{
    // Finished: return whether the robot is dead
    return m_health <= 0;
}


void Robot::forceMove(int dir)
{
    // Finished?:  move the robot in the indicated direction, if possible.
    //        If not, robot suffers 1 unit of damage.
    
    if(attemptMove(*m_arena, dir, m_row, m_col))
    {
        return;
    }
    else
        m_health--;
}

void Robot::move()
{
    // Attempt to move in a random direction; if we can't move, don't move
    if (!isDead())
        attemptMove(*m_arena, randInt(0, NUMDIRS-1), m_row, m_col);
}

///////////////////////////////////////////////////////////////////////////
//  Player implementation
///////////////////////////////////////////////////////////////////////////

Player::Player(Arena* ap, int r, int c)
{
    if (ap == nullptr)
    {
        cout << "***** The player must be created in some Arena!" << endl;
        exit(1);
    }
    if (r < 1  ||  r > ap->rows()  ||  c < 1  ||  c > ap->cols())
    {
        cout << "**** Player created with invalid coordinates (" << r
        << "," << c << ")!" << endl;
        exit(1);
    }
    m_arena = ap;
    m_row = r;
    m_col = c;
    m_dead = false;
}

int Player::row() const
{
    return m_row;
}

int Player::col() const
{
    return m_col;
}

string Player::stand()
{
    return "Player stands.";
}

string Player::move(int dir)
{
    // Finished:  Attempt to move the player one step in the indicated
    //        direction.  If this fails,
    //        return "Player couldn't move; player stands."
    //        A player who moves onto a robot dies, and this
    //        returns "Player walked into a robot and died."
    //        Otherwise, return one of "Player moved north.",
    //        "Player moved east.", "Player moved south.", or
    //        "Player moved west."
    if(attemptMove(*m_arena, dir, m_row, m_col))
    {
        if(m_arena->numberOfRobotsAt(m_row,m_col)!=0)
        {
            setDead();
            return "Player walked into a robot and died.";
        }
        switch(dir)
        {
            case NORTH:
                    return "Player moved north.";
                    break;
            case SOUTH:
                    return "Player moved south.";
                    break;
            case WEST:
                    return "Player moved west.";
                    break;
            case EAST:
                    return "Player moved east.";
                    break;
        }

    }
        return "Player couldn't move; player stands.";
    
}

bool Player::isDead() const
{
    // Finished: return whether the player is dead
    return m_dead;
}

void Player::setDead()
{
    m_dead = true;
}

///////////////////////////////////////////////////////////////////////////
//  Arena implementation
///////////////////////////////////////////////////////////////////////////

Arena::Arena(int nRows, int nCols)
{
    if (nRows <= 0  ||  nCols <= 0  ||  nRows > MAXROWS  ||  nCols > MAXCOLS)
    {
        cout << "***** Arena created with invalid size " << nRows << " by "
        << nCols << "!" << endl;
        exit(1);
    }
    m_rows = nRows;
    m_cols = nCols;
    m_player = nullptr;
    m_nRobots = 0;
    for (int r = 1; r <= m_rows; r++)
        for (int c = 1; c <= m_cols; c++)
            setCellStatus(r, c, EMPTY);
}

Arena::~Arena()
{
    // Finished:  release the player and all remaining dynamically allocated robots
    delete m_player;
    for (int k = 0; k < m_nRobots; k++)
        delete m_robots[k];
}

int Arena::rows() const
{
    return m_rows;
}

int Arena::cols() const
{
    return m_cols;
}

Player* Arena::player() const
{
    return m_player;
}

int Arena::robotCount() const
{
    return m_nRobots;
}

int Arena::getCellStatus(int r, int c) const
{
    checkPos(r, c);
    return m_grid[r-1][c-1];
}

int Arena::numberOfRobotsAt(int r, int c) const
{
    // Finished:  return the number of robots at row r, column c
    int count = 0;
    for(int k = 0; k < m_nRobots; k++)
    {
        const Robot* rp = m_robots[k];
        if(rp->row()==r && rp->col()==c)
            count++;
    }
    return count;
}

void Arena::display(string msg) const
{
    char displayGrid[MAXROWS][MAXCOLS];
    int r, c;
    
    // Fill displayGrid with dots (empty) and stars (wall)
    for (r = 1; r <= rows(); r++)
        for (c = 1; c <= cols(); c++)
            displayGrid[r-1][c-1] = (getCellStatus(r,c) == EMPTY ? '.' : '*');
    
    // Indicate robot positions by their channels.  If more than one robot
    // occupies a cell, show just one (any one will do).
    
    // Finished:  For each robot, set the cell to the digit character
    //        representing the channel number.
    for(int k = 0; k < m_nRobots; k++)
    {
        const Robot* rp = m_robots[k];
        displayGrid[rp->row()-1][rp->col()-1] = intToChar(rp->channel());
    }
    
    
    // Indicate player's position
    if (m_player != nullptr)
        displayGrid[m_player->row()-1][m_player->col()-1] = (m_player->isDead() ? 'X' : '@');
    
    // Draw the grid
    clearScreen();
    for (r = 1; r <= rows(); r++)
    {
        for (c = 1; c <= cols(); c++)
            cout << displayGrid[r-1][c-1];
        cout << endl;
    }
    cout << endl;
    
    // Write message, robot, and player info
    if (msg != "")
        cout << msg << endl;
    cout << "There are " << robotCount() << " robots remaining." << endl;
    if (m_player == nullptr)
        cout << "There is no player!" << endl;
    else if (m_player->isDead())
        cout << "The player is dead." << endl;
}

void Arena::setCellStatus(int r, int c, int status)
{
    checkPos(r, c);
    m_grid[r-1][c-1] = status;
}

bool Arena::addRobot(int r, int c, int channel)
{
    if (m_nRobots == MAXROBOTS)
        return false;
    m_robots[m_nRobots] = new Robot(this, r, c, channel);
    m_nRobots++;
    return true;
}

bool Arena::addPlayer(int r, int c)
{
    if (m_player != nullptr)
        return false;
    m_player = new Player(this, r, c);
    return true;
}

string Arena::moveRobots(int channel, int dir)
{
    // Robots on the channel will respond with probability 1/2
    bool willRespond = (randInt(0, 1) == 0);
    
    // Move all robots
    int nRobotsOriginally = m_nRobots;
    
    // TODO:  Move each robot.  Force robots listening on the channel
    //        to move in the indicated direction if willRespond is true.
    //        If willRespond is false, or if the robot listens on a
    //        different channel, it just moves.  Mark the player as dead
    //        if necessary.  Release any dead dynamically allocated robot.
    
    for(int k = 0; k < m_nRobots; k++)
    {
        Robot* rp = m_robots[k];
        Player* pp = m_player;
        
        if(rp->channel()==channel && willRespond)
            rp->forceMove(dir);
        else
            rp->move();
        
        //mark the player as dead if possible
        if(rp->row()==pp->row() && rp->col()==pp->col())
            pp->setDead();
        
        //dead robots
        if(rp->isDead())
        {
            delete m_robots[k];
            for(int p = k; p < m_nRobots-1; p++)
                m_robots[p]=m_robots[p+1];
            m_nRobots--;
        }
    }
    
    if (m_nRobots < nRobotsOriginally)
        return "Some robots have been destroyed.";
    else
        return "No robots were destroyed.";
}

void Arena::checkPos(int r, int c) const
{
    if (r < 1  ||  r > m_rows  ||  c < 1  ||  c > m_cols)
    {
        cout << "***** " << "Invalid arena position (" << r << ","
        << c << ")" << endl;
        exit(1);
    }
}

///////////////////////////////////////////////////////////////////////////
//  Game implementation
///////////////////////////////////////////////////////////////////////////

Game::Game(int rows, int cols, int nRobots)
{
    if (nRobots < 0  ||  nRobots > MAXROBOTS)
    {
        cout << "***** Game created with invalid number of robots:  "
        << nRobots << endl;
        exit(1);
    }
    int nEmpty = rows * cols - nRobots - 1;  // 1 for Player
    if (nEmpty < 0)
    {
        cout << "***** Game created with a " << rows << " by "
        << cols << " arena, which is too small too hold a player and "
        << nRobots << " robots!" << endl;
        exit(1);
    }
    
    // Create arena
    m_arena = new Arena(rows, cols);
    
    // Add some walls in WALL_DENSITY of the empty spots
    assert(WALL_DENSITY >= 0  &&  WALL_DENSITY <= 1);
    int nWalls = static_cast<int>(WALL_DENSITY * nEmpty);
    while (nWalls > 0)
    {
        int r = randInt(1, rows);
        int c = randInt(1, cols);
        if (m_arena->getCellStatus(r, c) == WALL)
            continue;
        m_arena->setCellStatus(r, c, WALL);
        nWalls--;
    }
    
    // Add player
    int rPlayer;
    int cPlayer;
    do
    {
        rPlayer = randInt(1, rows);
        cPlayer = randInt(1, cols);
    } while (m_arena->getCellStatus(rPlayer, cPlayer) != EMPTY);
    m_arena->addPlayer(rPlayer, cPlayer);
    
    // Populate with robots
    while (nRobots > 0)
    {
        int r = randInt(1, rows);
        int c = randInt(1, cols);
        if (m_arena->getCellStatus(r,c) != EMPTY  ||  (r == rPlayer && c == cPlayer))
            continue;
        m_arena->addRobot(r, c, randInt(1, MAXCHANNELS));
        nRobots--;
    }
}

Game::~Game()
{
    delete m_arena;
}

string Game::takePlayerTurn()
{
    for (;;)
    {
        cout << "Your move (n/e/s/w/x or nothing): ";
        string playerMove;
        getline(cin, playerMove);
        
        Player* player = m_arena->player();
        int dir;
        
        if (playerMove.size() == 0)
        {
            if (recommendMove(*m_arena, player->row(), player->col(), dir))
                return player->move(dir);
            else
                return player->stand();
        }
        else if (playerMove.size() == 1)
        {
            if (tolower(playerMove[0]) == 'x')
                return player->stand();
            else if (charToDir(playerMove[0], dir))
                return player->move(dir);
        }
        cout << "Player move must be nothing, or 1 character n/e/s/w/x." << endl;
    }
}

string Game::takeRobotsTurn()
{
    for (;;)
    {
        cout << "Broadcast (e.g., 2n): ";
        string broadcast;
        getline(cin, broadcast);
        if (broadcast.size() != 2)
            cout << "Broadcast must be channel followed by direction." << endl;
        else if (broadcast[0] < '1'  ||  broadcast[0] > '0'+MAXCHANNELS)
            cout << "Channel must be a valid digit." << endl;
        else
        {
            int dir;
            if (charToDir(broadcast[1], dir))
                return m_arena->moveRobots(broadcast[0]-'0', dir);
            else
                cout << "Direction must be n, e, s, or w." << endl;
        }
    }
}

void Game::play()
{
    m_arena->display("");
    while ( ! m_arena->player()->isDead()  &&  m_arena->robotCount() > 0)
    {
        string msg = takePlayerTurn();
        m_arena->display(msg);
        Player* player = m_arena->player();
        if (player->isDead())
            break;
        msg = takeRobotsTurn();
        m_arena->display(msg);
    }
    if (m_arena->player()->isDead())
        cout << "You lose." << endl;
    else
        cout << "You win." << endl;
}

///////////////////////////////////////////////////////////////////////////
//  Auxiliary function implementation
///////////////////////////////////////////////////////////////////////////

// Return a uniformly distributed random int from lowest to highest, inclusive
int randInt(int lowest, int highest)
{
    if (highest < lowest)
        swap(highest, lowest);
    return lowest + (rand() % (highest - lowest + 1));
}

int smallest(int a, int b, int c, int d)
{
    int small = a;
    
    if(b < small)
        small = b;
    if(c < small)
        small = c;
    if(d < small)
        small = d;
    
    return small;
}

char intToChar(int a)
{
    return a+'0';
}

bool charToDir(char ch, int& dir)
{
    switch (tolower(ch))
    {
        default:  return false;
        case 'n': dir = NORTH; break;
        case 'e': dir = EAST;  break;
        case 's': dir = SOUTH; break;
        case 'w': dir = WEST;  break;
    }
    return true;
}

// Return false without changing anything if moving one step from (r,c)
// in the indicated direction would hit a wall or run off the edge of the
// arena.  Otherwise, update r and c to the position resulting from the
// move and return true.
bool attemptMove(const Arena& a, int dir, int& r, int& c)
{
    // Finished:
    //test if this move is doable, if so, then move
    switch(dir)
    {
        case NORTH:
            if(r == 1 || a.getCellStatus(r-1, c)==WALL)
                return false;
            else
                r--;
            break;
        case SOUTH:
            if(r == a.rows() || a.getCellStatus(r+1, c)==WALL)
                return false;
            else
                r++;
            break;
        case WEST:
            if(c == 1 || a.getCellStatus(r, c-1)==WALL)
                return false;
            else
                c--;
            break;
        case EAST:
            if(c == a.cols() || a.getCellStatus(r, c+1)==WALL)
                return false;
            else
                c++;
            break;
    }
    
    return true;
}

// Recommend a move for a player at (r,c):  A false return means the
// recommendation is that the player should stand; otherwise, bestDir is
// set to the recommended direction to move.
bool recommendMove(const Arena& a, int r, int c, int& bestDir)
{
    
    const int numN = a.numberOfRobotsAt(r-1, c);
    const int numS = a.numberOfRobotsAt(r+1, c);
    const int numE = a.numberOfRobotsAt(r, c+1);
    const int numW = a.numberOfRobotsAt(r, c-1);
    
    //move North:
    const int numNE = a.numberOfRobotsAt(r-1, c+1);
    const int numNW = a.numberOfRobotsAt(r-1, c-1);
    const int numNN = a.numberOfRobotsAt(r-2, c);
    const int moveN = numNE+numNW+numNN;
    
    //move South:
    const int numSE = a.numberOfRobotsAt(r+1, c+1);
    const int numSW = a.numberOfRobotsAt(r+1, c-1);
    const int numSS = a.numberOfRobotsAt(r+2, c);
    const int moveS = numSE+numSW+numSS;
    
    //move East:
    //numNE and numSE
    const int numEE = a.numberOfRobotsAt(r, c+2);
    const int moveE = numNE+numSE+numEE;
    
    //move West:
    //numNW and numSW
    const int numWW = a.numberOfRobotsAt(r, c-2);
    const int moveW = numNW+numSW+numWW;
    
    int small=0;
    
    // TODO:
    //The player is safe at this position and should stand:
    if(numN+numS+numE+numW==0)
        return false;
    //The player is not safe and should move
    else
    {
        small = smallest(moveN, moveS, moveW, moveE);
        if(numN==0 && r!=1 && a.getCellStatus(r-1, c)!=WALL && small==moveN)
        {
            bestDir = NORTH;
            return true;
        }
        if(numS == 0 && r!=a.rows() && a.getCellStatus(r+1, c)!=WALL && small == moveS)
        {
            bestDir = SOUTH;
            return true;
        }
        if(numE == 0 && c!=1 && a.getCellStatus(r, c+1)!=WALL && small == moveE)
        {
            bestDir = EAST;
            return true;
        }
        if(numW == 0 && c!=a.cols() && a.getCellStatus(r, c-1)!=WALL && small == moveW)
        {
            bestDir = WEST;
            return true;
        }
        //the player couldn't move
        return false;
    }
    // Your replacement implementation should do something intelligent.
    // For example, if you're standing next to four robots, and moving
    // north would put you next to two robots, but moving east would put
    // you next to none, moving east is a safer choice than standing or
    // moving north.
}

///////////////////////////////////////////////////////////////////////////
//  main()
///////////////////////////////////////////////////////////////////////////

int main()
{
    // Initialize the random number generator
    srand(static_cast<unsigned int>(time(0)));
    
    // Create a game
    // Use this instead to create a mini-game:   Game g(3, 5, 2);
    Game g(3, 5, 2);
    
    // Play the game
    g.play();
}

///////////////////////////////////////////////////////////////////////////
//  clearScreen implementation
///////////////////////////////////////////////////////////////////////////

// DO NOT MODIFY OR REMOVE ANY CODE BETWEEN HERE AND THE END OF THE FILE!!!
// THE CODE IS SUITABLE FOR VISUAL C++, XCODE, AND g++ UNDER LINUX.

// Note to Xcode users:  clearScreen() will just write a newline instead
// of clearing the window if you launch your program from within Xcode.
// That's acceptable.  (The Xcode output window doesn't have the capability
// of being cleared.)

#ifdef _MSC_VER  //  Microsoft Visual C++

#pragma warning(disable : 4005)
#include <windows.h>

void clearScreen()
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    DWORD dwConSize = csbi.dwSize.X * csbi.dwSize.Y;
    COORD upperLeft = { 0, 0 };
    DWORD dwCharsWritten;
    FillConsoleOutputCharacter(hConsole, TCHAR(' '), dwConSize, upperLeft,
                               &dwCharsWritten);
    SetConsoleCursorPosition(hConsole, upperLeft);
}

#else  // not Microsoft Visual C++, so assume UNIX interface

#include <iostream>
#include <cstring>
#include <cstdlib>

void clearScreen()  // will just write a newline in an Xcode output window
{
    static const char* term = getenv("TERM");
    if (term == nullptr  ||  strcmp(term, "dumb") == 0)
        cout << endl;
    else
    {
        static const char* ESC_SEQ = "\x1B[";  // ANSI Terminal esc seq:  ESC [
        cout << ESC_SEQ << "2J" << ESC_SEQ << "H" << flush;
    }
}

#endif
