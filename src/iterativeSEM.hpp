//
//  iterativeSEM.hpp
//  SEMCPP
//
//  Created by Cody Morterud and Colten Williams on 11/9/16.
//  Copyright © 2016 Boyle Lab. All rights reserved.
//

#ifndef iterativeSEM_hpp
#define iterativeSEM_hpp

#include <string>
#include <array>
#include <fstream>
#include <vector>
#include <cassert>
#include <map>
#include <unordered_map>
#include <exception>
#include <iostream>
extern "C"{
    #include "./lib/sqlite3/sqlite3.h"
}

#define NAN_VALUE -256.0

/*
 example execution from command line
 "./iterativeSEM.pl -PWM examples/MA0114.1.pwm
    -merge_file examples/wgEncodeOpenChromDnaseHepg2Pk.narrowPeak
    -big_wig examples/wgEncodeHaibTfbsHepg2Hnf4asc8987V0416101RawRep1.bigWig
    -TF_name HNF4A -output examples/HNF4A/"
*/

// overview
// a struct to contain an instance of PWM data and DNase data
// data members made public for ease of access, otherwise wouldn't, given more time


struct Dataset {

    // Data should never be reassigned, triggers error upon
    // reassignment or copy construction
    Dataset(const Dataset &other) = delete;
    Dataset& operator=(const Dataset &other) = delete;


    Dataset(){ }
        // overview
        // a struct to contain and manage the PWM data as given in the example file
        struct PWM{
            static const int NUM_COLUMNS = 4;
            // holds the integer values of the matrix
            std::vector<std::vector<int>> matrix_arr;
        };

        struct TFMdata{
            // a c g t
            static const int LETTER_NUM = 4;
            // first letter is a, then c, then g, then t, at least in example
            std::array<std::vector<int>, LETTER_NUM> letter_array;
        };

        struct accumSummary_type{
            //Alignment summary data
            enum class accumSummary_dest{alignment, scrambled, enumerated, none};

            // lines generated by running accumSummary_scale
            std::vector<std::string> align_accum_lines;

            //baseline scramble kmer summary data
            std::vector<std::string> scramble_accum_lines;

            //baseline enumerated kmer summary data
            std::vector<std::string> enum_accum_lines;
        };

        // contains default settings
        struct SettingsForSNPEffectMatrix{
            bool delSNPList = true, delAlignmentBed = true, delFilteredBed = true;
            bool delSignalFile = false, useCache = true, fastrun = false,
            verbose = false;
            int iteration = -1;
            int threads = 1;
            int length = 0;
            double threshold = -1.0;
            // negative threshold value indicates not defined
        };

        // contains data from findMaximumAverageSignalWrapper
        struct MaximumAverageSignalData{
            double scramble_maximum = 0.0;
            int scramble_counter = 0;
            double scramble_stdev = 0.0;
            double scramble_sterr = 0.0;

            // Maxiumum average signal data will be built iteratively
            // as in all the calculations of average... will not occur
            // at once, but once the data is available to be calculated
            double alignment_maximum = 0.0;
            int alignment_counter = 0;
            double alignment_stdev = 0.0;
            double alignment_sterr = 0.0;
            double enumerate_maximum = 0.0;
            int enumerate_counter = 0;
            double enumerate_stdev = 0.0;
            double enumerate_sterr = 0.0;
        };

        std::map< std::pair<int, char>, double> sig_deets_maximum;
        std::map< std::pair<int, char>, int> sig_deets_counter;
        std::map< std::pair<int, char>, double> sig_deets_stdev;
        std::map< std::pair<int, char>, double> sig_deets_sterr;

        PWM PWM_data;
        TFMdata TFM_data;
        accumSummary_type accumSummary_data;
        SettingsForSNPEffectMatrix settings;
        MaximumAverageSignalData Signal_data;

        // name of original command passed in
        std::string command = "";

        // name of transcription factor
        std::string TF_name = "";

        // name of various files and directories
        std::string PWM_file = "";
        std::string bigwig_file = "";
        std::string DNase_file = "";
        std::string base_dir = "";
        std::string output_dir = "";

        std::map<std::string, double> kmerHash;
        std::map< std::pair<int, char>, std::vector<std::string> > signal_cache;
        std::vector<std::string> signal_cache_scramble;
        std::vector<std::string> signal_cache_enumerate;
        std::vector<std::string> signal_output;
        std::vector<std::string> signal_scramble_output;
        std::vector<std::string> signal_enumerate_output;

        // cache for processing
        std::string cachefile = "";
        sqlite3 *cacheDB;
        std::vector<bool> kmerSeen;

        #ifdef DEBUG
            size_t size_of_kmerHash = 0;
        #endif
};

//Declare functions in header to be used by other functions

//main files
void generateSNPEffectMatrix(Dataset &data);

//src files
std::vector<std::string> accumSummary_scale(const Dataset &data, const std::string &hfile,
                        const std::string &cfile, int scale);
void alignToGenomeWrapper(Dataset &data, std::string genome);
void bowtie_genome_map(int length, const std::string& genome,
                        const std::string& file, const std::string& final_output,
                        const std::string& dnase_file, const int threads, bool verbose);
void changeBase(const Dataset &data, int position, const char nucleotide,
                std::vector<std::string> &new_kmer_vec);
void checkCache(Dataset &data, std::vector<std::string> &in_file,
                std::vector<std::string> &out_cache, sqlite3 *cacheDB,
                Dataset::accumSummary_type::accumSummary_dest dest,
                int position = -1, char bp = 'Q');
void combineBedFiles(Dataset &data);
void Enumerate_kmer(Dataset &data);
void filterDNaseWrapper(const Dataset &data);
void findMaximumAverageSignalWrapper(const std::vector<std::string> &alignments,
                                     double &mean_out, int &count_out,
                                     double &stdev_out, double &sterr_out);
void generatePWMfromSEM(const Dataset &data, std::string input, std::string output);
void generateRmeplot(Dataset &data);
void generateRplot(const Dataset &data);
void generateSelfInfo(Dataset &data);
void generateSEM(const Dataset &data);
void generateSignalMethylTable(Dataset &data);
double get_threshold(Dataset &data, double pval);
void pwm_to_tfm(Dataset &data);
void quality_control(const Dataset &data);
void scramble_kmer(Dataset &data);
int seq_col_to_fa(const std::vector<std::string> &column,
                    const std::string &file);
void writeCache(Dataset &data, sqlite3 *cacheDB,
                Dataset::accumSummary_type::accumSummary_dest dest);
void connectCache(Dataset &data, const std::string &cachefile, sqlite3 *cacheDB);
void closeCache(const std::string &cachefile, sqlite3 *cacheDB);

std::string read_pwm(Dataset &data, std::string file);

#endif /* iterativeSEM_hpp */
