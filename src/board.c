#include "board.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>
#include <time.h>

FILE * debugfile;


// Helper private function to find and kill pacman at specific position
static int find_and_kill_pacman(board_t* board, int new_x, int new_y) {
    for (int p = 0; p < board->n_pacmans; p++) {
        pacman_t* pac = &board->pacmans[p];
        if (pac->pos_x == new_x && pac->pos_y == new_y && pac->alive) {
            pac->alive = 0;
            board->pacman_dead = 1;
            kill_pacman(board, p);
            return DEAD_PACMAN;
        }
    }
    return VALID_MOVE;
}

// Helper private function for getting board position index
static inline int get_board_index(board_t* board, int x, int y) {
    return y * board->width + x;
}

// Helper private function for checking valid position
static inline int is_valid_position(board_t* board, int x, int y) {
    return (x >= 0 && x < board->width) && (y >= 0 && y < board->height);
}

void sleep_ms(int milliseconds) {
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&ts, NULL);
}

int move_pacman(board_t* board, int pacman_index, command_t* command) {
    if (pacman_index < 0 || !board->pacmans[pacman_index].alive) {
        return DEAD_PACMAN;
    }

    pacman_t* pac = &board->pacmans[pacman_index];
    int new_x = pac->pos_x;
    int new_y = pac->pos_y;

    if (pac->waiting > 0) {
        pac->waiting -= 1;
        return VALID_MOVE;        
    }
    pac->waiting = pac->passo;

    char direction = command->command;

    if (direction == 'R') {
        char directions[] = {'W', 'S', 'A', 'D'};
        direction = directions[rand() % 4];
    }

    switch (direction) {
        case 'W': new_y--; break;
        case 'S': new_y++; break;
        case 'A': new_x--; break;
        case 'D': new_x++; break;
        case 'T':
            if (command->turns_left == 1) {
                pac->current_move += 1;
                command->turns_left = command->turns;
            }
            else command->turns_left -= 1;
            return VALID_MOVE;
        default:
            return INVALID_MOVE;
    }

    pac->current_move+=1;

    if (!is_valid_position(board, new_x, new_y)) {
        return INVALID_MOVE;
    }

    int new_index = get_board_index(board, new_x, new_y);
    int old_index = get_board_index(board, pac->pos_x, pac->pos_y);
    char target_content = board->board[new_index].content;

    if (board->board[new_index].has_portal) {
        board->board[old_index].content = ' ';
        board->board[new_index].content = 'P';
        board->portal_reached = 1;
        return REACHED_PORTAL;
    }

    if (target_content == 'W') {
        return INVALID_MOVE;
    }

    if (target_content == 'M') {
        board->pacman_dead = 1;
        kill_pacman(board, pacman_index);
        return DEAD_PACMAN;
    }

    if (board->board[new_index].has_dot) {
        pac->points++;
        board->board[new_index].has_dot = 0;
    }

    board->board[old_index].content = ' ';
    pac->pos_x = new_x;
    pac->pos_y = new_y;
    board->board[new_index].content = 'P';

    return VALID_MOVE;
}

static int move_ghost_charged_direction(board_t* board, ghost_t* ghost, char direction, int* new_x, int* new_y) {
    int x = ghost->pos_x;
    int y = ghost->pos_y;
    *new_x = x;
    *new_y = y;
    
    switch (direction) {
        case 'W':
            if (y == 0) return INVALID_MOVE;
            *new_y = 0;
            for (int i = y - 1; i >= 0; i--) {
                char target_content = board->board[get_board_index(board, x, i)].content;
                if (target_content == 'W' || target_content == 'M') {
                    *new_y = i + 1;
                    return VALID_MOVE;
                }
                else if (target_content == 'P') {
                    *new_y = i;
                    return find_and_kill_pacman(board, *new_x, *new_y);
                }
            }
            break;
        case 'S':
            if (y == board->height - 1) return INVALID_MOVE;
            *new_y = board->height - 1;
            for (int i = y + 1; i < board->height; i++) {
                char target_content = board->board[get_board_index(board, x, i)].content;
                if (target_content == 'W' || target_content == 'M') {
                    *new_y = i - 1;
                    return VALID_MOVE;
                }
                if (target_content == 'P') {
                    *new_y = i;
                    return find_and_kill_pacman(board, *new_x, *new_y);
                }
            }
            break;
        case 'A':
            if (x == 0) return INVALID_MOVE;
            *new_x = 0;
            for (int j = x - 1; j >= 0; j--) {
                char target_content = board->board[get_board_index(board, j, y)].content;
                if (target_content == 'W' || target_content == 'M') {
                    *new_x = j + 1;
                    return VALID_MOVE;
                }
                if (target_content == 'P') {
                    *new_x = j;
                    return find_and_kill_pacman(board, *new_x, *new_y);
                }
            }
            break;
        case 'D':
            if (x == board->width - 1) return INVALID_MOVE;
            *new_x = board->width - 1;
            for (int j = x + 1; j < board->width; j++) {
                char target_content = board->board[get_board_index(board, j, y)].content;
                if (target_content == 'W' || target_content == 'M') {
                    *new_x = j - 1;
                    return VALID_MOVE;
                }
                if (target_content == 'P') {
                    *new_x = j;
                    return find_and_kill_pacman(board, *new_x, *new_y);
                }
            }
            break;
        default:
            debug("DEFAULT CHARGED MOVE - direction = %c\n", direction);
            return INVALID_MOVE;
    }
    return VALID_MOVE;
}   

int move_ghost_charged(board_t* board, int ghost_index, char direction) {
    ghost_t* ghost = &board->ghosts[ghost_index];
    int new_x = ghost->pos_x;
    int new_y = ghost->pos_y;

    ghost->charged = 0;
    int result = move_ghost_charged_direction(board, ghost, direction, &new_x, &new_y);
    if (result == INVALID_MOVE) {
        return INVALID_MOVE;
    }

    int old_index = get_board_index(board, ghost->pos_x, ghost->pos_y);
    int new_index = get_board_index(board, new_x, new_y);

    board->board[old_index].content = ' ';
    ghost->pos_x = new_x;
    ghost->pos_y = new_y;
    board->board[new_index].content = 'M';
    return result;
}

int move_ghost(board_t* board, int ghost_index, command_t* command) {
    ghost_t* ghost = &board->ghosts[ghost_index];
    int new_x = ghost->pos_x;
    int new_y = ghost->pos_y;

    if (ghost->waiting > 0) {
        ghost->waiting -= 1;
        return VALID_MOVE;
    }
    ghost->waiting = ghost->passo;

    char direction = command->command;
    
    if (direction == 'R') {
        char directions[] = {'W', 'S', 'A', 'D'};
        direction = directions[rand() % 4];
    }

    switch (direction) {
        case 'W': new_y--; break;
        case 'S': new_y++; break;
        case 'A': new_x--; break;
        case 'D': new_x++; break;
        case 'C':
            ghost->current_move += 1;
            ghost->charged = 1;
            return VALID_MOVE;
        case 'T':
            if (command->turns_left == 1) {
                ghost->current_move += 1;
                command->turns_left = command->turns;
            }
            else command->turns_left -= 1;
            return VALID_MOVE;
        default:
            return INVALID_MOVE;
    }

    ghost->current_move++;
    if (ghost->charged)
        return move_ghost_charged(board, ghost_index, direction);

    if (!is_valid_position(board, new_x, new_y)) {
        return INVALID_MOVE;
    }

    int new_index = get_board_index(board, new_x, new_y);
    int old_index = get_board_index(board, ghost->pos_x, ghost->pos_y);
    char target_content = board->board[new_index].content;

    if (target_content == 'W' || target_content == 'M') {
        return INVALID_MOVE;
    }

    int result = VALID_MOVE;
    if (target_content == 'P') {
        result = find_and_kill_pacman(board, new_x, new_y);
    }

    board->board[old_index].content = ' ';
    ghost->pos_x = new_x;
    ghost->pos_y = new_y;
    board->board[new_index].content = 'M';
    return result;
}

void kill_pacman(board_t* board, int pacman_index) {
    debug("Killing %d pacman\n\n", pacman_index);
    pacman_t* pac = &board->pacmans[pacman_index];
    int index = pac->pos_y * board->width + pac->pos_x;
    board->board[index].content = ' ';
    pac->alive = 0;
}

static int ends_with(const char *str, const char *suffix) {
    size_t str_len = strlen(str);
    size_t suffix_len = strlen(suffix);
    if (suffix_len > str_len) return 0;
    return strcmp(str + str_len - suffix_len, suffix) == 0;
}

static int compare_strings(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

static int read_line(int fd, char *buffer, size_t max_size) {
    size_t i = 0;
    char c;
    
    while (i < max_size - 1) {
        if (read(fd, &c, 1) <= 0) break;
        if (c == '\n') break;
        buffer[i++] = c;
    }
    
    buffer[i] = '\0';
    return i > 0 ? (int) i : -1;
}

int load_pacman(board_t *board, const char *directory, const char *filename, int points) {
    char path[512];
    snprintf(path, sizeof(path), "%s/%s", directory, filename);
    
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        debug("Erro ao abrir ficheiro Pacman: %s\n", path);
        return -1;
    }
    
    char line[256];
    int passo = 0, pos_x = 1, pos_y = 1, n_moves = 0;
    
    while (read_line(fd, line, sizeof(line)) > 0) {
        if (line[0] == '#' || line[0] == '\0') continue;
        
        char *ptr = line;
        while (*ptr == ' ' || *ptr == '\t') ptr++;
        
        if (strncmp(ptr, "PASSO", 5) == 0) {
            sscanf(ptr + 5, "%d", &passo);
        }
        else if (strncmp(ptr, "POS", 3) == 0) {
            sscanf(ptr + 3, "%d %d", &pos_y, &pos_x);
        }
        else if (strlen(ptr) > 0 && n_moves < MAX_MOVES) {
            char cmd = ptr[0];
            if (cmd == 'A' || cmd == 'D' || cmd == 'W' || cmd == 'S' || cmd == 'R' || cmd == 'C' || cmd == 'Q' || cmd == 'G') {
                board->pacmans[0].moves[n_moves].command = cmd;
                board->pacmans[0].moves[n_moves].turns = 1;
                board->pacmans[0].moves[n_moves].turns_left = 1;
                n_moves++;
            }
            else if (cmd == 'T') {
                int wait = 1;
                sscanf(ptr + 1, "%d", &wait);
                board->pacmans[0].moves[n_moves].command = 'T';
                board->pacmans[0].moves[n_moves].turns = wait;
                board->pacmans[0].moves[n_moves].turns_left = wait;
                n_moves++;
            }
        }
    }
    close(fd);
    
    board->pacmans[0].passo = passo;
    board->pacmans[0].waiting = passo;
    board->pacmans[0].pos_x = pos_x;
    board->pacmans[0].pos_y = pos_y;
    board->pacmans[0].n_moves = n_moves;
    board->pacmans[0].alive = 1;
    board->pacmans[0].points = points;
    board->pacmans[0].current_move = 0;
    
    int idx = pos_y * board->width + pos_x;
    board->board[idx].content = 'P';
    board->board[idx].has_dot = 0;
    
   return 0;
}

int load_ghost(board_t *board, int ghost_index, const char *directory, const char *filename) {
    char path[512];
    snprintf(path, sizeof(path), "%s/%s", directory, filename);
    
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        debug("Erro ao abrir ficheiro Monstro: %s\n", path);
        return -1;
    }
    
    char line[256];
    int passo = 0, pos_x = 1, pos_y = 1, n_moves = 0;
    
    while (read_line(fd, line, sizeof(line)) > 0) {
        if (line[0] == '#' || line[0] == '\0') continue;
        
        char *ptr = line;
        while (*ptr == ' ' || *ptr == '\t') ptr++;
        
        if (strncmp(ptr, "PASSO", 5) == 0) {
            sscanf(ptr + 5, "%d", &passo);
        }
        else if (strncmp(ptr, "POS", 3) == 0) {
            sscanf(ptr + 3, "%d %d", &pos_y, &pos_x);
        }
        else if (strlen(ptr) > 0 && n_moves < MAX_MOVES) {
            char cmd = ptr[0];
            if (cmd == 'A' || cmd == 'D' || cmd == 'W' || cmd == 'S' || cmd == 'R' || cmd == 'C') {
                board->ghosts[ghost_index].moves[n_moves].command = cmd;
                board->ghosts[ghost_index].moves[n_moves].turns = 1;
                board->ghosts[ghost_index].moves[n_moves].turns_left = 1;
                n_moves++;
            }
            else if (cmd == 'T') {
                int wait = 1;
                sscanf(ptr + 1, "%d", &wait);
                board->ghosts[ghost_index].moves[n_moves].command = 'T';
                board->ghosts[ghost_index].moves[n_moves].turns = wait;
                board->ghosts[ghost_index].moves[n_moves].turns_left = wait;
                n_moves++;
            }
        }
    }
    close(fd);
    
    board->ghosts[ghost_index].passo = passo;
    board->ghosts[ghost_index].waiting = passo;
    board->ghosts[ghost_index].pos_x = pos_x;
    board->ghosts[ghost_index].pos_y = pos_y;
    board->ghosts[ghost_index].n_moves = n_moves;
    board->ghosts[ghost_index].current_move = 0;
    board->ghosts[ghost_index].charged = 0;
    
    int idx = pos_y * board->width + pos_x;
    board->board[idx].content = 'M';
    board->board[idx].has_dot = 0;
    
    return 0;
}

int load_level(board_t *board, const char *directory, int level_index, int points) {
    DIR *dir = opendir(directory);
    if (!dir) {
        debug("Erro ao abrir diretoria: %s\n", directory);
        return -1;
    }
    
    char **level_files = NULL;
    int n_levels = 0;
    int capacity = 8;
    level_files = malloc(capacity * sizeof(char *));
    
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (ends_with(entry->d_name, ".lvl")) {
            if (n_levels >= capacity) {
                capacity *= 2;
                level_files = realloc(level_files, capacity * sizeof(char *));
            }
            level_files[n_levels] = malloc(strlen(entry->d_name) + 1);
            strcpy(level_files[n_levels], entry->d_name);
            n_levels++;
        }
    }
    closedir(dir);
    
    if (n_levels == 0) {
        free(level_files);
        return -1;
    }
    
    qsort(level_files, n_levels, sizeof(char *), compare_strings);
    
    if (level_index >= n_levels) {
        for (int i = 0; i < n_levels; i++) free(level_files[i]);
        free(level_files);
        return -1;
    }
    
    char level_path[512];
    snprintf(level_path, sizeof(level_path), "%s/%s", directory, level_files[level_index]);
    
    int fd = open(level_path, O_RDONLY);
    if (fd < 0) {
        for (int i = 0; i < n_levels; i++) free(level_files[i]);
        free(level_files);
        return -1;
    }
    
    char line[256];
    char pacman_file[256] = {0};
    char ghost_files[MAX_GHOSTS][256];
    int n_ghosts = 0;
    char **map_lines = NULL;
    int map_height = 0;
    int map_capacity = 16;
    
    board->width = 0;
    board->height = 0;
    board->tempo = 0;
    
    while (read_line(fd, line, sizeof(line)) > 0) {
        if (line[0] == '#' || line[0] == '\0') continue;
        
        char *ptr = line;
        while (*ptr == ' ' || *ptr == '\t') ptr++;
        
        if (strncmp(ptr, "DIM", 3) == 0) {
            sscanf(ptr + 3, "%d %d", &board->height, &board->width);
        }
        else if (strncmp(ptr, "TEMPO", 5) == 0) {
            sscanf(ptr + 5, "%d", &board->tempo);
        }
        else if (strncmp(ptr, "PAC", 3) == 0) {
            sscanf(ptr + 3, "%s", pacman_file);
        }
        else if (strncmp(ptr, "MON", 3) == 0) {
            char *token = strtok(ptr + 3, " \t");
            while (token != NULL && n_ghosts < MAX_GHOSTS) {
                strcpy(ghost_files[n_ghosts], token);
                n_ghosts++;
                token = strtok(NULL, " \t");
            }
        }
        else if (ptr[0] == 'X' || ptr[0] == 'o' || ptr[0] == '@' || ptr[0] == ' ') {
            if (map_lines == NULL) {
                map_lines = malloc(map_capacity * sizeof(char *));
            }
            if (map_height >= map_capacity) {
                map_capacity *= 2;
                map_lines = realloc(map_lines, map_capacity * sizeof(char *));
            }
            map_lines[map_height] = malloc(strlen(ptr) + 1);
            strcpy(map_lines[map_height], ptr);
            map_height++;
        }
    }
    close(fd);
    
    board->board = calloc(board->width * board->height, sizeof(board_pos_t));
    board->n_pacmans = 1;
    board->pacmans = calloc(1, sizeof(pacman_t));
    board->n_ghosts = n_ghosts;
    board->ghosts = calloc(n_ghosts, sizeof(ghost_t));
    
    strncpy(board->level_name, level_files[level_index], sizeof(board->level_name) - 1);
    
    for (int y = 0; y < map_height && y < board->height; y++) {
       
        for (int x = 0; x < (int)strlen(map_lines[y]) && x < board->width; x++) {
            int idx = y * board->width + x;
            char c = map_lines[y][x];
            
            if (c == 'X') {
                board->board[idx].content = 'W';
                board->board[idx].has_dot = 0;
            } else if (c == '@') {
                board->board[idx].content = ' ';
                board->board[idx].has_portal = 1;
                board->board[idx].has_dot = 0;
            } else if (c == 'o') {
                board->board[idx].content = ' ';
                board->board[idx].has_dot = 1;
            } else {
                board->board[idx].content = ' ';
                board->board[idx].has_dot = 0;
            }
        }
        free(map_lines[y]);
    }
    free(map_lines);
    
    if (strlen(pacman_file) > 0) {
        load_pacman(board, directory, pacman_file, points);
        strncpy(board->pacman_file, pacman_file, sizeof(board->pacman_file) - 1);
    } else {
        int start_x = 1, start_y = 1;
        int found = 0;

        for (int y = 0; y < board->height && !found; y++) {
            for (int x = 0; x < board->width && !found; x++) {
                int idx = y * board->width + x;
                if (board->board[idx].content == ' ' && !board->board[idx].has_portal) {
                    start_x = x;
                    start_y = y;
                    found = 1;
                }
            }
        }

        if (!found) {
            for (int y = 0; y < board->height && !found; y++) {
                for (int x = 0; x < board->width && !found; x++) {
                    int idx = y * board->width + x;
                    if (board->board[idx].content != 'W') {
                        start_x = x;
                        start_y = y;
                        found = 1;
                    }
                }
            }
        }

        board->pacmans[0].n_moves = 0;
        board->pacmans[0].passo = 0;
        board->pacmans[0].waiting = 0;
        board->pacmans[0].pos_x = start_x;
        board->pacmans[0].pos_y = start_y;
        board->pacmans[0].alive = 1;
        board->pacmans[0].points = points;
        board->pacmans[0].current_move = 0;

        int idx = start_y * board->width + start_x;
        board->board[idx].content = 'P';
        board->board[idx].has_dot = 0;
        strcpy(board->pacman_file, "manual");

    }
    
    for (int i = 0; i < n_ghosts; i++) {
        load_ghost(board, i, directory, ghost_files[i]);
        strncpy(board->ghosts_files[i], ghost_files[i], sizeof(board->ghosts_files[i]) - 1);
    }
    
    for (int i = 0; i < n_levels; i++) free(level_files[i]);
    free(level_files);
    
    print_board(board);
    
    return 0;
}

void unload_level(board_t * board) {
    free(board->board);
    free(board->pacmans);
    free(board->ghosts);
}

void open_debug_file(char *filename) {
    debugfile = fopen(filename, "w");
}

void close_debug_file() {
    fclose(debugfile);
}

void debug(const char * format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(debugfile, format, args);
    va_end(args);
    fflush(debugfile);
}

void print_board(board_t *board) {
    if (!board || !board->board) {
        debug("[%d] Board is empty or not initialized.\n", getpid());
        return;
    }

    char buffer[8192];
    size_t offset = 0;

    offset += snprintf(buffer + offset, sizeof(buffer) - offset,
                       "=== [%d] LEVEL INFO ===\n"
                       "Dimensions: %d x %d\n"
                       "Tempo: %d\n"
                       "Pacman file: %s\n",
                       getpid(), board->height, board->width, board->tempo, board->pacman_file);

    offset += snprintf(buffer + offset, sizeof(buffer) - offset,
                       "Monster files (%d):\n", board->n_ghosts);

    for (int i = 0; i < board->n_ghosts; i++) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset,
                           "  - %s\n", board->ghosts_files[i]);
    }

    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "\n=== BOARD ===\n");

    for (int y = 0; y < board->height; y++) {
        for (int x = 0; x < board->width; x++) {
            int idx = y * board->width + x;
            if (offset < sizeof(buffer) - 2) {
                buffer[offset++] = board->board[idx].content;
            }
        }
        if (offset < sizeof(buffer) - 2) {
            buffer[offset++] = '\n';
        }
    }

    offset += snprintf(buffer + offset, sizeof(buffer) - offset, "==================\n");
    buffer[offset] = '\0';
    debug("%s", buffer);
}


void* pacman_thread_func(void* arg) {
    thread_arg_t* targ = (thread_arg_t*)arg;
    board_t* board = targ->board;
    int pacman_index = targ->index;
    free(targ);
    
    pacman_t* pac = &board->pacmans[pacman_index];
    
    while (board->game_running && pac->alive) {
        pthread_rwlock_wrlock(&board->board_lock);
        
        if (!board->game_running || !pac->alive) {
            pthread_rwlock_unlock(&board->board_lock);
            break;
        }
        
        command_t* cmd = &pac->moves[pac->current_move % pac->n_moves];
        
        // Processar comandos especiais
        if (cmd->command == 'Q') {
            board->game_running = 0;
            board->pacman_dead = 0;  // Q não é morte, é saída voluntária
            pac->current_move++;
            pthread_rwlock_unlock(&board->board_lock);
            break;
        }
        
        if (cmd->command == 'G') {
            // Sinalizar pedido de backup
            board->portal_reached = 2;  // Usa 2 para indicar pedido de backup
            pthread_rwlock_unlock(&board->board_lock);
            sleep_ms(board->tempo);
            
            // Esperar que o backup seja processado
            while (board->portal_reached == 2 && board->game_running) {
                sleep_ms(board->tempo);
            }
            continue;
        }
        
        int result = move_pacman(board, pacman_index, cmd);
        
        pthread_rwlock_unlock(&board->board_lock);
        
        if (result == REACHED_PORTAL) {
            board->portal_reached = 1;
            board->game_running = 0;
            break;
        }
        
        if (result == DEAD_PACMAN) {
            board->pacman_dead = 1;
            board->game_running = 0;
            break;
        }
        
        sleep_ms(board->tempo * 2);
    }
    
    return NULL;
}


void* ghost_thread_func(void* arg) {
    thread_arg_t* targ = (thread_arg_t*)arg;
    board_t* board = targ->board;
    int ghost_index = targ->index;
    free(targ);
    
    ghost_t* ghost = &board->ghosts[ghost_index];
    
    while (board->game_running) {
        pthread_rwlock_wrlock(&board->board_lock);
        
        if (!board->game_running) {
            pthread_rwlock_unlock(&board->board_lock);
            break;
        }
        
        command_t* cmd = &ghost->moves[ghost->current_move % ghost->n_moves];
        move_ghost(board, ghost_index, cmd);
        
        pthread_rwlock_unlock(&board->board_lock);
        
        sleep_ms(board->tempo * 2);
    }
    
    return NULL;
}

void init_board_threading(board_t* board) {
    pthread_rwlock_init(&board->board_lock, NULL);
    board->game_running = 1;
    board->portal_reached = 0;
    board->pacman_dead = 0;
}

void start_character_threads(board_t* board) {
    // Start pacman threads (only if automated)
    for (int i = 0; i < board->n_pacmans; i++) {
        if (board->pacmans[i].n_moves > 0) {
            thread_arg_t* arg = malloc(sizeof(thread_arg_t));
            arg->board = board;
            arg->index = i;
            pthread_create(&board->pacmans[i].thread, NULL, pacman_thread_func, arg);
        }
    }
    
    // Start ghost threads
    for (int i = 0; i < board->n_ghosts; i++) {
        thread_arg_t* arg = malloc(sizeof(thread_arg_t));
        arg->board = board;
        arg->index = i;
        pthread_create(&board->ghosts[i].thread, NULL, ghost_thread_func, arg);
    }
}

void stop_character_threads(board_t* board) {
    board->game_running = 0;
    
    // Join pacman threads
    for (int i = 0; i < board->n_pacmans; i++) {
        if (board->pacmans[i].n_moves > 0) {
            pthread_join(board->pacmans[i].thread, NULL);
        }
    }
    
    // Join ghost threads
    for (int i = 0; i < board->n_ghosts; i++) {
        pthread_join(board->ghosts[i].thread, NULL);
    }
}

void cleanup_board_threading(board_t* board) {
    pthread_rwlock_destroy(&board->board_lock);
}