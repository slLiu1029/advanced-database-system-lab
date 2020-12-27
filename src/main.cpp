#include <iostream>
#include <cstdio>
#include <ctime>

#include "BMgr.h"

using namespace std;

const char *IGNORE_CHARS = "\f\n\r\t\v,";

FILE *OpenFile(const char *file_name, const char *mode) {
    FILE *f = fopen(file_name, mode);
    if (f == NULL) {
        cout << "Failed to open the file" << file_name << endl;
        exit(0);
    }

    return f;
}

int main(int argc, char const *argv[]) {
    // open the file
    const char *file_name = "../data/data-5w-50w-zipf.txt";
    FILE *f = OpenFile(file_name, "r");
    clock_t start, end;
    long double time_cost = 0;

    BMgr *bmgr = new BMgr();
    long int num_lines = 0, op, page_id;
    char line[20], *token, *args[2];
    // read the file by line
    while (fgets(line, 20, f) != NULL) {
        num_lines++;
        // dive the line by comma and store operation code and page id in array args
        token = strtok(line, IGNORE_CHARS);
        args[0] = token;
        token = strtok(NULL, IGNORE_CHARS);
        args[1] = token;

        // transform strings to integers
        op = strtol(args[0], &token, 0);
        page_id = strtol(args[1], &token, 0);

        // request of reading or writing data and calculate total time
        start = clock();
        bmgr->FixPage((int) page_id, (int) op);
        end = clock();
        time_cost += (end - start) * 1.0 / CLOCKS_PER_SEC;
    }
    fclose(f);

    // write all dirty frame to the disk and calculate total time
    start = clock();
    bmgr->WriteDirtys();
    end = clock();
    time_cost += (end - start) / CLOCKS_PER_SEC;

    // user interface
    cout << "************************************" << endl;
    cout << "*    " << "Storage and Buffer Manager" << "    *" << endl;
    cout << "*          " << "Name: 刘思理" << "             *" << endl;
    cout << "*         " << "No. : SA20011902" << "         *" << endl;
    cout << "************************************" << endl << endl << endl;
    cout << "====================================" << endl;
    cout << "...................................." << endl;
    cout << "        " << "Information  Display" << "        " << endl;
    cout << "...................................." << endl;

    // get hits and hit ratio
    int hits = bmgr->GetHits();
    double hit_rate = hits * 1.0 / num_lines;
    double numIOs = bmgr->GetNumIOs();

    cout << "Buffer size: " << DEFBUFSIZE << endl;
    cout << "Requests: " << num_lines << endl;
    cout << "Hits: " << hits << endl;
    cout << "Hit ratio: " << hit_rate << endl;
    cout << "I/Os: " << numIOs << endl;
    cout << "Time cost: " << time_cost << " s" << endl;
    cout << "====================================" << endl;

    // free all the allocated space related to buffer manager
    bmgr->FreeBMgr();
    delete bmgr;

    return 0;
}