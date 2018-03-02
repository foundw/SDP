//
//  NGraph.hpp
//  TransitionBasedParser
//
//  Created by FoundW on 2017/6/17.
//  Copyright © 2017 FoundW. All rights reserved.
//

#ifndef NGraph_hpp
#define NGraph_hpp

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>

#include "config.h"

using namespace std;

/* Encode into INT */

class NNode{
private:
    int form;
    int lemma;
    int POS;
public:
    NNode(){}
    NNode(int f, int l, int p){
        form = f;
        lemma = l;
        POS = p;
    }
    int get_form(){
        return form;
    }
    int get_lemma(){
        return lemma;
    }
    int get_POS(){
        return POS;
    }
};

class NEdge{
private:
    int head;
    int depd;
    int label;
public:
    NEdge(int h, int d, int l){
        head = h; depd = d; label = l;
    }
    int get_head(){return head;}
    int get_depd(){return depd;}
    int get_label(){return label;}
};

class NGraph{
private:
    // for ALL Graphs
    int V = 0;
    bool complete = false;                      // if the parsing complete
    vector<int> transitions;                    // part or complete
    vector<vector<int>> edges;
    
    // for golden Graphs
    vector<int> terminal;                       // tag for SHIFT decision,
    // terminal[i] is the leftmost node that has an edge connected with i
    vector<vector<int>> features;            // ((feature-set-1), (feature-set-2)...) corresponding to (transition1, transition2, ...)
    
    // for half-generated Graphs
    vector<int> ld;
    vector<int> rd;
    vector<vector<pair<int, int>>> hd;
    int score = 0;
    int lambda1 = 0;
    int beta = 1;
    
public:
    NGraph(vector<NNode>& nSet, vector<NEdge>& eSet);
    // Generate golden Graph from structured data read in
    NGraph(int V_);
    // Generate raw Graph
    NGraph(NGraph& base, int transition, int score_);
    // Derive a new graph based on the given one to add one transition
    NGraph();
    int size(){return V;}
    bool operator< (const NGraph& g) const{
        return g.score < score;
    }
    vector<vector<int>> get_features(){return features;}
    int get_score(){return score;}
    vector<int> get_transitions(){return transitions;}
    vector<vector<int>> get_edges(){return edges;}
    int get_edge_label(int from, int to){return edges[from][to];}
    bool isCompleted(){return complete;};
    vector<int> get_ld(){return ld;}
    vector<int> get_rd(){return rd;}
    int get_ld(int i){return ld[i];}
    int get_rd(int i){return rd[i];}
    vector<vector<pair<int, int>>> get_hd(){return hd;}
    vector<pair<int, int>> get_hd(int i){return hd[i];}
    int get_lambda1(){return lambda1;}
    int get_beta(){return beta;}
};

/* DICTs AS GLOBAL VARIABLE */
template<typename KEY_TYPE>
struct Dict{
    unordered_map<KEY_TYPE, int> v2id;
    vector<KEY_TYPE> id2v;
    int size = 0;
    int lookup(KEY_TYPE value, bool insert = true){
        if(v2id.find(value) == v2id.end()){
            if(!insert){
                return 0;
            }
            v2id[value] = size;
            id2v.push_back(value);
            size++;
            return (size-1);
        }
        else{
            return v2id[value];
        }
    }
    void write2File(string filepath, bool byIdOrder = false){
        ofstream out(filepath);
        out << size << endl;
        if(byIdOrder){
            for(int i = 0; i < size; i++){
                out << i << "\t" << id2v[i] << endl;
            }
        }
        else{
            for (pair<KEY_TYPE, int> i : v2id){
                out << i.first << "\t" << i.second << endl;
            }
        }
        out.close();
    }
    void readDictFromFile(string file_path){
        ifstream in(file_path, ios::in);
        in >> size;
        v2id.clear();
        id2v = vector<KEY_TYPE>(size);
        int idx;
        KEY_TYPE key;
        while(in >> key >> idx){
            v2id[key] = idx;
            id2v[idx] = key;
        }
        in.close();
    }
    void readFDictFromFile(string file_path){
        ifstream in(file_path, ios::in);
        in >> size;
        v2id.clear();
        id2v = vector<KEY_TYPE>(size);
        int idx;
        KEY_TYPE key;
        while(in >> idx >> key){
            v2id[key] = idx;
            id2v[idx] = key;
        }
        in.close();
    }
};

typedef Dict<int> IntDict;
typedef Dict<string> StrDict;

class FeatureExtractor{
private:
    IntDict fDict = IntDict(); // unigram feature dictionary
    //IntDict bifDict = IntDict(); // bigram feature dictionary
    IntDict tDict = IntDict(); // transition dictionary
    
    StrDict word2int = StrDict();
    StrDict lemma2int = StrDict();
    StrDict pos2int = StrDict();
    StrDict label2int = StrDict();
    
public:
    //empty structure used as dummy param
    vector<int> empty_list = vector<int>(0);
    vector<vector<pair<int, int>>> empty2D_pair_list = vector<vector<pair<int, int>>>(0);
    vector<vector<int>> empty2D_list = vector<vector<int>>(0);
    
    int proc_word(string wd){return word2int.lookup(wd);}
    int proc_lemma(string lm){return lemma2int.lookup(lm);}
    int proc_postag(string pos){return pos2int.lookup(pos);}
    int proc_label(string lb){return label2int.lookup(lb);}
    int proc_trans(int trans){return tDict.lookup(trans);}
    int proc_feat(int fint){return fDict.lookup(fint);}
    int proc_feat(int fint, bool insert){return fDict.lookup(fint, insert);}
    
    string get_word(int wid){return word2int.id2v[wid];}
    string get_lemma(int lid){return lemma2int.id2v[lid];}
    string get_postag(int posid){return pos2int.id2v[posid];}
    string get_label(int lbsid){return label2int.id2v[lbsid];}
    int decode_trans(int tid){return tDict.id2v[tid];}
    int feats_size(){return fDict.size;}
    int trans_size(){return tDict.size;}
    //int trans_size(){return tDict.size + bifDict.size();}
    int genFeature_t_5_2(int type, int val1 = VOID, int val2 = VOID, int val3 = VOID);
    int genBigramFeature(int type, int val1 = VOID, int val2 = VOID, int val3 = VOID);
    void extractFeatures(vector<NNode>& nSet, int beta_st, int lambda1_ed, vector<vector<int>>& edges, vector<int> ld, vector<int> rd, vector<vector<pair<int, int>>> hd, vector<int>& features, bool add_new = true);
    void extractFeatures(vector<NNode>& nSet, int beta_st, int lambda1_ed, vector<vector<int>>& edges, vector<int>& features){
        extractFeatures(nSet, beta_st, lambda1_ed, edges, empty_list, empty_list, empty2D_pair_list, features);
    }
    void extractFeatures(vector<NNode>& nSet, NGraph& current, vector<int>& features){
        int beta_st = current.get_beta();
        int lambda1_ed = current.get_lambda1();
        vector<int> ld = current.get_ld();
        vector<int> rd = current.get_rd();
        vector<vector<pair<int, int>>> hd = current.get_hd();
        extractFeatures(nSet, beta_st, lambda1_ed, empty2D_list, ld, rd, hd, features, false);
    }
    void writeFDict(string fpath){fDict.write2File(fpath, true);}
    void writeInitDict(){
        tDict.write2File(TRANSITION_DICT_PATH);
        writeFDict(FEATURE_DICT_PATH);
        word2int.write2File(WORD_DICT_PATH);
        lemma2int.write2File(LEMMA_DICT_PATH);
        pos2int.write2File(POS_DICT_PATH);
        label2int.write2File(LABEL_DICT_PATH);
    }
    void loadDicts(){
        tDict.readDictFromFile(TRANSITION_DICT_PATH);
        fDict.readFDictFromFile(FEATURE_DICT_PATH);
        word2int.readDictFromFile(WORD_DICT_PATH);
        lemma2int.readDictFromFile(LEMMA_DICT_PATH);
        pos2int.readDictFromFile(POS_DICT_PATH);
        label2int.readDictFromFile(LABEL_DICT_PATH);
    }
};

extern void PrintSent(vector<NNode>& sent);
extern void PrintConfiguration(vector<NNode> sent, NGraph dep);
extern FeatureExtractor fExtractor;

#endif /* NGraph_hpp */
