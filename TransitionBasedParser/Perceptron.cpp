//
//  Perceptron.cpp
//  TransitionBasedSemanticDependencyParser
//
//  Created by FoundW on 2017/6/4.
//  Copyright © 2017 FoundW. All rights reserved.
//

#include "Perceptron.hpp"

Perceptron::Perceptron(){
    y_dimension = MAXFEAT;
    x_dimension = fExtractor.trans_size();
    alpha = vector<vector<int> >(x_dimension);
#ifdef AVERAGE
    avg_alpha = vector<vector<int> >(x_dimension);
#endif
    for(int i = 0; i < x_dimension; i++){
        alpha[i] = vector<int>(y_dimension, 0);
#ifdef AVERAGE
        avg_alpha[i] = vector<int>(y_dimension, 0);
#endif
    }
}

NGraph Perceptron::GEN(vector<NNode>& sent, int beam){
    int l = (int)sent.size();
    vector<NGraph> candidates;
    candidates.push_back(NGraph(l));
    while(true){
        vector<pair<int, pair<int, int>>> next;     //{score, {last_turn_id, act_id}}
        vector<NGraph> new_candidates;
        int ccnt = 0;
        int candi_id = 0;
        int stop = 0;
        for(NGraph tmp : candidates){
#ifdef PRINT_DETAIL
            PrintConfiguration(sent, tmp);
#endif
            if(tmp.isCompleted()){
                
#ifdef PRINT_DETAIL
                PrintConfiguration(sent, tmp);
#endif
                
                new_candidates.push_back(tmp);
                stop++;
                if(stop == beam){
                    return candidates[0];
                }
                candi_id++;
                continue;
            }
            
            vector<int> configurations = vector<int>(0);
            fExtractor.extractFeatures(sent, tmp, configurations);
            
            for(int act = 0; act < x_dimension; act++){
                int score = CalcScore(tmp, act, configurations);
                if(ccnt < beam){
                    next.push_back(make_pair(score, make_pair(candi_id, act)));
                }
                else{
                    if(ccnt == beam){
                        make_heap(next.begin(), next.end(), CmpScore);
                    }
                    // checkpoint
                    if(score > next[0].first){
                        next[0] = make_pair(score, make_pair(candi_id, act));
                        make_heap(next.begin(), next.end(), CmpScore);
                    }
                }
                ccnt++;
            }
            candi_id++;
        }
        for(pair<int, pair<int, int>> i : next){
            if(i.first == INT_MIN){
                continue;
            }
            NGraph g = NGraph(candidates[i.second.first], i.second.second, i.first);
            new_candidates.push_back(g);
        }
        sort(new_candidates.begin(), new_candidates.end());
        if(new_candidates.size() > beam){
            new_candidates.resize(beam);
        }
        candidates = new_candidates;
        
#ifdef FAST_GEN
        if(candidates[0].isCompleted()){
            break;
        }
#endif
        
    }
#ifdef PRINT_DETAIL
    PrintConfiguration(sent, candidates[0]);
#endif
    return candidates[0];
}

int Perceptron::CalcScore(NGraph& current, int act, vector<int>& configurations){
    int delta = current.get_score();
    //int beta_st = current.get_beta();
    int lambda1_ed = current.get_lambda1();
    if(lambda1_ed == -1 && act != SHIFT){
        return INT_MIN;
    }
    for(int config : configurations){
        delta += alpha[act][config];
    }
    return delta;
}

void Perceptron::Train(vector<vector<NNode>>& sentlib, vector<NGraph>& glib){
    int epoch = 0;
    int length = (int)glib.size();
    vector<pair<vector<NNode>, NGraph>> corpus = vector<pair<vector<NNode>, NGraph>>(length);
    for(int i = 0; i < length; i++){
        corpus[i] = make_pair(sentlib[i], glib[i]);
    }
    cout << "Initial Feature Size = " << fExtractor.feats_size() << endl;
    
#ifdef AVERAGE
    vector<vector<pair<int, int>>> history_of_punish;
    vector<vector<pair<int, int>>> history_of_reward;
#ifdef REVIVE_FROM_CHECKPOINT
    for(int i= 0; i < x_dimension; i++){
        for(int j = 0; j < y_dimension; j++){
            avg_alpha[i][j] += alpha[i][j] * length;
        }
    }
#endif
#endif
    
    while(epoch < EPOCH){
        unsigned long epoch_start = clock();
        
        unsigned seed = (unsigned)chrono::system_clock::now().time_since_epoch().count();
        shuffle(corpus.begin(), corpus.end(), default_random_engine(seed));
        
        int total_gold_edge = 0;
        int total_predict_edge = 0;
        int recalled_gold_edge = 0;
        int correct_predict_edge = 0;
        int correct = 0;
        int cnt = 0;
        
        for(pair<vector<NNode>, NGraph> instanse : corpus){
            cnt++;
            if(cnt % 200 == 0){
                cout << "Processing sentence number: " << cnt << ", UP: " << 100.0 * correct_predict_edge / total_predict_edge
                << ", UR: " << 100.0 * recalled_gold_edge / total_gold_edge << "." << endl;
                total_gold_edge = 0;
                total_predict_edge = 0;
                recalled_gold_edge = 0;
                correct_predict_edge = 0;
//                cout << "--Processing: " << cnt << endl;
                //cout << "--Current Feature Size = " << fDict.size << endl;
            }
            vector<NNode> nSet = instanse.first;
#ifdef PRINT_DETAIL
            cout << "Processing: " << cnt << endl;
            PrintSent(nSet);
#endif
            NGraph golden = instanse.second;
            NGraph predict = GEN(nSet, BEAM);
            
            vector<int> pre_trans_seq = predict.get_transitions();
            vector<vector<int>> output_graph = predict.get_edges();
            vector<vector<int>> gold_graph = golden.get_edges();
            bool no_mistake = true;
            for(int i = 0; i < gold_graph.size(); i++){
                for(int j = 0; j < gold_graph.size(); j++){
                    bool gold_exist = (gold_graph[i][j] != VOID);
                    bool output_exist = (output_graph[i][j] != VOID);
                    if(gold_exist and output_exist){
                        total_gold_edge += 1;
                        total_predict_edge += 1;
                        correct_predict_edge += 1;
                        recalled_gold_edge += 1;
                    }
                    else if(gold_exist && !output_exist){
                        total_gold_edge += 1;
                        no_mistake = false;
                    }
                    else if(!gold_exist && output_exist){
                        total_predict_edge += 1;
                        no_mistake = false;
                    }
                }
            }
//            if(pre_trans_seq == golden.get_transitions()){
            if(no_mistake){
                correct++;
                //cout << "~~ Hit!" << endl;
                //cout << "~~ Length of trans seq = " << pre_trans_seq.size() << endl;
#ifdef AVERAGE
                history_of_punish.push_back(vector<pair<int, int>>(0));
                history_of_reward.push_back(vector<pair<int, int>>(0));
#endif
                continue;
            }
            
            // Punish the wrong prediction
            // generate all the features concerning the mistake
            vector<pair<int, int>> to_punish;
            PunishAndMemory(nSet, predict, to_punish);
            
            // Reward the golden graph
            vector<int> transseq = golden.get_transitions();
            vector<vector<int>> configs = golden.get_features();
            vector<pair<int, int>> to_reward;
            for(int i = 0; i < transseq.size(); i++){
                int act = transseq[i];
                for(int fid : configs[i]){
                    alpha[act][fid] += 1;
                    to_reward.push_back(make_pair(act, fid));
                }
            }
            
#ifdef AVERAGE
            history_of_punish.push_back(to_punish);
            history_of_reward.push_back(to_reward);
#endif
        
        }
        epoch += 1;
        
        unsigned long epoch_finish = clock();
        cout << "Epoch " << epoch << ":\t Exactly Match / Total = " << correct << " / " << length << " = " << correct * 1.0 / length << endl;
        cout << "-- TIME = " << (double)(epoch_finish - epoch_start) / CLOCKS_PER_SEC << " sec. with Current Feature Size = " << fExtractor.feats_size() << endl;
        fExtractor.writeFDict(FEATURE_DICT_PATH);
        // write alpha to files
        WriteAlphaToFile(ALPHA_SAVE(epoch));
        
#ifdef AVERAGE
        for(int i = 0; i < length; i++){
            int accumulated_value = length - i;
            for(pair<int, int> xy: history_of_punish[i]){
                avg_alpha[xy.first][xy.second] -= accumulated_value;
            }
            for(pair<int, int> xy: history_of_reward[i]){
                avg_alpha[xy.first][xy.second] += accumulated_value;
            }
        }
        WriteAlphaToFile(AVGALPHA_SAVE(epoch), avg_alpha);
        history_of_punish.clear();
        history_of_reward.clear();
        
        if(epoch < EPOCH){
            for(int i = 0 ; i < x_dimension; i++){
                for(int j = 0; j < y_dimension; j++){
                    avg_alpha[i][j] += length * alpha[i][j];
                }
            }
        }
#endif
        
    }
}


void Perceptron::WriteAlphaToFile(string file_path){
    ofstream out(file_path, ios::out);
    for(int i = 0; i < x_dimension; i++){
        for(int j = 0; j < fExtractor.feats_size(); j++){
            out << i << "\t" << j << "\t" << alpha[i][j] << endl;
        }
    }
    out.close();
}

void Perceptron::WriteAlphaToFile(string file_path, vector<vector<int>>& a){
    ofstream out(file_path, ios::out);
    for(int i = 0; i < x_dimension; i++){
        for(int j = 0; j < fExtractor.feats_size(); j++){
            out << i << "\t" << j << "\t" << a[i][j] << endl;
        }
    }
    out.close();
}

void Perceptron::ReadAlphaFromFile(string file_path){
    ifstream in(file_path, ios::in);
    int x, y, val;
    while(in >> x >> y >> val){
        alpha[x][y] = val;
    }
    in.close();
}

#ifdef AVERAGE
void Perceptron::ReadAvgAlphaFromFile(string file_path){
    ifstream in(file_path, ios::in);
    int x, y, val;
    while(in >> x >> y >> val){
        avg_alpha[x][y] = val;
    }
    in.close();
}
#endif

void Perceptron::Predict(vector<pair<vector<NNode>, string>>& sents, string res_file){
#ifdef AVERAGE
    if(MODE == 1){
        for(int i = 0; i < x_dimension; i++){
            for(int j = 0; j < y_dimension; j++){
                alpha[i][j] = avg_alpha[i][j];
            }
        }
    }
#endif
    ofstream writer(res_file, ios::out);
    
    for(pair<vector<NNode>, string> sent_num : sents){
        vector<NNode> sent = sent_num.first;
        NGraph predict = GEN(sent, BEAM);
        // Encode the graph to standard file format
        writer << sent_num.second << endl;
        Graph2File(sent, predict, writer);
        writer << endl;
    }
    writer.close();
}

void Perceptron::Graph2File(vector<NNode>& sent, NGraph& predict, ofstream& writer){
    //ofstream writer("/Users/wangfang/Documents/Courses/Empirical Methods in Natural Language Processing/Projects/assignment2.1 SDP/result/test", ios::out);
    int V = (int)sent.size();
    vector<bool> isHead = vector<bool>(V, false);
    vector<unordered_map<int, string>> head_and_label = vector<unordered_map<int, string>>(V);
    int root = 0;
    vector<int> trans_seq = predict.get_transitions();
    int step = 0;
    
    for(int beta = 1; beta < V; beta++){
        for(int lam1 = beta - 1; lam1 >= -1; lam1--){
            if(trans_seq[step] == SHIFT){
                step++;
                break;
            }
            if(trans_seq[step] == NOARC){
                step++;
                continue;
            }
            int transition = fExtractor.decode_trans(trans_seq[step]);
            step++;
            
            if(transition >= 200){ // Left-Arc
                int label = transition - 200;
                isHead[beta] = true;
                head_and_label[lam1].insert(make_pair(beta, fExtractor.get_label(label)));
            }
            else if(transition >= 100){ //Right-Arc
                string label = fExtractor.get_label(transition-100);
                if(lam1 == 0 && label == "TOP"){
                    root = beta;
                }
                else{
                    isHead[lam1] = true;
                    head_and_label[beta].insert(make_pair(lam1, label));
                }
            }
        }
    }
    unordered_map<int, int> id2id;
    int cnt = 0;
    for(int i = 1; i < V; i++){
        if(isHead[i]){
            id2id[i] = cnt++;
        }
    }
        
    vector<vector<string>> output = vector<vector<string>>(V-1);
    for(int p = 1; p < V; p++){
        output[p-1] = vector<string>(cnt, "_");
        for(pair<int, string> t : head_and_label[p]){
            output[p-1][id2id[t.first]] = t.second;
        }
        writer << to_string(p) << "\t" << fExtractor.get_word(sent[p].get_form()) << "\t" << fExtractor.get_lemma(sent[p].get_lemma()) << "\t" << fExtractor.get_postag(sent[p].get_POS());
        if(root == p){
            writer << "\t" << "+";
        }
        else{
            writer << "\t" << "-";
        }
        if(isHead[p]){
            writer << "\t" << "+";
        }
        else{
            writer << "\t" << "-";
        }
        for(string sig : output[p-1]){
            writer << "\t" << sig;
        }
        writer << endl;
    }
}

void Perceptron::Graph2FileTest(vector<NGraph> glib, vector<vector<NNode>> train_sent_lib){
    ofstream writer(DECODE_TEST_PATH, ios::out);
    int length = (int)glib.size();
    for(int i = 0; i < length; i++){
        vector<NNode> sent = train_sent_lib[i];
        NGraph golden = glib[i];
        // Encode the graph to standard file format
        writer << "#no" << endl;
        Graph2File(sent, golden, writer);
        writer << endl;
    }
    writer.close();
}

void Perceptron::PunishAndMemory(vector<NNode>& nSet, NGraph& predict, vector<pair<int, int>>& feats){
    int V = (int)nSet.size();
    int count = 0;
    vector<int> trans_seq = predict.get_transitions();
    vector<vector<int>> pred_edges = predict.get_edges();
    for(int beta_st = 1; beta_st < V; beta_st++){
        int lambda1_ed = beta_st - 1;
        while(true){
            int trans = trans_seq[count];
            count++;
            vector<int> configurations = vector<int>(0);
            fExtractor.extractFeatures(nSet, beta_st, lambda1_ed, pred_edges, configurations);
            for(int fid : configurations){
                alpha[trans][fid] -= 1;
                feats.push_back(make_pair(trans, fid));
            }
            if(trans == SHIFT){
                break;
            }
            lambda1_ed--;
        }
    }
}
