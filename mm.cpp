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

//
//void matRead(fstream &fin, vector<vector<int> > &mat, int *rows, int *columns) {
//    string line;
//    getline(fin, line);
//
//    while(getline(fin, line)) {
//        if(line.size() == 0) break; // extra newline at the end of file
//        istringstream is(line);
//        mat.push_back(vector<int>(istream_iterator<int>(is), istream_iterator<int>()));
//    }
//
//    *rows = mat.size();
//    *columns = mat[0].size();
//}


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
        MPI_Finalize();
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
    int prod = 0;       // Resulting dot product for the field controled byt this process.
    int matrix_rows_1, matrix_rows_2, matrix_columns_1, matrix_columns_2;
//           M                N             N                 K

    if(my_id == 0) {
        std::vector<std::vector<int>> matrix_1 = read_input(FILENAME_MATRIX_1, &matrix_rows_1);
        std::vector<std::vector<int>> matrix_2 = read_input(FILENAME_MATRIX_2, &matrix_columns_2);

        matrix_columns_1 = matrix_1[0].size();
        matrix_rows_2 = matrix_2.size();

        if(matrix_columns_1 != matrix_rows_2){
            MPI_Finalize();
            exit(1);
        }

//        // DEBUG - time measurement
//        // double tFrom_wtime = MPI::Wtime();
//
//        // Send the number of columns of matrix 1 to each process
//        for(int i = 1; i < numprocs; ++i) {
//            MPI_Send(&m, 1, MPI_INT, i, TAG, MPI_COMM_WORLD);
//            MPI_Send(&n, 1, MPI_INT, i, TAG, MPI_COMM_WORLD);
//            MPI_Send(&k, 1, MPI_INT, i, TAG, MPI_COMM_WORLD);
//        }
//
//        // Mesh multiplication algorithm
//        for(int i = 0; i < max(m + n - 1, n + k - 1); ++i) {
//            // Send values from matrix 1 to all processes.
//            if(i < m + n - 1) {
//                int c = max(0, n - 1 - i);
//                int r = i - n + c + 1;
//
//                while(r < m && c < n) {
//                    if(r > 0) {
//                        int tmp = mat1[r][c];
//                        MPI_Send(&tmp, 1, MPI_INT, r * k, TAG, MPI_COMM_WORLD);
//                    }
//                    r += 1;
//                    c += 1;
//                }
//            }
//
//
//            // Send values from matrix 2 to all processes.
//            if(i < n + k - 1) {
//                int r = max(0, n - 1 - i);
//                int c = i - n + r + 1;
//
//                while(r < n && c < k) {
//                    if(c > 0) {
//                        int tmp = mat2[r][c];
//                        MPI_Send(&tmp, 1, MPI_INT, c, TAG, MPI_COMM_WORLD);
//                    }
//                    r += 1;
//                    c += 1;
//                }
//            }
//
//            // compute product and send values L -> R, T -> B
//            if(i < n) {
//                int left = mat1[0][n - 1 - i];
//                int top  = mat2[n - 1 - i][0];
//                prod += left * top;
//
//                // Send current values from left and top vector to right and bottom processes.
//                if(k > 1) MPI_Send(&left, 1, MPI_INT, 1, TAG, MPI_COMM_WORLD); // LEFT -> RIGHT
//                if(m > 1) MPI_Send(&top,  1, MPI_INT, k, TAG, MPI_COMM_WORLD); // TOP  -> DOWN
//            }
//        }
//
//        // Read resulting values from all processes and print out
//        cout << m << ":" << k << endl;
//        cout << prod;
//        for(int proci = 1; proci < numprocs; ++proci) {
//            int tmp;
//            MPI_Recv(&tmp, 1, MPI_INT, proci, TAG, MPI_COMM_WORLD, &stat);
//            cout << ((proci % mat2c == 0) ? "\n" : " ") << tmp;
//        }
//        cout << endl;
//
//
//        // DEBUG - time measurement
//        // double tTo_wtime = MPI::Wtime();
//        // cout << 1000.0 * (tTo_wtime - tFrom_wtime) << endl;
    }
//
//    /* --- All CPUs except the first one --- */
//    if(myid > 0) {
//        // Receive matrices dimensions frmo process 0.
//        MPI_Recv(&m, 1, MPI_INT, 0, TAG, MPI_COMM_WORLD, &stat);
//        MPI_Recv(&n, 1, MPI_INT, 0, TAG, MPI_COMM_WORLD, &stat);
//        MPI_Recv(&k, 1, MPI_INT, 0, TAG, MPI_COMM_WORLD, &stat);
//
//        // Iteratively compute the dot product of the top and vector left vector, send values to right and bottom.
//        int left, top;
//        for(int i = 0; i < n; ++i) {
//            // CPU [1][0]
//            if((m > 1) && (myid == k)) {
//                MPI_Recv(&top,  1, MPI_INT, 0, TAG, MPI_COMM_WORLD, &stat);
//                MPI_Recv(&left, 1, MPI_INT, 0, TAG, MPI_COMM_WORLD, &stat);
//            } else {
//                MPI_Recv(&left, 1, MPI_INT, (myid == 1 || myid % k == 0) ? 0 : myid - 1, TAG, MPI_COMM_WORLD, &stat);
//                MPI_Recv(&top,  1, MPI_INT, (myid <= k) ? 0 : myid - k,                  TAG, MPI_COMM_WORLD, &stat);
//            }
//            prod += left * top;
//
//            if((myid + 1) % k != 0)   MPI_Send(&left, 1, MPI_INT, myid + 1, TAG, MPI_COMM_WORLD);
//            if(myid < (numprocs - k)) MPI_Send(&top,  1, MPI_INT, myid + k, TAG, MPI_COMM_WORLD);
//        }
//        MPI_Send(&prod, 1, MPI_INT, 0, TAG, MPI_COMM_WORLD);
//    }

    // Finalize MPI
    MPI_Finalize();
    return 0;
}