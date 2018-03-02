//
//  Encoder.cpp
//  TransitionBasedParser
//
//  Created by FoundW on 2017/6/17.
//  Copyright © 2017 FoundW. All rights reserved.
//

#include "Encoder.hpp"

pair<vector<vector<NNode>>, vector<NGraph>> Encoder::readinTrainingData(){
    ifstream i(train_path);
    string sID;
    vector<vector<NNode> > sent_lib;
    vector<NGraph> graph_lib;
    while(getline(i, sID)){
        // cout << "Processing: " + sID.substr(1) << endl;
        // train_id_set.push_back(sID);
        
        // Generate a graph as SDP for the sentence
        string line;
        int HdCnt = 1;
        unordered_map<int, int> HdID2ID;
        vector<NNode> nSet;
        vector<NEdge> eSet;
        vector<NEdge> eSetTmp;
        
        int f = fExtractor.proc_word("<TOP>");
        int le = fExtractor.proc_lemma("<TOP>");
        int p = fExtractor.proc_postag("<TOP>");
        
        nSet.push_back(NNode(f, le, p));
        
        while(getline(i, line)){
            if(line == ""){
                //cout << "Sentence Finish!" << endl;
                break;
            }
            int ID;
            int mHdID = -1;
            string form;
            string lemma;
            string POS;
            string isTop;
            string isHead;
            stringstream istr(line);
            istr >> ID >> form >> lemma >> POS >> isTop >> isHead;
#ifndef NOTOP
            if(isTop == "+"){
                eSet.push_back(NEdge(0, ID, fExtractor.proc_label("TOP")));
            }
#endif
            if(isHead == "+"){
                mHdID = HdCnt;
                HdID2ID[mHdID] = ID;
                HdCnt++;
            }
            
            // Encoding
            int f = fExtractor.proc_word(form);
            int le = fExtractor.proc_lemma(lemma);
            int p = fExtractor.proc_postag(POS);
            nSet.push_back(NNode(f, le, p));
            //cout << line << endl;
            string label;
            int HdID = 1;
            while(istr >> label){
                if(label != "_"){
#ifndef UNLABEL
                    int la = fExtractor.proc_label(label);
#else
                    int la = fExtractor.proc_label("X");
#endif
                    eSetTmp.push_back(NEdge(HdID, ID, la));
                }
                HdID++;
            }
        }
        
        // Modify the expression of edges
        for(NEdge e : eSetTmp){
            int hd = HdID2ID[e.get_head()];
            int dp = e.get_depd();
            int label = e.get_label();
            eSet.push_back(NEdge(hd, dp, label));
            //cout << "(" << hd << ", " << dp << ", " << label << ")" << endl;
        }
        NGraph g(nSet, eSet);
        graph_lib.push_back(g);
        sent_lib.push_back(nSet);
    }
    return make_pair(sent_lib, graph_lib);
}

vector<pair<vector<NNode>, string>> Encoder::readinTestData(bool dummy){
    string tpath = test_path;
    if(dummy){
        tpath = test_path_dummy;
    }
    ifstream i(tpath);
    string sID;
    vector<pair<vector<NNode>, string>> sents;
    while(getline(i, sID)){
        //cout << "Processing: " + sID.substr(1) << endl;
        string num;
        stringstream nrd(sID);
        nrd >> num;
        //cout << num << "|||" << endl;
        string line;
        vector<NNode> sent;
        int f = fExtractor.proc_word("<TOP>");
        int le = fExtractor.proc_lemma("<TOP>");
        int p = fExtractor.proc_postag("<TOP>");
        sent.push_back(NNode(f, le, p));
        while(getline(i, line)){
            if(line == ""){
                //cout << "Sentence Finish!" << endl;
                sents.push_back(make_pair(sent, num));
                break;
            }
            int ID;
            string form;
            string lemma;
            string POS;
            stringstream istr(line);
            istr >> ID >> form >> lemma >> POS;
            /*
            f = UNK;
            le = UNK;
            p = UNK;
            if(word2int.v2id.find(form) != word2int.v2id.end()){
                f = word2int.v2id[form];
            }
            if(lemma2int.v2id.find(lemma) != lemma2int.v2id.end()){
                le = lemma2int.v2id[lemma];
            }
            if(pos2int.v2id.find(POS) != pos2int.v2id.end()){
                p = pos2int.v2id[POS];
            }
            */
            f = fExtractor.proc_word(form);
            le = fExtractor.proc_lemma(lemma);
            p = fExtractor.proc_postag(POS);
            sent.push_back(NNode(f, le, p));
            //cout << line << endl;
        }
    }
    return sents;
}
