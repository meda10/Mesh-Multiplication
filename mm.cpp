#include <mpi.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <iterator>
#include <string>
#include <cassert>
#include <algorithm>

const char * FILENAME_MATRIX_1 = "mat1";
const char * FILENAME_MATRIX_2 = "mat2";
#define TAG 0
#define NO_NEIGHBOUR -1

void print_matrix(const std::vector<std::vector<int>>& matrix){
    for (const auto& line: matrix) {
        for (auto num: line){
            std::cout << num << ' ';
        }
        std::cout << '\n';
    }
}

std::vector<std::vector<int>> read_input(const char* filename, int *size){
    std::vector<std::vector<int>> matrix;
    std::fstream file;
    std::string first_line;
    std::string tmp;

    file.open(filename, std::ios::in);
    if (!file.is_open()){
        MPI_Abort(MPI_COMM_WORLD, 1);
        exit(1);
    }

    std::getline(file, first_line);
    *size = std::stoi(first_line);

    while(!file.eof()){
        getline(file, tmp);
        if(!tmp.empty()) {
            std::istringstream str_stream(tmp);
            std::vector<int> matrix_line;
            int number;
            while (str_stream >> number) {
                matrix_line.push_back(number);
            }
            matrix.push_back(matrix_line);
        }
    }
    file.close();
    return matrix;
}

int main(int argc, char *argv[]) {
    int number_of_processors;
    int my_id;

    //MPI init
    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD,&number_of_processors);
    MPI_Comm_rank(MPI_COMM_WORLD,&my_id);

    MPI_Status stat;

    int matrix_rows_1, matrix_columns_1, matrix_rows_2, matrix_columns_2;
//           M                N               N                K

    if(my_id == 0) {
        std::vector<std::vector<int>> matrix_1 = read_input(FILENAME_MATRIX_1, &matrix_rows_1);
        std::vector<std::vector<int>> matrix_2 = read_input(FILENAME_MATRIX_2, &matrix_columns_2);

        matrix_columns_1 = matrix_1[0].size();
        matrix_rows_2 = matrix_2.size();

        if(matrix_columns_1 != matrix_rows_2){
            MPI_Abort(MPI_COMM_WORLD, 1);
            exit(1);
        }

        MPI_Bcast(&matrix_rows_1, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&matrix_columns_1, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&matrix_columns_2, 1, MPI_INT, 0, MPI_COMM_WORLD);
//        std::cout << my_id << " ROOT " << matrix_rows_1 << " " << matrix_columns_1 << " " << matrix_columns_2 << std::endl;

        //Sends
        for(int j = 0; j < matrix_columns_1; j++){
            for(int i = 0; i < matrix_rows_1; i++){
                if (i * matrix_columns_2 != 0){
//                    std::cout << "SEND " << matrix_1[i][j] << " Xfrom " << 0 << " sending to " << i * matrix_columns_2 << std::endl;
                    MPI_Send(&matrix_1[i][j], 1, MPI_INT, i * matrix_columns_2, TAG, MPI_COMM_WORLD);
                }
            }
        }

        for(int i = 0; i < matrix_rows_2; i++){
            for(int j = 0; j < matrix_columns_2; j++){
                if (j != 0){
//                    std::cout << "SEND " << matrix_2[i][j] << " Zfrom " << 0 << " sending to " << j << std::endl;
                    MPI_Send(&matrix_2[i][j], 1, MPI_INT, j, TAG, MPI_COMM_WORLD);
                }
            }
        }

        int sum = 0;
        for(int i = 0; i < matrix_columns_1; i++){ //2
            int a = matrix_1[0][matrix_columns_1 - 1 - i];
            int b = matrix_2[matrix_rows_2 - 1 - i][0];
            sum += a * b;
//            std::cout << a << "*" << b  << "=" << sum << std::endl;

            // Send current values from left and top vector to right and bottom processes.
            if(matrix_columns_2 > 1) {
//                std::cout << "SEND " << b << " from " << 0 << " sending to " << 1 << std::endl;
                MPI_Send(&a, 1, MPI_INT, 1, TAG, MPI_COMM_WORLD); // A -> RIGHT
            }
            if(matrix_rows_1 > 1){
//                std::cout << "SEND " << b << " from " << 0 << " sending to " << matrix_columns_2 << std::endl;
                MPI_Send(&b, 1, MPI_INT, matrix_columns_2, TAG, MPI_COMM_WORLD); // B  -> DOWN
            }
        }
//
//        // Read resulting values from all processes and print out
        std::cout << matrix_rows_1 << ":" << matrix_columns_2 << std::endl;
        std::cout << sum;
        for(int proci = 1; proci < number_of_processors - 1; ++proci) {
            int tmp;
            MPI_Recv(&tmp, 1, MPI_INT, proci, TAG, MPI_COMM_WORLD, &stat);
            std::cout << ((proci % matrix_columns_2 == 0) ? "\n" : " ") << tmp;
        }
        std::cout << std::endl;
    }

    if(my_id > 0) {
        MPI_Bcast(&matrix_rows_1, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&matrix_columns_1, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&matrix_columns_2, 1, MPI_INT, 0, MPI_COMM_WORLD);

        // Get IDs of processor neighbours
        int left = my_id % matrix_columns_2  == 0 ? 0 : my_id - 1;
        int right = (my_id + 1) % matrix_columns_2 == 0 ? NO_NEIGHBOUR : my_id + 1;
        int top = my_id - matrix_columns_2 < 0  ? 0 : my_id - matrix_columns_2;
        int bottom = my_id + matrix_columns_2 > number_of_processors - 1 ? NO_NEIGHBOUR :my_id + matrix_columns_2;

        int a, b;
        int sum = 0;
        for(int i = 0; i < matrix_columns_1; ++i) {
            MPI_Recv(&a, 1, MPI_INT, left, TAG, MPI_COMM_WORLD, &stat);
            MPI_Recv(&b,  1, MPI_INT, top, TAG, MPI_COMM_WORLD, &stat);

            sum += a * b;
            if(right != NO_NEIGHBOUR){
//                std::cout << my_id << " RECV " << a << " LRfrom: " << left << " to: " << right << std::endl;
                MPI_Send(&a, 1, MPI_INT, right, TAG, MPI_COMM_WORLD);
            }
            if(bottom != NO_NEIGHBOUR){
//                std::cout << my_id << " RECV " << b << " TBfrom: " << top << " to: " << bottom << std::endl;
                MPI_Send(&b,  1, MPI_INT, bottom, TAG, MPI_COMM_WORLD);
            }
        }
//        std::cout << my_id << " SUM " << sum << std::endl;
        MPI_Send(&sum, 1, MPI_INT, 0, TAG, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}