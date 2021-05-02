#include <mpi.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

const char * FILENAME_MATRIX_1 = "mat1";
const char * FILENAME_MATRIX_2 = "mat2";
#define TAG_MATRIX_1 1
#define TAG_MATRIX_2 2
#define TAG 2
#define NO_NEIGHBOUR (-1)


std::vector<std::vector<int>> read_input(const char* filename, int *size){
    std::vector<std::vector<int>> matrix;
    std::fstream file;
    std::string first_line;
    std::string tmp;

    file.open(filename, std::ios::in);
    if (!file.is_open()){
        MPI_Abort(MPI_COMM_WORLD, 1);
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

    if(my_id == 0) {
        //Load matrix from file
        std::vector<std::vector<int>> matrix_1 = read_input(FILENAME_MATRIX_1, &matrix_rows_1);
        std::vector<std::vector<int>> matrix_2 = read_input(FILENAME_MATRIX_2, &matrix_columns_2);

        matrix_columns_1 = int (matrix_1[0].size());
        matrix_rows_2 = int (matrix_2.size());

        //Check matrix dimensions
        if(matrix_columns_1 != matrix_rows_2){
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        //Send matrix dimensions
        MPI_Bcast(&matrix_rows_1, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&matrix_columns_1, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&matrix_columns_2, 1, MPI_INT, 0, MPI_COMM_WORLD);

        //Sends matrix 1 to first column of processors
        for(int j = 0; j < matrix_columns_1; j++){
            for(int i = 0; i < matrix_rows_1; i++){
                MPI_Send(&matrix_1[i][j], 1, MPI_INT, i * matrix_columns_2, TAG_MATRIX_1, MPI_COMM_WORLD);
            }
        }

        //Sends matrix 2 to first row of processors
        for(int i = 0; i < matrix_rows_2; i++){
            for(int j = 0; j < matrix_columns_2; j++){
                MPI_Send(&matrix_2[i][j], 1, MPI_INT, j, TAG_MATRIX_2, MPI_COMM_WORLD);
            }
        }

        //Mesh multiplication
        int sum = 0;
        for(int i = 0; i < matrix_columns_1; i++){
            //Receive A, B
            int a, b;
            MPI_Recv(&a,1, MPI_INT, 0, TAG_MATRIX_1, MPI_COMM_WORLD, &stat);
            MPI_Recv(&b,1, MPI_INT, 0, TAG_MATRIX_2, MPI_COMM_WORLD, &stat);
            //Compute Sum
            sum += a * b;
            //Send A, B to right and bottom neighbours (if exists)
            if(matrix_columns_2 > 1) {
                MPI_Send(&a, 1, MPI_INT, 1, TAG_MATRIX_1, MPI_COMM_WORLD);
            }
            if(matrix_rows_1 > 1){
                MPI_Send(&b, 1, MPI_INT, matrix_columns_2, TAG_MATRIX_2, MPI_COMM_WORLD);
            }
        }

        //Receive SUM from all processors and print to STDOUT
        std::cout << matrix_rows_1 << ":" << matrix_columns_2 << std::endl;
        std::cout << sum;
        for(int p = 1; p < number_of_processors; ++p) {
            int num;
            MPI_Recv(&num, 1, MPI_INT, p, TAG, MPI_COMM_WORLD, &stat);
            if(p % matrix_columns_2 == 0){
                std::cout << std::endl;
            } else{
                std::cout << " ";
            }
            std::cout << num;
        }
        std::cout << std::endl;
    }

    if(my_id > 0) {
        //Receive matrix dimensions
        MPI_Bcast(&matrix_rows_1, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&matrix_columns_1, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&matrix_columns_2, 1, MPI_INT, 0, MPI_COMM_WORLD);

        // Get IDs of processor neighbours
        int left = my_id % matrix_columns_2  == 0 ? 0 : my_id - 1;
        int right = (my_id + 1) % matrix_columns_2 == 0 ? NO_NEIGHBOUR : my_id + 1;
        int top = my_id - matrix_columns_2 < 0  ? 0 : my_id - matrix_columns_2;
        int bottom = my_id + matrix_columns_2 > number_of_processors - 1 ? NO_NEIGHBOUR : my_id + matrix_columns_2;

        //Mesh multiplication
        int sum = 0;
        for(int i = 0; i < matrix_columns_1; ++i) {
            //Receive A, B from left and top neighbours
            int a, b;
            MPI_Recv(&a,1, MPI_INT, left, TAG_MATRIX_1, MPI_COMM_WORLD, &stat);
            MPI_Recv(&b,1, MPI_INT, top, TAG_MATRIX_2, MPI_COMM_WORLD, &stat);
            //Compute Sum
            sum += a * b;
            //Send A, B to right and bottom neighbours (if exists)
            if(right != NO_NEIGHBOUR){
                MPI_Send(&a, 1, MPI_INT, right, TAG_MATRIX_1, MPI_COMM_WORLD);
            }
            if(bottom != NO_NEIGHBOUR){
                MPI_Send(&b,  1, MPI_INT, bottom, TAG_MATRIX_2, MPI_COMM_WORLD);
            }
        }
        //Send data to master
        MPI_Send(&sum, 1, MPI_INT, 0, TAG, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}