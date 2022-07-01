#include <iostream>
#include <map>
#include <set>
#include <algorithm>
#include <vector>
#include <fstream>
#include <sstream>
#include <climits>
#include <cfloat>
#include <math.h>


class Doc {       
public:
    int docId;
    int NewDocId;
    int lenght;
    std::set<std::string> docVocabulary;

    bool operator < (const Doc& other) const{
        return (lenght > other.lenght);
    }
};

void printAverageGap(std::map<std::string , std::vector<int>> & posting){

    std::cout << std::endl << "##################################################" << std::endl;
    std::cout << "Find average bit for storing gap..." << std::endl;

    float  rawbit = 0, TWOBYTE=0, byte=0, nibbles=0, halfNibbles=0, gamma =0, delta =0;  int gap=0;
    for (auto it = posting.begin(); it != posting.end(); it++){
        int partial_sum = 0;
        for (auto element : it->second ){
            gap++;
            int partial = element-partial_sum;
            partial_sum += partial;
            float logPartial =  std::floor(log2(partial));

            float bitPartial = logPartial+1;

            /////////////////////////////////////
            //RAW BIT
            ////////////////////////////////////
            rawbit += bitPartial;

            /////////////////////////////////////
            //V-HALF WORD (16 BIT) ENCODING
            ////////////////////////////////////
            TWOBYTE += std::ceil(bitPartial/15)*16;

            /////////////////////////////////////
            //VB ENCODING
            ////////////////////////////////////
            byte += std::ceil(bitPartial/7)*8;

            /////////////////////////////////////
            //V-NIBBLES BYTE ENCODING
            ////////////////////////////////////
            nibbles += std::ceil(bitPartial/3)*4;

            /////////////////////////////////////
            //V-HALF-NIBBLES BYTE ENCODING
            ////////////////////////////////////
            halfNibbles += std::ceil(bitPartial/1)*2;

            /////////////////////////////////////
            //GAMMA CODE ENCODING
            ////////////////////////////////////
            gamma += logPartial + logPartial + 1;

            /////////////////////////////////////
            //DELTA CODE ENCODING
            ////////////////////////////////////
            float appo = std::floor(log2(logPartial+1));
            delta += logPartial  + appo + appo  +1;

        }
    }

    std::cout << "RAW-BIT: " << rawbit / gap  << "   Total size in KB: " << rawbit/1024 << std::endl;
    std::cout << "VARIABLE 2 BYTE, Average bit for gap is: " << TWOBYTE /gap << "   Total size in KB: " << TWOBYTE / 1024 << std::endl;
    std::cout << "VARIABLE BYTE, Average bit for gap is: " << byte/gap << "   Total size in KB: " << byte / 1024 << std::endl;
    std::cout << "VARIABLE NIBBLES, Average bit for gap is: " << nibbles/gap << "   Total size in KB: " << nibbles / 1024 << std::endl;
    std::cout << "VARIABLE HALF-NIBBLES, Average bit for gap is: " << halfNibbles/gap << "   Total size in KB: " << halfNibbles / 1024 << std::endl;
    std::cout << "ELIAS GAMMA CODE, Average bit for gap is: " << gamma/gap << "   Total size in KB: " << gamma / 1024 << std::endl;
    std::cout << "ELIAS DELTA CODE, Average bit for gap is: " << delta/gap << "   Total size in KB: " << delta / 1024 << std::endl;
    std::cout << "##################################################" << std::endl << std::endl;
}


float computeJaccard(std::set<std::string> set1, std::set<std::string> set2, bool normalize) {
    float unione = 0, intersezione = 0;

    for (auto elem : set1)
        if(set2.find(elem) == set2.end())
            unione++;
        else{
            unione++;
            intersezione++;
        }

    for (auto elem : set2)
        if(set1.find(elem) == set1.end())
            unione++;

    if(normalize)
        return intersezione/std::sqrt(unione);
    else
        return intersezione / unione;
}


std::vector<int> nearestNeighbor(std::vector<std::vector<float>> matrix){

    std::vector<int> result;
    float absMin = FLT_MAX;
    int matrixSize = matrix.size();

    //CHANGE STARTING POINT
    for(int i = 0; i< matrixSize; i++) {
        std::vector<int> yetVisited;
        yetVisited.push_back(i);

        int pos = i;

        float pathlengh =0;
        //INITIAL
        for (int j = 0; j < matrixSize - 1; ++j) {
            float min = FLT_MAX;
            int minPos = INT_MAX;
            for (int k = 0; k < matrixSize; ++k) {
                if (matrix.at(pos).at(k) < min &&
                    std::find(yetVisited.begin(), yetVisited.end(), k) == yetVisited.end()) {
                    min = matrix.at(pos).at(k);
                    minPos = k;
                }
            }
            pathlengh += min;
            yetVisited.push_back(minPos);
            pos = minPos;
        }

        if(pathlengh < absMin){
             absMin = pathlengh;
             result = yetVisited;
        }

    }

    return  result;
}



int main(int argc, char** argv) {

    int TOTAL_DOCUMENT;
    float RADIUS;  //RADIUS
    bool NORMALIZEDJACCARD;

    

    if (argc != 4) return -1;
    else {
        TOTAL_DOCUMENT = std::stoi(argv[1]);
        RADIUS = std::stof(argv[2]);
        (argv[3][0] == '0')? NORMALIZEDJACCARD=false: NORMALIZEDJACCARD=true;
    }

    std::cout << TOTAL_DOCUMENT << "  " << RADIUS << "  " << NORMALIZEDJACCARD << std::endl;

    std::map<std::string, std::vector<int>> posting;                 //VOCABULARY + POSTING LIST

    std::vector<Doc> vectorDoc;                                      //ARRAY OF DOCUMENTS

    std::set<std::string> stopwords;                                  //STOPWORD 

    

    ///////////////////////////////////////////////////////////////
    // STOP WORD
    //////////////////////////////////////////////////////////////
    std::cout << "Creating stopword list..." << std::endl;
    std::ifstream data("STOPWORD.txt");
    std::string word;
    while(std::getline(data,word))
        stopwords.insert(word);


    ///////////////////////////////////////////////////////////////
    // VOCABULARY & POSTING LIST
    //////////////////////////////////////////////////////////////
    std::cout << "Creating vocabulary and posting list..." << std::endl;


    for (int i = 1; i < TOTAL_DOCUMENT; i++) {
        std::ifstream data(std::to_string(i) + ".txt");

        //CREATION OF THE DOCUMENT
        Doc myObj;
        int nWord = 0;

        while (std::getline(data, word, ' ')) {

            //APPLIYING SOME FILTERING
            {
                std::transform(word.begin(), word.end(), word.begin(), ::tolower);
                word.erase(std::remove(word.begin(), word.end(), '`'), word.end());
                word.erase(std::remove(word.begin(), word.end(), '\n'), word.end());
                word.erase(std::remove(word.begin(), word.end(), '\r'), word.end());
                word.erase(std::remove(word.begin(), word.end(), '\''), word.end());
                word.erase(std::remove(word.begin(), word.end(), '\\'), word.end());
                word.erase(std::remove(word.begin(), word.end(), '\"'), word.end());
                word.erase(std::remove(word.begin(), word.end(), '!'), word.end());
                word.erase(std::remove(word.begin(), word.end(), ','), word.end());
                word.erase(std::remove(word.begin(), word.end(), '.'), word.end());
                word.erase(std::remove(word.begin(), word.end(), '('), word.end());
                word.erase(std::remove(word.begin(), word.end(), ')'), word.end());
                word.erase(std::remove(word.begin(), word.end(), '['), word.end());
                word.erase(std::remove(word.begin(), word.end(), ']'), word.end());
                word.erase(std::remove(word.begin(), word.end(), '?'), word.end());
                word.erase(std::remove(word.begin(), word.end(), '&'), word.end());
                word.erase(std::remove(word.begin(), word.end(), '-'), word.end());
                word.erase(std::remove(word.begin(), word.end(), '/'), word.end());
                word.erase(std::remove(word.begin(), word.end(), '@'), word.end());
                word.erase(std::remove(word.begin(), word.end(), '1'), word.end());
                word.erase(std::remove(word.begin(), word.end(), '2'), word.end());
                word.erase(std::remove(word.begin(), word.end(), '3'), word.end());
                word.erase(std::remove(word.begin(), word.end(), '4'), word.end());
                word.erase(std::remove(word.begin(), word.end(), '5'), word.end());
                word.erase(std::remove(word.begin(), word.end(), '6'), word.end());
                word.erase(std::remove(word.begin(), word.end(), '7'), word.end());
                word.erase(std::remove(word.begin(), word.end(), '8'), word.end());
                word.erase(std::remove(word.begin(), word.end(), '9'), word.end());
                word.erase(std::remove(word.begin(), word.end(), '0'), word.end());
                word.erase(std::remove(word.begin(), word.end(), '$'), word.end());
                word.erase(std::remove(word.begin(), word.end(), '%'), word.end());
                word.erase(std::remove(word.begin(), word.end(), ';'), word.end());
                word.erase(std::remove(word.begin(), word.end(), '<'), word.end());
                word.erase(std::remove(word.begin(), word.end(), '>'), word.end());
                word.erase(std::remove(word.begin(), word.end(), '#'), word.end());
                word.erase(std::remove(word.begin(), word.end(), '*'), word.end());
                word.erase(std::remove(word.begin(), word.end(), '+'), word.end());
                word.erase(std::remove(word.begin(), word.end(), '-'), word.end());
                word.erase(std::remove(word.begin(), word.end(), '='), word.end());
                word.erase(std::remove(word.begin(), word.end(), ':'), word.end());
            }

            if (word.compare("") == 0) continue;

            if ((stopwords.find(word) == stopwords.end()) && (myObj.docVocabulary.find(word) ==
                                                            myObj.docVocabulary.end())) {               //CHECK IN DOC VOCABULARY

                myObj.docVocabulary.insert(word);

                if (posting.find(word) ==
                    posting.end()) {                                     // NOT FOUND IN GLOBAL VOCABULARY
                    std::vector<int> vect;
                    vect.push_back(i);

                    posting.insert({word, vect});                        //INSERT INTO GLOBAL VOCABULARY

                } else {                                                // FOUND IN GLOBAL VOCABULARY
                    posting.find(word)->second.push_back(i);                       //STORING DOCID
                }

            }
            nWord++;
        }
        myObj.docId = i;
        myObj.lenght = nWord;
        vectorDoc.push_back(myObj);                                          //INSERT DOCUMENTO
    }



    ///////////////////////////////////////////////////////////////
    // AVERAGE BIT GAP
    //////////////////////////////////////////////////////////////
    printAverageGap(posting);


    ///////////////////////////////////////////////////////////////
    // REORDERING DOC BY LENGTH
    //////////////////////////////////////////////////////////////
    std::cout << "Reordering doc..." << std::endl;
    std::sort(vectorDoc.begin(), vectorDoc.end());


    ///////////////////////////////////////////////////////////////
    // APPLYING JACCARD AND CREATE MEDOID
    //////////////////////////////////////////////////////////////
    std::cout << "Apllying JACCARD to find medoid..." <<  std::endl;
    std::vector<std::pair<Doc, std::vector<Doc>>> medoid;

   
    int cont = 0;
    int tenpercent = TOTAL_DOCUMENT*0.10;

    for(Doc it : vectorDoc){
        cont++;

        if(--tenpercent == 0) {   //FOR WATCHING PROGRESS
            int percentage = (float)cont / (float)TOTAL_DOCUMENT * 100;
            std::cout << "JACCARD progress: " << percentage << " %"<< std::endl;
            tenpercent = TOTAL_DOCUMENT*0.10;
        }

        float min = FLT_MAX;
        std::vector<Doc>* medoidFind;

        for (auto & elem : medoid){
            float distance = 1 - computeJaccard(it.docVocabulary, elem.first.docVocabulary, NORMALIZEDJACCARD);
            if(distance<min){
                min = distance;
                medoidFind  =  &(elem.second);
            }
        }

        if(min < RADIUS)
            medoidFind->push_back(it);

        else {
            std::vector<Doc> singleton {it};
            medoid.push_back({it,singleton});
        }
    }

    std::cout << "Number of medoid found: " << medoid.size() << std::endl;

    ///////////////////////////////////////////////////////////////
    // APPLYING TSP ON MEDOID
    //////////////////////////////////////////////////////////////
    std::cout << "Apllying TSP to medoid..." <<  std::endl;

    std::vector<std::vector<float>> matrix(medoid.size(), std::vector<float>(medoid.size()));

    std::cout << "1) Computing jaccard between medoid..." <<  std::endl;
    for (int i = 0; i < medoid.size(); ++i)
        for (int j = 0; j < medoid.size(); ++j){
            if(i==j) matrix[i][j] = 0;
            else matrix[i][j] = computeJaccard(medoid.at(i).first.docVocabulary, medoid.at(j).first.docVocabulary, NORMALIZEDJACCARD);
        }

    std::cout << "2) Calculating TSP on medoid..." <<  std::endl;
    std::vector<int> minimunPathIndexResult = nearestNeighbor(matrix);
    std::cout << "TSP found: "  <<  std::endl;

    for (int i: minimunPathIndexResult)
        std::cout << i << ' ';
    //WE PRINT AT THE END ALSO THE STARTIGN MEDOID OF THE TSP INDUCED ORDER, BECAUSE THE STARTING MEDOID IS ALSO THE FINAL MEDOID
    std::cout << minimunPathIndexResult[0] << ' ' << std::endl;                 

    vectorDoc.clear();                                                           //CLEAR VECTOR DOC

    std::cout << "Reassign doc id using TSP induced order..." <<  std::endl;

    int NewDocIdCont=1;
    for(int i : minimunPathIndexResult)                                     //ITERATE OVER TSP INDUCED ORDER
        for(Doc & j : medoid.at(i).second) {                               //GET LIST OF DOC UNDER MEDOID
            j.NewDocId = NewDocIdCont++;                                    //ASSING NEW CONTIGUOS DOC ID
            vectorDoc.push_back(j);
        }



    ///////////////////////////////////////////////////////////////
    //SCANNING POSTING LIST AND REWRITE NEW DOCID
    //////////////////////////////////////////////////////////////
    std::cout << "Replacing new doc id in posting list..." <<  std::endl;

    for (auto it = posting.begin(); it != posting.end(); it++){
        for (auto & element : it->second ){
            auto appo = std::find_if(vectorDoc.begin(), vectorDoc.end(),  [=](const Doc& e) {
                 if(e.docId == element)
                     return true;
                 else
                     return false;
            } );
            element = appo->NewDocId;
        }
    }



    ///////////////////////////////////////////////////////////////
    //REORDER VECTOR OF DOCID IN POSTING LIST
    //////////////////////////////////////////////////////////////
    for (auto it = posting.begin(); it != posting.end(); it++)
        std::sort(it->second.begin(), it->second.end());



    ///////////////////////////////////////////////////////////////
    // RECOMPUTE AVERAGE BIT GAP
    /////////////////////////////////////////////////////////////
    printAverageGap(posting);


    return 0;
}







