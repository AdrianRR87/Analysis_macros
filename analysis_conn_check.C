#include <RtypesCore.h>
#include <TBrowser.h>
#include <TCanvas.h>
#include <TF1.h>
#include <TFile.h>
#include <TFitResult.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TLatex.h>
#include <TLegend.h>
#include <TPad.h>
#include <TPaveStats.h>
#include <TProfile.h>
#include <TROOT.h>
#include <TString.h>
#include <TStyle.h>
#include <TTree.h>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <optional>
#include <sstream>
#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>
#include <cctype>

TString filename_data;
std::vector<std::string> file_names;
int vcnt[128][32];
// averages channel in the neighborhood for dynamic threshold
std::vector<float> neigh_even;
std::vector<float> neigh_odd;
// weighted average based on Y projection of problematic or suspicious channels
// based on the vicinity of a dynamic threshold
std::optional<std::vector<Double_t>> sus_even;
std::optional<std::vector<Double_t>> sus_odd;

void analyze() {
  //! Variables for reading, analyzing data and displaying histograms
  int ch_min = 0;
  int ch_max = 128;
  int ch_step = 1;

  int d_min = 0;
  int d_max = 31;
  int d_step = 1;

  filename_data = (filename_data + ".txt");
  std::ifstream scanfile;
  scanfile.open(filename_data);
  int fch;
  std::string line;
  std::string ele;

  int count = 0;
  int ch_cnt = 0;
  while (!scanfile.eof()) {
    std::getline(scanfile, line);
    std::istringstream iss(line);
    if (!line.empty()) {
      iss >> ele;
      iss >> ele;
      count = 0;
      for (int d = d_min; d < d_max + 1; d++) {
        iss >> count;
        vcnt[ch_cnt][d] = count;
      }
      ch_cnt++;
    }
  }

  scanfile.close();
}

bool read_file_tests() {
  // ----------------------------------------------------------------------------------------------------------------
  TString file_list = "a";
  // ----------------------------------------------------------------------------------------------------------------
  std::ifstream myfile;
  myfile.open(file_list + ".txt");
  std::string line;
  std::string delimiter = ".txt";
  std::size_t pos = 0;
  std::string token;

  // reading list file and storing it in a vector
  while (!myfile.eof()) {
    std::getline(myfile, line);
    if (line.empty())
      continue;
    else {
      while ((pos = line.find(delimiter)) != std::string::npos) {
        token = line.substr(0, pos);
        // cout << token << endl;
        line.erase(0, pos + delimiter.length());
      }
      file_names.push_back(token);
    }
  }
  if (file_names.size() != 0)
    return false;
  return true;
}

// get_file_name from the file list
std::string get_file_name(int i) { return file_names[i]; }

// getting the chip ID
std::string ExtractName(const std::string& filename) {
    // 1. Find the 4th underscore
    size_t pos = 0;
    int underscoreCount = 0;
    while (underscoreCount < 4 && pos != std::string::npos) {
        pos = filename.find('_', pos + 1);
        underscoreCount++;
    }
    if (pos == std::string::npos)
        return "";

    size_t start = pos + 1; // Start after the 4th "_"

    // 2. Find "_HW_"
    size_t hwPos = filename.find("_HW_", start);
    if (hwPos == std::string::npos)
        return "";

    // 3. Extend end to include "_HW_" + following digits
    size_t end = hwPos + 4;
    while (end < filename.size() && std::isdigit(filename[end]))
        end++;

    // 4. Find last underscore (for suffix)
    size_t lastUnderscore = filename.rfind('_');
    if (lastUnderscore == std::string::npos || lastUnderscore <= end)
        return filename.substr(start, end - start);  // no suffix found

    // 5. Extract suffix (after the last underscore)
    std::string suffix = filename.substr(lastUnderscore + 1);

    // 6. Combine both parts
    return filename.substr(start, end - start) + "_" + suffix;
}

// write the summary of broken channels for each file
void write_results(TString filename_str, std::vector<int> brk_ch) {
  std::ofstream myfile;
  myfile.open("conn_check_summary.txt", std::ios::out | std::ios::app);
  myfile << filename_str << ": ";
  if (brk_ch.size() != 0) {
    for (int i = 0; i < brk_ch.size(); i++) {
      myfile << brk_ch[i] << ",";
    }
    myfile << "\n";
  } else {
    myfile << "0"
           << "\n";
  }
  myfile.close();
}

/*  Check if a file exists
    returns  true if the file exists, else false */
bool file_exists(const std::string &filename) {
  struct stat buf;
  if (stat(filename.c_str(), &buf) != -1) {
    return true;
  }
  return false;
}
// write the parameters used to find the total number of broken channels
void write_parameters(float z_alpha, int n_neigh_ch,
                      std::optional<float> sus_perc,
                      std::optional<float> n_sigmas,
                      std::size_t n_broken_channels) {
  /* z_alpha - percentage of neighbors' median for the threshold
  n_neigh_ch - number of channels in the neighborhood for dynamic threshold
  OPTIONAL PARAMETERS FOR SUSPICIOUS CHANNEL DETECTION:
  sus_perc - percentage of threshold vicinity for determining suspicious
  channels n_sigmas - accepted deviation from the median of weighted average of
  suspicious channels' neighbors */
  std::ofstream my_file;
  if (!file_exists("conn_check_parameters.txt")) {
    my_file.open("conn_check_parameters.txt", std::ios::out);
    if (sus_perc.has_value() && n_sigmas.has_value()) {
      my_file << std::fixed << std::setw(20) << "z_alpha" << '\t' << std::fixed
              << std::setw(20) << "n_neigh_ch" << '\t' << std::fixed
              << std::setw(20) << "sus_perc" << '\t' << std::fixed
              << std::setw(20) << "n_sigmas" << '\t' << std::fixed
              << std::setw(20) << "n_broken_channels" << '\n';

      my_file << std::fixed << std::setw(20) << z_alpha << '\t' << std::fixed
              << std::setw(20) << n_neigh_ch << '\t' << std::fixed
              << std::setw(20) << sus_perc.value() << '\t' << std::fixed
              << std::setw(20) << n_sigmas.value() << '\t' << std::fixed
              << std::setw(20) << n_broken_channels << '\n';
    }

    else { // if no sus channels are processed
      my_file << std::fixed << std::setw(20) << "z_alpha" << '\t' << std::fixed
              << std::setw(20) << "n_neigh_ch" << '\t' << std::fixed
              << std::setw(20) << "        " << '\t' << std::fixed
              << std::setw(20) << "        " << '\t' << std::fixed
              << std::setw(20) << "n_broken_channels" << '\n';

      my_file << std::fixed << std::setw(20) << z_alpha << '\t' << std::fixed
              << std::setw(20) << n_neigh_ch << '\t' << std::fixed
              << std::setw(20) << "        " << '\t' << std::fixed
              << std::setw(20) << "        " << '\t' << std::fixed
              << std::setw(20) << n_broken_channels << '\n';
    }

  } else { // if the file exists
    my_file.open("conn_check_parameters.txt", std::ios::app);
    if (sus_perc.has_value() && n_sigmas.has_value()) {
      my_file << std::fixed << std::setw(20) << z_alpha << '\t' << std::fixed
              << std::setw(20) << n_neigh_ch << '\t' << std::fixed
              << std::setw(20) << sus_perc.value() << '\t' << std::fixed
              << std::setw(20) << n_sigmas.value() << '\t' << std::fixed
              << std::setw(20) << n_broken_channels << '\n';
    }

    else { // if no sus channels are processed
      my_file << std::fixed << std::setw(20) << z_alpha << '\t' << std::fixed
              << std::setw(20) << n_neigh_ch << '\t' << std::fixed
              << std::setw(20) << "        " << '\t' << std::fixed
              << std::setw(20) << "        " << '\t' << std::fixed
              << std::setw(20) << n_broken_channels << '\n';
    }
  }

  my_file.close();
}

template <typename T> T calculate_median(std::vector<T> v, std::size_t n) {
  // Sort the vector
  std::sort(v.begin(), v.end());
  // Check if the number of elements is odd
  if (n % 2 != 0) {
    return (double)v[n / 2];
  }
  // If the number of elements is even, return the average
  // of the two middle elements
  return (double)(v[(n - 1) / 2] + v[n / 2]) / 2.0;
}

Double_t calculate_amp_enc(int ch, TH2F *h2, float thr) {
  TString histoname(Form("h_ch_%d_amp", ch));
  TH1D *py_h2hits_amp = h2->ProjectionY(histoname, ch + 1, ch + 1);
  float amp_ptp = 0;
  //Calculating 10%
  int maxBin = py_h2hits_amp->GetMaximumBin();
  int thr_10 = thr*py_h2hits_amp->GetBinContent(maxBin);  
  //Finding lim_min
  int lim_min = 0;
  int lim_max = 0;
  for (Int_t n_bin = 2; n_bin < py_h2hits_amp->GetNbinsX(); n_bin++) {
    if (py_h2hits_amp->GetBinContent(n_bin)>thr_10){
      if (abs(py_h2hits_amp->GetBinContent(n_bin)-thr_10)<abs(py_h2hits_amp->GetBinContent(n_bin-1)-thr_10))
      {
        lim_min = py_h2hits_amp->GetBinCenter(n_bin);
      }
      else{
        lim_min = py_h2hits_amp->GetBinCenter(n_bin - 1);
      }
      break;
    }
    else{
      continue;
    }
  }
  //Finding lim_max
  for (Int_t n_bin = py_h2hits_amp->GetNbinsX()-2; n_bin >1; n_bin--) {
    if (py_h2hits_amp->GetBinContent(n_bin)>thr_10){
      if (abs(py_h2hits_amp->GetBinContent(n_bin)-thr_10)<abs(py_h2hits_amp->GetBinContent(n_bin+1)-thr_10)){
        lim_max = py_h2hits_amp->GetBinCenter(n_bin);
      }
      else{
        lim_max = py_h2hits_amp->GetBinCenter(n_bin + 1);
      }
      break;
    }
    else{
      continue;
    }
  }
  amp_ptp = (lim_max - lim_min);
  //py_h2hits_amp->Write();
  delete py_h2hits_amp;
  //cout<<"Channel:\t"<<ch<<"\t"<<"Max Bin:\t"<<maxBin<<"\t"<<thr_10<<"\tLim_min:\t"<<lim_min<<"\tLim_max:\t"<<lim_max<<"\t"<<lim_max-lim_min<<endl;
  return amp_ptp;
}

Double_t calculate_weighted_av_ch(int ch, TH2F *h2) {
  // histogram for keeping Y projection of h2hits_amp for this channel
  // to transition from bins to channels, we always need to add 1
  // since bins start from 1 but channels start from 0 we shift
  TH1D *py_h2hits_amp = h2->ProjectionY("py_h2hits_amp", ch + 1, ch + 1, "");
  // get the total sum of weights ing the Y projection
  Double_t sum_of_weights_py_h2hits = py_h2hits_amp->Integral();
  // initialize the weighted average sum of amplitude for this channel
  Double_t sum_average_amp = 0.0;

  for (Int_t n_bin = 1; n_bin < py_h2hits_amp->GetNbinsX(); n_bin++) {
    Double_t weight_py_h2hits = py_h2hits_amp->GetBinContent(n_bin);
    Double_t adc_value = py_h2hits_amp->GetXaxis()->GetBinLowEdge(n_bin);
    sum_average_amp += weight_py_h2hits * adc_value;
  }
  delete py_h2hits_amp;
  if (sum_of_weights_py_h2hits != 0) {
    Double_t weighted_av_ch = sum_average_amp / sum_of_weights_py_h2hits;
    return weighted_av_ch;
  } else {
    return sum_average_amp;
  }
}

// Write a vector of channels to the histogram
void fill_hist_with_ch(TH1F *hist, std::vector<int> channels, int *ch_hits) {
  if (channels.size() != 0) {
    for (int j = 0; j < channels.size(); j++) {
      hist->Fill(channels[j], ch_hits[channels[j]]);
    }
  }
}

// ------------------------ Main function ---------------------
int analysis_conn_check(bool detect_sus_ch = false) {
  // mtx.lock();
  for (int ch = 0; ch < 128; ch++) {
    for (int d = 0; d < 32; d++) {
      vcnt[ch][31 - d] = 0;
    }
  }

  TString dir = "";
  TString filename_root = "";
  std::vector<int> broken_channels;
  std::optional<std::vector<int>> suspicious_channels;
  std::optional<std::vector<int>> noisy_channels;

  // counter of broken channels accross all the files
  std::size_t n_broken_channels = 0;
  // percentage of neighbors' median for the threshold
  float z_alpha = 0.55;
  // number of channels in the neighborhood for dynamic threshold
  int n_neigh_ch = 25;

  std::optional<float> sus_perc, n_sigmas;
  /*sus_perc - percentage of threshold vicinity for determining suspicious
   channels n_sigmas - accepted deviation from the median of weighted average of
   suspicious channels' neighbors */
  if (detect_sus_ch) {
    sus_perc = 0.10;
    n_sigmas = 4.5;
    suspicious_channels = std::vector<int>();
    noisy_channels = std::vector<int>();
    sus_even = std::vector<Double_t>();
    sus_odd = std::vector<Double_t>();
  } else {
    sus_perc.reset();
    n_sigmas.reset();
    suspicious_channels.reset();
    noisy_channels.reset();
    sus_even.reset();
    sus_odd.reset();
  }
  // reading file with list of measurements files.
  read_file_tests();
  
  // analyze function is called for every file in the list. At the end,
  // everything is closed and root files can be accessed via TBrowser
  for (int i = 0; i < int(file_names.size()); i++) {
    filename_data = dir + get_file_name(i).c_str();
    filename_root = dir + get_file_name(i).c_str();
    std::string filename_asic_id = get_file_name(i).c_str();
    // cout << "Filename_selected: " << filename_data << endl;
    analyze();
    // Getting the ASIC ID
    std::string asic_id = ExtractName(filename_asic_id);
    // erase the previous channels
    broken_channels.clear();
    if (detect_sus_ch) {
      suspicious_channels.value().clear();
      noisy_channels.value().clear();
    }

    // Creating files
    TFile *file1 = new TFile(filename_root + ".root", "recreate");
    file1->cd();
    // Creating histograms
    TH1F h_ave_hits("h_hits", "h_hits", 128, 0, 128);
    TH1F h_broken_ch("h_broken_ch", "h_broken_ch", 128, 0, 128);

    TH1F h1_ave_even("h1_ave_even", "h1_ave_even", 100, 0, 400000);
    TH1F h1_ave_odd("h1_ave_odd", "h1_ave_odd", 100, 0,
                    400000); // access these channels
    TH1F h_ave_even("h_ave_even", "h_ave_even", 64, 0, 128);
    TH1F h_ave_odd("h_ave_odd", "h_ave_odd", 64, 1, 129);
    TH2F h2hits_amp("h2hits_amp", "h2hits_amp", 128, 0, 128, 31, 1, 31);

    // histograms storing thresholds for each channel, even and odd separately
    TH1F h_thr_even("h_thr_even", "h_thr_even", 128, 0, 128);
    TH1F h_thr_odd("h_thr_odd", "h_thr_odd", 128, 0, 128);

    // histograms storing noise amplitude for each channel
    TH1F h_ch_amp("h_ch_amp", "h_ch_amp", 128, 0, 128);
    // histograms storing noise amplitude for each channel
    TH1F h_ch_enc("h_ch_enc", "h_ch_enc", 128, 0, 128);

    // histograms storing median values based on the neighboring channels
    TH1F h_med_even("h_med_even", "h_med_even", 128, 0, 128);
    TH1F h_med_odd("h_med_odd", "h_med_odd", 128, 0, 128);

    // histogram storing suspicious channels
    std::optional<TH1F> h_sus_ch;
    // histogram storing extra noisy channels
    std::optional<TH1F> h_noisy_ch;
    // histogram for weighted average ADC of each channel
    std::optional<TH1D> h_weighted_av;
    // histogram for storing median of weighed average ADC of neighbors
    std::optional<TH1D> h_med_weighted_av;
    // histogram to get std dev of weighted average
    std::optional<TH1D> h1_weighted_av_std;
    if (detect_sus_ch) {
      h_sus_ch.emplace("h_sus_ch", "h_sus_ch", 128, 0, 128);
      h_noisy_ch.emplace("h_noisy_ch", "h_noisy_ch", 128, 0, 128);
      h_weighted_av.emplace("h_weighted_av", "h_weighted_av", 128, 0, 128);
      h_med_weighted_av.emplace("h_med_weighted_av", "h_med_weighted_av", 128,
                                0, 128);
      h1_weighted_av_std.emplace("h1_weighted_av_std", "h1_weighted_av_std",
                                 100, 0, 15);

    } else {
      h_sus_ch.reset();
      h_noisy_ch.reset();
      h_weighted_av.reset();
      h_med_weighted_av.reset();
      h1_weighted_av_std.reset();
    }

    // ---------------------------------------------- Histograms name
    
    h_ave_hits.SetTitle(asic_id.c_str()); 
    h_ave_hits.GetXaxis()->SetTitle("Channel number"); 
    h_ave_hits.GetYaxis()->SetTitle("Counts");
    h_broken_ch.SetTitle("Noise counts vs channel; Channel number; Counts");
    h1_ave_even.SetTitle("Noise counts; Number of noise hits; Entries");
    h1_ave_odd.SetTitle("Noise counts; Number of noise hits; Entries");
    h2hits_amp.SetTitle("Noise counts distribution; Channel number; ADC value");
    h_thr_even.SetTitle("Threshold vs channel; Channel number; Threshold");
    h_thr_odd.SetTitle("Threshold vs channel; Channel number; Threshold");
    h_med_even.SetTitle(
        "Median of neighbors vs channel; Channel number; Median of neighbors");
    h_med_odd.SetTitle(
        "Median of neighbors vs channel; Channel number; Median of neighbors");

    int ch_hits[128] = {0};
    float ave_even = 0.;
    float ave_odd = 0.;


    float enc_amp_adc = 0.;
    // Initial iteration to calculate averages. Odd and Even channels separated
    for (int ch = 0; ch < 128; ch++) {
      for (int d = 1; d < 32; d++) {
        h2hits_amp.Fill(ch, 31 - d, vcnt[ch][d]);
        ch_hits[ch] += vcnt[ch][d];
      }
      h_ave_hits.Fill(ch, ch_hits[ch]);
      if (ch % 2 == 0) {
        h1_ave_even.Fill(ch_hits[ch]);
        h_ave_even.Fill(ch, ch_hits[ch]);
      } else {
        h1_ave_odd.Fill(ch_hits[ch]);
        h_ave_odd.Fill(ch, ch_hits[ch]);
      }
    }
    
    // Displaying the noise amplitude
    /*/
    float thr_enc = 0.5;
    // Extracted from pscan files 
    float adc_gain = 800;
    float ptp_sigma = 10.;
    for (int ch = 0; ch < 128; ch++) {
      enc_amp_adc = calculate_amp_enc(ch, &h2hits_amp, thr_enc);
      //From peak to peak to sigma
      enc_amp_adc = enc_amp_adc/
      h_ch_amp.Fill(ch, enc_amp_adc);
    }*/


    // stddev of weighted average
    std::optional<Double_t> sus_neigh_sigma;
    if (detect_sus_ch) {
      // fill the weighted average hist with the value for current channel
      for (int ch = 0; ch < 128; ch++) {
        if (!std::isnan(calculate_weighted_av_ch(ch, &h2hits_amp))) {
          h_weighted_av.value().Fill(ch,
                                     calculate_weighted_av_ch(ch, &h2hits_amp));
          h1_weighted_av_std.value().Fill(
              calculate_weighted_av_ch(ch, &h2hits_amp));
        }
      }
      sus_neigh_sigma = h1_weighted_av_std.value().GetStdDev();
    } else {
      sus_neigh_sigma.reset();
    }
    // medians of neigh_even, neigh_odd vectors
    float neigh_med_even, neigh_med_odd;
    // dynamic thresholds for even and odd channels based on the neighborhood
    float thr_even, thr_odd;

    // Iteration to discriminate broken channels
    for (int ch = 0; ch < 128; ch++) {

      // erase the neighbors of the previous channel
      neigh_odd.clear();
      neigh_even.clear();
      if (detect_sus_ch) {
        // erase the weighted sums of surroundings of the previous channel
        sus_even.value().clear();
        sus_odd.value().clear();
      }

      // the channel may not have enough neighbors to the left or right
      int start_ch = std::max(0, ch - n_neigh_ch);
      int end_ch = std::min(128, ch + n_neigh_ch + 1);

      if (ch % 2 == 0) {
        // if the channel is even

        for (int i = start_ch; i < end_ch; i++) {
          // Loop through neighboring channels to the left and right of the
          // current channel
          if (i != ch) {

            neigh_even.push_back(ch_hits[i]);
            if (detect_sus_ch) {
              // add weighted sum of ADC in neighboring channels
              sus_even.value().push_back(
                  calculate_weighted_av_ch(i, &h2hits_amp));
            }
          }
        }

        // calculate the median of neighboring channels
        neigh_med_even = calculate_median(neigh_even, neigh_even.size());
        h_med_even.Fill(ch, neigh_med_even);
        thr_even = z_alpha * neigh_med_even;
        h_thr_even.Fill(ch, thr_even);

        if (detect_sus_ch) {
          // calculate the median of weighted sum in neighboring channels
          h_med_weighted_av.value().Fill(
              ch, calculate_median(sus_even.value(), sus_even.value().size()));

          if (ch_hits[ch] > thr_even * n_sigmas.value()) {
            // if the channel is extra noisy it's also masked
            std::cout << "noisy channel " << ch << '\n';
            noisy_channels.value().push_back(ch);
          }
        }
        if (ch_hits[ch] < thr_even) {
          // if the channel is broken based on the dynamic threshold

          broken_channels.push_back(ch);
          //______________________
          if (h_ave_hits.GetBinContent(ch + 1) == 0) {
            // No analog response (NAR): broken channel
            std::cout << "broken dynamic threshold NAR channel " << ch << '\n';
          } else {
            std::cout << "broken dynamic threshold channel " << ch << '\n';
          }
        } else if (detect_sus_ch && std::abs(ch_hits[ch] - thr_even) <
                                        thr_even * sus_perc.value()) {
          //  std::cout << "suspicious channel " << ch << '\n';

          // if the channel is suspicious based on the vicinity of threshold
          // criteria separate for each channel
          // remembering about the ch + 1 to bin shift
          if (std::abs(h_weighted_av.value().GetBinContent(ch + 1) -
                       h_med_weighted_av.value().GetBinContent(ch + 1)) >
              n_sigmas.value() * sus_neigh_sigma.value()) {
            // if the median of ADC weighted sum in neighboring channels is
            // significantly different than for the suspicious channel
            suspicious_channels.value().push_back(ch);
            broken_channels.push_back(ch);
            std::cout << "broken suspicious channel " << ch << '\n';

          } else {
            suspicious_channels.value().push_back(ch);
          }
        }
      }

      else if (ch % 2 != 0) {
        // if the channel is odd

        for (int i = start_ch; i < end_ch; i++) {
          // Loop through neighboring channels to the left and right of the
          // current channel
          if (i != ch) {

            neigh_odd.push_back(ch_hits[i]);
            if (detect_sus_ch) {
              sus_odd.value().push_back(
                  calculate_weighted_av_ch(i, &h2hits_amp));
            }
          }
        }

        // calculate the median of neighboring channels
        neigh_med_odd = calculate_median(neigh_odd, neigh_odd.size());
        h_med_odd.Fill(ch, neigh_med_odd);
        thr_odd = z_alpha * neigh_med_odd;
        h_thr_odd.Fill(ch, thr_odd);

        if (detect_sus_ch) {
          // calculate the median of weighted sum in neighboring channels
          h_med_weighted_av.value().Fill(
              ch, calculate_median(sus_odd.value(), sus_odd.value().size()));

          if (ch_hits[ch] > thr_odd * n_sigmas.value()) {
            // if the channel is extra noisy it's also masked
            std::cout << "noisy" << '\n';

            noisy_channels.value().push_back(ch);
          }
        }

        if (ch_hits[ch] < thr_odd) {
          // if the channel is broken based on the dynamic threshold

          broken_channels.push_back(ch);
          if (h_ave_hits.GetBinContent(ch + 1) == 0) {
            // No analog response (NAR): broken channel
            std::cout << "broken dynamic threshold NAR channel " << ch << '\n';
          } else {
            std::cout << "broken dynamic threshold channel " << ch << '\n';
          }
        } else if (detect_sus_ch && std::abs(ch_hits[ch] - thr_odd) <
                                        thr_odd * sus_perc.value()) {
          //  std::cout << "suspicious channel " << ch << '\n';

          // if the channel is suspicious based on the vicinity of threshold
          // criteria separate for each channel
          //___Last criterium to find broken channels___
          // remembering about the ch + 1 to bin shift
          if (std::abs(h_weighted_av.value().GetBinContent(ch + 1) -
                       h_med_weighted_av.value().GetBinContent(ch + 1)) >
              n_sigmas.value() * sus_neigh_sigma.value()) {
            // if the median of ADC weighted sum in neighboring channels if
            // significantly different
            suspicious_channels.value().push_back(ch);
            broken_channels.push_back(ch);
            std::cout << "broken suspicious channel " << ch << '\n';

          } else {
            suspicious_channels.value().push_back(ch);
          }
        }
      }
    }

    // Writing the histogram of the broken, suspicious and noisy channels
    fill_hist_with_ch(&h_broken_ch, broken_channels, ch_hits);
    if (detect_sus_ch) {
      fill_hist_with_ch(&h_sus_ch.value(), suspicious_channels.value(),
                        ch_hits);
      fill_hist_with_ch(&h_noisy_ch.value(), noisy_channels.value(), ch_hits);
    }

    TCanvas *c1 = new TCanvas(asic_id.c_str(), asic_id.c_str(), 1300, 1200);
    c1->SetGrid();
    h_ave_hits.Draw("HIST");
    h_ave_hits.SetStats(0);
    h_ave_hits.GetYaxis()->SetTitleOffset(1.2);
    h_ave_hits.GetYaxis()->SetNdivisions(510, 4);
    //h_ave_hits.SetLineColor(kAzure - 3);
    h_ave_hits.SetLineColor(kGray+1);
    h_ave_hits.SetLineWidth(2);
    h_ave_hits.GetYaxis()->SetRangeUser(0, 1.5 * h_ave_hits.GetMaximum());

    // Adding broken channels
    h_broken_ch.Draw("PHISTSAME");
    h_broken_ch.SetMarkerSize(1.8);
    h_broken_ch.SetMarkerStyle(23);
    h_broken_ch.SetMarkerColor(kBlack);

    if (detect_sus_ch) {
      // Adding suspicious channels
      h_sus_ch.value().Draw("HISTSAME");
      h_sus_ch.value().SetMarkerSize(1.5);
      h_sus_ch.value().SetMarkerStyle(22);
      h_sus_ch.value().SetMarkerColor(kAzure + 2);

      // Adding extra noisy channels
      h_noisy_ch.value().Draw("HISTSAME");
      h_noisy_ch.value().SetMarkerSize(2);
      h_noisy_ch.value().SetMarkerStyle(33);
      h_noisy_ch.value().SetMarkerColor(kOrange + 1);
    }

    h_med_even.Draw("PEHISTSAME");
    h_med_even.SetLineWidth(0);
    h_med_even.SetMarkerStyle(21);
    h_med_even.SetMarkerColor(kRed);
    h_med_even.SetMarkerSize(0.6);

    for (int i=1; i<=h_thr_even.GetNbinsX(); ++i){
      h_thr_even.SetBinError(i, 1);}
    h_thr_even.Draw("PEHISTSAME");
    h_thr_even.SetLineWidth(3);
    h_thr_even.SetLineColor(kRed);
    //h_thr_even.SetMarkerStyle(kFullDotLarge);
    //h_thr_even.SetMarkerColor(kGreen + 2);

    
    h_med_odd.Draw("PEHISTSAME");
    h_med_odd.SetLineWidth(0);
    h_med_odd.SetMarkerStyle(21);
    h_med_odd.SetMarkerSize(0.6);
    h_med_odd.SetMarkerColor(kBlue);
    
    for (int i=1; i<=h_thr_odd.GetNbinsX(); ++i){
      h_thr_odd.SetBinError(i, 1);}
    h_thr_odd.Draw("PEHISTSAME");
    h_thr_odd.SetLineWidth(3);
    h_thr_odd.SetLineColor(kBlue);
    //h_thr_odd.SetMarkerStyle(kFullDotLarge);
    h_thr_odd.SetMarkerStyle(1);
    //h_thr_odd.SetMarkerColor(kMagenta + 2);

    TLegend *lg_brk_summ = new TLegend(0.45, 0.65, 0.85, 0.85, "", "nbNDC");
    lg_brk_summ->SetHeader(asic_id.c_str());
    lg_brk_summ->AddEntry(&h_ave_hits, "Noise hits", "l");
    lg_brk_summ->AddEntry(&h_broken_ch, "Broken channels", "p");
    if (detect_sus_ch) {
      if (suspicious_channels.value().size() != 0)
        lg_brk_summ->AddEntry(&h_sus_ch.value(), "Suspicious channels", "p");
      if (noisy_channels.value().size() != 0)
        lg_brk_summ->AddEntry(&h_noisy_ch.value(), "Noisy channels", "p");
    }
    lg_brk_summ->AddEntry(&h_med_even, "Median of neighboring even channels","p");
    lg_brk_summ->AddEntry(&h_med_odd, "Median of neighboring odd channels","p");
    lg_brk_summ->AddEntry(&h_thr_even, "Threshold broken even channels", "lp");
    lg_brk_summ->AddEntry(&h_thr_odd, "Threshold broken odd channels", "lp");

    lg_brk_summ->SetBorderSize(0);
    lg_brk_summ->SetFillColor(0);
    lg_brk_summ->Draw("");
    c1->Write();
    //c1->SetCanvasSize(2400, 1800); // scale up internal buffer before saving
    c1->SaveAs(filename_root+".png");
    c1->Close();

    //  Writing histograms to file
    gStyle->SetOptStat();
    h_ave_hits.Write();
    h2hits_amp.Write();
    //h_ch_amp.Write();
    h_broken_ch.Write();
    h1_ave_even.Write();
    h1_ave_odd.Write();
    h_ave_even.Write();
    h_ave_odd.Write();
    if (detect_sus_ch) {
      // add weighted average, its stddev and median
      h_weighted_av.value().Write();
      h_med_weighted_av.value().Write();
      h1_weighted_av_std.value().Write();
    }

    // close the opened root file
    file1->Close();
    // Writing summary file
    write_results(filename_root, broken_channels);
    n_broken_channels += broken_channels.size();
    delete file1;
    delete c1;
    delete lg_brk_summ;
  }
  //write_parameters(z_alpha, n_neigh_ch, sus_perc, n_sigmas, n_broken_channels);
  return 0;
}
