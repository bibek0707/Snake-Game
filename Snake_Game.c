#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

// Structure 'drawableObj' defines the basic building block for the game elements like the snake.
typedef struct drawableObj{
   int y,x; 
   chtype ch; 
} dObj;

// Enumeration 'Direction' for the snake's movement. It includes four directions - up, down, left, and right.
// Each direction is assigned a unique integer value for easy reference and comparison.
enum Direction { 
    up = 0,
    down = 1,
    left = 2,
    right = 3
} currentDirection;

// Function declarations for various functionalities of the game like rendering the board, handling input, 
// and game state updates.
void board(void);
void displayObj(dObj);
void checkInput(void);
void updateState(void);
void updateDisplay(void);
void addSnakePiece(dObj);
void removeSnakePiece(void);
dObj snakeTail(void);
void initializeGame(void);
void setDirection(enum Direction);
dObj nextHead(void);
void getEmptyCoords(int*, int*);
dObj trophy(int, int);
dObj empty(int, int);
void displayMessage(char*);
void exitGame();
chtype getCharAt(int,int);

// Global variables to define the game's board size and control the game's state.
#define BOARD_ROWS (LINES - 1) 
#define BOARD_COLUMNS (COLS - 2)
#define BOARD_HALF_PERIMETER (LINES + COLS - 3) 

bool gameOver, trophyPresent, winGame; 
int xMax, yMax, snakeSize = 5, refreshDelay = 250, randNumber, trophy_time; 
time_t trophyCreationTime;
dObj prevTrophy;

// The 'main' function serves as the entry point of the game. It initializes the game and contains the main game loop.
// The loop continues until a game-over condition is met. Post-game sequences are also handled here.
int main () {
    // Ncurses library setup for window management and keyboard input handling
    initscr(); 
    refresh();
    curs_set(false);
    noecho(); 
    keypad(stdscr, true); 
    signal(SIGINT, exitGame); 

    initializeGame(); 

    // The main game loop which continues until 'gameOver' flag is set
    while (!gameOver) {
        updateState(); 
        checkInput(); 
        refresh(); 
    }

    // Post-game sequences: displaying game over message and handling game exit
    usleep(700000);
    clear(); 
    start_color(); 
    init_pair(1, COLOR_RED, COLOR_BLACK); 
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    attron(A_BOLD); 
    char *scoreMsg; 
    asprintf(&scoreMsg, "Score: %d", snakeSize); 

    // Displaying the game over message
    if (winGame) {
        attron(COLOR_PAIR(2)); 
        displayMessage("You Won!"); 
        sleep(1); 
        displayMessage(scoreMsg); 
        attroff(COLOR_PAIR(2)); 
    }else{
        attron(COLOR_PAIR(1));
        displayMessage("Game Over");
        sleep(1);
        displayMessage(scoreMsg);
        attroff(COLOR_PAIR(1));
    }
    
    usleep(1500000); 
    exitGame(); 
    attroff(A_BOLD); 
}

// 'initializeGame' function sets up the initial game environment, such as the board and the initial state of the snake.
// It also adjusts game settings based on the screen size.
void initializeGame() {
    board(); 
    refreshDelay -= (COLS < 250) ? (COLS/1.3) : 150; 
    srand(time(NULL)); 
    gameOver = false; 
    winGame = false; 
    trophyPresent = false; 

    // Setting up the initial snake
    currentDirection = rand()%4;
    dObj nextSnakePeice = {BOARD_ROWS/2, (BOARD_COLUMNS/2)-2, '0'}; 
    displayObj(nextSnakePeice); 
    addSnakePiece(nextSnakePeice); 

    for (int i = 0; i < 4; i++) {
        nextSnakePeice = nextHead(); 
        displayObj(nextSnakePeice); 
        addSnakePiece(nextSnakePeice); 
    }

    //create the initial trophy
    if (!trophyPresent) {
        int y,x;
        getEmptyCoords(&y,&x);
        displayObj(prevTrophy = trophy(y, x));
        trophyPresent = true;
    }
}

// 'board' function creates the visual boundary of the game area.
// It draws a box around the playable area and sets the refresh rate.
void board() {
    getmaxyx(stdscr, yMax, xMax); 
    box(stdscr, 0, 0); 
    refresh(); 
    timeout(refreshDelay); 
}

// 'displayCharAt' function displays a single character at a specific position on the screen.
void displayCharAt(int yPos, int xPos, chtype ch) {
    mvaddch(yPos, xPos, ch); 
}

// 'displayObj' function displays a 'drawableObj' (like a snake piece) on the screen.
void displayObj(dObj obj) {
    displayCharAt(obj.y, obj.x, obj.ch); 
}

// 'checkInput' function listens for and processes user input.
// It handles arrow keys and WASD for controlling the snake's direction.
void checkInput() {
    chtype input = getch(); 

    switch (input) {
        case KEY_UP:
        case 'w':
            setDirection(up); 
            break;
        case KEY_DOWN:
        case 's':
            setDirection(down);
            break;
        case KEY_RIGHT:
        case 'd':
            setDirection(right); 
            break;
        case KEY_LEFT:
        case 'a':
            setDirection(left); 
            break;
        default:
            break; 
    }
}


// trophy() function creates a trophy of type dObj with 
// a random integer and sets random display time.
dObj trophy(int y, int x) {
    randNumber = (rand()%9)+1;
    trophy_time = (rand()%9)+1;
    trophyCreationTime = time(NULL);
    dObj trophy = {y, x, (randNumber + '0')};
    return trophy;
}

int increaseLengthBy = 0;

// 'updateState' function is responsible for updating the game state.
// It moves the snake, checks for collisions, and updates the game status.
void updateState() {
    dObj nextSnakePeice = nextHead(); 
    int nextY = nextSnakePeice.y, nextX = nextSnakePeice.x; 

    if (getCharAt(nextY, nextX) == ' ') { 
        displayObj(empty(snakeTail().y, snakeTail().x));
        removeSnakePiece();
    }
    else if (getCharAt(nextY, nextX) >= 49 && getCharAt(nextY, nextX) <= 57) { //if snake eats a trophy
        snakeSize += randNumber;
        increaseLengthBy += randNumber;
        trophyPresent = false;
    }
    else {
        gameOver = true;
        displayObj(empty(snakeTail().y, snakeTail().x));
        removeSnakePiece();
    }
    // increases length of the snake
    if (increaseLengthBy > 1) {
        displayObj(nextSnakePeice);
        addSnakePiece(nextSnakePeice);
        increaseLengthBy--;
        if (refreshDelay >= 60) refreshDelay -= 6; //increase snake speed proportionl to size
    }
    displayObj(nextSnakePeice);
    addSnakePiece(nextSnakePeice);

    //Check the time from trophy creation against trophy lifespan
    if((time(NULL) - trophyCreationTime) >= trophy_time){
        if (getCharAt(prevTrophy.y, prevTrophy.x) != '0')
            displayObj(empty(prevTrophy.y, prevTrophy.x));
      trophyPresent = false;
    }

    //Create new trophy when snake eats existing one. 
    if (!trophyPresent) {
        int y,x;
        getEmptyCoords(&y,&x);
        displayObj(prevTrophy = trophy(y, x));
        trophyPresent = true;
    }

    //check if snakeSize reaches half the perimeter.
    if (snakeSize >= BOARD_HALF_PERIMETER) {
        winGame = true;
        gameOver = true;
    }
}

// Queue implementation for handling the snake's body parts.
// It uses a linked list to efficiently add and remove elements.
struct s_node {
    dObj *object; 
    struct s_node *prev; 
    struct s_node *next; 
} *front=NULL, *back=NULL; 
typedef struct s_node node;

dObj* peek() {
    return front ? front->object : NULL; 
}

dObj* dequeue() { 
    node *oldfront = front; 
    front = front->next; 
    return oldfront->object; 
}

// Adds an object to the back of the queue
void enqueue( dObj object ) { 
   dObj *newobject = (dObj*) malloc( sizeof( object ) ); 
   node *newnode = (node*) malloc( sizeof( node ) ); 

   newobject->x = object.x;
   newobject->y = object.y;
   newobject->ch = object.ch;
   newnode->object = newobject;

   if( front == NULL && back == NULL )
       front = back = newnode;
   else {
       back->next = newnode;
       newnode->prev = back;
       back = newnode;
   }
}

dObj* peekBack() {
    return back ? back->object : NULL; 
} 
// End of Queue implementation. 

//Snake editing functions:

// Adds a piece to the snake's body
void addSnakePiece(dObj piece) {
    enqueue(piece); 
} 
// Removes a piece from the snake's body
void removeSnakePiece() {
    dequeue(); 
} 
// Returns the tail of the snake
dObj snakeTail() {
    return *peek(); 
} 
// Returns the head of the snake
dObj snakeHead() {
    return *peekBack(); 
} 

// 'setDirection' function changes the direction of the snake's movement.
// It checks for illegal direction changes (like reversing) and sets the new direction.
void setDirection(enum Direction newDirection) {
    int num = currentDirection + newDirection; 
    if(num == 1 || num == 5) { 
        gameOver = true; 
        displayMessage("Wrong Direction! You ran into yourself."); 
        sleep(2); 
        return; 
    }
    currentDirection = newDirection; 
}

// 'nextHead' function calculates the next position of the snake's head based on the current direction.
// It is used to move the snake on the board.
dObj nextHead() {
    int currRow = snakeHead().y; 
    int currCol = snakeHead().x; 

    switch (currentDirection) {
        case down: 
            currRow++; 
            break; 
        case up: 
            currRow--; 
            break; 
        case left: 
            currCol--; 
            break; 
        case right: 
            currCol++; 
            break; 
    }
    dObj newSnakeHead = {currRow, currCol, '0'};    
    return newSnakeHead;
}

//empty() function creates a blank object to 'erase' characters on board. 
dObj empty(int y, int x) {
    dObj blank = {y, x, ' '};
    return blank;
} 
//getCharAt() function gets character at a certain location on board. 
chtype getCharAt(int y, int x) {
    return mvinch(y, x); 
} 

// getEmptyCoords() function gets a set of random coords within
// the border for trophy.
void getEmptyCoords(int *y, int *x) { 
    while (getCharAt(*y = rand() % (BOARD_ROWS-1), *x = rand() % (BOARD_COLUMNS-1)) != ' ');
}

// 'displayMessage' function is used for showing messages on the board.
// It clears a line and then displays the given message in the center.
void displayMessage(char* str) {
    move(BOARD_ROWS/2, 5); 
    hline(' ', BOARD_COLUMNS-5);
    move(BOARD_ROWS/2, (BOARD_COLUMNS - strlen(str)) / 2); 
    printw("%s", str); 
    refresh(); 
}

// Exit the game cleanly
void exitGame() {
    displayMessage("Exiting");
    usleep(1300000);
    endwin();
    exit(0);
}
