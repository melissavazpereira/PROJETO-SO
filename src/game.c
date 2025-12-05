#include "board.h"
#include "display.h"
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <stdarg.h>
#include <dirent.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>

#define CONTINUE_PLAY 0
#define NEXT_LEVEL 1
#define QUIT_GAME 2
#define LOAD_BACKUP 3
#define CREATE_BACKUP 4

int has_backup = 0;
pid_t backup_process = 0;

int strings_compare(const void *a, const void *b){
    return strcmp(*(const char**)a, *(const char**)b);
}

void screen_refresh(board_t * game_board, int mode) {
    debug("REFRESH\n");
    draw_board(game_board, mode);
    refresh_screen();
    if(game_board->tempo != 0)
        sleep_ms(game_board->tempo);       
}

int play_board(board_t * game_board) {
    pacman_t* pacman = &game_board->pacmans[0];
    command_t* play;
    
    if (pacman->n_moves == 0) {
        command_t c; 
        c.command = get_input();

        if(c.command == '\0')
            return CONTINUE_PLAY;

        c.turns = 1;
        play = &c;
    }
    else {
        play = &pacman->moves[pacman->current_move%pacman->n_moves];
    }

    debug("KEY %c\n", play->command);

    if (play->command == 'Q') {
        if(has_backup && backup_process > 0){
            kill(backup_process, SIGKILL);
            waitpid(backup_process, NULL, 0);
        }
        return QUIT_GAME;
    }

    if (play->command == 'G') {
        if(!backup_process){
            return CREATE_BACKUP;
        }
    }

    int result = move_pacman(game_board, 0, play);
    
    if (result == REACHED_PORTAL) {
        if(has_backup && backup_process > 0){
            kill(backup_process, SIGKILL);
            waitpid(backup_process, NULL, 0);
            has_backup = 0;
            backup_process = 0;
        }
        return NEXT_LEVEL;
    }

    if(result == DEAD_PACMAN) {
        return QUIT_GAME;
    }
    
    for (int i = 0; i < game_board->n_ghosts; i++) {
        ghost_t* ghost = &game_board->ghosts[i];
        move_ghost(game_board, i, &ghost->moves[ghost->current_move%ghost->n_moves]);
    }

    if (!game_board->pacmans[0].alive) {
        return QUIT_GAME;
    }     

    return CONTINUE_PLAY;  
}

int play_board_backup() {
    if (has_backup == 1){
        return CONTINUE_PLAY;
    }

    pid_t pid = fork();

    if (pid == 0) {
        // ===== PROCESSO FILHO =====
        // Este processo continua a jogar
        has_backup = 0;
        backup_process = 0;
        return CONTINUE_PLAY;  // Continua o loop do jogo
    }

    if (pid > 0) {
        // ===== PROCESSO PAI =====
        // Este processo ESPERA que o filho termine
        has_backup = 1;
        backup_process = pid;
        
        int status;
        waitpid(pid, &status, 0);  // PAI FICA BLOQUEADO AQUI

        terminal_cleanup();
        terminal_init();
        
        // Quando chega aqui, o filho morreu
        // O pai retoma do estado guardado
        has_backup = 0;
        backup_process = 0;
        
        return CONTINUE_PLAY;  // Retoma o jogo do ponto guardado
    }

    return CONTINUE_PLAY;
}

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Usage: %s <level_directory>\n", argv[0]);
        return 1;
    }
    
    char *level_directory = argv[1];
    int current_level_index = 0;

    srand((unsigned int)time(NULL));
    open_debug_file("debug.log");
    terminal_init();
    
    int accumulated_points = 0;
    bool end_game = false;
    board_t game_board;
    
    while (!end_game) {
        if (load_level(&game_board, level_directory, current_level_index, accumulated_points) < 0) {
            debug("Todos os níveis completados!\n");
            break;
        }
        
        draw_board(&game_board, DRAW_MENU);
        refresh_screen();
        
        while(true) {
            int result = play_board(&game_board); 

            if (result == CREATE_BACKUP) {
                play_board_backup();
                // Quando retorna aqui, o pai retomou após o filho morrer
                screen_refresh(&game_board, DRAW_MENU);
                continue;
            }

            if(result == NEXT_LEVEL) {
                screen_refresh(&game_board, DRAW_WIN);
                sleep_ms(game_board.tempo);
                current_level_index++;
                break;
            }
            
            if(result == QUIT_GAME) {
                screen_refresh(&game_board, DRAW_GAME_OVER); 
                sleep_ms(game_board.tempo);
                end_game = true;
                break;
            }
    
            screen_refresh(&game_board, DRAW_MENU); 
            accumulated_points = game_board.pacmans[0].points;      
        }
        
        print_board(&game_board);
        unload_level(&game_board);
    }    
    
    terminal_cleanup();
    close_debug_file();
    return 0;
}