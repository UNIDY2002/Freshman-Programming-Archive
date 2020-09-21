//
// Created by SunXun on 2019/12/24.
//

#ifndef PILL_PILL_H
#define PILL_PILL_H

#include<iostream>
#include<fstream>
#include<cstring>
#include<stdexcept>

namespace SunXun {
    using namespace std;

    typedef int num;

    enum TOKEN {
        EMPTY, DEF, ADD, MOVE, TURN, COLOR, CLOAK, LOOP, END_LOOP, CALL, FUNC, END_FUNC, EXIT
    };

    struct COLORING {
        int R = 0, G = 0, B = 0;
    };

    struct COMMAND {
        TOKEN token = EMPTY;
        COLORING color{};
        string name{}, raw{}, raws[3]{};
        num param = 0;
    } command[1000];

    struct META {
        int width = 0, height = 0;
        COLORING background{};
        int x_pos = 0, y_pos = 0;
        int direction = 0;
        bool visible = true;
    } meta;

    struct VAR {
        string name{};
        num val = 0;
        int special = 0; //used for func returns

        VAR() = default;

        VAR(string Name, num Val, int Special) {
            name = Name, val = Val, special = Special;
        }
    } var_stack[10000];

    int command_count = 0;
    int stack_height = 0;
    int field = 0;
    int jump_list[1000];
    int times[1000] = {};
    int counter[1000] = {};

    struct Search_Result {
        num val = 0;
        int id = -1;

        Search_Result(num Val, int Id) {
            val = Val, id = Id;
        }
    };

    Search_Result search_var(const string &var) {
        for (int i = stack_height - 1; i >= 0; --i) {
            if (var_stack[i].name == var)
                return Search_Result{var_stack[i].val, i};
        }
        return Search_Result{0, -1};
    }

    void define_var(const string &name, num val) {
        var_stack[stack_height].name = name;
        var_stack[stack_height].val = val;
        ++stack_height;
    }

    void edit_var(const string &name, num val) {
        var_stack[search_var(name).id].val = val;
    }

    num parse_raw(const string &raw) {
        if (isalpha(raw[0])) {
            return search_var(raw).val;
        } else {
            return atoi(raw.c_str()); //TODO: currently only supports int
        }
    }

    struct Dictionary {
        string name[12]{};
        int value = 0;
    } func_list[1000];

    int func_count = 0;

    bool is_milestone[10000] = {true};

    void parse(const string &filename) {
        ifstream in(filename.c_str());
        string line, yield;

        // Get meta info
        in >> yield >> meta.width >> meta.height;
        in >> yield >> meta.background.R >> meta.background.G >> meta.background.B;
        in >> yield >> meta.x_pos >> meta.y_pos;

        while (in >> yield) {
            if (yield == "DEF") {
                command[command_count].token = DEF;
                in >> command[command_count].name >> command[command_count].param;
            } else if (yield == "ADD") {
                command[command_count].token = ADD;
                in >> command[command_count].name >> command[command_count].raw;
            } else if (yield == "MOVE") {
                command[command_count].token = MOVE;
                in >> command[command_count].raw;
            } else if (yield == "TURN") {
                command[command_count].token = TURN;
                in >> command[command_count].raw;
            } else if (yield == "COLOR") {
                command[command_count].token = COLOR;
                in >> command[command_count].raws[0]
                   >> command[command_count].raws[1]
                   >> command[command_count].raws[2];
            } else if (yield == "CLOAK") {
                command[command_count].token = CLOAK;
            } else if (yield == "LOOP") {
                command[command_count].token = LOOP;
                in >> times[command_count];
            } else if (yield == "END") {
                in >> yield;
                if (yield == "LOOP") {
                    command[command_count].token = END_LOOP;
                } else if (yield == "FUNC") {
                    command[command_count].token = END_FUNC;
                } else {
                    command[command_count].token = EMPTY;
                }
            } else if (yield == "CALL") {
                command[command_count].token = CALL;
                getline(in, command[command_count].raw);
            } else if (yield == "FUNC") {
                command[command_count].token = FUNC;
                getline(in, command[command_count].raw);
            } else {
                command[command_count].token = EMPTY;
            }
            ++command_count;
        }
    }

    int parse_line(int line) {
        COMMAND current_command = command[line];
        switch (current_command.token) {
            //EMPTY, DEF, ADD, MOVE, TURN, COLOR, CLOAK, LOOP, END_LOOP, CALL, FUNC, END_FUNC, EXIT
            case DEF: {
                define_var(current_command.name, current_command.param);
                return line + 1;
            }
            case ADD: {
                Search_Result result = search_var(current_command.name);
                edit_var(current_command.name, result.val + parse_raw(current_command.raw));
                return line + 1;
            }
            case MOVE: {
                //TODO: move not implemented
                cout << "MOVE " << parse_raw(current_command.raw) << endl;
                return line + 1;
            }
            case TURN: {
                //TODO: turn not implemented
                cout << "TURN " << parse_raw(current_command.raw) << endl;
                return line + 1;
            }
            case COLOR: {
                //TODO: color not implemented
                cout << "COLOR "
                     << parse_raw(current_command.raws[0]) << " "
                     << parse_raw(current_command.raws[1]) << " "
                     << parse_raw(current_command.raws[2]) << endl;
                return line + 1;
            }
            case CLOAK: {
                meta.visible = not meta.visible;
                return line + 1;
            }
            case LOOP: {
                if (jump_list[line] == -1) {
                    int loop_count = 1, temp_line = line + 1;
                    while (loop_count) {
                        if (command[temp_line].token == LOOP)
                            ++loop_count;
                        else if (command[temp_line].token == END_LOOP)
                            --loop_count;
                        ++temp_line;
                    }
                    jump_list[temp_line - 1] = line + 1;
                    times[temp_line - 1] = times[line];
                    counter[temp_line - 1] = 1;
                    jump_list[line] = 0;
                }
                is_milestone[stack_height] = true;
                field = stack_height;
                var_stack[stack_height++].name = "_loop_anonymous"; //Unnecessary
                return line + 1;
            }
            case END_LOOP: {
                ++counter[line];
                stack_height = field;
                if (counter[line] <= times[line]) {
                    ++stack_height;
                    return jump_list[line];
                } else {
                    while (!is_milestone[--field]);
                    is_milestone[stack_height] = false;
                    return line + 1;
                }
            }
            case CALL: {
                string raw = current_command.raw;
                int param_count = 0;
                int start = 0, end;
                string raws[12]{};
                while (true) {
                    while (start < raw.length() && !isalnum(raw[start++]));
                    if (start >= raw.length())
                        break;
                    --start;
                    end = start;
                    while (end < raw.length() && isalnum(raw[end++]));
                    raws[param_count++] = raw.substr(start, end - start - 1);
                    start = end;
                }
                int index;
                for (index = 0; index < func_count; ++index) {
                    if (func_list[index].name[0] == raws[0])
                        break;
                }
                if (index == func_count)
                    throw runtime_error("CRITICAL ERROR: Please contact the manufacturer!");
                is_milestone[stack_height] = true;
                field = stack_height;
                var_stack[stack_height++] = {func_list[index].name[0], 0, line + 1};
                for (int i = 1; i < param_count; ++i)
                    define_var(func_list[index].name[i], parse_raw(raws[i]));
                return func_list[index].value + 1;
            }
            case FUNC: {
                func_list[func_count].value = line;
                string raw = current_command.raw;
                int param_count = 0;
                int start = 0, end;
                while (true) {
                    while (start < raw.length() && !isalpha(raw[start++]));
                    if (start >= raw.length())
                        break;
                    --start;
                    end = start;
                    while (end < raw.length() && isalpha(raw[end++]));
                    func_list[func_count].name[param_count++] = raw.substr(start, end - start - 1);
                    start = end;
                }
                ++func_count;
                int temp_line = line;
                while (temp_line < command_count && command[temp_line++].token != END_FUNC);
                return temp_line;
            }
            case END_FUNC: {
                stack_height = field;
                while (!is_milestone[--field]);
                is_milestone[stack_height] = false;
                return var_stack[stack_height].special;
            }
            default:
                return line + 1;
        }
    }

    void run() {
        int line = 0;     //Note! This line may not be the original line number!
        memset(jump_list, -1, sizeof(jump_list));
        while ((line = parse_line(line)) < command_count);
    }

    void execute(const string &filename) {
        parse(filename);
        run();
    }
}

#endif //PILL_PILL_H
