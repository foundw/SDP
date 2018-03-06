//
//  NGraph.cpp
//  TransitionBasedParser
//
//  Created by FoundW on 2017/6/17.
//  Copyright © 2017 FoundW. All rights reserved.
//

#include "NGraph.hpp"

NGraph::NGraph(vector<NNode>& nSet, vector<NEdge>& eSet){
    V = (int)nSet.size();
    terminal = vector<int>(V);
    complete = true;
    
    //edge_noted = true;
    edges = vector<vector<int>>(V);
    for(int i = 0; i < V; i++){
        edges[i] = vector<int>(V);
        for(int j = 0; j < V; j++){
            edges[i][j] = VOID;
        }
    }
    
    for(NEdge e : eSet){
        int hd = e.get_head();
        int dp = e.get_depd();
        int label = e.get_label();
        
        edges[hd][dp] = label;
    }
    
    for(int i = 0; i < V; i++){
        terminal[i] = i;
        for(int j = 0; j < i; j++){
            if(edges[i][j] != VOID || edges[j][i] != VOID){
                terminal[i] = j;
                break;
            }
        }
    }
    
    // extract features and golden transition sequence, and transition dictionary extending
    for(int beta_st = 1; beta_st < V; beta_st++){
        int lambda1_ed = beta_st - 1;
        while(true){
            // transition deduction
            int trans = SHIFT;
            if(terminal[beta_st] > lambda1_ed || lambda1_ed == -1){
                ;
            }
            else if(edges[lambda1_ed][beta_st] != VOID){  // Right-Arc
                trans = 100 + edges[lambda1_ed][beta_st];
                trans = fExtractor.proc_trans(trans);
            }
            else if(edges[beta_st][lambda1_ed] != VOID){  // Left-Arc
                trans = 200 + edges[beta_st][lambda1_ed];
                trans = fExtractor.proc_trans(trans);
            }
            else{
                trans = NOARC;
            }
            transitions.push_back(trans);
            
            vector<int> feats = vector<int>(0);
            fExtractor.extractFeatures(nSet, beta_st, lambda1_ed, edges, feats);
            features.push_back(feats);
            
            if(trans == SHIFT){
                break;
            }
            lambda1_ed--;
        }
    }
}

NGraph::NGraph(int V_){
    V = V_;
    complete = false;
    transitions = vector<int>();
    edges = vector<vector<int>>(V);
    for(int i = 0; i < V; i++){
        edges[i] = vector<int>(V);
        for(int j = 0; j < V; j++){
            edges[i][j] = VOID;
        }
    }
    // for half-generated Graphs
    ld = vector<int>(V);
    rd = vector<int>(V);
    hd = vector<vector<pair<int, int>>>(V);
    for(int i = 0 ; i < V; i++){
        hd[i] = vector<pair<int, int>>(0);
        ld[i] = VOID;
        rd[i] = VOID;
    }
    score = 0;
    lambda1 = 0;
    beta = 1;
}

NGraph::NGraph(NGraph& base, int transition, SCORE score_){
    V = base.size();
    complete = base.isCompleted();
    transitions = base.get_transitions();
    ld = base.get_ld();
    rd = base.get_rd();
    hd = base.get_hd();
    score = score_;
    lambda1 = base.get_lambda1();
    beta = base.get_beta();
    edges = base.get_edges();
    //features = base.get_features();
    
    transitions.push_back(transition);
    //features.insert(features.end(), fs.begin(), fs.end());
    int trans = fExtractor.decode_trans(transition);
    if(trans >= 200){ // Left-Arc
        int label = trans - 200;
        edges[beta][lambda1] = label;
        ld[beta] = label;
        hd[lambda1].push_back(make_pair(beta, label));
    }
    else if(trans >= 100){ //Right-Arc
        int label = trans - 100;
        edges[lambda1][beta] = label;
        rd[lambda1] = label;
        hd[beta].push_back(make_pair(lambda1, label));
    }
    
    if(trans == SHIFT){
        beta += 1;
        lambda1 = beta - 1;
    }
    else{
        lambda1--;
    }
    
    if(beta == V){
        complete = true;
    }
}

NGraph::NGraph(){
    ;
    // checkpoint
}

void PrintSent(vector<NNode>& sent){
    int l = (int)sent.size();
    for(int i = 0; i < l; i++){
        cout << fExtractor.get_word(sent[i].get_form()) << " ";
    }
    cout << endl;
}

// Print the configuration after a transition is committed
void PrintConfiguration(vector<NNode> sent, NGraph dep){
    int lam1 = dep.get_lambda1();
    int beta = dep.get_beta();
    int l = (int)sent.size();
    cout << "[";
    for(int i = 0; i <= lam1; i++){
        cout << fExtractor.get_word(sent[i].get_form()) << " ";
    }
    cout << "]\t[";
    for(int i = lam1 + 1; i < beta; i++){
        cout << fExtractor.get_word(sent[i].get_form()) << " ";
    }
    cout << "]\t[";
    for(int i = beta; i < l; i++){
        cout << fExtractor.get_word(sent[i].get_form()) << " ";
    }
    cout << "]\n";
    
    
    vector<vector<int>> eSet = dep.get_edges();
    for(int i = 0; i < l; i++){
        for(int j = 0; j < l; j++){
            if(eSet[i][j] != VOID){
                cout << "(" << i << ", " << j << ", " << fExtractor.get_label(eSet[i][j]) << ") ";
            }
        }
    }
    cout << "\nSCORE = " << dep.get_score() << endl;
}

/*
 * feature extraction - GLOBAL FUNCTION
 * encode feature: F = TYPE-VAL1-VAL2, to calculate:
 *   feature = 10000000 * TYPE + 100 * VAL1 + VAL2
 */
FEATURE FeatureExtractor::genFeature_t_5_2(long long type, long long val1, long long val2){
    return 10000000 * type + 100 * val1 + val2;
}

FEATURE FeatureExtractor::genBigramFeature(long long type, long long lm1, long long lm2, long long ps1, long long ps2){
    return type * 100000000000000 + lm1 * VACAB_SIZE_MAX + lm2 + ps1 * PTAG_SIZE_MAX + ps2;
}

void FeatureExtractor::extractFeatures(vector<NNode>& nSet, int beta_st, int lambda1_ed, vector<vector<int>>& edges, vector<int> ld, vector<int> rd, vector<vector<pair<int, int>>> hd, vector<int>& features, bool add_new){
    int V = (int)nSet.size();
    int wd_beta = nSet[beta_st].get_form();
    int lm_beta = nSet[beta_st].get_lemma();
    int ps_beta = nSet[beta_st].get_POS();
    int distance = beta_st - lambda1_ed;
    
    // unigram feature 1
    FEATURE f = genFeature_t_5_2(1, wd_beta);
    int fid = proc_feat(f, add_new);
    features.push_back(fid);
    // unigram feature 2
    f = genFeature_t_5_2(2, lm_beta);
    fid = proc_feat(f, add_new);
    features.push_back(fid);
    // unigram feature 3
    f = genFeature_t_5_2(3, ps_beta);
    fid = proc_feat(f, add_new);
    features.push_back(fid);
    // unigram feature 4
    f = genFeature_t_5_2(4, lm_beta, ps_beta);
    fid = proc_feat(f, add_new);
    features.push_back(fid);
    
    int edge_label = VOID;
    if(ld.size() > 0){
        edge_label = ld[beta_st];
    }
    else{
        int ld_beta = lambda1_ed + 1;
        while(ld_beta < beta_st){
            if(edges[beta_st][ld_beta] != VOID){
                break;
            }
            ld_beta++;
        }
        if(ld_beta < beta_st){
            edge_label = edges[beta_st][ld_beta];
        }
    }
    // structural feature 1
    f = genFeature_t_5_2(51, edge_label);
    fid = proc_feat(f, add_new);
    features.push_back(fid);
    // structural feature 2
    f = genFeature_t_5_2(52, lm_beta, edge_label);
    fid = proc_feat(f, add_new);
    features.push_back(fid);
    // structural feature 3
    f = genFeature_t_5_2(53, ps_beta, edge_label);
    fid = proc_feat(f, add_new);
    features.push_back(fid);
    
    if(lambda1_ed > -1){
        int wd_lam1 = nSet[lambda1_ed].get_form();
        int lm_lam1 = nSet[lambda1_ed].get_lemma();
        int ps_lam1 = nSet[lambda1_ed].get_POS();
        // unigram feature 5
        f = genFeature_t_5_2(5, wd_lam1);
        fid = proc_feat(f, add_new);
        features.push_back(fid);
        // unigram feature 6
        f = genFeature_t_5_2(6, lm_lam1);
        fid = proc_feat(f, add_new);
        features.push_back(fid);
        // unigram feature 7
        f = genFeature_t_5_2(7, ps_lam1);
        fid = proc_feat(f, add_new);
        features.push_back(fid);
        // unigram feature 8
        f = genFeature_t_5_2(8, lm_lam1, ps_lam1);
        fid = proc_feat(f, add_new);
        features.push_back(fid);
        
        // get Dep(ld(lam1[0]))
        
        
        int lam1_lc_label = VOID;
        if(ld.size() > 0){
            lam1_lc_label = ld[lambda1_ed];
        }
        else{
            int ld_lam1 = 1;
            while(ld_lam1 < lambda1_ed){
                if(edges[lambda1_ed][ld_lam1] != VOID){
                    break;
                }
                ld_lam1++;
            }
            
            if(ld_lam1 < lambda1_ed){
                lam1_lc_label = edges[lambda1_ed][ld_lam1];
            }
        }
        // structural feature 4
        f = genFeature_t_5_2(54, lam1_lc_label);
        fid = proc_feat(f, add_new);
        features.push_back(fid);
        // structural feature 5
        f = genFeature_t_5_2(55, lm_lam1, lam1_lc_label);
        fid = proc_feat(f, add_new);
        features.push_back(fid);
        // structural feature 6
        f = genFeature_t_5_2(56, ps_lam1, lam1_lc_label);
        fid = proc_feat(f, add_new);
        features.push_back(fid);
        
        
        // get Dep(rd(lam1[0]))
        int lam1_rc_label = VOID;
        if(rd.size() > 0){
            lam1_rc_label = rd[lambda1_ed];
        }
        else{
            int rd_lam1 = beta_st - 1;
            while(rd_lam1 > lambda1_ed){
                if(edges[lambda1_ed][rd_lam1] != VOID){
                    break;
                }
                rd_lam1--;
            }
            if(rd_lam1 > lambda1_ed){
                lam1_rc_label = edges[lambda1_ed][rd_lam1];
            }
        }
        // structural feature 7
        f = genFeature_t_5_2(57, lam1_rc_label);
        fid = proc_feat(f, add_new);
        features.push_back(fid);
        // structural feature 8
        f = genFeature_t_5_2(58, lm_lam1, lam1_rc_label);
        fid = proc_feat(f, add_new);
        features.push_back(fid);
        // structural feature 9
        f = genFeature_t_5_2(59, ps_lam1, lam1_rc_label);
        fid = proc_feat(f, add_new);
        features.push_back(fid);
        
        // Head of lam1[0] and the Denpendency Label (possible more than one)
        // structural feature 10
        bool nohead = true;
        if (hd.size() > 0){
            vector<pair<int, int>> heads = hd[lambda1_ed];
            if(!heads.empty()){
                nohead = false;
                for(pair<int, int> head : heads){
                    f = genFeature_t_5_2(60, nSet[head.first].get_lemma(), head.second);
                    fid = proc_feat(f, add_new);
                    features.push_back(fid);
                }
            }
        }
        else{
            for(int i = 0; i < beta_st; i++){
                if(edges[i][lambda1_ed] != VOID){
                    nohead = false;
                    f = genFeature_t_5_2(60, nSet[i].get_lemma(), edges[i][lambda1_ed]);
                    fid = proc_feat(f, add_new);
                    features.push_back(fid);
                }
            }
        }
        if(nohead){
            f = genFeature_t_5_2(60, NONODE, VOID);
            fid = proc_feat(f, add_new);
            features.push_back(fid);
        }
        
        // bi-gram features
        f = genBigramFeature(80, lm_lam1, lm_beta);
        fid = proc_feat(f, add_new);
        features.push_back(fid);
        f = genFeature_t_5_2(81, lm_lam1, ps_beta);
        fid = proc_feat(f, add_new);
        features.push_back(fid);
        f = genFeature_t_5_2(82, lm_beta, ps_lam1);
        fid = proc_feat(f, add_new);
        features.push_back(fid);
        f = genFeature_t_5_2(83, ps_lam1, ps_beta);
        fid = proc_feat(f, add_new);
        features.push_back(fid);
        f = genBigramFeature(84, lm_lam1, lm_beta, ps_lam1, ps_beta);
        fid = proc_feat(f, add_new);
        features.push_back(fid);
        f = genBigramFeature(85, lm_lam1, lm_beta, distance);
        fid = proc_feat(f, add_new);
        features.push_back(fid);
        f = genBigramFeature(86, lm_lam1, VOID, ps_beta, distance);
        fid = proc_feat(f, add_new);
        features.push_back(fid);
        f = genBigramFeature(87, VOID, lm_beta, ps_lam1, distance);
        fid = proc_feat(f, add_new);
        features.push_back(fid);
        f = genBigramFeature(88, distance, ps_lam1, ps_beta);
        fid = proc_feat(f, add_new);
        features.push_back(fid);
        
        if(lambda1_ed > 0){
            int wd_lam1_1 = nSet[lambda1_ed-1].get_form();
            int lm_lam1_1 = nSet[lambda1_ed-1].get_lemma();
            int ps_lam1_1 = nSet[lambda1_ed-1].get_POS();
            // unigram feature 9
            f = genFeature_t_5_2(9, wd_lam1_1);
            fid = proc_feat(f, add_new);
            features.push_back(fid);
            // unigram feature 10
            f = genFeature_t_5_2(10, lm_lam1_1);
            fid = proc_feat(f, add_new);
            features.push_back(fid);
            // unigram feature 11
            f = genFeature_t_5_2(11, ps_lam1_1);
            fid = proc_feat(f, add_new);
            features.push_back(fid);
            // unigram feature 12
            f = genFeature_t_5_2(12, lm_lam1_1, ps_lam1_1);
            fid = proc_feat(f, add_new);
            features.push_back(fid);
            
            // bi-gram features
            f = genBigramFeature(90, lm_lam1_1, lm_lam1);
            fid = proc_feat(f, add_new);
            features.push_back(fid);
            f = genFeature_t_5_2(91, lm_lam1_1, ps_lam1);
            fid = proc_feat(f, add_new);
            features.push_back(fid);
            f = genFeature_t_5_2(92, lm_lam1, ps_lam1_1);
            fid = proc_feat(f, add_new);
            features.push_back(fid);
            f = genFeature_t_5_2(93, ps_lam1_1, ps_lam1);
            fid = proc_feat(f, add_new);
            features.push_back(fid);
            f = genBigramFeature(94, lm_lam1_1, lm_lam1, ps_lam1_1, ps_lam1);
            fid = proc_feat(f, add_new);
            features.push_back(fid);
        }
    }
    
    
    
    if(beta_st + 1 < V){
        int wd_beta_1 = nSet[beta_st+1].get_form();
        int lm_beta_1 = nSet[beta_st+1].get_lemma();
        int ps_beta_1 = nSet[beta_st+1].get_POS();
        // unigram feature 13
        f = genFeature_t_5_2(13, wd_beta_1);
        fid = proc_feat(f, add_new);
        features.push_back(fid);
        // unigram feature 14
        f = genFeature_t_5_2(14, lm_beta_1);
        fid = proc_feat(f, add_new);
        features.push_back(fid);
        // unigram feature 15
        f = genFeature_t_5_2(15, ps_beta_1);
        fid = proc_feat(f, add_new);
        features.push_back(fid);
        // unigram feature 16
        f = genFeature_t_5_2(16, lm_beta_1, ps_beta_1);
        fid = proc_feat(f, add_new);
        features.push_back(fid);
        
        // bi-gram features
        f = genBigramFeature(100, lm_beta, lm_beta_1);
        fid = proc_feat(f, add_new);
        features.push_back(fid);
        f = genFeature_t_5_2(101, lm_beta, ps_beta_1);
        fid = proc_feat(f, add_new);
        features.push_back(fid);
        f = genFeature_t_5_2(102, lm_beta_1, ps_beta);
        fid = proc_feat(f, add_new);
        features.push_back(fid);
        f = genFeature_t_5_2(103, ps_beta, ps_beta_1);
        fid = proc_feat(f, add_new);
        features.push_back(fid);
        f = genBigramFeature(104, lm_beta, lm_beta_1, ps_beta, ps_beta_1);
        fid = proc_feat(f, add_new);
        features.push_back(fid);
        
        if(beta_st + 2 < V){
            int wd_beta_2 = nSet[beta_st+2].get_form();
            int lm_beta_2 = nSet[beta_st+2].get_lemma();
            int ps_beta_2 = nSet[beta_st+2].get_POS();
            // unigram feature 17
            f = genFeature_t_5_2(17, wd_beta_2);
            fid = proc_feat(f, add_new);
            features.push_back(fid);
            // unigram feature 18
            f = genFeature_t_5_2(18, lm_beta_2);
            fid = proc_feat(f, add_new);
            features.push_back(fid);
            // unigram feature 19
            f = genFeature_t_5_2(19, ps_beta_2);
            fid = proc_feat(f, add_new);
            features.push_back(fid);
            // unigram feature 20
            f = genFeature_t_5_2(20, lm_beta_2, ps_beta_2);
            fid = proc_feat(f, add_new);
            features.push_back(fid);
            
            // bi-gram features
            f = genBigramFeature(110, lm_beta_1, lm_beta_2);
            fid = proc_feat(f, add_new);
            features.push_back(fid);
            f = genFeature_t_5_2(111, lm_beta_1, ps_beta_2);
            fid = proc_feat(f, add_new);
            features.push_back(fid);
            f = genFeature_t_5_2(112, lm_beta_2, ps_beta_1);
            fid = proc_feat(f, add_new);
            features.push_back(fid);
            f = genFeature_t_5_2(113, ps_beta_1, ps_beta_2);
            fid = proc_feat(f, add_new);
            features.push_back(fid);
            f = genBigramFeature(114, lm_beta_1, lm_beta_2, ps_beta_1, ps_beta_2);
            fid = proc_feat(f, add_new);
            features.push_back(fid);
            
            f = genBigramFeature(120, lm_beta, lm_beta_2);
            fid = proc_feat(f, add_new);
            features.push_back(fid);
            f = genFeature_t_5_2(121, lm_beta, ps_beta_2);
            fid = proc_feat(f, add_new);
            features.push_back(fid);
            f = genFeature_t_5_2(122, lm_beta_2, ps_beta);
            fid = proc_feat(f, add_new);
            features.push_back(fid);
            f = genFeature_t_5_2(123, ps_beta, ps_beta_2);
            fid = proc_feat(f, add_new);
            features.push_back(fid);
            f = genBigramFeature(124, lm_beta, lm_beta_2, ps_beta, ps_beta_2);
            fid = proc_feat(f, add_new);
            features.push_back(fid);
        }
    }
    
    
    
    if(beta_st + 3 < V){
        int wd_beta_3 = nSet[beta_st+3].get_form();
        int lm_beta_3 = nSet[beta_st+3].get_lemma();
        int ps_beta_3 = nSet[beta_st+3].get_POS();
        // unigram feature 21
        f = genFeature_t_5_2(21, wd_beta_3);
        fid = proc_feat(f, add_new);
        features.push_back(fid);
        // unigram feature 22
        f = genFeature_t_5_2(22, lm_beta_3);
        fid = proc_feat(f, add_new);
        features.push_back(fid);
        // unigram feature 23
        f = genFeature_t_5_2(23, ps_beta_3);
        fid = proc_feat(f, add_new);
        features.push_back(fid);
        // unigram feature 24
        f = genFeature_t_5_2(24, lm_beta_3, ps_beta_3);
        fid = proc_feat(f, add_new);
        features.push_back(fid);
    }
    
    if(distance > 1){
        int lambda2_st = lambda1_ed + 1;
        int lambda2_ed = beta_st - 1;
        int lm_lam2_st = nSet[lambda2_st].get_lemma();
        int lm_lam2_ed = nSet[lambda2_ed].get_lemma();
        int ps_lam2_st = nSet[lambda2_st].get_POS();
        int ps_lam2_ed = nSet[lambda2_ed].get_POS();
        // unigram feature 25
        f = genFeature_t_5_2(25, lm_lam2_st);
        fid = proc_feat(f, add_new);
        features.push_back(fid);
        // unigram feature 26
        f = genFeature_t_5_2(26, ps_lam2_st);
        fid = proc_feat(f, add_new);
        features.push_back(fid);
        // unigram feature 27
        f = genFeature_t_5_2(27, lm_lam2_st, ps_lam2_st);
        fid = proc_feat(f, add_new);
        features.push_back(fid);
        // unigram feature 28
        f = genFeature_t_5_2(28, lm_lam2_ed);
        fid = proc_feat(f, add_new);
        features.push_back(fid);
        // unigram feature 29
        f = genFeature_t_5_2(29, ps_lam2_ed);
        fid = proc_feat(f, add_new);
        features.push_back(fid);
        // unigram feature 30
        f = genFeature_t_5_2(30, lm_lam2_ed, ps_lam2_ed);
        fid = proc_feat(f, add_new);
        features.push_back(fid);
        
        // bi-gram features
        f = genBigramFeature(130, lm_lam2_ed, lm_beta);
        fid = proc_feat(f, add_new);
        features.push_back(fid);
        f = genFeature_t_5_2(131, lm_lam2_ed, ps_beta);
        fid = proc_feat(f, add_new);
        features.push_back(fid);
        f = genFeature_t_5_2(132, lm_beta, ps_lam2_ed);
        fid = proc_feat(f, add_new);
        features.push_back(fid);
        f = genFeature_t_5_2(133, ps_lam2_ed, ps_beta);
        fid = proc_feat(f, add_new);
        features.push_back(fid);
        f = genBigramFeature(134, lm_lam2_ed, lm_beta, ps_lam2_ed, ps_beta);
        fid = proc_feat(f, add_new);
        features.push_back(fid);
        if(lambda1_ed > -1){
            int lm_lam1 = nSet[lambda1_ed].get_lemma();
            int ps_lam1 = nSet[lambda1_ed].get_POS();
            f = genBigramFeature(140, lm_lam1, lm_lam2_st);
            fid = proc_feat(f, add_new);
            features.push_back(fid);
            f = genFeature_t_5_2(141, lm_lam1, ps_lam2_st);
            fid = proc_feat(f, add_new);
            features.push_back(fid);
            f = genFeature_t_5_2(142, lm_lam2_st, ps_lam1);
            fid = proc_feat(f, add_new);
            features.push_back(fid);
            f = genFeature_t_5_2(143, ps_lam1, ps_lam2_st);
            fid = proc_feat(f, add_new);
            features.push_back(fid);
            f = genBigramFeature(144, lm_lam1, lm_lam2_st, ps_lam1, ps_lam2_st);
            fid = proc_feat(f, add_new);
            features.push_back(fid);
        }
    }
}

FeatureExtractor fExtractor;

