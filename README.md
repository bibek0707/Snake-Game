# Terminal Snake Game (C + Ncurses)

This is a Snake game written in C using the Ncurses library. It runs in the terminal and uses keyboard input to control the snake.

---

## How to Play

- Move the snake using arrow keys or WASD.
- Eat randomly appearing numbers (1–9) to grow and increase your score.
- The game ends if the snake hits itself or the wall.
- You win if your snake reaches half the size of the board's perimeter.

---

## Requirements

### macOS

Install ncurses using Homebrew:

brew install ncurses
```

### Windows

Use Windows Subsystem for Linux (WSL). Then install:

sudo apt update
sudo apt install gcc libncurses5-dev
```

---

## Compile and Run

1. Open your terminal (or WSL on Windows).

2. Navigate to the folder containing the file:

cd path/to/your/folder
```

3. Compile the program:

gcc Snake_Game.c -lncurses -o snake
```

4. Run the game:

./snake
```

