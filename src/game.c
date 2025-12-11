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
#include <pthread.h>
#include <stdbool.h>

#define CONTINUE_PLAY 0
#define NEXT_LEVEL 1
#define QUIT_GAME 2
#define LOAD_BACKUP 3
#define CREATE_BACKUP 4

int has_backup = 0;
pid_t backup_process = 0;
pid_t original_pid;


void screen_refresh(board_t * game_board, int mode) {
    pthread_rwlock_rdlock(&game_board->board_lock); // bloqueia o board para leitura
    debug("REFRESH\n");
    draw_board(game_board, mode);
    refresh_screen();
    pthread_rwlock_unlock(&game_board->board_lock); // desbloqueia o board
    
    if(game_board->tempo != 0)
        sleep_ms(game_board->tempo);       
}

int play_board(board_t * game_board) {

    pacman_t* pacman = &game_board->pacmans[0];
    
    // verifica se pediu backup
    if (game_board->portal_reached == 2) {
        if (getpid() == original_pid && backup_process == 0) {
            return CREATE_BACKUP;
        }
        // se nao possui backup, ignora
        game_board->portal_reached = 0;
        return CONTINUE_PLAY;
    }
    
    // verifica se chegou ao portal
    if (game_board->portal_reached == 1) {
        return NEXT_LEVEL;
    }
    
    // verifica se pacman morreu
    if (game_board->pacman_dead) {
        return QUIT_GAME;
    }
    
    // verifica se o jogo acabou sem ser morte, no caso em que Q e automatico
    if (!game_board->game_running && !game_board->pacman_dead && !game_board->portal_reached) {
        return QUIT_GAME;
    }
    
    // se pacman e automatico, nao le input do teclado
    if (pacman->n_moves > 0) {
        return CONTINUE_PLAY;
    }
    
    // le input do teclado
    char input = get_input();

    // se nao houve input, continua
    if(input == '\0')
        return CONTINUE_PLAY;

    debug("KEY %c\n", input);

    if (input == 'Q') {
        return QUIT_GAME;
    }

    if (input == 'G') {
        if (getpid() == original_pid && backup_process == 0) {
            return CREATE_BACKUP;
        }
    }

    // bloqueia o board para escrita
    pthread_rwlock_wrlock(&game_board->board_lock);
    
    command_t c;
    c.command = input;
    c.turns = 1;
    c.turns_left = 1;
    
    int result = move_pacman(game_board, 0, &c);
    
    // desbloqueia o board
    pthread_rwlock_unlock(&game_board->board_lock);
    
    if (result == REACHED_PORTAL) {
        game_board->portal_reached = 1;
        return NEXT_LEVEL;
    }

    if(result == DEAD_PACMAN) {
        game_board->pacman_dead = 1;
        return QUIT_GAME;
    }

    return CONTINUE_PLAY;  
}

int play_board_backup() {

    // cria a copia do processo atual (pai) - cria o filho
    pid_t pid = fork();

    if (pid == 0) {
        // filho continua a jogar
        has_backup = 0;
        backup_process = 0;
        return 0;
    }

    if (pid > 0) {
        has_backup = 1; // pai espera o filho terminar
        backup_process = pid; // guarda o pid do filho
        
        int status; // guarda o status do filho
        waitpid(pid, &status, 0); // bloqueia o pai ate o filho terminar

        // verifica se o filho saiu normalmente
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status); // obtem o codigo de saida do filho
            
            if (exit_code == 1) {
                // filho saiu com Q, por isso pai tambem sai
                terminal_cleanup();
                close_debug_file();
                exit(0); 
            }
            // se exit_code == 0, filho morreu, pai retoma
        }

        terminal_cleanup();
        terminal_init();
        
        has_backup = 0;
        backup_process = 0;
        
        return 0;
    }

    return 0;
}

int main(int argc, char** argv) {

    if (argc != 2) {
        printf("Usage: %s <level_directory>\n", argv[0]);
        return 1;
    }
    
    char *level_directory = argv[1];
    int current_level_index = 0;
    original_pid = getpid();

    srand((unsigned int)time(NULL));
    open_debug_file("debug.log");
    terminal_init();
    
    int accumulated_points = 0;
    bool end_game = false;
    board_t game_board;
    
    while (!end_game) {
        if (load_level(&game_board, level_directory, current_level_index, accumulated_points) < 0) {
            
            // nao ha mais niveis, termina o jogo
            if (getpid() != original_pid) {
                terminal_cleanup();
                close_debug_file();
                exit(1);
            }
            break;
        }
        
        // inicializa as threads 
        start_board_thread(&game_board);
        start_character_threads(&game_board);
        
        draw_board(&game_board, DRAW_MENU);
        refresh_screen();
        
        while(true) {
            int result = play_board(&game_board); 

            if (result == CREATE_BACKUP) {
                stop_character_threads(&game_board);
                
                game_board.portal_reached = 0;
                
                play_board_backup();
                
                start_board_thread(&game_board);
                start_character_threads(&game_board);
                
                screen_refresh(&game_board, DRAW_MENU);
                continue;
            }

            if(result == NEXT_LEVEL) {
                stop_character_threads(&game_board);
                
                screen_refresh(&game_board, DRAW_WIN);
                sleep_ms(game_board.tempo);
                current_level_index++;
                break;
            }
            
            if(result == QUIT_GAME) {
                stop_character_threads(&game_board);
                
                // verifica se pacman morreu
                if (!game_board.pacmans[0].alive) {
                    if (getpid() != original_pid) {
                        terminal_cleanup();
                        close_debug_file();
                        exit(0);    // filho morre, pai retoma
                    }
                    end_game = true;

                // se Q foi pedido
                } else {
                    if (getpid() != original_pid) {
                        terminal_cleanup();
                        close_debug_file();
                        screen_refresh(&game_board, DRAW_GAME_OVER);
                        sleep_ms(game_board.tempo);
                        exit(1);    // pai sai com codigo 1
                    }
                    end_game = true;
                }
                
                screen_refresh(&game_board, DRAW_GAME_OVER); 
                sleep_ms(game_board.tempo);
                break;
            }
    
            screen_refresh(&game_board, DRAW_MENU); 
            accumulated_points = game_board.pacmans[0].points;      
        }
        
        print_board(&game_board);
        stop_board_thread(&game_board);
        unload_level(&game_board);
    }    
    
    terminal_cleanup();
    close_debug_file();
    return 0;
}