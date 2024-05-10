#include <iostream>
#include <ctime>

using namespace std;

int ran(){
    return 1 + rand() % 100;
}

int main(int argc, char *argv[]){
    srand(time(0));
    FILE *f = fopen(argv[1], "a+");
    int count;
    sscanf(argv[2], "%d", &count);
    for(int i = 0; i < count; i++){
        fprintf(f, "%d ", ran());
        if(i % 1000000 == 0)
            cout << i << endl;
    }
    fclose(f);
    return 0;
}