/*
 * ============================================================================
 *
 *       Filename:  query.cc
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2017-10-27 12:56:50 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Prashant Pandey (), ppandey@cs.stonybrook.edu
 *   Organization:  Stony Brook University
 *
 * ============================================================================
 */


#include <iostream>
#include <fstream>
#include <utility>
#include <algorithm>
#include <string>
#include <vector>
#include <map>
#include <queue>
#include <set>
#include <unordered_set>
#include <set>
#include <bitset>
#include <cassert>
#include <fstream>

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <openssl/rand.h>

#include "kmer.h"
#include "coloreddbg.h"
#include "common_types.h"
#include "CLI/CLI.hpp"
#include "CLI/Timer.hpp"

#define MAX_NUM_SAMPLES 2600
#define OUTPUT_FILE "samples.output"

void output_results(mantis::QuerySets& multi_kmers, 	ColoredDbg<SampleObject<CQF<KeyObject>*>, KeyObject>& cdbg,
                    std::ofstream& opfile) {
  mantis::QueryResults qres;
	uint32_t cnt= 0;
  {
    CLI::AutoTimer timer{"Query time ", CLI::Timer::Big};
    //size_t qctr{0};
    //size_t nquery{multi_kmers.size()};
    for (auto& kmers : multi_kmers) {
      //std::sort(kmers.begin(), kmers.end());
      opfile <<  cnt++ << '\t' << kmers.size() << '\n';
      mantis::QueryResult result = cdbg.find_samples(kmers);
      for (auto it = result.begin(); it != result.end(); ++it) {
        opfile << cdbg.get_sample(it->first) << '\t' << it->second << '\n';
      }
      //++qctr;
    }
  }
}


void output_results_json(mantis::QuerySets& multi_kmers, 	ColoredDbg<SampleObject<CQF<KeyObject>*>, KeyObject>& cdbg,
                    std::ofstream& opfile) {
  mantis::QueryResults qres;
	uint32_t cnt= 0;
  {
    CLI::AutoTimer timer{"Query time ", CLI::Timer::Big};
    opfile << "[\n";
    size_t qctr{0};
    size_t nquery{multi_kmers.size()};
    for (auto& kmers : multi_kmers) {
      //std::sort(kmers.begin(), kmers.end());
      opfile << "{ \"qnum\": " << cnt++ << ",  \"num_kmers\": " << kmers.size() << ", \"res\": {\n";
      mantis::QueryResult result = cdbg.find_samples(kmers);
      for (auto it = result.begin(); it != result.end(); ++it) {
        opfile << " \"" <<cdbg.get_sample(it->first) << "\": " << it->second ;
        if (std::next(it) != result.end()) {
          opfile << ",\n";
        }
      }
      opfile << "}}";
      if (qctr < nquery - 1) { opfile << ","; }
      opfile << "\n";
      ++qctr;
    }
    opfile << "]\n";
  }

}


/* 
 * ===  FUNCTION  =============================================================
 *         Name:  main
 *  Description:  
 * ============================================================================
 */
int main ( int argc, char *argv[] )
{
  CLI::App app("Mantis query");

  std::string prefix;
  std::string query_file;
  std::string output_file{"samples.output"};
  bool use_json{false};
  app.add_option("-i,--input-prefix", prefix, "Prefix of input files.")->required();
  app.add_option("-o,--outout", output_file, "Where to write query output.");
  app.add_option("query", query_file, "Prefix of input files.")->required();
  app.add_flag("-j,--json", use_json, "Write the output in JSON format");
  CLI11_PARSE(app, argc, argv);

  // Make sure the prefix is a full folder
  if (prefix.back() != '/') {
    prefix.push_back('/');
  }

	std::cout << "Reading colored dbg from disk." << std::endl;
	std::string cqf_file(prefix + CQF_FILE);
	std::string eqclass_file(prefix + EQCLASS_FILE);
	std::string sample_file(prefix + SAMPLEID_FILE);
	ColoredDbg<SampleObject<CQF<KeyObject>*>, KeyObject> cdbg(cqf_file,
																														eqclass_file,
																														sample_file);
	std::cout << "Read colored dbg with " << cdbg.get_cqf()->size() << " k-mers and "
						 << cdbg.get_bitvector().bit_size() / cdbg.get_num_samples() <<
						 " equivalence classes." << std::endl;
	
	//cdbg.get_cqf()->dump_metadata(); 
	//std::string query_file(argv[2]);
	//CQF<KeyObject> cqf(query_file);
	//CQF<KeyObject>::Iterator it = cqf.begin(1);
	//std::vector<uint64_t> input_kmers;
	//do {
		//KeyObject k = *it;
		//input_kmers.push_back(k.key);
		//++it;
	//} while (!it.done());

	std::cout << "Reading query kmers from disk." << std::endl;
	uint32_t seed = 2038074743;
  mantis::QuerySets multi_kmers = Kmer::parse_kmers(query_file.c_str(),
																																		 seed,
																																		 cdbg.range());

	std::ofstream opfile(output_file);
	std::cout << "Querying the colored dbg." << std::endl;

if (use_json) {
output_results_json(multi_kmers, cdbg, opfile);
} else {
output_results(multi_kmers, cdbg, opfile);
}
	//std::cout << "Writing samples and abundances out." << std::endl;
	opfile.close();
	std::cout << "Writing done." << std::endl;

	return EXIT_SUCCESS;
}				/* ----------  end of function main  ---------- */
