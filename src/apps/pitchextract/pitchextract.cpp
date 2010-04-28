/*
** Copyright (C) 2000-2010 George Tzanetakis <gtzan@cs.uvic.ca>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/


#include "common.h"

#include <cstdio>
#include <cstdlib>

#include "Collection.h"
#include "MarSystemManager.h"

#include "CommandLineOptions.h" 
#include "Conversions.h"

#include <string>

using namespace std;
using namespace Marsyas;

CommandLineOptions cmd_options;
int helpopt;
int usageopt;
int wopt = 2 * MRS_DEFAULT_SLICE_NSAMPLES;
int hopt = 2 * MRS_DEFAULT_SLICE_NSAMPLES;
int lpopt = 36;
int upopt = 128;
int plopt = 0;
float topt = 0.2f;
int yinopt = 0;

void 
printUsage(string progName)
{
	MRSDIAG("pitchextract.cpp - printUsage");
	cerr << "Usage : " << progName << "[-c collection] [-w windowSize] [-s hopSize] [-l lowerPitch] [-u upperPitch] [-t threshold] -p file1 file2 file3" << endl;
	cerr << "where file1, ..., fileN are sound files in a MARSYAS supported format" << endl;
	cerr << endl; 
	cerr << "Options:" << endl;
	cerr << " -y - Use the YIN algorithm to determine pitches" << endl;
	exit(1);
}

void 
printHelp(string progName)
{
	MRSDIAG("pitchextract.cpp - printHelp");
	cerr << "pitchextract, MARSYAS, Copyright George Tzanetakis " << endl;
	cerr << "--------------------------------------------" << endl;
	cerr << "Extracts pitch from the sound files provided as arguments " << endl;
	cerr << endl;
	cerr << "Usage : " << progName << "[-c collection] [-w windowSize] [-p hopSize] [-l lowerPitch] [-u upperPitch] file1 file2 file3" << endl;
	cerr << endl;
	cerr << "where file1, ..., fileN are sound files in a Marsyas supported format" << endl;
	cerr << "Help Options:" << endl;
	cerr << "-u --usage      : display short usage info" << endl;
	cerr << "-h --help       : display this information " << endl;
	cerr << "-v --verbose    : verbose output " << endl;
	cerr << "-c --collection : Marsyas collection of sound files " << endl;
	cerr << "-w --windowSize : windowSize " << endl;
	cerr << "-p --hopSize    : hopSize " << endl;
	cerr << "-l --lowerPitch : lowerPitch " << endl;
	cerr << "-u --upperPitch : upperPitch " << endl;
	exit(1);
}


void 
pitchextract(string sfName, mrs_natural winSize, mrs_natural hopSize, 
			 mrs_real lowPitch, mrs_real highPitch, mrs_real threshold, 
			 mrs_bool playPitches)
{
	MRSDIAG("pitchextract.cpp - pitchextract");	

	MarSystemManager mng;
	// Build pitch contour extraction network 
	MarSystem* pitchContour     = mng.create("Series", "pitchContour");

	MarSystem* pitchExtractor = mng.create("Series", "pitchExtractor");
	pitchExtractor->addMarSystem(mng.create("SoundFileSource", "src"));
	pitchExtractor->addMarSystem(mng.create("Stereo2Mono", "s2m"));
	pitchExtractor->addMarSystem(mng.create("ShiftInput", "si"));
	pitchExtractor->addMarSystem(mng.create("PitchPraat", "pitchPraat"));
	pitchExtractor->updctrl("SoundFileSource/src/mrs_string/filename", sfName);

	mrs_natural fileSize; 
	fileSize= pitchExtractor->getctrl("SoundFileSource/src/mrs_natural/size")->to<mrs_natural>();
	mrs_natural contourSize = fileSize / hopSize;
	
	// Accumulate the extracted pitches and confidences in a single vector 
	// of size contourSize 
	MarSystem* pitchAccumulator = mng.create("Accumulator", "pitchAccumulator");
	pitchAccumulator->addMarSystem(pitchExtractor);
	pitchAccumulator->updctrl("mrs_natural/nTimes", contourSize);
	pitchContour->addMarSystem(pitchAccumulator);
	
	// Extract the pitch contour using Accumulator 
	// pitchContour->tick();
	// mrs_realvec contour = pitchContour->getctrl("mrs_realvec/processedData")->to<mrs_realvec>();
	// de-interleave the pitch contour
	// for (int i=0; i < len; i++) 
	// {
	// confidences(i) = contour(2*i);
	// pitches(i) = contour(2*i+1);
	// }


	// Using explicit loop 
	mrs_natural len = contourSize;
	mrs_realvec pitches(len);
	mrs_realvec confidences(len);
	mrs_realvec pitchres;
	mrs_realvec peak_in;
	for (int i=0; i < contourSize; i++) 
	{
	    pitchExtractor->tick();
	    pitchres = pitchExtractor->getctrl("mrs_realvec/processedData")->to<mrs_realvec>(); 
	    confidences(i) = pitchres(0);
	    pitches(i) = pitchres(1);
		
		cout << confidences(i) << endl;
		cout << pitches(i) << endl;
		
        /*
		  peak_in = pitchExtractor->getctrl("PitchPraat/pitchPraat/AutoCorrelation/acr/mrs_realvec/processedData")->to<mrs_realvec>();
		  mrs_natural pos = pitchExtractor->getctrl("SoundFileSource/src/mrs_natural/pos")->to<mrs_natural>();
		  MATLAB_PUT(peak_in, "peak_in");
		  MATLAB_PUT(pos, "pos");
		  MATLAB_EVAL("plot(peak_in); title(num2str(pos));");
		  getchar();
	    */ 
    }

	// Normalize confidence to 0-1 range
	confidences.normMaxMin();

	// Optionally plot the pitches 	
#ifdef MARSYAS_MATLAB
	mrs_realvec foo(len);
	MATLAB_PUT(confidences, "confidences");
	MATLAB_PUT(pitches, "pitches");
	MATLAB_EVAL("plot(confidences)");
    cerr << "Enter any character to continue" << endl;
	getchar();
	MATLAB_EVAL("a = pitches .* pitches;");
	MATLAB_GET("a", foo);
	MATLAB_EVAL("plot(a)");
	getchar();
	MATLAB_EVAL("plot(pitches)");
    cerr << "Enter any character to continue" << endl;
	getchar();	
	MATLAB_CLOSE();
#endif 

	// Playback the pitches
	if (playPitches) 
	{
		MarSystem* playback = mng.create("Series", "playback");
		playback->addMarSystem(mng.create("SineSource", "ss"));
		playback->addMarSystem(mng.create("Gain", "g"));
		playback->addMarSystem(mng.create("AudioSink", "dest"));
		playback->updctrl("mrs_natural/inSamples", 512);
		playback->updctrl("mrs_real/israte", 44100.0);
		playback->updctrl("AudioSink/dest/mrs_bool/initAudio", true);
		playback->updctrl("mrs_real/israte", pitchContour->getctrl("mrs_real/osrate"));
				
		for (int i=0; i < len; i++) 
		{
			playback->updctrl("SineSource/ss/mrs_real/frequency", pitches(i));
			playback->updctrl("Gain/g/mrs_real/gain", confidences(i));
			playback->tick();
		}
	}
}


void 
old_pitchextract(string sfName, mrs_natural winSize, mrs_natural hopSize, 
				 mrs_natural lowPitch, mrs_natural highPitch, mrs_real threshold, 
				 mrs_real playPitches)
{
	MRSDIAG("pitchextract.cpp - pitchextract");
  
	MarSystemManager mng;
  
	// Build the pitch extractor network 
	MarSystem* pitchExtractor = mng.create("Series", "pitchExtractor");
	// pitchExtractor->addMarSystem(mng.create("AudioSource", "src"));

	pitchExtractor->addMarSystem(mng.create("SoundFileSource", "src"));
	pitchExtractor->updctrl("SoundFileSource/src/mrs_string/filename", sfName);
 

	pitchExtractor->addMarSystem(mng.create("AutoCorrelation", "acr"));
	pitchExtractor->updctrl("AutoCorrelation/acr/mrs_real/magcompress", 0.67);
	pitchExtractor->addMarSystem(mng.create("HalfWaveRectifier", "hwr"));
  
	MarSystem* fanout = mng.create("Fanout", "fanout");
	fanout->addMarSystem(mng.create("Gain", "id1"));
	fanout->addMarSystem(mng.create("TimeStretch", "tsc"));
  
	pitchExtractor->addMarSystem(fanout);
  
	MarSystem* fanin = mng.create("Fanin", "fanin");
	fanin->addMarSystem(mng.create("Gain", "id2"));
	fanin->addMarSystem(mng.create("Negative", "nid"));
  
	pitchExtractor->addMarSystem(fanin);
	pitchExtractor->addMarSystem(mng.create("HalfWaveRectifier", "hwr"));

	pitchExtractor->addMarSystem(mng.create("PlotSink", "psink0"));
	pitchExtractor->addMarSystem(mng.create("Peaker", "pkr"));
	// pitchExtractor->addMarSystem(mng.create("PlotSink", "psink1"));
	pitchExtractor->addMarSystem(mng.create("MaxArgMax", "mxr"));

	// update controls 
	pitchExtractor->updctrl("mrs_natural/inSamples", 1024);
	pitchExtractor->updctrl("Fanout/fanout/TimeStretch/tsc/mrs_real/factor", 0.5);  
	// Convert pitch bounds to samples 
	cout << "lowPitch = " << lowPitch << endl;
	cout << "highPitch = " << highPitch << endl;
  
	mrs_real lowFreq = pitch2hertz(lowPitch);
	mrs_real highFreq = pitch2hertz(highPitch);

	mrs_natural lowSamples = 
		// hertz2samples(highFreq, pitchExtractor->getctrl("AudioSource/src/mrs_real/osrate")->to<mrs_real>());
		hertz2samples(highFreq, pitchExtractor->getctrl("SoundFileSource/src/mrs_real/osrate")->to<mrs_real>());
	mrs_natural highSamples = 
		//     hertz2samples(lowFreq, pitchExtractor->getctrl("AudioSource/src/mrs_real/osrate")->to<mrs_real>());
		hertz2samples(lowFreq, pitchExtractor->getctrl("SoundFileSource/src/mrs_real/osrate")->to<mrs_real>());


	cout << "lowSamples" << lowSamples << endl;
	cout << "highSamples" << highSamples << endl;


	pitchExtractor->updctrl("Peaker/pkr/mrs_real/peakSpacing", 0.00);
	pitchExtractor->updctrl("Peaker/pkr/mrs_real/peakStrength", 0.4);
	pitchExtractor->updctrl("Peaker/pkr/mrs_natural/peakStart", lowSamples);
	pitchExtractor->updctrl("Peaker/pkr/mrs_natural/peakEnd", highSamples);
	pitchExtractor->updctrl("MaxArgMax/mxr/mrs_natural/nMaximums", 1);
   
	cout << (*pitchExtractor) << endl;
   
	realvec pitchres(pitchExtractor->getctrl("mrs_natural/onObservations")->to<mrs_natural>(), pitchExtractor->getctrl("mrs_natural/onSamples")->to<mrs_natural>());
   
  
	realvec win(pitchExtractor->getctrl("mrs_natural/inObservations")->to<mrs_natural>(), 
				pitchExtractor->getctrl("mrs_natural/inSamples")->to<mrs_natural>());
  
  	
	/// playback network 
	MarSystem* playback = mng.create("Series", "playback");
	playback->addMarSystem(mng.create("SineSource", "ss"));
	playback->addMarSystem(mng.create("AudioSink", "dest"));
	playback->updctrl("mrs_natural/inSamples", 512);
	//playback->updctrl("mrs_bool/initAudio", true);

  
    mrs_real pitch;
    while (pitchExtractor->getctrl("SoundFileSource/src/mrs_bool/hasData")->to<mrs_bool>())
	{
		if (plopt) 
			playback->tick();
    
		pitchExtractor->process(win, pitchres);
      
		// pitch = samples2hertz((mrs_natural)pitchres(1), pitchExtractor->getctrl("AudioSource/src/mrs_real/osrate")->to<mrs_real>());
		pitch = samples2hertz((mrs_natural)pitchres(1), pitchExtractor->getctrl("SoundFileSource/src/mrs_real/osrate")->to<mrs_real>());
      
		cout << "conf" << "---" << pitchres(0) << endl;
		cout << "midi" << "---" << hertz2pitch(pitch) << endl ;
		cout << "hz " << "---" << pitch << endl;

		if (pitchres(0) > 0.05) 
			playback->updctrl("SineSource/ss/mrs_real/frequency", pitch);
    }
}

//
// Use the YIN algorithm (de Chevigne) for doing pitch extraction
//
void yinpitchextract(string inAudioFileName, int buffer_size, int overlap_size, mrs_bool playPitches)
{
	// Fill up the realvec with a sine wave
	MarSystemManager mng;

	// A series to contain everything
	MarSystem* net = mng.create("Series", "series");
	net->addMarSystem(mng.create("SoundFileSource", "src"));

	MarSystem* fanout = mng.create("Fanout", "fanout");

	// Process the input data with Yin
	MarSystem* yin_series = mng.create("Series", "yin_series");
	yin_series->addMarSystem(mng.create("ShiftInput", "si"));
	yin_series->addMarSystem(mng.create("Yin", "yin"));
	yin_series->addMarSystem(mng.create("Gain", "gain"));
	fanout->addMarSystem(yin_series);

	MarSystem* rms_series = mng.create("Series", "rms_series");
	rms_series->addMarSystem(mng.create("Rms", "rms"));
	rms_series->addMarSystem(mng.create("Gain", "gain"));
	fanout->addMarSystem(rms_series);

	net->addMarSystem(fanout);

	net->updctrl("mrs_natural/inSamples",overlap_size);

	net->updctrl("SoundFileSource/src/mrs_string/filename",inAudioFileName);
	yin_series->updctrl("ShiftInput/si/mrs_natural/winSize", buffer_size*4);
	yin_series->updctrl("Yin/yin/mrs_natural/inSamples",buffer_size*4);
	yin_series->updctrl("Yin/yin/mrs_real/tolerance",0.7);

	realvec r;
	realvec r1;
	double pitch;
	double rms;
	double time;
	int count = 0;
	mrs_real srate = net->getctrl("mrs_real/osrate")->to<mrs_real>();
	mrs_natural inSamples = net->getctrl("mrs_natural/inSamples")->to<mrs_natural>();

	// Using explicit loop 
	mrs_natural fileSize = net->getctrl("SoundFileSource/src/mrs_natural/size")->to<mrs_natural>();
	mrs_natural len = fileSize / overlap_size;
	mrs_realvec pitches(len);
	mrs_realvec confidences(len);

	int i = 0;
	while (net->getctrl("SoundFileSource/src/mrs_bool/hasData")->to<mrs_bool>()) {
		net->tick();
		r = net->getctrl("Fanout/fanout/Series/yin_series/Yin/yin/mrs_realvec/processedData")->to<mrs_realvec>();
		r1 = net->getctrl("Fanout/fanout/Series/rms_series/Rms/rms/mrs_realvec/processedData")->to<mrs_realvec>();

		time = count / srate;
		pitch = r(0,0);
		rms = r1(0,0);
		//printf("%12.12f\t%12.12f\t%12.12f\n",time,pitch,rms);
		printf("%12.12f\n",pitch);

		pitches.stretchWrite(i,pitch);
		// sness - Just give it all a confidence of 1 for now.  You can
		// get the confidence out of the YIN algorithm, but I haven't
		// implemented it yet.
		confidences.stretchWrite(i,rms);

		count += inSamples;
		i++;
	}

	len = i;

	// Playback the pitches
	if (playPitches) 
	{
		cout << "Playing pitches" << endl;

		MarSystem* playback = mng.create("Series", "playback");
		playback->addMarSystem(mng.create("SineSource", "ss"));
		playback->addMarSystem(mng.create("Gain", "g"));
		playback->addMarSystem(mng.create("AudioSink", "dest"));
		playback->updctrl("mrs_natural/inSamples", buffer_size);
		playback->updctrl("mrs_real/israte", 44100.0);
		playback->updctrl("AudioSink/dest/mrs_bool/initAudio", true);
		playback->updctrl("mrs_real/israte", net->getctrl("mrs_real/osrate"));
				
		for (int i=0; i < len; i++) 
		{
			playback->updctrl("SineSource/ss/mrs_real/frequency", pitches(i));
			playback->updctrl("Gain/g/mrs_real/gain", confidences(i));
			playback->tick();
		}
	}

}


void 
initOptions()
{
	cmd_options.addBoolOption("help", "h", false);
	cmd_options.addBoolOption("usage", "u", false);
	cmd_options.addBoolOption("verbose", "v", false);
	cmd_options.addNaturalOption("winSize", "w", 2 * MRS_DEFAULT_SLICE_NSAMPLES);
	cmd_options.addNaturalOption("hopSize", "h", 2 * MRS_DEFAULT_SLICE_NSAMPLES);
	cmd_options.addNaturalOption("lowerPitch", "l", 36);
	cmd_options.addNaturalOption("upperPitch", "u", 79);
	cmd_options.addBoolOption("playback", "p", false);
	cmd_options.addRealOption("threshold", "t", 0.2);
	cmd_options.addBoolOption("yin", "y", false);

}


void 
loadOptions()
{
	helpopt = cmd_options.getBoolOption("help");
	usageopt = cmd_options.getBoolOption("usage");
	wopt = cmd_options.getNaturalOption("winSize");
	hopt = cmd_options.getNaturalOption("hopSize");
	lpopt = cmd_options.getNaturalOption("lowerPitch");
	upopt = cmd_options.getNaturalOption("upperPitch");
	plopt = cmd_options.getBoolOption("playback");
	topt  = (float)cmd_options.getRealOption("threshold");
	yinopt = cmd_options.getBoolOption("yin");
}


int
main(int argc, const char **argv)
{
	MRSDIAG("pitchextract.cpp - main");

	string progName = argv[0];  
	progName = progName.erase(0,3);

	initOptions();
	cmd_options.readOptions(argc, argv);
	loadOptions();

	vector<string> soundfiles = cmd_options.getRemaining();
  
	if (helpopt) 
		printHelp(progName);
  
	if (usageopt)
		printUsage(progName);

	// cout << "PitchExtract windowSize = " << wopt << endl;
	// cout << "PitchExtract hopSize = " << hopt << endl;
	// cout << "PitchExtract lowerPitch = " << lpopt << endl;
	// cout << "PitchExtract upperPitch = " << upopt << endl;
	// cout << "PitchExtract threshold  = " << topt << endl;
	// cout << "PitchExtract playback   = " << plopt << endl;
  
    vector<string>::iterator sfi;
	for (sfi = soundfiles.begin(); sfi != soundfiles.end(); ++sfi) 
    {
		string sfname = *sfi;
		if (yinopt == 0) {
			pitchextract(sfname, wopt, hopt, lpopt, upopt, topt, plopt != 0);
		} else {
			yinpitchextract(sfname, wopt, hopt, plopt != 0);
		}

    }

    exit(0);
}

	
 
