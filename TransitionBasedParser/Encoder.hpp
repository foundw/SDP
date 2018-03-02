//
//  Encoder.hpp
//  TransitionBasedParser
//
//  Created by FoundW on 2017/6/17.
//  Copyright © 2017 FoundW. All rights reserved.
//

#ifndef Encoder_hpp
#define Encoder_hpp

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "config.h"
#include "NGraph.hpp"

class Encoder{
private:
    string train_path = TRAIN_DATA;
    
#ifndef WATCH_FIT
    string test_path = TEST_DATA;
#else
    string test_path = TRAIN_DATA;
#endif
    
    string test_path_dummy = TEST_DUMMY;
public:
    pair<vector<vector<NNode>>, vector<NGraph>> readinTrainingData();
    vector<pair<vector<NNode>, string>> readinTestData(bool dummy = false);
};

#endif /* Encoder_hpp */
