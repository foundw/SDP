//
//  Perceptron.hpp
//  TransitionBasedSemanticDependencyParser
//
//  Created by FoundW on 2017/6/4.
//  Copyright © 2017 FoundW. All rights reserved.
//

#ifndef Perceptron_hpp
#define Perceptron_hpp

#include <stdio.h>
#include <memory.h>
#include <random>       // default_random_engine
#include <chrono>       // chrono::system_clock
#include <fstream>

#include "config.h"
#include "NGraph.hpp"

using namespace std;

class Perceptron{
private:
    vector<vector<int>> alpha;
#ifdef AVERAGE
    vector<vector<int>> avg_alpha;
#endif
    int y_dimension;
    int x_dimension;
public:
    Perceptron();
    void Train(vector<vector<NNode>>& sentlib, vector<NGraph>& glib);
    void Predict(vector<pair<vector<NNode>, string>>& sents, string res_file, bool average = false); // predict and decode
    void Graph2File(vector<NNode>& sent, NGraph& predict, ofstream& writer);
    void Graph2FileTest(vector<NGraph> glib, vector<vector<NNode>> train_sent_lib);
    NGraph GEN(vector<NNode>& sent, int beam, bool average = false);
    SCORE CalcScore(NGraph& current, int act, vector<int>& configurations, bool average = false);
    static bool CmpScore(const pair<int, pair<int, int>>& a, const pair<int, pair<int, int>>& b){
        return a.first > b.first;
    }
    void PunishAndMemory(vector<NNode>& nSet, NGraph& predict, vector<pair<int, int>>& feats);
    void WriteAlphaToFile(string file_path);
    void WriteAlphaToFile(string file_path, vector<vector<int>>& alpha_);
    void ReadAlphaFromFile(string file_path);
#ifdef AVERAGE
    void ReadAvgAlphaFromFile(string file_path);
#endif
};

#endif /* Perceptron_hpp */
