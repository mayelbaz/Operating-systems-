#ifndef __GAMERUN_H
#define __GAMERUN_H

#include "Thread.hpp"
#include "utils.hpp"
#include "PCQueue.hpp"
//#include "Semaphore.hpp"

#define FINISH -1
#define MIN_SURVIVAL 2
#define REBIRTH 3

//typedef struct game_params *GP;
//typedef struct work_sections WS;
/*--------------------------------------------------------------------------------
								  Auxiliary Structures
--------------------------------------------------------------------------------*/
struct game_params {
	// All here are derived from ARGV, the program's input parameters.
	uint n_gen;
	uint n_thread;
	string filename;
	bool interactive_on;
	bool print_on;
};

/*typedef struct work_section {
	int num_of_lines;
	int start;
} WS;*/
/*--------------------------------------------------------------------------------
									Class Declaration
--------------------------------------------------------------------------------*/
class Game {
public:

	Game( game_params);
	~Game();
	void run(); // Runs the game
	const vector<float> gen_hist() const; // Returns the generation timing histogram  
	const vector<float> tile_hist() const; // Returns the tile timing histogram
	uint thread_num() const; //Returns the effective number of running threads = min(thread_num, field_height)


protected: // All members here are protected, instead of private for testing purposes

	// See Game.cpp for details on these three functions
	void _init_game(); 
	void _step(uint curr_gen); 
	void _destroy_game();

	inline void print_board(const char* header);

	uint m_gen_num; 			 // The number of generations to run
	uint m_thread_num; 			 // Effective number of threads = min(thread_num, field_height)
	vector<float> m_tile_hist; 	 // Shared Timing history for tiles: First m_gen_num cells are the calculation durations for tiles in generation 1 and so on. 
							   	 // Note: In your implementation, all m_thread_num threads must write to this structure. 
	vector<float> m_gen_hist;  	 // Timing history for generations: x=m_gen_hist[t] iff generation t was calculated in x microseconds
	vector<Thread*> m_threadpool; // A storage container for your threads. This acts as the threadpool.

	bool interactive_on; // Controls interactive mode - that means, prints the board as an animation instead of a simple dump to STDOUT 
	bool print_on; // Allows the printing of the board. Turn this off when you are checking performance (Dry 3, last question)
	
	// TODO: Add in your variables and synchronization primitives

	int **game_matrix_curr;
	int **game_matrix_next;
	int matrix_height;
	int matrix_width;
    uint actual_num_of_threads;
	string input_file_name;
    PCQueue<int> PCQ;
    int num_of_rows_per_thread;
    int num_of_extra_rows;
    //Semaphore* vector_lock;
    pthread_mutex_t vector_lock;


    class GameThreads : public Thread {


        Game* game;

		GameThreads(uint thread_id, Game* game) : Thread(thread_id) {
            this->game = game;
        }
        ~GameThreads() {
            this->game = NULL;
        }

		friend class Game;

        void check_curr_tile(int *num_of_alive_neighbors, int row, int col);
        int determine_cell_status(int num_of_alive_neighbors, int row, int col);

		void thread_workload(){
			int curr_work_section;
			int num_of_alive_neighbors = 0;

			while (1){
				curr_work_section = game->PCQ.pop();
                if(curr_work_section == FINISH) {//die thread, die!!!!
                    break;
                }
				auto start_time = std::chrono::system_clock::now();
				for (int i = curr_work_section; i < (curr_work_section + game->num_of_rows_per_thread); ++i) {
					for (int j = 0; j < game->matrix_width; ++j) {
                        check_curr_tile(&num_of_alive_neighbors, i, j);
                        game->game_matrix_next[i][j] = determine_cell_status(num_of_alive_neighbors, i, j);
                        num_of_alive_neighbors=0;
					}
				}
				if(curr_work_section == (game->matrix_height - game->num_of_extra_rows - game->num_of_rows_per_thread)){
                    for (int ii = (game->matrix_height - game->num_of_extra_rows); ii < game->matrix_height; ++ii) {
                        for (int jj = 0; jj < game->matrix_width; ++jj) {
                            check_curr_tile(&num_of_alive_neighbors, ii, jj);
                            game->game_matrix_next[ii][jj] = determine_cell_status(num_of_alive_neighbors, ii, jj);
                            num_of_alive_neighbors=0;
                        }
                    }
				}
                auto end_time = std::chrono::system_clock::now();
                pthread_mutex_lock(&game->vector_lock);
                game->m_tile_hist.push_back((float)std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count());
                pthread_mutex_unlock(&game->vector_lock);
			}
		}
	};
	friend class GameThreads;
};


#endif
