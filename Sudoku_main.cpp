#include<iostream>
#include<cstring>
#include<ctime>

using namespace std;

struct Position {
    int x, y;
};

struct Sudoku {
    int array[9][9] = {};
    bool finished = false;
    bool available[9][9][10] = {};
    int freedom[9][9] = {};
    Position min_pos = {};
};

/**
 * @brief Prepares a newly initialized sudoku for further processing.
 *        Besides, returns if any bad block exists.
 *
 * @param sudoku  The sudoku to be prepared.
 * @return Whether any bad block exists.
 *
 * @note If any bad block does exist, this function no longer
 *       ensures that finished, available, etc. are still correct.
 *
 * @author Sun Xun
 */
bool prepare(Sudoku &sudoku) {
    // For safety.
    sudoku.finished = false;

    // Set available.
    memset(sudoku.available, true, sizeof(sudoku.available));
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            if (int t = sudoku.array[i][j]) {
                for (int k = 0; k < 9; k++) {
                    sudoku.available[i][k][t] = sudoku.available[k][j][t]
                            = sudoku.available[(i / 3) * 3 + k % 3][(j / 3) * 3 + k / 3][t] = false;
                }
            }
        }
    }

    // Set freedom and find min_pos.
    int minimum = 10;
    memset(sudoku.freedom, 0, sizeof(sudoku.freedom));
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            if (!sudoku.array[i][j]) {
                for (int t = 1; t <= 9; t++)
                    sudoku.freedom[i][j] += sudoku.available[i][j][t];
                if (sudoku.freedom[i][j] == 0)
                    return false;
                if (minimum > sudoku.freedom[i][j]) {
                    minimum = sudoku.freedom[i][j];
                    sudoku.min_pos = Position{i, j};
                } else if (minimum == sudoku.freedom[i][j]
                           && rand() % 4 == 0) {
                    sudoku.min_pos = Position{i, j};
                }
            }
        }
    }

    return true;
}

/**
 * @brief A temporary function aiming to safely cross out a
 *        possible candidate for a block.
 *
 * @param sudoku  The sudoku to be edited.
 * @param x  The x_pos of the block.
 * @param y  The y_pos of the block.
 * @param num  The candidate to be crossed out.
 * @return Whether the block becomes a bad block.
 *
 * @author Sun Xun
 */
bool safe_decrease(Sudoku &sudoku, int x, int y, int num) {
    if (!sudoku.array[x][y] && sudoku.available[x][y][num]) {
        sudoku.available[x][y][num] = false;
        return --sudoku.freedom[x][y];
    }
    return true;
}

/**
 * @brief Updates finished, available, freedom and min_pos
 *        as position p is filled with number num.
 *        Besides, returns if any bad block appears.
 *
 * @param sudoku  The sudoku to be updated.
 * @param p  The position to be edited.
 * @param num  The number to be filled.
 * @return Whether any bad block appears.
 *
 * @note If any bad block does appear, this function no longer
 *       ensures that finished, available, etc. are still correct.
 *
 * @author Sun Xun
 */
bool edit(Sudoku &sudoku, Position p, int num) {
    sudoku.array[p.x][p.y] = num;

    // Update available and freedom.
    for (int k = 0; k < 9; k++) {
        if (!safe_decrease(sudoku, p.x, k, num) ||
            !safe_decrease(sudoku, k, p.y, num) ||
            !safe_decrease(sudoku, (p.x / 3) * 3 + k % 3, (p.y / 3) * 3 + k / 3, num))
            return false;
    }

    // Find min_pos and check finished.
    int minimum = 10;
    sudoku.finished = true;
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            if (!sudoku.array[i][j]) {
                sudoku.finished = false;
                if (minimum > sudoku.freedom[i][j]) {
                    minimum = sudoku.freedom[i][j];
                    sudoku.min_pos = Position{i, j};
                } else if (minimum == sudoku.freedom[i][j]
                           && rand() % 4 == 0) {
                    sudoku.min_pos = Position{i, j};
                }
            }
        }
    }

    return true;
}

/**
 * @brief 寻找当前数独可能的解的个数 , 在solve函数中被调用
 *
 * @param sudoku  待检查的数独
 * @param ans1, ans2  待接收解的两个结构指针
 * @param solution_cnt  找到的解的个数
 * @return 解的个数
 *
 * @date 05/12/19
 *
 * @author 王通泽
 */
int find_solution(Sudoku sudoku, Sudoku *ans1, Sudoku *ans2, int solution_cnt) {
    for (int i = 1; i <= 9; i++) {  // 尝试填充最小自由度的格子
        Sudoku my_sudoku = sudoku;  // 创建副本供不同的填充尝试使用
        int x = my_sudoku.min_pos.x;
        int y = my_sudoku.min_pos.y;
        if (my_sudoku.available[x][y][i] && edit(my_sudoku, my_sudoku.min_pos, i)) {  // 填充有效
            if (my_sudoku.finished) {  // 判断数独是否填充完成
                solution_cnt++;
                if (solution_cnt == 1)
                    *ans1 = my_sudoku; // 给对应的两个指针赋值
                else
                    *ans2 = my_sudoku;
                return solution_cnt;
            }
            solution_cnt = find_solution(my_sudoku, ans1, ans2, solution_cnt);
            if (solution_cnt == 2)
                return solution_cnt;
        }
    }
    return solution_cnt;
}

/**
 * @brief 调用prepare() 函数初始化数独， 调用 find_solution() 函数寻找解的个数， 返回解的个数
 *
 * @param sudoku  待检查的数独
 * @param ans1, ans2  待接收解的两个结构指针
 * @return 解的个数
 *
 * @author 王通泽
 */
int solve(Sudoku sudoku, Sudoku *ans1 = new Sudoku, Sudoku *ans2 = new Sudoku) {
    if (!prepare(sudoku)) return 0;    // 初始化数独    如果数独无解直接输出
    return find_solution(sudoku, ans1, ans2, 0);
}

/**
 * @brief Generates a random permutation of the unused numbers.
 *
 * @param n  The length of the permutation list.
 * @param p  The array to receive the permutation.
 * @param used  Whether a certain number is used.
 *
 * @author Sun Xun
 */
void random_perm(int n, int p[], bool used[]) {
    for (int i = 0, pos = 0, t; i < n; i++) {
        t = rand() % (n - i) + 1;
        while (t--)
            while (used[pos = pos % 9 + 1]);
        used[p[i] = pos] = true;
    }
}

/**
 * @brief Randomly generates a complete and valid sudoku.
 *
 * @return The sudoku generated.
 *
 * @author Sun Xun
 */
Sudoku shuffle() {
    Sudoku sudoku = Sudoku();
    auto *p = new Sudoku;

    // 1. Generate the top-left block.
    int temp[9] = {};
    bool used[10] = {};
    random_perm(9, temp, used);
    for (int i = 0; i < 9; i++)
        sudoku.array[i / 3][i % 3] = temp[i];

    //2. Complete the first row.
    memset(used, 0, sizeof(used));
    used[sudoku.array[0][0]] =
    used[sudoku.array[0][1]] =
    used[sudoku.array[0][2]] = true;
    random_perm(6, temp, used);
    for (int i = 0; i < 6; i++) {
        sudoku.array[0][i + 3] = temp[i];
    }

    //3. Complete the rest.
    if (solve(sudoku, p))
        return *p;
    else
        return shuffle(); //For robustness.
}

/**
 * @brief Randomly generates a coordinate.
 *
 * @return A random coordinate.
 *
 * @author Lin Jiayin
 */
Position random_position() {
    Position pos{};
    pos.x = rand() % 9;
    pos.y = rand() % 9;
    return pos;
}

/**
 * @brief Randomly hollows a valid Sudoku one coordinate after another.
		  Makes sure it has only 1 solution.
 *
 * @return A sudoku with only 1 solution.
 *
 * @author Lin Jiayin
 */
Sudoku create() {
    Sudoku sudoku = shuffle(); //随机生成一个数独终盘
    int count = 0; //挖空数（>=41）
    int result, temp;
    Position pos{};
    do { //唯一解
        do {
            pos = random_position(); //随机坐标
            temp = sudoku.array[pos.x][pos.y];
        } while (temp == 0); //防止挖重复了
        sudoku.array[pos.x][pos.y] = 0;
        result = solve(sudoku); //解的个数
        count++;
        if (result != 1)
            sudoku.array[pos.x][pos.y] = temp; //恢复成原来的数
    } while (result == 1);
    if (count >= 42) return sudoku;
    else return create();
}

/**
 * @brief Output the given sudoku.
 *
 * @param sudoku Sudoku to be printed.
 *
 * @author Lin Jiayin
 */
void print(Sudoku sudoku) {
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            if (sudoku.array[i][j])
                cout << sudoku.array[i][j] << " ";
            else
                cout << "- ";
        }
        cout << endl;
    }
}

/**
 * @brief verify whether the sudoku received has already existed repetitive numbers in a row,
 *         a column or a 3 by 3 palace that will lead to no solution
 *
 * @param sudoku
 * @return if no repetitive numbers occur in a row,  a column or a 3 by 3 palace , return true
 *          otherwise, return false
 *
 * @author  Wang Tongze
 */
bool valid(Sudoku sudoku) {
    // check any nonzero entry
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            // check if there exists any equal entry in the corresponding row, column and palace
            if (sudoku.array[i][j] != 0) {
                for (int k = 0; k < 9; k++) {
                    if ((k != j && sudoku.array[i][j] == sudoku.array[i][k]) ||
                        (k != i && sudoku.array[i][j] == sudoku.array[k][j]) ||
                        (k != (i % 3) * 3 + (j % 3) &&
                         sudoku.array[i][j] == sudoku.array[(i / 3) * 3 + k / 3][(j / 3) * 3 + k % 3]))
                        return false;
                }
            }
        }
    }
    return true;
}

/**
 * @brief The main host of the solving process.
 *
 * @param sudoku  The sudoku to be processed.
 *
 * @author Sun Xun
 */
void process(Sudoku sudoku) {
    auto *p1 = new Sudoku, *p2 = new Sudoku;
    if (!valid(sudoku)) {
        cout << "No_solution\n";
    } else {
        switch (solve(sudoku, p1, p2)) {
            case 0: {
                cout << "No_solution\n";
                break;
            }
            case 1: {
                cout << "OK\n";
                print(*p1);
                break;
            }
            case 2: {
                cout << "Multiple_solutions\n";
                print(*p1);
                cout << '\n';
                print(*p2);
                break;
            }
            default:
                break;
        }
    }
}

int main() {
    srand(time(nullptr));
    int c;
    cin >> c;
    if (c == 1) {
        print(create());
    } else {
        char ch;
        Sudoku sudoku = Sudoku();
        for (int i = 0; i < 9; i++) {
            for (int j = 0; j < 9; j++) {
                cin >> ch;
                if (ch == '-')
                    sudoku.array[i][j] = 0;
                else
                    sudoku.array[i][j] = ch - '0';
            }
        }
        process(sudoku);
    }
    return 0;
}
