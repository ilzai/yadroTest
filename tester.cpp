#include <iostream>
#include <limits.h>

using namespace std;

int main(int argc, char *argv[]){
    int curr, prev = INT_MIN;
    int count = 0;
    FILE *f = fopen(argv[1], "r");
    for (int i = 1; i < argc; i++)
    {
    while(fscanf(f, "%d", &curr) != EOF){
        if(curr < prev){
            cout << argv[i]  << " ";
            cout << "failed " << prev << " " << curr << endl;
            cout << count << endl;
            return 0;
        }
        prev = curr;
        count++;
    }
    }
    cout << "success" << endl;
    return 0;
}