/*
 * PlotUtil.h
 *
 *  Created on: Mar 7, 2016
 *      Author: max
 */

#ifndef PLOTUTIL_H_
#define PLOTUTIL_H_

#include <string>
#include <vector>
#include <set>
#include <iostream>

#define AVERAGE_INSERT_DIM_PLOT_NAME 		"phtree_average_insert_dimensions"
#define RANGE_QUERY_RATIO_PLOT_NAME 		"phtree_average_range_query_ratio"
#define RANGE_QUERY_SELECTIVITY_PLOT_NAME 	"phtree_range_query_selectivity"
#define AVERAGE_INSERT_ENTRIES_PLOT_NAME 	"phtree_average_insert_entries"
#define INSERT_SERIES_PLOT_NAME 			"phtree_insert_series"
#define AXONS_DENDRITES_PLOT_NAME 			"phtree_axons_dendrites"
#define INSERT_ORDER_NAME		 			"phtree_insert_order"

#define PLOT_DATA_PATH "./plot/data/"
#define PLOT_DATA_EXTENSION ".dat"
#define GNUPLOT_FILE_PATH "./plot/"
#define GNUPLOT_FILE_EXTENSION ".p"

#define BIT_LENGTH 	42
#define ENTRY_DIM 	6
#define ENTRY_DIM_INSERT_SERIES 3
#define FLOAT_ACCURACY_DECIMALS 14

#define INSERT_ENTRY_DIMS {2, 3, 4, 6, 8, 10};
#define INSERT_ENTRY_NUMBERS {1000, 10000, 100000, 1000000};
#define SQUARE_WIDTH_PERCENT {0.5};
#define SELECTIVITY {0.1, 0.01, 0.001};

#define N_REPETITIONS 1
#define N_RANDOM_ENTRIES_AVERAGE_INSERT 500000
#define N_RANDOM_ENTRIES_INSERT_SERIES 1000
#define N_RANDOM_ENTRIES_RANGE_QUERY 1000000

template <unsigned int DIM, unsigned int WIDTH>
class Entry;

class PlotUtil {
public:
	template <unsigned int DIM, unsigned int WIDTH>
	static std::set<std::vector<unsigned long>>* generateUniqueRandomEntries(size_t nUniqueEntries);

	template <unsigned int DIM, unsigned int WIDTH>
	static void plotAverageInsertTimePerDimension(std::string file, bool bulk);
	static void plotAverageInsertTimePerDimensionRandom(bool bulk);

	template <unsigned int DIM, unsigned int WIDTH>
	static void plotAverageInsertTimePerNumberOfEntries(std::vector<std::vector<std::vector<unsigned long>>*> entries);
	template <unsigned int DIM, unsigned int WIDTH>
	static void plotAverageInsertTimePerNumberOfEntries(std::string file, bool isFloat);
	static void plotAverageInsertTimePerNumberOfEntriesRandom();
	static void plotAverageInsertTimePerNumberOfEntriesRandom(std::vector<size_t> nEntries);

	template <unsigned int DIM, unsigned int WIDTH>
	static void plotRangeQueryTimePerPercentFilled(std::vector<vector<unsigned long>>& entries);
	static void plotRangeQueryTimePerPercentFilledRandom();

	template <unsigned int DIM, unsigned int WIDTH>
	static void plotRangeQueryTimePerSelectivity(std::vector<vector<unsigned long>>& entries);
	static void plotRangeQueryTimePerSelectivityRandom();

	static void plotTimeSeriesOfInserts();

	template <unsigned int DIM, unsigned int WIDTH>
	static void plotAxonsAndDendrites(std::vector<std::string> axonsFiles, std::vector<std::string> dendritesFiles);

	template <unsigned int DIM, unsigned int WIDTH>
	static void plotInsertPerformanceDifferentOrder(std::string file, bool isFloat);

private:
	static void plot(std::string gnuplotFileName);
	static void clearPlotFile(std::string dataFileName);
	static std::ofstream* openPlotFile(std::string dataFileName, bool removePreviousData);
	template <unsigned int DIM, unsigned int WIDTH>
	static inline std::vector<std::vector<unsigned long>>* generateUniqueRandomEntriesList(size_t nUniqueEntries);
	template <unsigned int DIM, unsigned int WIDTH>
	static double writeInsertPerformanceOrder(vector<vector<unsigned long>>* entries,
			ofstream* plotFile, size_t runNumber, std::string lable, bool bulk);
	template <unsigned int DIM, unsigned int WIDTH>
	static void writeAverageInsertTimeOfDimension(size_t runNumber, std::vector<std::vector<unsigned long>>* entries, bool bulk);
};

#include <fstream>
#include <algorithm>
#include <set>
#include <stdexcept>
#include <assert.h>
#include <valgrind/callgrind.h>
#include <cstdlib>

#include "Entry.h"
#include "PHTree.h"
#include "visitors/CountNodeTypesVisitor.h"
#include "visitors/AssertionVisitor.h"
#include "visitors/SizeVisitor.h"
#include "visitors/SuffixVisitor.h"
#include "visitors/PrefixSharingVisitor.h"
#include "util/FileInputUtil.h"
#include "util/rdtsc.h"
#include "util/RangeQueryUtil.h"
#include "util/RandUtil.h"
#include "util/DynamicNodeOperationsUtil.h"

using namespace std;

template <unsigned int DIM, unsigned int WIDTH>
set<vector<unsigned long>>* PlotUtil::generateUniqueRandomEntries(size_t nUniqueEntries) {
	assert (WIDTH <= 8 * sizeof (unsigned long));
	set<vector<unsigned long>>* randomDimEntries = new set<vector<unsigned long>>();
	for (size_t nEntry = 0; nEntry < nUniqueEntries; nEntry++) {
		vector<unsigned long> entryValues(DIM);
		for (size_t d = 0; d < DIM; d++) {
			unsigned long r = RandUtil::generateRandValue();
			if (WIDTH < 8 * sizeof(unsigned long))
				entryValues.at(d) = r % (1ul << WIDTH);
			else entryValues.at(d) = r;
		}
		bool inserted = randomDimEntries->insert(entryValues).second;
		if (!inserted) {
			nEntry--;
		}
	}

	return randomDimEntries;
}

template <unsigned int DIM, unsigned int WIDTH>
std::vector<vector<unsigned long>>* PlotUtil::generateUniqueRandomEntriesList(size_t nUniqueEntries) {
	set<vector<unsigned long>>* randomDimEntriesSet = generateUniqueRandomEntries<DIM, WIDTH>(nUniqueEntries);
	vector<vector<unsigned long>>* randomDimEntries = new vector<vector<unsigned long>>(randomDimEntriesSet->begin(),
			randomDimEntriesSet->end());
	return randomDimEntries;
}

void PlotUtil::plot(string gnuplotFileName) {
	string dataPath = PLOT_DATA_PATH + gnuplotFileName + PLOT_DATA_EXTENSION;
	// first sort the file
	string sortCommand = "sort " + dataPath + " -o " + dataPath;
	cout << "sorting plot file..." << flush;
	system(sortCommand.c_str());
	cout << " ok | plotting... " << flush;
	// second plot the file
	string plotPath = GNUPLOT_FILE_PATH + gnuplotFileName + GNUPLOT_FILE_EXTENSION;
	string gnuplotCommand = "gnuplot -p '" + plotPath + "'";
	system(gnuplotCommand.c_str());
	cout << " ok" << endl;
}

template <unsigned int DIM, unsigned int WIDTH>
bool zOrderCompare(vector<unsigned long> i, vector<unsigned long> j) {
	Entry<DIM, WIDTH> e1(i, 0);
	Entry<DIM, WIDTH> e2(j, 1);

	for (unsigned index = 0; index < WIDTH; ++index) {
		unsigned long hcAddress1 = MultiDimBitset<DIM>::interleaveBits(e1.values_, index, DIM * WIDTH);
		unsigned long hcAddress2 = MultiDimBitset<DIM>::interleaveBits(e2.values_, index, DIM * WIDTH);

		if (hcAddress1 != hcAddress2) {
			return hcAddress1 < hcAddress2;
		}
	}

	assert (false);
	return true;
}

template <unsigned int DIM, unsigned int WIDTH>
void PlotUtil::plotInsertPerformanceDifferentOrder(std::string file, bool isFloat) {

	vector<vector<unsigned long>>* original;
	if (isFloat) {
		original= FileInputUtil::
			readFloatEntries<DIM>(file, FLOAT_ACCURACY_DECIMALS);
	} else {
		original = FileInputUtil::readEntries<DIM>(file);
	}

	ofstream* plotFile = openPlotFile(INSERT_ORDER_NAME, true);

	writeInsertPerformanceOrder<DIM, WIDTH>(original, plotFile, 5, "original-bulk", true);
/*	const double normalMs = writeInsertPerformanceOrder<DIM, WIDTH>(original, plotFile, 2, "original", false);
	cout << "shuffling... " << flush;

	random_shuffle(original->begin(), original->end());
	cout << "ok" << endl;
	const double shuffledMs = writeInsertPerformanceOrder<DIM, WIDTH>(original, plotFile, 4, "shuffled", false);
	writeInsertPerformanceOrder<DIM, WIDTH>(original, plotFile, 6, "shuffled-bulk", false);

	cout << "sorting (default)... " << flush;
	sort(original->begin(), original->end());
	cout << "ok" << endl;
	writeInsertPerformanceOrder<DIM, WIDTH>(original, plotFile, 3, "sorted", false);
	writeInsertPerformanceOrder<DIM, WIDTH>(original, plotFile, 7, "sorted-bulk", false);

	cout << "sorting (z-order)... " << flush;
	sort(original->begin(), original->end(), zOrderCompare<DIM,WIDTH>);
	cout << "ok" << endl;
	const double zOrderMs = writeInsertPerformanceOrder<DIM, WIDTH>(original, plotFile, 1, "z-ordered", false);
	writeInsertPerformanceOrder<DIM, WIDTH>(original, plotFile, 8, "z-ordered-bulk", false);

	const double betterThanWorst = 100.0 * ((shuffledMs / normalMs) - 1.0);
	const double worseThanBest = 100.0 * (1.0 - (zOrderMs / normalMs));
	cout << "The given order of the data was:" << endl
			<< "\t" << betterThanWorst << "% better than the worst case (shuffled)" << endl
			<< "\t" << worseThanBest << "% worse than the best case (z-ordered)" << endl;*/

	delete original;
	delete plotFile;
	plot(INSERT_ORDER_NAME);
}

template <unsigned int DIM, unsigned int WIDTH>
double PlotUtil::writeInsertPerformanceOrder(vector<vector<unsigned long>>* entries, ofstream* plotFile, size_t run, string lable, bool bulk) {

	PHTree<DIM, WIDTH>* phtree = new PHTree<DIM,WIDTH>();
	unsigned int smallestInsertTime = (-1u);
	cout << "run with " << N_REPETITIONS << " repetitions: " << flush;
	vector<int>* ids = new vector<int>();
	for (unsigned iEntry = 0; iEntry < entries->size() && bulk; ++iEntry) {
		ids->push_back(iEntry);
	}

	for (unsigned repeat = 0; repeat < N_REPETITIONS; ++repeat) {
		DynamicNodeOperationsUtil<DIM, WIDTH>::resetCounters();
		delete phtree;
		phtree = new PHTree<DIM,WIDTH>();

		unsigned int startInsertTime, insertTime;
		if (bulk) {
			CALLGRIND_START_INSTRUMENTATION;
			startInsertTime = clock();
			phtree->bulkInsert(*entries, *ids);
			insertTime = clock() - startInsertTime;
			CALLGRIND_STOP_INSTRUMENTATION;
		} else {
			startInsertTime = clock();
			for (unsigned iEntry = 0; iEntry < entries->size(); ++iEntry) {
				phtree->insert((*entries)[iEntry], iEntry);
			}
			insertTime = clock() - startInsertTime;
		}

		if (smallestInsertTime > insertTime) {
			cout << "<" << flush;
			smallestInsertTime = insertTime;
		} else {
			cout << "-" << flush;
		}
	}

	delete ids;
	cout << endl;

	// validation only
	for (unsigned iEntry = 0; iEntry < entries->size(); ++iEntry) {
		assert (phtree->lookup((*entries)[iEntry]).second == iEntry);
	}

	cout << "insert calls:" << endl;
	cout << "\t#suffix insertion = " << DynamicNodeOperationsUtil<DIM, WIDTH>::nInsertSuffix
			<< " (with enlarging node: " << DynamicNodeOperationsUtil<DIM, WIDTH>::nInsertSuffixEnlarge << ")" << endl;
	cout << "\t#split prefix = " << DynamicNodeOperationsUtil<DIM, WIDTH>::nInsertSplitPrefix << endl;
	if (bulk) {
		cout << "\t#new suffix buffer = " << DynamicNodeOperationsUtil<DIM, WIDTH>::nInsertSuffixBuffer << endl;
		cout << "\t#suffix insertion into buffer = " << DynamicNodeOperationsUtil<DIM, WIDTH>::nInsertSuffixIntoBuffer << endl;
		cout << "\t#flushes (bulk) = " << DynamicNodeOperationsUtil<DIM, WIDTH>::nFlushCountWithin << endl;
		cout << "\t#flushes (clean up) = " << DynamicNodeOperationsUtil<DIM, WIDTH>::nFlushCountAfter << endl;
	} else {
		cout << "\t#split suffix = " << DynamicNodeOperationsUtil<DIM, WIDTH>::nInsertSplitSuffix << endl;
	}

	CountNodeTypesVisitor<DIM>* typesVisitor = new CountNodeTypesVisitor<DIM>();
	SizeVisitor<DIM>* sizeVisitor = new SizeVisitor<DIM>();
	PrefixSharingVisitor<DIM>* prefixVisitor = new PrefixSharingVisitor<DIM>();
	SuffixVisitor<DIM>* suffixVisitor = new SuffixVisitor<DIM>();
	phtree->accept(typesVisitor);
	phtree->accept(sizeVisitor);
	phtree->accept(prefixVisitor);
	phtree->accept(suffixVisitor);
	cout << *typesVisitor << *prefixVisitor << *sizeVisitor << *suffixVisitor << endl;
	delete typesVisitor;
	delete sizeVisitor;
	delete prefixVisitor;
	delete suffixVisitor;
	delete phtree;

	const double insertMs = double(smallestInsertTime) / double(CLOCKS_PER_SEC);
	(*plotFile) << run << "\t" << lable << "\t" << insertMs << endl;
	cout << "Run nr. " << run << " (" << lable << "): " << insertMs << " ms" << endl;
	return insertMs;
}

template <unsigned int DIM, unsigned int WIDTH>
void PlotUtil::plotAxonsAndDendrites(vector<string> axonsFiles, vector<string> dendritesFiles) {
	assert (axonsFiles.size() == dendritesFiles.size());

	ofstream* plotFile = openPlotFile(AXONS_DENDRITES_PLOT_NAME, true);

	for (unsigned run = 0; run < axonsFiles.size(); ++run) {
		PHTree<DIM, WIDTH>* phtree = new PHTree<DIM, WIDTH>();
		DynamicNodeOperationsUtil<DIM, WIDTH>::resetCounters();

		// insert all dendrites into a PH-Tree
		cout << "loading dendrites... " << flush;
		vector<vector<unsigned long>>* dendritesRectValues = FileInputUtil::readFloatEntries<DIM>(dendritesFiles[run], FLOAT_ACCURACY_DECIMALS);
		cout << "ok" << endl;
		cout << "inserting dendrites into a PH-Tree... " << flush;
		const size_t nDendrites = dendritesRectValues->size();
		const unsigned int startInsertTime = clock();
		for (unsigned iEntry = 0; iEntry < nDendrites; ++iEntry) {
			phtree->insert((*dendritesRectValues)[iEntry], iEntry);
		}
		const unsigned int totalInsertTime = clock() - startInsertTime;

		for (unsigned iEntry = 0; iEntry < nDendrites; ++iEntry) {
			assert (phtree->lookup((*dendritesRectValues)[iEntry]).second == iEntry);
		}
		cout << "ok" << endl;
		dendritesRectValues->clear();
		delete dendritesRectValues;

		cout << "insert calls:" << endl;
		cout << "\t#suffix insertion = " << DynamicNodeOperationsUtil<DIM, WIDTH>::nInsertSuffix
				<< " (with enlarging node: " << DynamicNodeOperationsUtil<DIM, WIDTH>::nInsertSuffixEnlarge << ")" << endl;
		cout << "\t#split suffix = " << DynamicNodeOperationsUtil<DIM, WIDTH>::nInsertSplitSuffix << endl;
		cout << "\t#split prefix = " << DynamicNodeOperationsUtil<DIM, WIDTH>::nInsertSplitPrefix << endl;

		CountNodeTypesVisitor<DIM>* typesVisitor = new CountNodeTypesVisitor<DIM>();
		SizeVisitor<DIM>* sizeVisitor = new SizeVisitor<DIM>();
		PrefixSharingVisitor<DIM>* prefixVisitor = new PrefixSharingVisitor<DIM>();
		SuffixVisitor<DIM>* suffixVisitor = new SuffixVisitor<DIM>();
		phtree->accept(typesVisitor);
		phtree->accept(sizeVisitor);
		phtree->accept(prefixVisitor);
		phtree->accept(suffixVisitor);
		cout << *typesVisitor << *prefixVisitor << *sizeVisitor << *suffixVisitor << endl;
		const size_t sizeMByte = sizeVisitor->getTotalMByteSize();
		delete typesVisitor;
		delete sizeVisitor;
		delete prefixVisitor;
		delete suffixVisitor;

		// check for overlaps between axons and axons
		cout << "loading axons... " << flush;
		vector<vector<unsigned long>>* axonsRectValues = FileInputUtil::readFloatEntries<DIM>(axonsFiles[run], FLOAT_ACCURACY_DECIMALS);
		cout << "ok" << endl;
		cout << "performing range queries on dendrites in the PH-Tree... " << flush;
		size_t nIntersectingDendrites = 0;
		const size_t nAxons = axonsRectValues->size();
		unsigned int totalRangeQueryIntiTime = 0;
		unsigned int totalRangeQueuryNextTime = 0;

//		std::string idPath = "./myDendriteIds.dat";
//		ofstream* idsFile = new ofstream();
//		idsFile->open(idPath.c_str(), ofstream::out | ofstream::trunc);

		CALLGRIND_START_INSTRUMENTATION;
		for (unsigned iAxon = 0; iAxon < nAxons; ++iAxon) {
//			cout << "Axon: " << iAxon << " | previous intersections: " << nIntersectingDendrites << endl;
			const unsigned int startInitTime = clock();
			RangeQueryIterator<DIM, WIDTH>* it = phtree->intersectionQuery((*axonsRectValues)[iAxon]);
			totalRangeQueryIntiTime += (clock() - startInitTime);
			const unsigned int startRangeQueryTime = clock();
//			unsigned i = 0;
			while (it->hasNext()) {
//				cout << "Intersect nr. " << (++i) << endl;
//				(*idsFile) << iAxon << ": " <<  it->next().id_ << endl;
				it->next();
				++nIntersectingDendrites;
			}
			delete it;
			totalRangeQueuryNextTime += (clock() - startRangeQueryTime);
		}
		CALLGRIND_STOP_INSTRUMENTATION;
		cout << "ok" << endl;
//		delete idsFile;

		axonsRectValues->clear();
		delete axonsRectValues;

		const double rangeQueryInitsSec = double(totalRangeQueryIntiTime) / double(CLOCKS_PER_SEC);
		const double rangeQueryNextsSec = double(totalRangeQueuryNextTime) / double(CLOCKS_PER_SEC);
		const double totalRangeQuerySec = rangeQueryInitsSec + rangeQueryNextsSec;
		const double insertSec = double(totalInsertTime) / double(CLOCKS_PER_SEC);

		(*plotFile) << (run + 1) << "\t" << nAxons << "\t" << nDendrites << "\t"
				<< nIntersectingDendrites << "\t" << insertSec << "\t"
				<< rangeQueryInitsSec << "\t" << rangeQueryNextsSec << "\t" << sizeMByte << endl;
		cout << "results run nr. " << (run + 1) << ":" << endl
				<< "#axons=" << nAxons << "\t#dendrites=" << nDendrites
				<< "\t#intersections=" << nIntersectingDendrites << endl
				<< "insert time: " << insertSec << "s\t"
				<< "range query time: " << totalRangeQuerySec << "s (init time: " << rangeQueryInitsSec << "s)\t"
				<< "size: " << sizeMByte << "MB" << endl;

		delete phtree;
	}

	delete plotFile;
	plot(AXONS_DENDRITES_PLOT_NAME);
}

template <unsigned int DIM, unsigned int WIDTH>
void PlotUtil::writeAverageInsertTimeOfDimension(size_t runNumber, vector<vector<unsigned long>>* entries, bool bulk)  {
		cout << "inserting all entries into a PH-Tree while logging the time per insertion..." << endl;

		PHTree<DIM, WIDTH>* phtree = new PHTree<DIM, WIDTH>();

		CountNodeTypesVisitor<DIM>* visitor = new CountNodeTypesVisitor<DIM>();
		SizeVisitor<DIM>* sizeVisitor = new SizeVisitor<DIM>();
		PrefixSharingVisitor<DIM>* prefixVisitor = new PrefixSharingVisitor<DIM>();
		SuffixVisitor<DIM>* suffixVisitor = new SuffixVisitor<DIM>();

		// insertion
		// clock() -> insert all entries of one dim into the appropriate tree -> clock()
		unsigned int startInsertTime;
		if (bulk) {
			vector<int> ids(entries->size());
			for (unsigned i = 0; i < entries->size(); ++i) {ids[i] = i;}
			startInsertTime = clock();
			phtree->bulkInsert(*entries, ids);
		} else {
			startInsertTime = clock();
			for (size_t iEntry = 0; iEntry < entries->size(); ++iEntry) {
				vector<unsigned long> entry = (*entries)[iEntry];
				phtree->insert(entry, iEntry);
			}
		}
		unsigned int insertTicks = clock() - startInsertTime;
		// lookup
		unsigned int startLookupTime = clock();
		for (size_t iEntry = 0; iEntry < entries->size(); ++iEntry) {
			vector<unsigned long> entry = (*entries)[iEntry];
			bool contained = phtree->lookup(entry).first;
			assert (contained);
		}
		unsigned int lookupTicks = clock() - startLookupTime;
		// range query
		const unsigned int startRangeQueryTicks = clock();
		RangeQueryIterator<DIM, WIDTH>* it = RangeQueryUtil<DIM, WIDTH>::getSkewedRangeIterator(*phtree, 0.1, 0.7);
		unsigned int nElementsInRange = 0;
		while (it->hasNext()) {
			it->next();
			++nElementsInRange;
		}
		const unsigned int rangeQueryTicks = clock() - startRangeQueryTicks;

		phtree->accept(visitor);
		phtree->accept(sizeVisitor);
		phtree->accept(prefixVisitor);
		phtree->accept(suffixVisitor);
		cout << "d=" << DIM << endl << *visitor << *prefixVisitor << *sizeVisitor << *suffixVisitor << endl;
		const unsigned int nAHCNodes = visitor->getNumberOfVisitedAHCNodes();
		const unsigned int nLHCNodes = visitor->getNumberOfVisitedLHCNodes();
		const unsigned int totalAhcBitSize = sizeVisitor->getTotalAhcBitSize();
		const unsigned int totalLhcBitSize = sizeVisitor->getTotalLhcBitSize();
		const unsigned int totalLeafBitSize = sizeVisitor->getTotalLeafBitSize();

		// write gathered data into a file
		ofstream* plotFile = openPlotFile(AVERAGE_INSERT_DIM_PLOT_NAME, false);
		float insertMs = (float (insertTicks) / entries->size() / (CLOCKS_PER_SEC / 1000));
		float lookupMs = (float (lookupTicks) / entries->size() / (CLOCKS_PER_SEC / 1000));
		float rangeQueryMs = (float (rangeQueryTicks) / nElementsInRange / (CLOCKS_PER_SEC / 1000));
		float totalSizeBit = (float(totalAhcBitSize + totalLhcBitSize + totalLeafBitSize)) / entries->size();
		(*plotFile) << runNumber
			<< "\t" << DIM
			<< "\t"	<< insertMs
			<< "\t" << lookupMs
			<< "\t" << rangeQueryMs
			<< "\t"	<< nAHCNodes
			<< "\t" << nLHCNodes
			<< "\t" << (float(totalAhcBitSize) / entries->size() / DIM)
			<< "\t" << (float(totalLhcBitSize) / entries->size() / DIM)
			<< "\t" << (float(totalLeafBitSize) / entries->size() / DIM) << "\n";
		cout << "\tdim\tinsert [ms]\t\tlookup [ms]\t\trange query [ms]\tsize [bit per dimension]" << endl;
		cout << runNumber << "\t" << DIM << "\t" << insertMs << "\t\t"
				<< lookupMs << "\t\t" << rangeQueryMs  << "\t\t" << totalSizeBit << endl;

		// clear
		delete phtree;
		delete visitor;
		delete sizeVisitor;
		delete prefixVisitor;
		delete suffixVisitor;
		plotFile->close();
		delete plotFile;
		delete entries;
}

template <unsigned int DIM, unsigned int WIDTH>
void PlotUtil::plotAverageInsertTimePerDimension(std::string file, bool bulk) {
	cout << "loading entries from file..." << flush;
	vector<vector<unsigned long>>* entries = FileInputUtil::readEntries<DIM>(file);
	cout << " ok" << endl;

	writeAverageInsertTimeOfDimension<DIM, WIDTH>(0, entries, bulk);
}

template <unsigned int DIM, unsigned int WIDTH>
void PlotUtil::plotRangeQueryTimePerSelectivity(std::vector<vector<unsigned long>>& entries) {

	// create a PH-Tree with the given entries
	cout << "inserting all entries into a PH-Tree..." << flush;
	PHTree<DIM, WIDTH>* phtree = new PHTree<DIM, WIDTH>();
	for (size_t iEntry = 0; iEntry < entries.size(); iEntry++) {
		phtree->insert(entries[iEntry], iEntry);
	}
	cout << " ok" << endl;

	entries.clear();

	double selectivity[] = SELECTIVITY;
		size_t nTests = sizeof (selectivity) / sizeof (double);

		ofstream* plotFile = openPlotFile(RANGE_QUERY_SELECTIVITY_PLOT_NAME, true);
		cout << "range width\tinit [ms]\tquery time [ms]\t #elements in range" << endl;
		CALLGRIND_START_INSTRUMENTATION;
		for (unsigned test = 0; test < nTests; ++test) {

			// create a centered square with the given side length
			const unsigned int startInitRangeQueryTicks = clock();
			RangeQueryIterator<DIM, WIDTH>* it = RangeQueryUtil<DIM, WIDTH>::getSelectiveRangeIteratorRandom(*phtree, selectivity[test]);
			const unsigned int initRangeQueryTicks = clock() - startInitRangeQueryTicks;
			unsigned int nElementsInRange = 0;
			const unsigned int startRangeQueryTicks = clock();
			while (it->hasNext()) {
				it->next();
				++nElementsInRange;
			}
			const unsigned int rangeQueryTicks = clock() - startRangeQueryTicks;
			delete it;

			const double initMs = double(initRangeQueryTicks) / CLOCKS_PER_SEC * 1000;
			const double rangeQueryMs = double(rangeQueryTicks) / CLOCKS_PER_SEC * 1000;

			cout << selectivity[test] << "\t\t" << initMs << "\t\t"
					<< rangeQueryMs << "\t\t" << nElementsInRange << endl;
			(*plotFile) << test << "\t" << selectivity[test] << "\t"
					<< initMs << "\t" << rangeQueryMs << "\t"
					<< nElementsInRange << endl;
		}
		CALLGRIND_STOP_INSTRUMENTATION;

		plotFile->close();
		delete plotFile;
		delete phtree;

		plot(RANGE_QUERY_SELECTIVITY_PLOT_NAME);
}

void PlotUtil::plotRangeQueryTimePerSelectivityRandom() {
	cout << "creating " << N_RANDOM_ENTRIES_RANGE_QUERY << " entries for the range query..." << flush;
	vector<vector<unsigned long>>* entries = generateUniqueRandomEntriesList<ENTRY_DIM, BIT_LENGTH>(N_RANDOM_ENTRIES_RANGE_QUERY);
	cout << " ok" << endl;
	plotRangeQueryTimePerSelectivity<ENTRY_DIM, BIT_LENGTH>(*entries);
}

void PlotUtil::plotRangeQueryTimePerPercentFilledRandom() {

	cout << "creating " << N_RANDOM_ENTRIES_RANGE_QUERY << " entries for the range query..." << flush;
	vector<vector<unsigned long>>* entries = generateUniqueRandomEntriesList<ENTRY_DIM, BIT_LENGTH>(N_RANDOM_ENTRIES_RANGE_QUERY);
	cout << " ok" << endl;
	plotRangeQueryTimePerPercentFilled<ENTRY_DIM, BIT_LENGTH>(*entries);
}

template <unsigned int DIM, unsigned int WIDTH>
void PlotUtil::plotRangeQueryTimePerPercentFilled(std::vector<vector<unsigned long>>& entries) {

	// create a PH-Tree with the given entries
	cout << "inserting all entries into a PH-Tree..." << flush;
	PHTree<DIM, WIDTH>* phtree = new PHTree<DIM, WIDTH>();
	for (size_t iEntry = 0; iEntry < entries.size(); iEntry++) {
		phtree->insert(entries[iEntry], iEntry);
	}
	cout << " ok" << endl;

	entries.clear();

	double squareWidth[] = SQUARE_WIDTH_PERCENT;
	size_t nTests = sizeof (squareWidth) / sizeof (double);

	ofstream* plotFile = openPlotFile(RANGE_QUERY_RATIO_PLOT_NAME, true);
	cout << "range width\taverage init [ms]\taverage query time [ms]\t #elements in range" << endl;
	CALLGRIND_START_INSTRUMENTATION;
	for (unsigned test = 0; test < nTests; ++test) {

		const double sideLengthPercent = squareWidth[test];
		// create a centered square with the given side length
		const double lowerPercent = (1.0 - sideLengthPercent) / 2.0;
		const double upperPercent = 1.0 - lowerPercent;
		const unsigned int startInitRangeQueryTicks = clock();
		RangeQueryIterator<DIM, WIDTH>* it = RangeQueryUtil<DIM, WIDTH>::getSkewedRangeIterator(*phtree, lowerPercent, upperPercent);
		const unsigned int initRangeQueryTicks = clock() - startInitRangeQueryTicks;
		unsigned int nElementsInRange = 0;
		const unsigned int startRangeQueryTicks = clock();
		while (it->hasNext()) {
			it->next();
			++nElementsInRange;
		}
		const unsigned int rangeQueryTicks = clock() - startRangeQueryTicks;
		delete it;

		const double avgInitMs = double(initRangeQueryTicks) / CLOCKS_PER_SEC * 1000 / nElementsInRange;
		const double avgRangeQueryMs = double(rangeQueryTicks) / CLOCKS_PER_SEC * 1000 / nElementsInRange;

		cout << squareWidth[test] << "\t\t" << avgInitMs << "\t\t"
				<< avgRangeQueryMs << "\t\t" << nElementsInRange << endl;
		(*plotFile) << test << "\t" << squareWidth[test] << "\t"
				<< avgInitMs << "\t" << avgRangeQueryMs << "\t"
				<< nElementsInRange << endl;
	}
	CALLGRIND_STOP_INSTRUMENTATION;

	plotFile->close();
	delete plotFile;
	delete phtree;

	plot(RANGE_QUERY_RATIO_PLOT_NAME);
}

void PlotUtil::plotAverageInsertTimePerDimensionRandom(bool bulk) {
	size_t dimTests[] = INSERT_ENTRY_DIMS
	;
	size_t dimTestsSize = sizeof(dimTests) / sizeof(*dimTests);
	clearPlotFile(AVERAGE_INSERT_DIM_PLOT_NAME);

	for (size_t test = 0; test < dimTestsSize; test++) {
		// resolve dynamic dimensions
		switch (dimTests[test]) {
		case 2: {
			vector<vector<unsigned long>>* randomDimEntries =
								generateUniqueRandomEntriesList<2, BIT_LENGTH>(N_RANDOM_ENTRIES_AVERAGE_INSERT);
						writeAverageInsertTimeOfDimension<2, BIT_LENGTH>(test, randomDimEntries, bulk);
			break;
		}
		case 3: {
			vector<vector<unsigned long>>* randomDimEntries =
								generateUniqueRandomEntriesList<3, BIT_LENGTH>(N_RANDOM_ENTRIES_AVERAGE_INSERT);
						writeAverageInsertTimeOfDimension<3, BIT_LENGTH>(test, randomDimEntries, bulk);
			break;
		}
		case 4: {
			vector<vector<unsigned long>>* randomDimEntries =
								generateUniqueRandomEntriesList<4, BIT_LENGTH>(N_RANDOM_ENTRIES_AVERAGE_INSERT);
						writeAverageInsertTimeOfDimension<4, BIT_LENGTH>(test, randomDimEntries, bulk);
			break;
		}
		case 5: {
			vector<vector<unsigned long>>* randomDimEntries =
								generateUniqueRandomEntriesList<5, BIT_LENGTH>(N_RANDOM_ENTRIES_AVERAGE_INSERT);
						writeAverageInsertTimeOfDimension<5, BIT_LENGTH>(test, randomDimEntries, bulk);
			break;
		}
		case 6: {
			vector<vector<unsigned long>>* randomDimEntries =
								generateUniqueRandomEntriesList<6, BIT_LENGTH>(N_RANDOM_ENTRIES_AVERAGE_INSERT);
						writeAverageInsertTimeOfDimension<6, BIT_LENGTH>(test, randomDimEntries, bulk);
			break;
		}
		case 8: {
			vector<vector<unsigned long>>* randomDimEntries =
								generateUniqueRandomEntriesList<8, BIT_LENGTH>(N_RANDOM_ENTRIES_AVERAGE_INSERT);
						writeAverageInsertTimeOfDimension<8, BIT_LENGTH>(test, randomDimEntries, bulk);
			break;
		}
		case 10: {
			vector<vector<unsigned long>>* randomDimEntries =
								generateUniqueRandomEntriesList<10, BIT_LENGTH>(N_RANDOM_ENTRIES_AVERAGE_INSERT);
						writeAverageInsertTimeOfDimension<10, BIT_LENGTH>(test, randomDimEntries, bulk);
			break;
		}
		default:
			throw std::runtime_error(
					"the given dimensionality is currently not supported by boilerplate code");
		}
	}

	// step 2: call Gnuplot
	cout << "calling gnuplot..." << endl;
	plot(AVERAGE_INSERT_DIM_PLOT_NAME);
}

template <unsigned int DIM, unsigned int WIDTH>
void PlotUtil::plotAverageInsertTimePerNumberOfEntries(vector<vector<vector<unsigned long>>*> entries) {
		vector<unsigned int> insertTicks(entries.size());
		vector<unsigned int> lookupTicks(entries.size());
		vector<unsigned int> rangeQueryTicks(entries.size());
		vector<unsigned int> nElementsInRange(entries.size());
		vector<unsigned int> nAHCNodes(entries.size());
		vector<unsigned int> nLHCNodes(entries.size());
		vector<unsigned int> totalLhcBitSize(entries.size());
		vector<unsigned int> totalAhcBitSize(entries.size());
		vector<unsigned int> totalLeafBitSize(entries.size());
		vector<unsigned int> sizes(entries.size());

		cout << "start insertions..." << flush;
		CountNodeTypesVisitor<DIM>* visitor = new CountNodeTypesVisitor<DIM>();
		SizeVisitor<DIM>* sizeVisitor = new SizeVisitor<DIM>();
		PrefixSharingVisitor<DIM>* prefixVisitor = new PrefixSharingVisitor<DIM>();
		SuffixVisitor<DIM>* suffixVisitor = new SuffixVisitor<DIM>();

		for (size_t test = 0; test < entries.size(); test++) {
			PHTree<DIM, WIDTH>* tree = new PHTree<DIM, WIDTH>();
			const unsigned int startInsertTime = clock();
			for (size_t iEntry = 0; iEntry < entries[test]->size(); iEntry++) {
				vector<unsigned long> entry = (*entries[test])[iEntry];
				tree->insert(entry, iEntry);
			}
			const unsigned int totalInsertTicks = clock() - startInsertTime;
			CALLGRIND_START_INSTRUMENTATION;
			const unsigned int startLookupTime = clock();
			for (size_t iEntry = 0; iEntry < entries[test]->size(); iEntry++) {
				vector<unsigned long> entry = (*entries[test])[iEntry];
				tree->lookup(entry);
			}
			const unsigned int totalLookupTicks = clock() - startLookupTime;
			CALLGRIND_STOP_INSTRUMENTATION;

			const unsigned int startRangeQueryTicks = clock();
			RangeQueryIterator<DIM, WIDTH>* it = RangeQueryUtil<DIM, WIDTH>::getSkewedRangeIterator(*tree, 0.1, 0.7);
			unsigned int elementsInRange = 0;
			while (it->hasNext()) {
				it->next();
				++elementsInRange;
			}
			const unsigned int totalRangeQueryTicks = clock() - startRangeQueryTicks;

			tree->accept(visitor);
			tree->accept(sizeVisitor);
			tree->accept(prefixVisitor);
			tree->accept(suffixVisitor);
			cout << "n=" << entries[test]->size() << endl << *visitor << *prefixVisitor << *sizeVisitor << *suffixVisitor << endl;

			insertTicks.at(test) = totalInsertTicks;
			lookupTicks.at(test) = totalLookupTicks;
			rangeQueryTicks.at(test) = totalRangeQueryTicks;
			nElementsInRange.at(test) = elementsInRange;
			nAHCNodes.at(test) = visitor->getNumberOfVisitedAHCNodes();
			nLHCNodes.at(test) = visitor->getNumberOfVisitedLHCNodes();
			totalLhcBitSize.at(test) = sizeVisitor->getTotalLhcBitSize();
			totalAhcBitSize.at(test) = sizeVisitor->getTotalAhcBitSize();
			totalLeafBitSize.at(test) = sizeVisitor->getTotalLeafBitSize();
			sizes.at(test) = entries[test]->size();

			visitor->reset();
			sizeVisitor->reset();
			prefixVisitor->reset();
			suffixVisitor->reset();
			delete tree;
			delete entries[test];
		}
		delete visitor;
		delete sizeVisitor;
		delete prefixVisitor;
		delete suffixVisitor;

		cout << " ok" << endl;
		// write gathered data into a file
		ofstream* plotFile = openPlotFile(AVERAGE_INSERT_ENTRIES_PLOT_NAME, true);
		for (size_t test = 0; test < entries.size(); test++) {
			(*plotFile) << test << "\t"
					<< sizes[test] << "\t"
					<< (float (insertTicks[test]) / sizes[test] / CLOCKS_PER_SEC * 1000) << "\t"
					<< (float (lookupTicks[test]) / sizes[test] / CLOCKS_PER_SEC * 1000) << "\t"
					<< (float (rangeQueryTicks[test]) / nElementsInRange[test] / CLOCKS_PER_SEC * 1000) << "\t"
					<< nAHCNodes.at(test) << "\t"
					<< nLHCNodes.at(test) << "\t"
					<< (float(totalAhcBitSize.at(test)) / sizes[test] / ENTRY_DIM_INSERT_SERIES) << "\t"
					<< (float(totalLhcBitSize.at(test)) / sizes[test] / ENTRY_DIM_INSERT_SERIES) << "\t"
					<< (float(totalLeafBitSize.at(test)) / sizes[test] / ENTRY_DIM_INSERT_SERIES) << "\n";
		}
		plotFile->close();
		delete plotFile;

		// step 2: call Gnuplot
		cout << "calling gnuplot..." << flush;
		plot(AVERAGE_INSERT_ENTRIES_PLOT_NAME);
		cout << " ok" << endl;
}

template <unsigned int DIM, unsigned int WIDTH>
void PlotUtil::plotAverageInsertTimePerNumberOfEntries(std::string file, bool isFloat) {
	vector<vector<unsigned long>>* entries = NULL;
	if (isFloat) {
		entries = FileInputUtil::readFloatEntries<DIM>(file, FLOAT_ACCURACY_DECIMALS);
	} else {
		entries = FileInputUtil::readEntries<DIM>(file);
	}

	vector<vector<vector<unsigned long>>*> e(1);
	e[0] = entries;
	plotAverageInsertTimePerNumberOfEntries<DIM, WIDTH>(e);
}

void PlotUtil::plotAverageInsertTimePerNumberOfEntriesRandom() {
	size_t numberOfEntries[] = INSERT_ENTRY_NUMBERS;
	size_t numberOfEntriesSize = sizeof(numberOfEntries) / sizeof(*numberOfEntries);
	vector<size_t> nEntries(numberOfEntries, numberOfEntries + numberOfEntriesSize);

	plotAverageInsertTimePerNumberOfEntriesRandom(nEntries);
}

void PlotUtil::plotAverageInsertTimePerNumberOfEntriesRandom(vector<size_t> nEntries) {
	vector<vector<vector<unsigned long>>*> testEntries(nEntries.size());
	for (unsigned test = 0; test < nEntries.size(); ++test) {
		testEntries.at(test) = generateUniqueRandomEntriesList<ENTRY_DIM, BIT_LENGTH>(nEntries.at(test));
	}

	plotAverageInsertTimePerNumberOfEntries<ENTRY_DIM, BIT_LENGTH>(testEntries);
}

void PlotUtil::clearPlotFile(std::string dataFileName) {
	std::string path = PLOT_DATA_PATH + dataFileName + PLOT_DATA_EXTENSION;
	ofstream* plotFile = new ofstream();
	plotFile->open(path.c_str(), ofstream::out | ofstream::trunc);
	plotFile->close();
	delete plotFile;

}

ofstream* PlotUtil::openPlotFile(std::string dataFileName, bool removePreviousData) {
	std::string path = PLOT_DATA_PATH + dataFileName + PLOT_DATA_EXTENSION;
	ofstream* plotFile = new ofstream();
	if (removePreviousData) {
		plotFile->open(path.c_str(), ofstream::out | ofstream::trunc);
	} else {
		plotFile->open(path.c_str(), ofstream::out | ofstream::app);
	}
	return plotFile;
}

void PlotUtil::plotTimeSeriesOfInserts() {
	set<vector<unsigned long>>* entries = generateUniqueRandomEntries<ENTRY_DIM, BIT_LENGTH>(N_RANDOM_ENTRIES_INSERT_SERIES);
	PHTree<ENTRY_DIM, BIT_LENGTH> phtree;
	ofstream* plotFile = openPlotFile(INSERT_SERIES_PLOT_NAME, true);

	CountNodeTypesVisitor<ENTRY_DIM>* visitor = new CountNodeTypesVisitor<ENTRY_DIM>();
	AssertionVisitor<ENTRY_DIM>* assertVisitor = new AssertionVisitor<ENTRY_DIM>();
	SizeVisitor<ENTRY_DIM>* sizeVisitor = new SizeVisitor<ENTRY_DIM>();
	try {
		size_t iEntry = 0;
		for (auto entry : (*entries)) {
//			cout << "inserting: " << (&entry) << endl;
			assert (!phtree.lookup(entry).first && "should not contain the entry before insertion");
			uint64_t startInsert = RDTSC();
			phtree.insert(entry, iEntry);
			uint64_t totalInsertTicks = RDTSC() - startInsert;
//			cout << phtree << endl;
			size_t nEntries = RangeQueryUtil<ENTRY_DIM, BIT_LENGTH>::countEntriesInFullRange(phtree);
			assert (nEntries == iEntry + 1);
			bool entryInFullRange = RangeQueryUtil<ENTRY_DIM, BIT_LENGTH>::fullRangeContainsId(phtree, iEntry);
			assert (entryInFullRange);
			phtree.accept(assertVisitor);
			phtree.accept(visitor);
			phtree.accept(sizeVisitor);
			uint64_t startLookup = RDTSC();
			pair<bool, int> contained = phtree.lookup(entry);
			uint64_t totalLookupTicks = RDTSC() - startLookup;
			assert (contained.first && "should contain the entry after insertion");
			(*plotFile) << iEntry << "\t" << totalInsertTicks;
			(*plotFile) << "\t" << totalLookupTicks;
			(*plotFile) << "\t" << visitor->getNumberOfVisitedAHCNodes();
			(*plotFile) << "\t" << visitor->getNumberOfVisitedLHCNodes();
			(*plotFile) << "\t" << sizeVisitor->getTotalAhcByteSize();
			(*plotFile) << "\t" << sizeVisitor->getTotalLhcByteSize();
			(*plotFile) << "\n";
			assertVisitor->reset();
			visitor->reset();
			sizeVisitor->reset();
			plotFile->flush();
			iEntry++;
		}
	} catch (const exception& e) {
		cout << e.what();
	}

	size_t iEntry = 0;
	for (auto entry : (*entries)) {
		pair<bool, int> contained = phtree.lookup(entry);
		assert (contained.first && contained.second == int(iEntry));
		iEntry++;
	}

	plotFile->close();
	delete plotFile;
	delete visitor;
	delete assertVisitor;
	delete sizeVisitor;
	delete entries;

//	plot(INSERT_SERIES_PLOT_NAME);
}

#endif /* PLOTUTIL_H_ */
