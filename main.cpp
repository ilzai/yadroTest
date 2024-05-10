#include <iostream>
#include <vector>
#include <algorithm>
#include <limits.h>
#include <cmath>
#include <sys/sysinfo.h>
#include <chrono>
#include <thread>

using namespace std;

class TapeInterface{
    public:
    virtual void openFile(const char *filename){}
    virtual int getElem(){}
    virtual bool isEOF(){}
    virtual ~TapeInterface(){}
    virtual void writeToArray(int elem){}
    virtual void sortArray(){}
    virtual void writeToFile(){}
    virtual void externalSort(){}
    virtual void incrmtSecondaryArrayIndx(){}
    virtual void setClusterSize(int cluster){}
    virtual void setSleepForShiftBtwTapes(int sleep){}
    virtual void setSleepForWrite(int sleep){}
    virtual void setSleepForRead(int sleep){}
    virtual void setSleepForShiftInTape(int sleep){}
    static int clusterSize;
    protected:
    FILE* mainFile;
    static vector<FILE*> secondaryFiles;
    static int countSecondaryFiles;
    FILE* outFile;
    void sleepForShiftBtwTapes(int sleep){
        for(int i = 0; i < sleep; i++){
            this_thread::sleep_for(chrono::milliseconds(sleepForShiftBtwTapesVal));
        }
    }
    void sleepForWrite(){
        this_thread::sleep_for(chrono::milliseconds(sleepForWriteVal));
    }
    void sleepForRead(){
        this_thread::sleep_for(chrono::milliseconds(sleepForReadVal));
    }
    void sleepForShiftInTape(){
        this_thread::sleep_for(chrono::milliseconds(sleepForShiftInTapeVal));
    }
    int sleepForShiftBtwTapesVal;
    int sleepForWriteVal;
    int sleepForReadVal;
    int sleepForShiftInTapeVal;
};

vector<FILE*> TapeInterface::secondaryFiles;
int TapeInterface::clusterSize;//fix me
int TapeInterface::countSecondaryFiles = 0;

class MainTape : public TapeInterface{
    public:
    void setSleepForShiftBtwTapes(int sleep){sleepForShiftBtwTapesVal = sleep;}
    void setSleepForWrite(int sleep){sleepForWriteVal = sleep;}
    void setSleepForRead(int sleep){sleepForReadVal = sleep;}
    void setSleepForShiftInTape(int sleep){sleepForShiftInTapeVal = sleep;}
    void openFile(const char *filename) override{
        mainFile = fopen(filename, "r");
    }
    void setClusterSize(int cluster){
        clusterSize = cluster;
    }
    bool isEOF() override{
        int buf;
        string strBuf;
        if(countElems == clusterSize){
            countElems = 0;
            return false;
        }
        if(feof(mainFile)){
            return false;
        }else{
            if(fscanf(mainFile, "%d", &buf) == EOF){
                return false;
            }else{
                strBuf = to_string(buf);
                fseek(mainFile, -strBuf.size(), SEEK_CUR);
                countElems++;
                return true;
            }
        }
    }
    int getElem() override{
        sleepForRead();
        sleepForShiftInTape();
        int elem;
        fscanf(mainFile, "%d", &elem);
        return elem;
    }
    ~MainTape(){
        fclose(mainFile);
    }
    private:
    int countElems = 0;
};

class SecondaryTape : public TapeInterface{
    public:
    SecondaryTape(){
        countSecondaryFiles++;

        string fileNumber = to_string(countSecondaryFiles);
        string fileExtention = ".txt";
        filename = (fileNumber + fileExtention).c_str();

        secondaryFiles.push_back(fopen(filename, "a+"));
    }
    void writeToArray(int elem){
        currArray.push_back(elem);
        arrIndx++;
    }
    void sortArray(){
        sort(currArray.begin(), currArray.end());
    }
    void writeToFile() override{
        for(int i = 0; i < arrIndx; i++){
            sleepForWrite();
            sleepForShiftInTape();
            fprintf(secondaryFiles[currFile], "%d ", currArray[i]);
        }
        currArray.clear();
        currFile++;
    }

    private:
    int arrIndx = 0;
    static int currFile;
    static vector<int> currArray;
    const char *filename;
};

vector<int> SecondaryTape::currArray;
int SecondaryTape::currFile = 0;

class OutTape : public TapeInterface{
    public:
    OutTape(){
        currArray.resize(countSecondaryFiles);
        currFiles = secondaryFiles;
        countCurrArray = countSecondaryFiles;
    }
    void openFile(const char *filename) override{
        outFile = fopen(filename, "a+");
    }
    bool isEOF() override{
        int buf;
        string strBuf;
        if(feof(currFiles[secondaryFilesIndx])){
            return false;
        }else{
            if(fscanf(currFiles[secondaryFilesIndx], "%d", &buf) == EOF){
                return false;
            }else{
                strBuf = to_string(buf);
                fseek(currFiles[secondaryFilesIndx], -strBuf.size(), SEEK_CUR);
                return true;
            }
        }
    }
    int getElem() override{
        sleepForRead();
        sleepForShiftInTape();
        int elem;
        fscanf(currFiles[secondaryFilesIndx], "%d", &elem);
        return elem;
    }
    void incrementSecondaryArrayIndx(){
        secondaryFilesIndx++;
    }
    void writeToFile() override{
        sleepForShiftInTape();
        sleepForWrite();
        fprintf(outFile, "%d ", min);
    }
    void externalSort(){
        for(int i = 0; i < countSecondaryFiles; i++){
            sleepForShiftBtwTapes(1);
            fseek(currFiles[i], 0L, SEEK_SET);
            if(isEOF()){
                currArray[i] = getElem();
                secondaryFilesIndx++;
            }
        }
        while(!currArray.empty()){
            min = INT_MAX;
            int indxMin;
            for(int i = 0; i < countCurrArray; i++){
                if(currArray[i] < min){
                    min = currArray[i];
                    indxMin = i;
                }
            }
            secondaryFilesIndxPrev = secondaryFilesIndx;
            secondaryFilesIndx = indxMin;
            sleepForShiftBtwTapes(fabs(secondaryFilesIndxPrev - secondaryFilesIndx));
            if(isEOF()){
                currArray[indxMin] = getElem();
            }else{
                currArray.erase(currArray.begin() + indxMin);
                currFiles.erase(currFiles.begin() + indxMin);
                countCurrArray--;
            }
            writeToFile();
            
        }
    }
    ~OutTape(){
        fclose(outFile);
        for(int i = 0; i < countSecondaryFiles; i++){
            fclose(secondaryFiles[i]);

            string fileNumber = to_string(i + 1);
            string fileExtention = ".txt";
            const char* filename = (fileNumber + fileExtention).c_str();

            remove(filename);
        }
    }
    private:
    int min;
    int secondaryFilesIndxPrev;
    int secondaryFilesIndx;
    int countCurrArray;
    vector<int> currArray;
    vector<FILE*> currFiles;
};

int main(int argc, char *argv[]){
    
    //подсчет количества элементов в сортируемом файле
    double bigCountElems = 0;
    FILE* main = fopen(argv[1], "r");
    int buf;
    while(fscanf(main, "%d", &buf) != EOF){
        bigCountElems++;
    }
    fclose(main);

    //подсчет максимально возможного количества 
    //чисел в оперативной памяти
    struct sysinfo info;
    sysinfo(&info);
    int bigClusterSize = info.freeram / sizeof(int);
    //общее количество лент
    int countTapes = ceil(bigCountElems / bigClusterSize) + 2;

    //массив указателей на все ленты
    TapeInterface* allTapes[countTapes];
    allTapes[0] = new MainTape();
    allTapes[0]->openFile(argv[1]);
    allTapes[0]->setClusterSize(bigClusterSize);

    //чтение задержек из файла
    FILE* config = fopen("config.txt", "r");
    int param;
    //задержка на переход с ленты на ленту
    fscanf(config, "%d", &param);
    allTapes[0]->setSleepForShiftBtwTapes(param);
    //задержка на запись
    fscanf(config, "%d", &param);
    allTapes[0]->setSleepForWrite(param);
    //задержка на чтение
    fscanf(config, "%d", &param);
    allTapes[0]->setSleepForRead(param);
    //задержка на переход на следующий элемент в ленте
    fscanf(config, "%d", &param);
    allTapes[0]->setSleepForShiftInTape(param);
    fclose(config);

    //чтенеие входной ленты и распределение чисел по 
    //временным лентам
    for(int i = 1; i < countTapes - 1; i++){
        allTapes[i] = new SecondaryTape();
        while(allTapes[0]->isEOF()){
            allTapes[i]->writeToArray(allTapes[0]->getElem());
        }
        allTapes[i]->sortArray();
        allTapes[i]->writeToFile();
    }

    //создание выходной ленты
    allTapes[countTapes - 1] = new OutTape();
    allTapes[countTapes - 1]->openFile(argv[2]);

    //заключительный этап сортировки
    allTapes[countTapes - 1]->externalSort();

    //закрытие всех файлов и удаление временных
    for(int i = 0; i < countTapes; i++){
        delete allTapes[i];
    }
    return 0;
}
