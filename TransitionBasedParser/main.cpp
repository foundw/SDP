//
//  main.cpp
//  TransitionBasedSemanticDependencyParser
//
//  Created by FoundW on 2017/5/31.
//  Copyright © 2017 FoundW. All rights reserved.
//

#include <iostream>
#include <sstream>
#include <map>

#include "config.h"
#include "Encoder.hpp"
#include "Perceptron.hpp"

void trainModel(){
    // Preparation for Dict Construction
    fExtractor.proc_feat(NORES);
    fExtractor.proc_trans(SHIFT);
    fExtractor.proc_trans(NOARC);
    fExtractor.proc_label("<null>");
    
    Encoder R = Encoder();
    pair<vector<vector<NNode>>, vector<NGraph>> data_read_in = R.readinTrainingData();
    vector<NGraph> glib = data_read_in.second;
    vector<vector<NNode>> train_sent_lib = data_read_in.first;
    fExtractor.writeInitDict();
    
    Perceptron p = Perceptron();
    
    //cout << "DECODING TEST!" << endl;
    //p.DecodeTest(train_sent_lib[0], glib[0]);
    
    cout << "BEGIN TRAINING!" << endl;
#ifdef PRINT_DETAIL
    fstream detail(LOG_PATH, ios::out);
#endif
    p.Train(train_sent_lib, glib);
    cout << "FINISH TRAINING!\n";
#ifdef PRINT_DETAIL
    detail.close();
#endif
    
    cout << "NOW TESTING!\n";
    vector<pair<vector<NNode>, string>> t = R.readinTestData();
    p.Predict(t, RESULT_FILE);
}

void predictWithPreTrainedModel(){
    fExtractor.loadDicts();
    Perceptron p = Perceptron();
    p.ReadAlphaFromFile(SAVED_ALPHA);
    Encoder R = Encoder();
    cout << "NOW TESTING!\n" << endl;
    vector<pair<vector<NNode>, string>> t = R.readinTestData();
    p.Predict(t, RESULT_FILE);
}

int main(int argc, const char * argv[]) {
    if(MODE == 1){
        trainModel();
    }
    else if(MODE == 0){
        predictWithPreTrainedModel();
    }
    else{
        cout << "Wrong MODE code! Exit without doing anything." << endl;
    }
    return 0;
}
