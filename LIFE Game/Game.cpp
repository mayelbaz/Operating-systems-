#include "Game.hpp"

/*--------------------------------------------------------------------------------
								
--------------------------------------------------------------------------------*/
void Game::run() {

	_init_game(); // Starts the threads and all other variables you need
	print_board("Initial Board");
	for (uint i = 0; i < m_gen_num; ++i) {
		auto gen_start = std::chrono::system_clock::now();
		_step(i); // Iterates a single generation 
		auto gen_end = std::chrono::system_clock::now();
		m_gen_hist.push_back((float)std::chrono::duration_cast<std::chrono::microseconds>(gen_end - gen_start).count());
		print_board(NULL);
	} // generation loop
	print_board("Final Board");
	_destroy_game();
}

void Game::_init_game() {
	// Create threads
	// Create game fields
	// Start the threads
	// Testing of your implementation will presume all threads are started here
    actual_num_of_threads = thread_num();
    for (uint thread_id_num = 0; thread_id_num < actual_num_of_threads; ++thread_id_num) { // if prob. revert to int
        Thread* thread = new GameThreads(thread_id_num, this);
        m_threadpool.push_back(thread);
        if(!thread->start()){
            break; //error. shouldn't happen.
        }
    }
    num_of_rows_per_thread = matrix_height / actual_num_of_threads;
    num_of_extra_rows = 0;
    if(matrix_height % actual_num_of_threads != 0){ //the last thread should get extra work
        num_of_extra_rows = matrix_height % actual_num_of_threads;
    }
}




void Game::_step(uint curr_gen) {
	// Push jobs to queue
	// Wait for the workers to finish calculating 
	// Swap pointers between current and next field
    int start_line_num = 0;
    for (uint i = 0; i < actual_num_of_threads; ++i) {
        this->PCQ.push(start_line_num);
        start_line_num += num_of_rows_per_thread;
    }
    while (m_tile_hist.size() != curr_gen * actual_num_of_threads + actual_num_of_threads){
        continue;
    }
    int **temp = game_matrix_curr;
    game_matrix_curr = game_matrix_next;
    game_matrix_next = temp;

}

void Game::_destroy_game(){
	// Destroys board and frees all threads and resources 
	// Not implemented in the Game's destructor for testing purposes. 
	// Testing of your implementation will presume all threads are joined here
    for (uint i = 0; i < actual_num_of_threads; ++i) {
        this->PCQ.push(FINISH);
    }
    for (int i = 0; i <(int)actual_num_of_threads ; ++i) {
        m_threadpool[i]->join();
        delete m_threadpool[i];
    }
    for (int i = 0; i < matrix_height; ++i) {
        delete[] game_matrix_curr[i];
        delete[] game_matrix_next[i];
    }
    delete[] game_matrix_curr;
    delete[] game_matrix_next;
    pthread_mutex_destroy(&vector_lock);
}

uint Game::thread_num() const {
    if (matrix_height < (int)m_thread_num)
        return (uint)matrix_height;
    return m_thread_num;
}

const vector<float> Game::gen_hist() const {
    return this->m_gen_hist;
}

const vector<float> Game::tile_hist() const {
    return this->m_tile_hist;
}

Game::Game(game_params game_params) {
    m_gen_num = game_params.n_gen;
    m_thread_num = game_params.n_thread;
    input_file_name = game_params.filename;
    interactive_on = game_params.interactive_on;
    print_on = game_params.print_on;
    vector<string> lines = utils::read_lines(input_file_name);
    matrix_height = (int)lines.size();
    vector<string> cells = utils::split(lines.front(), ' ');
    matrix_width = (int)cells.size();
    game_matrix_curr = new int* [matrix_height];
    game_matrix_next = new int* [matrix_height];
    for (int i = 0; i < matrix_height; ++i) {
        game_matrix_curr[i] = new int [matrix_width];
        game_matrix_next[i] = new int [matrix_width];
    }
    for (int row = 0; row < matrix_height; ++row) {
        cells = utils::split(lines[row], ' ');
        for (int col = 0; col < matrix_width; ++col) {
            game_matrix_curr[row][col] = std::stoi(cells[col]);
            game_matrix_next[row][col] = std::stoi(cells[col]);
        }
    }
    vector_lock = PTHREAD_MUTEX_INITIALIZER;
}

Game::~Game() {
    /*for (int i = 0; i < matrix_height; ++i) {
        delete[] game_matrix_curr[i];
        delete[] game_matrix_next[i];
    }
    delete[] game_matrix_curr;
    delete[] game_matrix_next;*/
}

/*--------------------------------------------------------------------------------
								
--------------------------------------------------------------------------------*/
inline void Game::print_board(const char* header) {

	if(print_on){ 

		// Clear the screen, to create a running animation 
		if(interactive_on) {
            system("clear");
        }

		// Print small header if needed
		if (header != NULL)
			cout << "<------------" << header << "------------>" << endl;

        if(interactive_on) {
            cout << u8"╔" << string(u8"═") * matrix_width << u8"╗" << endl;
            for (int i = 0; i < matrix_height; ++i) {
                cout << u8"║";
                for (int j = 0; j < matrix_width; ++j) {
                    cout << (game_matrix_curr[i][j] ? u8"█" : u8"░");
                }
                cout << u8"║" << endl;
            }
            cout << u8"╚" << string(u8"═") * matrix_width << u8"╝" << endl;
        }
        /*int temp;

        for (int i = 0; i < matrix_height; ++i) {
            for (int j = 0; j < matrix_width; ++j) {
                temp = (int)game_matrix_curr[i][j];
                printf("%d ", temp);
            }
            printf("\n");
        }*/

		// Display for GEN_SLEEP_USEC micro-seconds on screen 
		if(interactive_on)
			usleep(GEN_SLEEP_USEC);
	}

}

void Game::GameThreads::check_curr_tile(int *num_of_alive_neighbors, int i, int j){
    if (i>0 && j>0 && i<game->matrix_height-1 && j<game->matrix_width-1) {//borders check, everything good
        *num_of_alive_neighbors = game->game_matrix_curr[i-1][j-1] +
                                 game->game_matrix_curr[i-1][j] + game->game_matrix_curr[i-1][j+1] +
                                 game->game_matrix_curr[i][j-1] + game->game_matrix_curr[i][j+1] +
                                 game->game_matrix_curr[i+1][j-1] + game->game_matrix_curr[i+1][j] + game->game_matrix_curr[i+1][j+1];
        return;
    }
    if (i==0 && j>0 && i<game->matrix_height-1 && j<game->matrix_width-1) {//top row
        *num_of_alive_neighbors = game->game_matrix_curr[i][j-1] + game->game_matrix_curr[i][j+1] +
                                 game->game_matrix_curr[i+1][j-1] + game->game_matrix_curr[i+1][j] + game->game_matrix_curr[i+1][j+1];
        return;
    }
    if (i>0 && j==0 && i<game->matrix_height-1 && j<game->matrix_width-1) {//leftest coll
        *num_of_alive_neighbors = game->game_matrix_curr[i-1][j] + game->game_matrix_curr[i-1][j+1] +
                                  game->game_matrix_curr[i][j+1] +
                                  game->game_matrix_curr[i+1][j] + game->game_matrix_curr[i+1][j+1];
        return;
    }
    if (i>0 && j>0 && i==game->matrix_height-1 && j<game->matrix_width-1) {//bottom row
        *num_of_alive_neighbors = game->game_matrix_curr[i-1][j-1] +
                                 game->game_matrix_curr[i-1][j] + game->game_matrix_curr[i-1][j+1] +
                                 game->game_matrix_curr[i][j-1] + game->game_matrix_curr[i][j+1];
        return;
    }
    if (i>0 && j>0 && i<game->matrix_height-1 && j==game->matrix_width-1) {//rightest coll
        *num_of_alive_neighbors = game->game_matrix_curr[i-1][j-1] +
                                 game->game_matrix_curr[i-1][j] + game->game_matrix_curr[i][j-1] +
                                 game->game_matrix_curr[i+1][j-1] + game->game_matrix_curr[i+1][j];
        return;
    }
    if (i==0 && j==0 && i<game->matrix_height-1 && j<game->matrix_width-1) {//top left corner
        *num_of_alive_neighbors = game->game_matrix_curr[i][j+1] +
                                  game->game_matrix_curr[i+1][j] + game->game_matrix_curr[i+1][j+1];
        return;
    }
    if (i==0 && j>0 && i<game->matrix_height-1 && j==game->matrix_width-1) {//top right corner
        *num_of_alive_neighbors = game->game_matrix_curr[i][j-1] + game->game_matrix_curr[i+1][j] +
                                  game->game_matrix_curr[i+1][j-1];
        return;
    }
    if (i>0 && j==0 && i==game->matrix_height-1 && j<game->matrix_width-1) {//bottom left corner
        *num_of_alive_neighbors = game->game_matrix_curr[i-1][j] + game->game_matrix_curr[i-1][j+1] +
                                  game->game_matrix_curr[i][j+1];
        return;
    }
    if (i>0 && j>0 && i==game->matrix_height-1 && j==game->matrix_width-1) {//bottom right corner
        *num_of_alive_neighbors = game->game_matrix_curr[i-1][j-1] +
                                  game->game_matrix_curr[i-1][j] +
                                  game->game_matrix_curr[i][j-1];
        return;
    }

}

int Game::GameThreads::determine_cell_status(int num_of_alive_neighbors, int i, int j){
    /*if(game->game_matrix_curr[i][j] == 1 && num_of_alive_neighbors >= OVER_CROWDED){ //a live cell has too many neighbors
        return 0;
    }
    if(game->game_matrix_curr[i][j] == 1 && num_of_alive_neighbors >= MIN_SURVIVAL && num_of_alive_neighbors <= REBIRTH){ //cell stays alive
        return 1;
    }
    if(game->game_matrix_curr[i][j] == 1 && num_of_alive_neighbors < MIN_SURVIVAL){ // a live cell dies of loneliness
        return 0;
    }
    if(game->game_matrix_curr[i][j]== 0 && num_of_alive_neighbors == REBIRTH){ // a dead cell is reborn
        return 1;
    }
    return 0; //should never reach here*/
    if(game->game_matrix_curr[i][j]== 0 && num_of_alive_neighbors == REBIRTH){ // a dead cell is reborn
        return 1;
    }
    if(game->game_matrix_curr[i][j] == 1 && (num_of_alive_neighbors == MIN_SURVIVAL || num_of_alive_neighbors == REBIRTH)){ //cell stays alive
        return 1;
    }
    return 0;
}


/* Function sketch to use for printing the board. You will need to decide its placement and how exactly 
	to bring in the field's parameters. 

		cout << u8"╔" << string(u8"═") * field_width << u8"╗" << endl;
		for (uint i = 0; i < field_height ++i) {
			cout << u8"║";
			for (uint j = 0; j < field_width; ++j) {
				cout << (field[i][j] ? u8"█" : u8"░");
			}
			cout << u8"║" << endl;
		}
		cout << u8"╚" << string(u8"═") * field_width << u8"╝" << endl;
*/ 



