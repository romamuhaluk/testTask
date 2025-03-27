#include <iostream>
#include <vector>
#include <random>
#include <time.h>

/*
You are given a locked container represented as a two-dimensional grid of boolean values (true = locked, false = unlocked).
Your task is to write an algorithm that fully unlocks the box, i.e.,
transforms the entire matrix into all false.

Implement the function:
bool openBox(uint32_t y, uint32_t x);
This function should:
    - Use the SecureBox public API (toggle, isLocked, getState).
    - Strategically toggle cells to reach a state where all elements are false.
    - Return true if the box remains locked, false if successfully unlocked.
You are not allowed to path or modify the SecureBox class.

Evaluation Criteria:
    - Functional correctness
    - Computational efficiency
    - Code quality, structure, and comments
    - Algorithmic insight and clarity
*/

class SecureBox
{
private:
    std::vector<std::vector<bool>> box;

public:

    //================================================================================
    // Constructor: SecureBox
    // Description: Initializes the secure box with a given size and 
    //              shuffles its state using a pseudo-random number generator 
    //              seeded with current time.
    //================================================================================
    SecureBox(uint32_t y, uint32_t x) : ySize(y), xSize(x)
    {
        rng.seed(time(0));
        box.resize(y);
        for (auto& it : box)
            it.resize(x);
        shuffle();
    }

    //================================================================================
    // Method: toggle
    // Description: Toggles the state at position (x, y) and also all cells in the
    //              same row above and the same column to the left of it.
    //================================================================================
    void toggle(uint32_t y, uint32_t x)
    {
        box[y][x] = !box[y][x];
        for (uint32_t i = 0; i < xSize; i++)
            box[y][i] = !box[y][i];
        for (uint32_t i = 0; i < ySize; i++)
            box[i][x] = !box[i][x];
    }

    //================================================================================
    // Method: isLocked
    // Description: Returns true if any cell 
    //              in the box is true (locked); false otherwise.
    //================================================================================
    bool isLocked()
    {
        for (uint32_t x = 0; x < xSize; x++)
            for (uint32_t y = 0; y < ySize; y++)
                if (box[y][x])
                    return true;

        return false;
    }

    //================================================================================
    // Method: getState
    // Description: Returns a copy of the current state of the box.
    //================================================================================
    std::vector<std::vector<bool>> getState()
    {
        return box;
    }

private:
    std::mt19937_64 rng;
    uint32_t ySize, xSize;

    //================================================================================
    // Method: shuffle
    // Description: Randomly toggles cells in the box to 
    // create an initial locked state.
    //================================================================================
    void shuffle()
    {
        for (uint32_t t = rng() % 1000; t > 0; t--)
            toggle(rng() % ySize, rng() % xSize);
    }
};

//================================================================================
// Function: openBox
// Description: Your task is to implement this function to unlock the SecureBox.
//              Use only the public methods of SecureBox (toggle, getState, isLocked).
//              You must determine the correct sequence of toggle operations to make
//              all values in the box 'false'. The function should return false if
//              the box is successfully unlocked, or true if any cell remains locked.
//================================================================================

// Метод Гауса для вирішення системи рівнянь у полі GF(2)
void gaussGF2(std::vector<std::vector<bool>>& mat, std::vector<bool>& result, uint32_t rows, uint32_t cols)
{
    // Перетворення на верхню трикутну матрицю
    for (uint32_t col = 0; col < cols; ++col)
    {
        uint32_t rowToSwap = col;
        for (uint32_t row = col; row < rows; ++row)
        {
            if (mat[row][col])
            {
                rowToSwap = row;
                break;
            }
        }

        if (mat[rowToSwap][col] == 0)
            continue; // Якщо в стовпці немає 1, пропускаємо стовпець

        // Обмінюємо рядки
        if (rowToSwap != col)
        {
            for (uint32_t j = 0; j < mat[0].size(); ++j)
            {
                bool temp = mat[rowToSwap][j];
                mat[rowToSwap][j] = mat[col][j];
                mat[col][j] = temp;
            }

            bool tempResult = result[rowToSwap];
            result[rowToSwap] = result[col];
            result[col] = tempResult;
        }

        // Застосовуємо XOR для всіх нижчих рядків, щоб знищити елементи в стовпці
        for (uint32_t row = col + 1; row < rows; ++row)
        {
            if (mat[row][col])
            {
                for (uint32_t j = col; j < cols; ++j)
                    mat[row][j] = mat[row][j] ^ mat[col][j]; // XOR рядка
                result[row] = result[row] ^ result[col]; // XOR правої частини
            }
        }
    }

    // Зворотна підстановка
    for (int32_t row = rows - 1; row >= 0; --row)
    {
        for (uint32_t col = row + 1; col < cols; ++col)
        {
            if (mat[row][col])
                result[row] = result[row] ^ result[col]; // XOR правої частини
        }
    }
}

// Основна функція для відкриття коробки
bool openBox(uint32_t y, uint32_t x)
{
    SecureBox box(y, x);

    std::vector<std::vector<bool>> mat(y, std::vector<bool>(x, 0));
    std::vector<bool> result(y, 0);

    // Створюємо матрицю залежностей з поточного стану коробки
    for (uint32_t i = 0; i < y; ++i)
    {
        for (uint32_t j = 0; j < x; ++j)
        {
            if (box.getState()[i][j])
                mat[i][j] = 1;  // Якщо клітинка заблокована, ставимо 1
        }
    }

    // Використовуємо метод Гауса для вирішення системи рівнянь у GF(2)
    gaussGF2(mat, result, y, x);

    // Застосовуємо перемикання для клітинок, де результат == 1
    for (uint32_t i = 0; i < y; ++i)
    {
        if (result[i] == 1)
        {
            for (uint32_t j = 0; j < x; ++j)
            {
                box.toggle(i, j);  // Перемикаємо всі клітинки в рядку
            }
        }
    }

    // Перевіряємо, чи розблоковано контейнер
    return !box.isLocked();  // Повертаємо true, якщо коробка розблокована
}

int main(int argc, char* argv[])
{
    uint32_t y = std::atol(argv[1]);
    uint32_t x = std::atol(argv[2]);
    bool state = openBox(y, x);

    if (state)
        std::cout << "BOX: LOCKED!" << std::endl;
    else
        std::cout << "BOX: OPENED!" << std::endl;

    return state;
}

