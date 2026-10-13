#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <cmath>
#include <chrono>  

const double EPSILON = 1e-8;


void elimina(std::vector<std::vector<double>>& A, int n, int k, int start, int end) {
    for (int i = start; i < end; ++i) {
        if (i != k) {
            double factor = A[i][k] / A[k][k];
            for (int j = k; j <= n; ++j) {
                A[i][j] -= factor * A[k][j];
            }
        }
    }
}

int main() {
    std::string inputFilename;
    std::cout << "Introdu numele fisierului de intrare: ";
    std::cin >> inputFilename;

    std::ifstream fin(inputFilename);
    if (!fin) {
        std::cerr << "Eroare la deschiderea fisierului " << inputFilename << "\n";
        return 1;
    }

    int n;
    fin >> n;
    std::vector<std::vector<double>> A(n, std::vector<double>(n + 1));

    
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j) {
            fin >> A[i][j];
            if (i == j)
                A[i][j] += 1e-6;  
        }

    
    for (int i = 0; i < n; ++i)
        fin >> A[i][n];
    fin.close();

    
    int numThreads = std::thread::hardware_concurrency();
    if (numThreads == 0) numThreads = 4;
    std::cout << "Se folosesc " << numThreads << " thread-uri pentru calculare.\n";

    
    auto startTime = std::chrono::high_resolution_clock::now();

    
    for (int k = 0; k < n; ++k) {
       
        int maxRow = k;
        for (int i = k + 1; i < n; ++i) {
            if (fabs(A[i][k]) > fabs(A[maxRow][k])) {
                maxRow = i;
            }
        }

        
        if (fabs(A[maxRow][k]) < EPSILON) {
            std::cerr << "Matrice singulara la pasul " << k << "\n";
            return 1;
        }

        
        std::swap(A[k], A[maxRow]);

        
        std::vector<std::thread> threads;
        int chunk = (n + numThreads - 1) / numThreads;

        for (int t = 0; t < numThreads; ++t) {
            int start = k + 1 + t * chunk;
            int end = std::min(n, start + chunk);
            if (start < end)
                threads.emplace_back(elimina, std::ref(A), n, k, start, end);
        }

        
        for (auto& th : threads) {
            if (th.joinable()) th.join();
        }
    }

    
    std::vector<double> x(n);
    for (int i = n - 1; i >= 0; --i) {
        x[i] = A[i][n];
        for (int j = i + 1; j < n; ++j)
            x[i] -= A[i][j] * x[j];
        x[i] /= A[i][i];
    }

    
    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = endTime - startTime;

    
    std::cout << "Solutia sistemului:\n";
    for (int i = 0; i < n; ++i)
        std::cout << "x[" << i << "] = " << x[i] << "\n";

    
    std::cout << "Timp de executie: " << duration.count() << " secunde.\n";

    std::string outputFilename = "solution.txt";
    std::ofstream fout(outputFilename);
    if (!fout) {
        std::cerr << "Eroare la deschiderea fisierului " << outputFilename << " pentru scriere.\n";
        return 1;
    }

    for (int i = 0; i < n; ++i)
        fout << x[i] << "\n";

    fout.close();
    std::cout << "Solutia a fost salvata in fisierul " << outputFilename << "\n";

    return 0;
}