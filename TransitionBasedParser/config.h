//
//  config.h
//  TransitionBasedSemanticDependencyParser
//
//  Created by FoundW on 2017/5/31.
//  Copyright © 2017 FoundW. All rights reserved.
//

#ifndef config_h
#define config_h

#include <stdio.h>

//#include <map>

using namespace std;

typedef long long SCORE;
typedef long long FEATURE;

/****************************************** SETTINGS ***************************************/
/****** Set file paths and global constants before compiling and running the project. ******/

/* MODE */
#define MODE 0      // 1: Train a new model; 0: Predict with pre-trained model
//#define UNLABEL     // annotate this line if train a model assign dependency labels
//#define NOTOP       // annotate this line if train a model could predict the root of a graph

/* FILE PATHS */
#define TRAIN_DATA "/Users/wangfang/Documents/data/train/dm.sdp"
#define TEST_DATA "/Users/wangfang/Documents/data/test/dm.test.sdp"
#define RESULT_FILE "/Users/wangfang/Documents/data/checkpoint/result.ep.20"
#define CHECKP_PATH "/Users/wangfang/Documents/data/checkpoint/"

#define FEATURE_DICT_PATH "/Users/wangfang/Documents/data/dict/feature.dict"
#define TRANSITION_DICT_PATH "/Users/wangfang/Documents/data/dict/transition.dict"
#define WORD_DICT_PATH "/Users/wangfang/Documents/data/dict/word.dict"
#define LEMMA_DICT_PATH "/Users/wangfang/Documents/data/dict/lemma.dict"
#define POS_DICT_PATH "/Users/wangfang/Documents/data/dict/pos.dict"
#define LABEL_DICT_PATH "/Users/wangfang/Documents/data/dict/label.dict"
#define SAVED_ALPHA "/Users/wangfang/Documents/data/checkpoint/alpha.epoch.20"
//#define LOG_PATH "/path/to/log/file"

/* PARAMETER SETTING */
#define BEAM 32     // set the width of beam when applying beam search
#define EPOCH 20    // set how many epochs for training

/************************************* DO NOT MODIFY ***************************************/
/* FILE PATHS */
#define ALPHA_SAVE(epoch) CHECKP_PATH + ("alpha.epoch." + to_string(epoch))
#define AVGALPHA_SAVE(epoch) CHECKP_PATH + ("alpha.avg.epoch." + to_string(epoch))

/* WASTED FILE PATHS */
#define TEST_DUMMY "/Users/xxx/yyy/test/dm.sdp.sample"
#define DECODE_TEST_PATH "/Users/xxx/yyy/data/result/training.decode"

/* OPTIMIZATION OPTION */
#define AVERAGE
#define FAST_GEN

/* NUMBERS TO ENCODE TRASITION */
#define SHIFT 0
#define NOARC 1

/* SIZE OF VOCABULARY & POSTTAG */
#define VACAB_SIZE_MAX 99999
#define PTAG_SIZE_MAX 99
/* OTHER COSTANCES */
#define VOID 0
#define NORES 0
#define NONODE 99999
#define MAXFEAT 10000000

/* DATA RECORD OPTIONS */
#define WRITE_ALPHA_TO_FILE
//#define REVIVE_FROM_CHECKPOINT

/* PEOCESS WATCH OPTIONS */
//#define PRINT_DETAIL
//#define WATCH_FIT
//#define CURRENT_AVG_VACANT

/* GLOBAL OBJECT */
#ifdef PRINT_DETAIL
extern fstream detail;
#endif

//extern int last_update;

#endif /* config_h */
