#include "TString.h"
#include <sstream>
#include "string"
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <cmath>
#include <stdint.h>
#include <unistd.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TCanvas.h>
#include <TFitResult.h>
#include <TFile.h>
#include <TProfile.h>
#include <TTree.h>
#include <TROOT.h>
#include <TStyle.h>
#include <TLegend.h>
#include <TPad.h>
#include <TF1.h>
#include <TBrowser.h>
#include <TLatex.h>
#include <vector>
#include <stdlib.h> 
#include <cstdio>
#include <ctime>
#include <filesystem>

TString filename_data;
std::vector<std::string> file_names;

float enc_h[1024];
float enc_e[1024];
float encf_h[1024];
float encf_e[1024];

float thr_h[1024];
float thr_e[1024];
float thrf_h[1024];
float thrf_e[1024];

float gain_h[1024];
float gain_e[1024];

void Analysis(int k = 0, int pol= 1, int fast_flag_user = 0){
  
  int ch_min = 1;
  int ch_max = 127;
  int ch_step =1;
  int fast_flag = fast_flag_user;
  filename_data = (filename_data+".root");
  TFile *file_open = new TFile(filename_data);
  
  cout<<"FILENAME: "<<filename_data <<endl;
  TH1F *henc = (TH1F*)file_open->Get("h_enc");
  //TH1F *henc = (TH1F*)file_open->Get("hfit_s_erfc");
  TH1F *hthr = (TH1F*)file_open->Get("h_adc_thr");
  TH1F *hgain  = (TH1F*)file_open->Get("h_adc_gain");

  TH1F *hmean_fast = (TH1F*)file_open->Get("h_fast_thr");
  TH1F *hnoise_fast= (TH1F*)file_open->Get("h_enc_fast");
  
  for (int ch = 0; ch <128; ch++){
    if (pol == 1){
      enc_h[k*128+ch]  = henc->GetBinContent(ch+1);
      thr_h[k*128+ch]  = hthr->GetBinContent(ch+1);
      gain_h[k*128+ch] = hgain->GetBinContent(ch+1);
      if (fast_flag == 1){
            encf_h[k*128+ch] = hnoise_fast->GetBinContent(ch+1);
            thrf_h[k*128+ch] = hmean_fast->GetBinContent(ch+1);
      }
    }
    else {
      enc_e[k*128+ch]  = henc->GetBinContent(ch+1);
      thr_e[k*128+ch]  = hthr->GetBinContent(ch+1);
      gain_e[k*128+ch] = hgain->GetBinContent(ch+1); 
      if (fast_flag == 1){
          encf_e[k*128+ch] = hnoise_fast->GetBinContent(ch+1);
          thrf_e[k*128+ch] = hmean_fast->GetBinContent(ch+1);
      }
    }
  }  
 file_open->Close();
}

bool Read_file_tests(){
  // Path of the file with name hmodule_lab_test
  // ----------------------------------------------------------------------------------------------------------------  
  TString file_list = "plot";
  // ----------------------------------------------------------------------------------------------------------------  
  
  ifstream myfile;
  myfile.open(file_list+ ".txt");
  std::string line;
  std::string delimiter = ".root";
  size_t pos = 0;
  std::string token;

 // reading list file and storing it in a vector
  cout<<"......................oooo00000oooo..............................."<<endl;
  while(!myfile.eof()){
    std::getline(myfile, line);
    if (line.empty()) continue;
     else {
    while ((pos = line.find(delimiter)) != std::string::npos) {
      token = line.substr(0, pos);
      cout << token << endl;
      line.erase(0, pos + delimiter.length());
      }  
      file_names.push_back(token);
     }
    }
  cout<<file_names.size()<<endl;  
  if (file_names.size()!=0) return false;
  return true; 
}

// ............... oooo00000oooo........................
//! Get_file_name from the file list
std::string Get_file_name(int i) {return file_names[i];}

TString get_module_ID() {
  std::string current_path = std::filesystem::current_path().string();
  std::string parent_path =
      std::filesystem::path(current_path).parent_path().string();
  std::string module_ID =
      std::filesystem::path(parent_path).filename().string();
  return TString(module_ID);
}

auto extract_ladder_number = [](const TString &module_ID) {
  return TString(module_ID(10, 2)); //.Atoi()
};

auto extract_module_type = [](const TString &module_ID) {
  return TString(module_ID(6, 1));
};

std::pair<float, float> find_cable_length_sensor_size() {

  TString module_ID = get_module_ID();
  std::cout << "module ID: " << module_ID << std::endl;
  std::cout << "module type: " << extract_module_type(module_ID) << std::endl;
  std::cout << "ladder number: " << extract_ladder_number(module_ID)
            << std::endl;

  //___________find__matching__ladder__number__and_module_type_____________//
  TString fileName = "../../../20251112/Ladder_sensors_cables.csv";
  const std::string LADDER_HEADER = "Ladder Type / Module";
  std::ifstream input_stream(fileName.Data());
  std::string line;
  std::string ladder_number, module_type;
  float n_side_cable, p_side_cable, sensor_size_mm, average_cm;
  float cable_length, sensor_size;

  while (getline(input_stream, line)) {

    std::istringstream string_stream(line);
    string_stream >> ladder_number >> module_type >> n_side_cable >>
        p_side_cable >> sensor_size_mm >> average_cm;
    
    if (ladder_number.find("L") != std::string::npos &&
        ladder_number != LADDER_HEADER) {

      if (ladder_number.substr(ladder_number.find_last_of(ladder_number.back()) - 1 , 2) ==
          extract_ladder_number(module_ID)) {
        if((module_type.substr(module_type.find("M")+ 2)) == extract_module_type(module_ID)){
                  std::cout << "found ladder " <<  extract_ladder_number(module_ID) << std::endl;
                  std::cout << "found module " <<  extract_module_type(module_ID) << std::endl;

     cable_length = average_cm;
     sensor_size = sensor_size_mm / 10.0; // convert mm to cm
        }
      }
    }
  }
std::cout << "cable_length " << cable_length << std::endl;
std::cout << "sensor_size " << sensor_size << std::endl;
    return std::make_pair(cable_length, sensor_size);
}
// ---------------------- MAIN function -------------------------
int plot_1024_MT(
) {
  TString module_ID = get_module_ID();      // module ID
  std::pair<float, float> module_params = find_cable_length_sensor_size();
  float cable_length = module_params.first; // microcable length
  float sensor_size = module_params.second; // sensor size

  //  --------------------------------------- General paramters of the ASIC analysis ----------------------------------
  time_t now = time(0); 
  char* date = ctime(&now);
  std::cout << "Date: "<< date<<std::endl;                                                                              // Running time of the analysis	
  TString operatorID = "AdrianRR";													                                                  // Operator's ID
  TString LabID = "GSI";															                                                        // Lab's ID      				
  float z_alpha = 2.0; 														                                                              // confidence levels to determine broken channels (95%) 
  float slope_enc_cap = 25.0;														                                                      // Measured ENC vs C slope
  float z_strips_cap = 17 ;                                                                                   // Capacitance of the Z strips for Hamamatsu sensors
  float enc_asic = 350;
  float enc_ana = enc_asic + ((cable_length)*0.38 + (sensor_size)*1.02)*slope_enc_cap;    		                  // ENC calculation based on ENC vs C parametrization
  float enc_ana_zz = enc_asic + ((cable_length)*0.38 + (sensor_size)*1.0 + z_strips_cap)*slope_enc_cap;    		// ENC calculation based on ENC vs C parametrization for Z strips
  float enc_mic = enc_asic +  ((cable_length)*0.38)*slope_enc_cap;						                                // ENC microcable based on ENC vs C parametrization
  int ch_min = 0;
  int ch_max = 128;
  int ch_max_module = 1024;
  float enc_fit_min = 100;
  static const std::map<float, int> db_metal_array = {                                                        // Map of the number of ZZ strips per sensor size       
    {0.0, 1},
    {2.2, 42},
    {4.2, 88},
    {6.2, 134},
    {12.4, 274}
  };
  // ---------------- IMPORTANT enabling or disabling ASICs in the final histogram ----------------------
  // In the reading file, named "plot.txt", the files should appear in the right order (holes first, elect after)
  // The ASICs order should coincide with the ena_asics array that is provided below. The nomber of files for each
  // olarity, should also be consistent with this 
  int ena_asics_h[8] = {1,1,1,1,1,1,1,1};
  int ena_asics_e[8] = {1,1,1,1,1,1,1,1};
  int fast_flag = 1;    // To enable the FAST discriminator data reading. To disable use 0
     
  // Here is the directory of the test files, same place where root files will be created   
  // ----------------------------------------------------------------------------------------------------  
  TString dir = "";
  // ----------------------------------------------------------------------------------------------------
  // ------------------ Assigning the exact number of ZZ strip channels per sensor ----------------------
  //int db_metal_ch = int(1024/6.2*sensor_size*(TMath::Tan(7.5/180.*TMath::Pi())));				  // Number of Z strips double metal 
  int db_metal_ch = db_metal_array.count(sensor_size) == 1 ? db_metal_array.at(sensor_size) : -1;
  cout<<"Number of db metal channels: "<<db_metal_ch<<endl;

  // ------------- Initial condition for finding channels outliers and broken channels ------------------
  float enc_lim_min = 0.7*enc_ana;                                                                            // Lower limit to discard broken channels in the average calculations
  float enc_lim_max = 6.0*enc_ana;                                                                            // Higher limit to discard noisy channels in the average calculations
  float temp_rate = 1.0;                                                                                      // to calculate broken channels on n-side based on high noise. 
  float disc_enc_e_even = 0;
  float disc_enc_e_odd  = 0;
  float disc_enc_h_even = 0;
  float disc_enc_h_odd  = 0;

  // ---------------------- Printing initial discrimination limits --------------------------------------
  cout <<"ENC limits: "<<"enc_lim_min: "<<enc_lim_min<<"\t"<<"enc_lim_max: "<<enc_lim_max<<endl;

  int sum_ch = 18;
  float Res_thr = 6;                                                                                         // Threshold or number of times the sigma of the dist (ave - enc_ch) is used to discriminate the broken channels in the final stage
  int thr_noresp = 200;                                                                                      // Threshold or higher limit for discriminating broken channels. 1st cause: No Analog Response (NAR)
  int thr_asic = enc_asic+50.;                                                                                   // Threshold of higher limit for separating cahnnels broken @ the ASIC level
  TString brk_cat[3] = {"NAR","ASIC","SENS"};                                                              // Broken channlls type of defect
  int brk_stats[2][3];
  std::vector<TString> ch_defects_e;
  std::vector<TString> ch_defects_h;

  // ----------------------------- Variables declaration ------------------------------------------------
  int h_max = 0;
  int e_max = 0;
  
  float ave_e_even = 0.;
  float ave_e_odd = 0.;
  float ave_h_even = 0.;
  float ave_h_odd = 0.;
  float ave_hzz_even = 0.;
  float ave_hzz_odd = 0.;
  float std_e_even = 0;
  float std_e_odd = 0;
  float std_h_even = 0;
  float std_h_odd  = 0.;
  float std_hzz_even = 0;
  float std_hzz_odd  = 0.;

  int ich_e_even =0;
  int ich_e_odd = 0;
  int ich_h_even = 0;
  int ich_h_odd = 0;
  int ich_hzz_even = 0;
  int ich_hzz_odd = 0;
  
  float ave_sum_h_odd = 0;
  float ave_sum_h_even = 0;
  float ave_sum_e_odd = 0;
  float ave_sum_e_even = 0;
  float ave_sum_h[1024]={0.};
  float ave_sum_e[1024]={0.};
  int ch_counter_e = 0;
  int ch_counter_h = 0;

  vector<int> brk_ch_elect;
  vector<int> brk_ch_holes;
  vector<int> brk_semi_elect;
  vector<int> brk_semi_holes;
  int brk_ch_total_odd = 0;
  int brk_ch_total_even = 0;
  
  for (int i =0; i <1024; i++){
     enc_h[i] = 0.;
     enc_e[i] = 0.;
     encf_h[i] = 0.;
     encf_e[i] = 0.;
     thr_h[i] = 0.;
     thr_e[i] = 0.;
     thrf_h[i] = 0.;
     thrf_e[i] = 0.;
     gain_h[i] = 0.;
     gain_e[i] = 0.;
  }
  
  for (int i =0; i<8; i++){
    h_max += ena_asics_h[i];
    e_max += ena_asics_e[i];
  } 
  
  for (int i =0; i<2; i++){
    for (int j = 0; j<3; j++){
      brk_stats[i][j] = 0;
    }
  }
  
  
  // ----------------------------------------- Creating file with histograms  ------------------------------------------
  TFile *f1;
  //TString filename_root = dir + "module_test_" + module_ID + ".root";
  TString filename_root = dir + "module_test_" + module_ID + ".root";
  cout <<filename_root<<endl;
  f1 = new TFile(filename_root, "recreate");
  
  // -------------------------------------- Creating result file with information --------------------------------------
  
  ofstream logfile;
  TString filename_log = dir + "module_test_" + module_ID + ".txt";
  logfile.open(filename_log);

  TString filename_pdf = dir + "module_test_" + module_ID + ".pdf";
  
  // ------------------------------------- Creating histograms to display results --------------------------------------
  // --------------------------------------------- Histograms [Mag vs ch] ----------------------------------------------
  // ENC - ADC
  TH1F *h_enc_elect = new TH1F("h_enc_elect", "h_enc_elect", 1024, 0, 1024);
  TH1F *h_enc_holes = new TH1F("h_enc_holes", "h_enc_holes", 1024, 0, 1024);
  // ENC - FAST
  TH1F *h_encfast_elect = new TH1F("h_encfast_elect", "h_encfast_elect", 1024, 0, 1024);
  TH1F *h_encfast_holes = new TH1F("h_encfast_holes", "h_encfast_holes", 1024, 0, 1024);
  // GAIN - ADC
  TH1F *h_gain_elect = new TH1F("h_gain_elect", "h_gain_elect", 1024, 0, 1024);
  TH1F *h_gain_holes = new TH1F("h_gain_holes", "h_gain_holes", 1024, 0, 1024);
  // THR - ADC
  TH1F *h_thr_elect = new TH1F("h_thr_elect", "h_thr_elect", 1024, 0, 1024);
  TH1F *h_thr_holes = new TH1F("h_thr_holes", "h_thr_holes", 1024, 0, 1024);
  //THR - FAST
  TH1F *h_thrfast_elect = new TH1F("h_thrfast_elect", "h_thrfast_elect", 1024, 0, 1024);
  TH1F *h_thrfast_holes = new TH1F("h_thrfast_holes", "h_thrfast_holes", 1024, 0, 1024);
  
  // -------------------------------------------- BROKEN CHANNELS HISTO ------------------------------------------------
  TH1F *h_brk_elect_vis = new TH1F("h_brk_elect_vis", "h_brk_elect_vis", 1024, 0, 1024);
  TH1F *h_brk_holes_vis = new TH1F("h_brk_holes_vis", "h_brk_holes_vis", 1024, 0, 1024);
  TH1F *h_brk_ave_elect = new TH1F("h_brk_ave_elect", "h_brk_ave_elect", 1024, 0, 1024);
  TH1F *h_brk_ave_holes = new TH1F("h_brk_ave_holes", "h_brk_ave_holes", 1024, 0, 1024);
  TH1F *h_brk_semi_elect = new TH1F("h_brk_semi_elect", "h_brk_semi_elect", 1024, 0, 1024);
  TH1F *h_brk_semi_holes = new TH1F("h_brk_semi_holes", "h_brk_semi_holes", 1024, 0, 1024);
  TH1F *h_brk_elect = new TH1F("h_brk_elect", "h_brk_elect", 1024, 0, 1024);
  TH1F *h_brk_holes = new TH1F("h_brk_holes", "h_brk_holes", 1024, 0, 1024);
  TH1F *h_brk_stats_elect = new TH1F("h_brk_stats_elect","h_brk_stats_elect", 3, 0, 3);
  TH1F *h_brk_stats_holes = new TH1F("h_brk_stats_holes","h_brk_stats_holes", 3, 0, 3);
   
  // ---------------------------------------------- Histograms name ----------------------------------------------------
  h_enc_elect->SetTitle("ENC vs channel; Channel number; ENC [e]");
  h_enc_holes->SetTitle("ENC vs channel; Channel number; ENC [e]");
  h_encfast_elect->SetTitle("ENC vs channel; Channel number; ENC [e]");
  h_encfast_holes->SetTitle("ENC vs channel; Channel number; ENC [e]");  
  h_gain_elect->SetTitle("ADC gain vs channel; Channel number; ADC gain [fC/LSB]");
  h_gain_holes->SetTitle("ADC gain vs channel; Channel number; ADC gain [fC/LSB]");
  h_thr_elect->SetTitle("ADC threshold vs channel; Channel number; ADC threshold [e]");
  h_thr_holes->SetTitle("ADC threshold vs channel; Channel number; ADC threshold [e]");
  h_thrfast_elect->SetTitle("FAST threshold vs channel; Channel number; FAST threshold [e]");
  h_thrfast_holes->SetTitle("FAST threshold vs channel; Channel number; FAST threshold [e]"); 
  h_brk_stats_elect->SetTitle("Broken channels statistics elect; Category; Entries");
  h_brk_stats_holes->SetTitle("Broken channels statistics holes; Category; Entries");
  
  // ---------------------------------------- Histograms [Distributions] ----------------------------------------------
  // ENC - ADC
  TH1F *h1_enc_elect = new TH1F("h1_enc_elect", "h1_enc_elect", 200, 0, 10000);
  TH1F *h1_enc_holes = new TH1F("h1_enc_holes", "h1_enc_holes", 200, 0, 10000);
  // ENC - ADC - ZZ channels 
  TH1F *h1_zz = new TH1F("h1_zz", "h1_zz", 100, 0, 5000);
  TH1F *h1_zz_even = new TH1F("h1_zz_even", "h1_zz_even", 100, 0, 5000);
  TH1F *h1_zz_odd = new TH1F("h1_zz_odd", "h1_zz_odd", 100, 0, 5000);
  // ENC - FAST
  TH1F *h1_encfast_elect = new TH1F("h1_encfast_elect", "h1_encfast_elect", 100, 0, 5000);
  TH1F *h1_encfast_holes = new TH1F("h1_encfast_holes", "h1_encfast_holes", 100, 0, 5000);
  // GAIN - ADC
  TH1F *h1_gain_elect = new TH1F("h1_gain_elect", "h1_gain_elect", 500, 0, 3100);
  TH1F *h1_gain_holes = new TH1F("h1_gain_holes", "h1_gain_holes", 500, 0, 3100);
  // THR -ADC
  TH1F *h1_thr_elect = new TH1F("h1_thr_elect", "h1_thr_elect", 250, 0, 25000);
  TH1F *h1_thr_holes = new TH1F("h1_thr_holes", "h1_thr_holes", 250, 0, 25000);
  // THR -FAST
  TH1F *h1_thrfast_elect = new TH1F("h1_thrfast_elect", "h1_thrfast_elect", 100, 0, 25000);
  TH1F *h1_thrfast_holes = new TH1F("h1_thrfast_holes", "h1_thrfast_holes", 100, 0, 25000);
  // BRK - CHANNELS
  TH1F *h1_brk_thr_elect = new TH1F("h1_brk_thr_elect", "h1_brk_thr_elect", 100, -250, 250);
  TH1F *h1_brk_thr_holes = new TH1F("h1_brk_thr_holes", "h1_brk_thr_holes", 100, -250, 250);

  // ---------------------------------------------- Histograms name ---------------------------------------------------
  h1_enc_elect->SetTitle("ENC distribution; ENC [e];  Entries");
  h1_enc_holes->SetTitle("ENC distribution; ENC [e];  Entries");
  h1_zz->SetTitle("ENC Z distribution; ENC [e];  Entries");
  h1_zz_even->SetTitle("ENC Z-even distribution; ENC [e];  Entries");
  h1_zz_odd->SetTitle("ENC Z-odd distribution; ENC [e];  Entries");
  h1_encfast_elect->SetTitle("ENC distribution; ENC [e];  Entries");
  h1_encfast_holes->SetTitle("ENC distribution; ENC [e];  Entries");
  h1_gain_elect->SetTitle("ADC gain distribution; ADC gain [e/LSB]; Entries");
  h1_gain_holes->SetTitle("ADC gain distribution; ADC gain [e/LSB]; Entries");
  h1_thr_elect->SetTitle("ADC threshold distribution; ADC threshold [e]; Entries");
  h1_thr_holes->SetTitle("ADC threshold distribution; ADC threshold [e]; Entries");
  h1_thrfast_elect->SetTitle("FAST threshold distribution; FAST threshold [e]; Entries");
  h1_thrfast_holes->SetTitle("FAST threshold distribution; FAST threshold [e]; Entries");
  h1_brk_thr_elect->SetTitle("Channels ENC residuals. Pol: Elect; Channel_enc_meas - Cannel_enc_ave [e]; Entries");
  h1_brk_thr_holes->SetTitle("Channels ENC residuals. Pol: Holes; Channel_enc_meas - Cannel_enc_ave [e]; Entries");
  
  // ----------------------------------------- Odd - Even difference --------------------------------------------------
  // ENC - ODD/EVEN -ELECT
  TH1F *h1_enc_elect_odd = new TH1F("h1_enc_elect_odd", "h1_enc_elect_odd", 140, 0, 7000);
  TH1F *h1_enc_elect_even = new TH1F("h1_enc_elect_even", "h1_enc_elect_even", 140, 0, 7000);
  // ENC - ODD/EVEN -HOLES
  TH1F *h1_enc_holes_odd = new TH1F("h1_enc_holes_odd", "h1_enc_holes_odd", 140, 0, 7000);
  TH1F *h1_enc_holes_even = new TH1F("h1_enc_holes_even", "h1_enc_holes_even", 140, 0, 7000);
  
  // ---------------------------------------------- Histograms name ---------------------------------------------------
  h1_enc_elect_odd->SetTitle("ENC_n-side_Odd channels; ENC [e];  Entries");
  h1_enc_elect_even->SetTitle("ENC_n-side_Even channels; ENC [e];  Entries");
  h1_enc_holes_odd->SetTitle("ENC_p-side_Odd channels; ENC [e];  Entries");
  h1_enc_holes_even->SetTitle("ENC_p-side_Even channels; ENC [e];  Entries");
  
  // ------------------------------------- Filling the log file with initial details ----------------------------------
  logfile<<"LAB_ID:\t"<<LabID<<endl;
  logfile<<"DATE:\t"<<date<<endl;
  logfile<<"OPERATOR_ID:   "<<operatorID<<endl;
  logfile<<"MODULE_ID:     "<<module_ID<<endl;
  logfile<<"SENSOR_SIZE:   "<<sensor_size<<" cm"<<endl;
  logfile<<"MIC_LENGTH:    "<<cable_length<<" cm"<<endl;
  logfile<<"ESTIMATED CAP: "<<(enc_ana-350)/slope_enc_cap<<" pF"<<endl;  
  logfile<<"No._DB_METAL_CHANNELS: "<<db_metal_ch<<endl;  
  logfile<<"No._FUNCTIONAL_ASICs_N-side: "<< e_max <<endl;
  logfile<<"ACTIVE_ASICs_N-side: ";
  for (int i =0; i<8; i++){
    logfile<<ena_asics_e[i]<<" ";
   }
  logfile<<endl;  
  logfile<<"No._FUNCTIONAL_ASICs_P-side: "<< h_max <<endl;
  logfile<<"ACTIVE_ASICs_P-side: ";
  for (int i =0; i<8; i++){
    logfile<<ena_asics_h[i]<<" ";
   }
  logfile<<endl; 

  // ------------------------------------- reading file with list of measurements files ------------------------------
  Read_file_tests();
  if (file_names.size()!= (h_max+e_max)) {
    f1->Close();
    cout<< "------------------------------------------------------"<<endl;
    cout<< " Please, check the ASICs array and the plot.txt file. "<<endl;
    cout<< " The number of *.root files does not match the number "<<endl;
    cout<< "      of ASICs declared in the ena_ASICs arrays       "<<endl;
    cout<< "------------------------------------------------------"<<endl;
    cout<<endl;
    cout<<"Number of *.root files in the plot.txt file: "<<file_names.size()<<endl;
    cout<<"Number of ASICs alive declared in the ena_ASICs arrays: "<<h_max+e_max<<endl;
    exit(1);
  }
  else{
    int file_counter = 1;
    for (int e = 0; e<8; e++){
      if (ena_asics_e[e]!= 0) {
        filename_data= dir + Get_file_name(file_counter-1).c_str();
        cout<<"Filename_selected: "<<filename_data<<endl;
        Analysis(e,0, fast_flag);
        file_counter++;
      }
      else {
        for (int j = ch_min; j<ch_max; j++){
          enc_e[e*128+j]  =  0.1;
          encf_e[e*128+j]  = 0.1;
          thr_e[e*128+j]  =  0.1;
          thrf_e[e*128+j] =  0.1;
          gain_e[e*128+j] = 0.001;
        }  
      }
    }
    cout<<"file counter: "<<file_counter<<"\t emax: "<<e_max<<endl;
    for (int h = 0; h<8; h++){
      if (ena_asics_h[h]!= 0) {
	      filename_data= dir + Get_file_name(file_counter-1).c_str();
	      cout<<"Filename_selected: "<<filename_data<<endl;
	      Analysis(h,1, fast_flag);
	      file_counter++;
      }
      else {
        for (int j = ch_min; j<ch_max; j++){
          enc_h[h*128+j]  = 0.1;
          encf_h[h*128+j] = 0.1;
          thr_h[h*128+j]  = 0.1;
          thrf_h[h*128+j] = 0.1;
          gain_h[h*128+j] = 0.001;
        }  
      }
    }
    cout<<"file counter: "<<file_counter<<"\t hmax: "<<h_max<<endl;
    
    
    // --------------------------------------------- Analysis and plotting of the histograms -------------------------------------------- 
    ich_e_even = 0;
    ich_e_odd = 0;
    ich_h_even = 0;
    ich_h_odd = 0;
    for (int ch = ch_min; ch< ch_max_module; ch++){
      // -------------------- filling Magnitude vs channels histograms -------------------
      h_enc_elect->SetBinContent(ch+1, enc_e[ch]);
      h_enc_holes->SetBinContent(ch+1, enc_h[ch]);
      h_encfast_elect->SetBinContent(ch+1, encf_e[ch]);
      h_encfast_holes->SetBinContent(ch+1, encf_h[ch]);
      h_thr_elect->SetBinContent(ch+1, thr_e[ch]);
      h_thr_holes->SetBinContent(ch+1, thr_h[ch]);
      h_thrfast_elect->SetBinContent(ch+1, thrf_e[ch]);
      h_thrfast_holes->SetBinContent(ch+1, thrf_h[ch]);
      h_gain_elect->SetBinContent(ch+1, gain_e[ch]);
      h_gain_holes->SetBinContent(ch+1, gain_h[ch]);
      // ------------------------------- filling the distributions ------------------------
      h1_enc_elect->Fill(enc_e[ch]);
      h1_enc_holes->Fill(enc_h[ch]);
      if (ch<db_metal_ch) {
        h1_zz->Fill(enc_h[ch]);
        if (ch%2==0) h1_zz_even->Fill(enc_h[ch]);
        else h1_zz_odd->Fill(enc_h[ch]);
        }
      h1_encfast_elect->Fill(encf_e[ch]);
      h1_encfast_holes->Fill(encf_h[ch]);
      h1_thr_elect->Fill(thr_e[ch]);
      h1_thr_holes->Fill(thr_h[ch]);
      h1_thrfast_elect->Fill(thrf_e[ch]);
      h1_thrfast_holes->Fill(thrf_h[ch]);
      h1_gain_elect->Fill(gain_e[ch]);
      h1_gain_holes->Fill(gain_h[ch]);
      // ------------------------ filling Odd - Even distributions -----------------------
      if (ch%2 == 0){
        h1_enc_elect_even->Fill(enc_e[ch]);
        h1_enc_holes_even->Fill(enc_h[ch]);  
      }
      else{
        h1_enc_elect_odd->Fill(enc_e[ch]);
        h1_enc_holes_odd->Fill(enc_h[ch]);
      }
      // -------------------------------------- Finding broken channels. Calculating averages -------------------------------------------
      // -------------------------------------------------- Calculating averages --------------------------------------------------------
      if (ch%2 == 0){
        if (enc_e[ch]>enc_lim_min && enc_e[ch]<temp_rate*enc_lim_max){
          ave_e_even += enc_e[ch];
          ich_e_even++;
        }
        if (ch >= (db_metal_ch-1) && enc_h[ch]>enc_lim_min && enc_h[ch]<enc_lim_max){
          ave_h_even += enc_h[ch];
          ich_h_even++;
        }
        if (ch < (db_metal_ch-1) && enc_h[ch]>enc_lim_min && enc_h[ch]<enc_lim_max){
          ave_hzz_even += enc_h[ch];
          ich_hzz_even++;
        }
	    }	
      else {
        if (enc_e[ch]>enc_lim_min && enc_e[ch]<temp_rate*enc_lim_max){
          ave_e_odd += enc_e[ch];
          ich_e_odd++;
       }
        if (ch >= (db_metal_ch-1) && enc_h[ch]>enc_lim_min && enc_h[ch]<enc_lim_max){
          ave_h_odd += enc_h[ch];
          ich_h_odd++;
        }
        if (ch < (db_metal_ch-1) && enc_h[ch]>enc_lim_min && enc_h[ch]<enc_lim_max){
          ave_hzz_odd += enc_h[ch];
          ich_hzz_odd++;
        }
      }
    }

    // ----------------------------------------- Averages ----------------------------------------------
    if (ich_e_even == 0) ich_e_even = 1;
    ave_e_even = ave_e_even/ich_e_even;
    if (ich_h_even == 0) ich_h_even = 1;
    ave_h_even = ave_h_even/ich_h_even;
    if (ich_hzz_even == 0) ich_hzz_even = 1;
    ave_hzz_even = ave_hzz_even/ich_hzz_even;
    if (ich_e_odd == 0) ich_e_odd = 1;
    ave_e_odd = ave_e_odd/ich_e_odd;
    if (ich_h_odd == 0) ich_h_odd = 1;
    ave_h_odd = ave_h_odd/ich_h_odd;
    if (ich_hzz_odd == 0) ich_hzz_odd = 1;
    ave_hzz_odd = ave_hzz_odd/ich_hzz_odd;
       
    ich_e_even = 0;
    ich_e_odd = 0;
    ich_h_even = 0;
    ich_h_odd = 0;
    ich_hzz_even = 0;
    ich_hzz_odd = 0;
    // ------------------------- Finding broken channels. Calculating stddev ----------------------------
    for (int ch = ch_min; ch<ch_max_module; ch++){
      if (ch%2 == 0){
        if (enc_e[ch]>enc_lim_min && enc_e[ch]<temp_rate*enc_lim_max){
          std_e_even += (enc_e[ch] - ave_e_even)*(enc_e[ch] - ave_e_even) ;
          ich_e_even++;
        }
          if (ch >= (db_metal_ch-1) && enc_h[ch]>enc_lim_min && enc_h[ch]<enc_lim_max){
            std_h_even += (enc_h[ch] - ave_h_even)*(enc_h[ch] - ave_h_even);
            ich_h_even++;
          }
          if (ch < (db_metal_ch-1) && enc_h[ch]>enc_lim_min && enc_h[ch]<enc_lim_max){
            std_hzz_even += (enc_h[ch] - ave_hzz_even)*(enc_h[ch] - ave_hzz_even);
            ich_hzz_even++;
          }
      }
      else {
        if (enc_e[ch]>enc_lim_min && enc_e[ch]<temp_rate*enc_lim_max){
          std_e_odd += (enc_e[ch] - ave_e_odd)*(enc_e[ch] - ave_e_odd);
          ich_e_odd++;
        }
        if (ch >= (db_metal_ch-1) && enc_h[ch]>enc_lim_min && enc_h[ch]<enc_lim_max){
          std_h_odd += (enc_h[ch]- ave_h_odd)*(enc_h[ch]- ave_h_odd);
          ich_h_odd++;
        }
        if (ch < (db_metal_ch-1) && enc_h[ch]>enc_lim_min && enc_h[ch]<enc_lim_max){
          std_hzz_odd += (enc_h[ch]- ave_hzz_odd)*(enc_h[ch]- ave_hzz_odd);
          ich_hzz_odd++;
        }
      }
    }
    
    // ----------------------------------------- Stddev ----------------------------------------------
    if (ich_e_even <= 1) ich_e_even = 2;
    std_e_even = sqrt(std_e_even/(ich_e_even-1));
    if (ich_h_even <= 1) ich_h_even = 2;
    std_h_even = sqrt(std_h_even/(ich_h_even-1));
    if (ich_hzz_even <= 1) ich_hzz_even = 2;
    std_hzz_even = sqrt(std_hzz_even/(ich_hzz_even-1));
    if (ich_e_odd <= 1) ich_e_odd = 2;
    std_e_odd = sqrt(std_e_odd/(ich_e_odd-1));
    if (ich_h_odd <= 1) ich_h_odd = 2;
    std_h_odd = sqrt(std_h_odd/(ich_h_odd-1));
    if (ich_hzz_odd <= 1) ich_hzz_odd = 2;
    std_hzz_odd = sqrt(std_hzz_odd/(ich_hzz_odd-1));
    
    // ----------------------- Printing the calculated averages and stddev --------------------------- 
    cout <<"Average_e_even: "<<ave_e_even<<" Std_dev: "<<std_e_even<<endl;
    cout <<"Average_e_odd: "<<ave_e_odd<<" Std_dev: "<<std_e_odd<<endl;
    cout <<"Average_h_even: "<<ave_h_even<<" Std_dev: "<<std_h_even<<endl;
    cout <<"Average_h_odd: "<<ave_h_odd<<" Std_dev: "<<std_h_odd<<endl;
    cout <<"Average_hzz_even: "<<ave_hzz_even<<" Std_dev: "<<std_hzz_even<<endl;
    cout <<"Average_hzz_odd: "<<ave_hzz_odd<<" Std_dev: "<<std_hzz_odd<<endl;
    
    // ------------------------------- Updating the lower minimun ------------------------------------- 
    disc_enc_e_even = ave_e_even -  z_alpha*std_e_even;
    disc_enc_e_odd  = ave_e_odd  -  z_alpha*std_e_odd;
    disc_enc_h_even = ave_h_even -  z_alpha*std_h_even;
    disc_enc_h_odd  = ave_h_odd  -  z_alpha*std_h_odd;
    
    // ----------- Calculating channel to channel averages to discriminate broken channels ------------
    for (int i = 0; i<8; i++){
      for (int ch = ch_min; ch<ch_max; ch++){
          ave_sum_h_odd = 0;
          ave_sum_h_even = 0;
          ave_sum_e_odd = 0;
          ave_sum_e_even = 0;
          ch_counter_h = 0;
          ch_counter_e = 0;
          if (ch%2==0){
              // debugging
              //cout<< ch<<"\t"; 
              if (ch<int(sum_ch/2)){
                  for (int j=-ch; j<sum_ch-ch; j+=2){
                      if (enc_h[i*128 + ch + j]>disc_enc_h_even && enc_h[i*128 + ch+ j]<enc_lim_max){
                          ave_sum_h_even+=enc_h[i*128 + ch + j];
                          ch_counter_h ++;}
                      if (enc_e[i*128 + ch + j]>disc_enc_e_even && enc_e[i*128 + ch + j]<temp_rate*enc_lim_max){
                          ave_sum_e_even+=enc_e[i*128 + ch + j];
                          ch_counter_e ++;}}
                          //cout<<ave_sum_h_even<<"\t"<<ch_counter_h<<"\t"<<ave_sum_e_even<<"\t"<<ch_counter_e<<"\t";
              }
              else if(ch>ch_max-int(sum_ch/2)) {
                  for (int j=ch_max-sum_ch-ch; j<ch_max-ch; j+=2){
                      if (enc_h[i*128 + ch + j]>disc_enc_h_even && enc_h[i*128 + ch + j]<enc_lim_max){
                          ave_sum_h_even+=enc_h[i*128 + ch + j];
                          ch_counter_h ++; }
                      if (enc_e[i*128 + ch + j]>disc_enc_e_even && enc_e[i*128 + ch + j]<temp_rate*enc_lim_max){
                          ave_sum_e_even+=enc_e[i*128 + ch + j];
                          ch_counter_e ++;}}
                          //cout<<ave_sum_h_even<<"\t"<<ch_counter_h<<"\t"<<ave_sum_e_even<<"\t"<<ch_counter_e<<"\t";
              }
              else{
                  for (int j = -1*int(sum_ch/2); j<int(sum_ch/2); j+=2){
                      if (enc_h[i*128 + ch + j - 1]>disc_enc_h_even && enc_h[i*128 + ch + j - 1]<enc_lim_max){
                          ave_sum_h_even+=enc_h[i*128 + ch +j - 1];
                          ch_counter_h ++; }
                      if (enc_e[i*128 + ch + j - 1]>disc_enc_e_even && enc_e[i*128 + ch + j - 1]<temp_rate*enc_lim_max){
                          ave_sum_e_even+=enc_e[i*128 + ch +j - 1];
                          ch_counter_e ++; }}
                          //cout<<ave_sum_h_even<<"\t"<<ch_counter_h<<"\t"<<ave_sum_e_even<<"\t"<<ch_counter_e<<"\t";
              }        
              if (ch_counter_h==0) {
                  // In case there is a cluster of broken channels around it, the channel average will be taken from the entire module average
                  ch_counter_h = 1;
                  ave_sum_h[i*128 + ch] = ave_h_even/ch_counter_h;
              }
              else{
                  ave_sum_h[i*128 + ch] = ave_sum_h_even/ch_counter_h;
              }
              //cout<<ave_sum_h[i*128 + ch]<<"\t";
              if (ch_counter_e==0) {
                  // In case there is a cluster of broken channels around it, the channel average will be taken from the entire module average
                  ch_counter_e = 1;
                  ave_sum_e[i*128 + ch] = ave_e_even/ch_counter_e;
              }
              else{
                  ave_sum_e[i*128 + ch] = ave_sum_e_even/ch_counter_e;
              }
              //cout<<ave_sum_e[i*128 + ch]<<endl;
              h_brk_ave_elect->SetBinContent(i*128 + ch+1, ave_sum_e[i*128 + ch]);
              h_brk_ave_holes->SetBinContent(i*128 + ch+1, ave_sum_h[i*128 + ch]);
          }
          else{
              if (ch<int(sum_ch/2)){
                  for (int j=-ch; j<sum_ch-ch; j+=2){
                      if (enc_h[i*128 + ch + j + 1]>disc_enc_h_odd && enc_h[i*128 + ch + j + 1]<enc_lim_max){
                            ave_sum_h_odd+=enc_h[i*128 + ch + j + 1];
                              ch_counter_h ++;}
                      if (enc_e[i*128 + ch + j + 1]>disc_enc_e_odd && enc_e[i*128 + ch + j + 1]<temp_rate*enc_lim_max){
                              ave_sum_e_odd+=enc_e[i*128 + ch + j+1];
                              ch_counter_e ++;}}
              }
              else if(ch>ch_max-int(sum_ch/2)) {
                  for (int j=ch_max-sum_ch-ch; j<ch_max-ch; j+=2){
                      if (enc_h[i*128 + ch + j + 1]>disc_enc_h_odd && enc_h[i*128 + ch + j + 1]<enc_lim_max){
                          ave_sum_h_odd+=enc_h[i*128 + ch + j+1];
                          ch_counter_h ++; }
                      if (enc_e[i*128 + ch + j + 1]>disc_enc_e_odd && enc_e[i*128 + ch + j + 1]<temp_rate*enc_lim_max){
                          ave_sum_e_odd+=enc_e[i*128 + ch + j+1];
                          ch_counter_e ++;}}
              }
              else{
                  for (int j = -1*int(sum_ch/2); j<int(sum_ch/2); j+=2){
                      if (enc_h[i*128 + ch + j + 1]>disc_enc_h_odd && enc_h[i*128 + ch + j + 1]<enc_lim_max){
                          ave_sum_h_odd+=enc_h[i*128 + ch +j + 1];
                          ch_counter_h ++; }
                      if (enc_e[i*128 + ch + j + 1]>disc_enc_e_odd && enc_e[i*128 + ch + j + 1]<temp_rate*enc_lim_max){
                          ave_sum_e_odd+=enc_e[i*128 + ch +j + 1];
                          ch_counter_e ++; }}
              }
              if (ch_counter_h==0) { 
                  // In case there is a cluster of broken channels around it, the channel average will be taken from the entire module average
                  ch_counter_h = 1;
                  ave_sum_h[i*128 + ch] = ave_h_odd/ch_counter_h;
              }
              else{
                  ave_sum_h[i*128 + ch] = ave_sum_h_odd/ch_counter_h;
              }
              if (ch_counter_e==0) {
                  // In case there is a cluster of broken channels around it, the channel average will be taken from the entire module average
                  ch_counter_e = 1;
                  ave_sum_e[i*128 + ch] = ave_e_odd/ch_counter_e;
              }
              else{
                  ave_sum_e[i*128 + ch] = ave_sum_e_odd/ch_counter_e;
              }
              h_brk_ave_elect->SetBinContent(i*128 + ch+1, ave_sum_e[i*128 + ch]);
              h_brk_ave_holes->SetBinContent(i*128 + ch+1, ave_sum_h[i*128 + ch]);
          } 
      }
    }

    //recalculating the ENC average around the edge of the Z strip.
    for (int ch = ch_min; ch<ch_max_module; ch++){
      if (abs(ch - (db_metal_ch - 1)) <= int(sum_ch/2)){
        ch_counter_h = 0;
        ave_sum_h_even = 0;
        ave_sum_h_odd  = 0;
        // even channels
        if (ch%2 == 0){
          if (ch > (db_metal_ch - 2)){
            for (int j = ((db_metal_ch) - ch); j<((db_metal_ch) + int(sum_ch) - ch); j+=2){
              ave_sum_h_even +=enc_h[ch + j];
              ch_counter_h ++;
              cout<< ch + j <<"\t"<<j<<"\t"<< enc_h[ch+j]<<"\t"<<ave_sum_h_even<<"\t"<<ch_counter_h<<endl;
            }
          }
          else{
            for (int j = ((db_metal_ch - 2) - int(sum_ch) - ch); j<((db_metal_ch) - ch); j+=2){
              ave_sum_h_even +=enc_h[ch + j];
              ch_counter_h ++;
              cout<< ch + j <<"\t"<<j<<"\t"<< enc_h[ch+j]<<"\t"<<ave_sum_h_even<<"\t"<<ch_counter_h<<endl;
            }
          }
          if (ch_counter_h==0) { 
            // In case there is a cluster of broken channels around it, the channel average will be taken from the entire module average
            ch_counter_h = 1;
            ave_sum_h[ch] = ave_sum_h_even/ch_counter_h;
          }
          else{
            ave_sum_h[ch] = ave_sum_h_even/ch_counter_h;
            cout<< ch <<"\t"<<ave_sum_h_even<<"\t"<<ch_counter_h<<"\t"<<ave_sum_h[ch]<<endl;
          }
          // Updating the average value around the db metal channel
          h_brk_ave_holes->SetBinContent(ch+1, ave_sum_h[ch]);
        }
        // odd channels
        else{
          if (ch > (db_metal_ch - 2)){
            for (int j = ((db_metal_ch - 1) - ch); j<((db_metal_ch - 1) + int(sum_ch) - ch); j+=2){
              ave_sum_h_odd +=enc_h[ch+j];
              ch_counter_h ++;
              cout<< ch+j <<"\t"<<j<<"odd\t"<< enc_h[ch+j]<<"\t"<<ave_sum_h_odd<<"\t"<<ch_counter_h<<endl;
            }
          }
          else{
            for (int j = ((db_metal_ch - 3) - int(sum_ch) - ch); j<((db_metal_ch - 1) - ch); j+=2){
              ave_sum_h_odd +=enc_h[ch+j];
              ch_counter_h ++;
              cout<< ch+j <<"\t"<<j<<"odd\t"<< enc_h[ch+j]<<"\t"<<ave_sum_h_odd<<"\t"<<ch_counter_h<<endl;
            }
          }
          if (ch_counter_h==0) { 
            // In case there is a cluster of broken channels around it, the channel average will be taken from the entire module average
            ch_counter_h = 1;
            ave_sum_h[ch] = ave_sum_h_odd/ch_counter_h;
          }
          else{
            ave_sum_h[ch] = ave_sum_h_odd/ch_counter_h;
            cout<< ch <<"\t"<<ave_sum_h_odd<<"\t"<<ch_counter_h<<"\t"<<ave_sum_h[ch]<<endl;
          }
        }
        // Updating the average value around the db metal channel
        h_brk_ave_holes->SetBinContent(ch+1, ave_sum_h[ch]);
      }
    }

   // Setting proper directory for writting the file
   f1->cd();

   // Initial iteration to discriminate broken channels (2*sigma)
   ch_counter_h = 0;
   ch_counter_e = 0;   
   float sum_diff_h = 0;
   float sum_diff_e = 0;
   float sigma_brk_h = 0;
   float sigma_brk_e = 0;
    for (int ch = ch_min; ch<ch_max_module; ch++){
        if (ch%2==0){
            if((ave_e_even - enc_e[ch]) > z_alpha*std_e_even){ brk_semi_elect.push_back(ch); h_brk_semi_elect->SetBinContent(ch+1, enc_e[ch]);}
            if (ch >= (db_metal_ch-1)){
              if((ave_h_even - enc_h[ch]) > z_alpha*std_h_even){ brk_semi_holes.push_back(ch); h_brk_semi_holes->SetBinContent(ch+1, enc_h[ch]);}
            }
            else{
              if((ave_hzz_even - enc_h[ch]) > z_alpha*std_hzz_even){ brk_semi_holes.push_back(ch); h_brk_semi_holes->SetBinContent(ch+1, enc_h[ch]);}
            }
        }
        else {
            if((ave_e_odd - enc_e[ch]) > z_alpha*std_e_odd){ brk_semi_elect.push_back(ch); h_brk_semi_elect->SetBinContent(ch+1, enc_e[ch]);}
            if (ch >= (db_metal_ch-1)){
              if((ave_h_odd - enc_h[ch]) > z_alpha*std_h_odd){ brk_semi_holes.push_back(ch); h_brk_semi_holes->SetBinContent(ch+1, enc_h[ch]);}
            }
            else {
              if((ave_hzz_odd - enc_h[ch]) > z_alpha*std_hzz_odd){ brk_semi_holes.push_back(ch); h_brk_semi_holes->SetBinContent(ch+1, enc_h[ch]);}
            }
        }  
        // calculating the sigma of the enc[ch] - <ave_ch_to_ch [ch]> distribution
        if (enc_h[ch]>enc_lim_min & enc_h[ch]<enc_lim_max){
            sum_diff_h =(ave_sum_h[ch] - enc_h[ch]);
            h1_brk_thr_holes->Fill(sum_diff_h);
            ch_counter_h++;
        }                
        if (enc_e[ch]>enc_lim_min & enc_e[ch]<temp_rate*enc_lim_max){
            sum_diff_e =(ave_sum_e[ch] - enc_e[ch]);
            h1_brk_thr_elect->Fill(sum_diff_e);
            ch_counter_e++;
        }
    }
    
    // --------- Fitting the distributions to find the spread of channels around average values --------------
    h1_brk_thr_holes->Fit("gaus", "swr", "same", -500  , 500);
    sigma_brk_h = h1_brk_thr_holes->GetFunction("gaus")->GetParameter(2);
    h1_brk_thr_elect->Fit("gaus", "swr", "same", -500  , 500);
    sigma_brk_e = h1_brk_thr_elect->GetFunction("gaus")->GetParameter(2);
    cout << "Sigma channels holes: "<< sigma_brk_h<<endl;
    cout << "Sigma channels elect: "<< sigma_brk_e<<endl;
    
    // --------------------- Final iteration to discriminate broken channels (2*sigma) -----------------------
    // -------------------------------- Classifying the type of defects --------------------------------------
    for (int i =0; i <brk_semi_elect.size(); i++){
      if (brk_semi_elect[i]%2 == 0){
          if ((ave_sum_e[brk_semi_elect[i]] - enc_e[brk_semi_elect[i]])>Res_thr*sigma_brk_e) {
            h_brk_elect->SetBinContent(brk_semi_elect[i]+1, enc_e[brk_semi_elect[i]]);
            brk_ch_elect.push_back(brk_semi_elect[i]);
            // filling arrays with broken channels
            if (enc_e[brk_semi_elect[i]] < thr_noresp){
              brk_stats[0][0]++;
              ch_defects_e.push_back(brk_cat[0]);
            } 
            else if (enc_e[brk_semi_elect[i]] < thr_asic) {
              brk_stats[0][1]++;
              ch_defects_e.push_back(brk_cat[1]);
            }
            else {
              brk_stats[0][2]++;
              ch_defects_e.push_back(brk_cat[2]);
              }
            }
      }
      else{
          if ((ave_sum_e[brk_semi_elect[i]] - enc_e[brk_semi_elect[i]])>Res_thr*sigma_brk_e){
            h_brk_elect->SetBinContent(brk_semi_elect[i]+1, enc_e[brk_semi_elect[i]]);
            brk_ch_elect.push_back(brk_semi_elect[i]);
            // filling arrays with broken channels
            if (enc_e[brk_semi_elect[i]] < thr_noresp){
                brk_stats[0][0]++; 
                ch_defects_e.push_back(brk_cat[0]);
                }
            else if (enc_e[brk_semi_elect[i]] < thr_asic){
              brk_stats[0][1]++;
              ch_defects_e.push_back(brk_cat[1]);
              }
            else {
              brk_stats[0][2]++;
              ch_defects_e.push_back(brk_cat[2]);
            }
          } 
      }
    }
      
    for (int i =0; i <brk_semi_holes.size(); i++){
      if (brk_semi_holes[i]%2 == 0){
          if ((ave_sum_h[brk_semi_holes[i]] - enc_h[brk_semi_holes[i]])>Res_thr*sigma_brk_h){
            h_brk_holes->SetBinContent(brk_semi_holes[i]+1, enc_h[brk_semi_holes[i]]);
            brk_ch_holes.push_back(brk_semi_holes[i]);
            if (enc_h[brk_semi_holes[i]] < thr_noresp){ 
              brk_stats[1][0]++;
              ch_defects_h.push_back(brk_cat[0]);
            }
            else if (enc_h[brk_semi_holes[i]] < thr_asic){
                brk_stats[1][1]++;
                ch_defects_h.push_back(brk_cat[1]);
            }
            else {
              brk_stats[1][2]++; 
              ch_defects_h.push_back(brk_cat[2]);
              }
          }  
      }
      else{
          if ((ave_sum_h[brk_semi_holes[i]] - enc_h[brk_semi_holes[i]])>Res_thr*sigma_brk_h){
            h_brk_holes->SetBinContent(brk_semi_holes[i]+1, enc_h[brk_semi_holes[i]]);
            brk_ch_holes.push_back(brk_semi_holes[i]); 
            if (enc_h[brk_semi_holes[i]] < thr_noresp){ 
              brk_stats[1][0]++; 
              ch_defects_h.push_back(brk_cat[0]);
              }
            else if (enc_h[brk_semi_holes[i]] < thr_asic){ 
              brk_stats[1][1]++; 
              ch_defects_h.push_back(brk_cat[1]);
              }
            else {
              brk_stats[1][2]++; 
              ch_defects_h.push_back(brk_cat[2]);
              }
          }
      }
    }
    // ---------------------------- Filling histogram with broken channels statistics ----------------- 
    for (int i = 0; i<3; i++){
        h_brk_stats_elect->Fill(brk_cat[i],brk_stats[0][i]);
        h_brk_stats_holes->Fill(brk_cat[i],brk_stats[1][i]);
    }

    // ------------------------------------------------ Writing histograms in root file -----------------------------------------------
    // ------------------- Channel to channel ---------------------
    h_enc_elect->Write();
    h_enc_holes->Write();
    h_encfast_elect->Write();
    h_encfast_holes->Write();
    h_thr_elect->Write();
    h_thr_holes->Write();
    h_thrfast_elect->Write();
    h_thrfast_holes->Write();
    h_gain_elect->Write();
    h_gain_holes->Write();
      
    // ---------------------- Distributions -----------------------
    h1_enc_elect->Write();
    h1_enc_holes->Write();
    h1_zz->Write();
    h1_zz_even->Write();
    h1_zz_odd->Write();
    h1_thr_elect->Write();
    h1_thr_holes->Write();
    h1_gain_elect->Write();
    h1_gain_holes->Write();
    h1_encfast_elect->Write();
    h1_encfast_holes->Write();
    h1_thrfast_elect->Write();
    h1_thrfast_holes->Write();
    
    // ------------------------ Odd - Even ------------------------
    h1_enc_elect_even->Write();
    h1_enc_holes_even->Write();
    h1_enc_elect_odd->Write();
    h1_enc_holes_odd->Write();

    // ------------------------ Broken channels ------------------------
    //h_brk_semi_elect->Write();
    //h_brk_semi_holes->Write();
    //h_brk_elect->Write();
    //h_brk_holes->Write();
    //h_brk_ave_elect->Write();
    //h_brk_ave_holes->Write();
    //h_brk_stats_elect->Write();
    //h_brk_stats_holes->Write();
    h1_brk_thr_elect->Write();
    h1_brk_thr_holes->Write();

    //h_brk_semi_elect->SetNdivisions(8,"KFALSE");
    //h_brk_semi_holes->SetNdivisions(8,"KFALSE");
    //h_brk_elect->SetNdivisions(8,"KFALSE");
    //h_brk_holes->SetNdivisions(8,"KFALSE");
    //h_brk_ave_elect->SetNdivisions(8,"KFALSE");
    //h_brk_ave_holes->SetNdivisions(8,"KFALSE");
    
    
    // ------------------------------------------------ Drawing histograms on canvas -----------------------------------------------
    TCanvas *c1 = new TCanvas("c1","c1", 1200, 900);
    c1->Close();
    // --------------------- Canvas ENC vs channels ------------------------- 
    gStyle->SetOptStat(0); 
    gStyle->SetOptTitle(0);
    gStyle->SetLegendTextSize(0.035);
    logfile<<"------------------------- ADC ---------------------------"<<endl;
    TCanvas *c_enc_summary = new TCanvas(module_ID + "_ENC",module_ID + "_ENC", 1300, 600);
    TPad *pad1_enc_adc = new TPad("pad1_enc_adc", "ADC_enc",0.0,0.0,0.65,0.98);
    pad1_enc_adc->Draw();
    pad1_enc_adc->cd();
    pad1_enc_adc->SetFillColor(0);
    pad1_enc_adc->SetBorderMode(0);
    pad1_enc_adc->SetBorderSize(2);
    pad1_enc_adc->SetGridx();
    pad1_enc_adc->SetGridy();
    h_enc_elect->Draw("HIST");
    h_enc_elect->SetLineColor(kRed);
    h_enc_elect->GetYaxis()->SetRangeUser(0,3500);
    h_enc_elect->GetXaxis()->SetNdivisions(8,kFALSE);
    h_enc_holes->Draw("HISTsame");
    h_enc_holes->SetLineColor(kBlue);
    // ------------ Drawing the broken channels for visualization -----------
    h_brk_elect->Draw("PHISTsame");
    h_brk_elect->SetMarkerSize(1.5);
    h_brk_elect->SetMarkerStyle(23);
    h_brk_elect->SetMarkerColor(kBlack);
    h_brk_holes->Draw("PHISTsame");
    h_brk_holes->SetMarkerSize(1.5);
    h_brk_holes->SetMarkerStyle(23);
    h_brk_holes->SetMarkerColor(kBlack);

    // --------------------- Lines of ENC parametrization -------------------  
    TF1 *f_enc_ana = new TF1("f_enc_ana","[0]",db_metal_ch-1,1024);
    f_enc_ana->SetParameter(0,enc_ana);
    TF1 *f_enc_ana_zz = new TF1("f_enc_ana_zz","[0]",0,db_metal_ch-1);
    f_enc_ana_zz->SetParameter(0,enc_ana_zz);
    TF1 *f_enc_asic = new TF1("f_enc_asic","[0]",0,1024);
    f_enc_asic->SetParameter(0,enc_asic);
    // ------------- Drawing noise parametrization lines --------------------
    f_enc_ana->Draw("same");
    f_enc_ana_zz->Draw("same");
    f_enc_ana->SetLineWidth(2);
    f_enc_ana->SetLineStyle(7);
    f_enc_ana->SetLineColor(kGreen+2);
    f_enc_ana_zz->SetLineWidth(2);
    f_enc_ana_zz->SetLineStyle(7);
    f_enc_ana_zz->SetLineColor(kGreen+2);  
    // ------------------ Drawing broken channels line on ASIC e ------------
    f_enc_asic->Draw("same");
    f_enc_asic->SetLineWidth(2);
    f_enc_asic->SetLineStyle(7);
    f_enc_asic->SetLineColor(kOrange);
     
    TLegend *lg_enc_summ = new TLegend(0.50,0.65, 0.85, 0.85);
    lg_enc_summ->SetHeader(module_ID+"_ADC_ENC");
    lg_enc_summ->AddEntry(h_enc_elect,"n-side","l");
    lg_enc_summ->AddEntry(h_enc_holes,"p-side","l");
    lg_enc_summ->AddEntry(h_brk_elect,"broken ch","p");
    lg_enc_summ->AddEntry(f_enc_ana,"enc_par_sensor","l");
    lg_enc_summ->AddEntry(f_enc_asic,"enc_asic","l");
    lg_enc_summ->SetBorderSize(0);
    lg_enc_summ->SetFillColor(0);
    lg_enc_summ->Draw("");

    pad1_enc_adc->Modified();
    c_enc_summary->cd();
    c_enc_summary->Update();

    gStyle->SetOptStat(0); 
    gStyle->SetOptTitle(0);
    gStyle->SetOptStat(0);
    TPad *pad2_enc_adc = new TPad("pad2_enc_adc", "ADC ENC distribution",0.65,0.0,0.98,0.98);
    pad2_enc_adc->Draw();
    pad2_enc_adc->cd();
    pad2_enc_adc->SetFillColor(0);
    pad2_enc_adc->SetBorderMode(0);
    pad2_enc_adc->SetBorderSize(2);
    pad2_enc_adc->SetGridx();
    pad2_enc_adc->SetGridy();
    h1_enc_elect->Draw("");
    h1_enc_elect->SetLineColor(kRed);
    h1_enc_elect->SetLineWidth(2);
    h1_enc_elect->SetFillStyle(3003);
    h1_enc_elect->SetFillColor(kRed);
    h1_enc_elect->GetXaxis()->SetRangeUser(0,5000);
    h1_enc_elect->GetXaxis()->SetNdivisions(505,4);
    h1_enc_elect->GetYaxis()->SetRangeUser(0,500);
    h1_enc_elect->SetStats(0);
    h1_enc_holes->Draw("same");
    h1_enc_holes->SetLineColor(kBlue);
    h1_enc_holes->SetFillStyle(3003);
    h1_enc_holes->SetFillColor(kBlue);
    h1_enc_holes->SetLineWidth(2);
    h1_enc_holes->SetStats(0);
  
    TLegend *lg1_enc = new TLegend(0.35,0.7, 0.85, 0.85);
    lg1_enc->SetHeader(module_ID + "_ENC");
    lg1_enc->AddEntry(h1_enc_elect,"n-side","lf");
    lg1_enc->AddEntry(h1_enc_holes,"p-side","lf");
    lg1_enc->SetBorderSize(0);
    lg1_enc->SetFillColor(0);
    lg1_enc->Draw("");

    if (e_max!=0){
      h1_enc_elect->Fit("gaus", "SWR","same", enc_fit_min,10000);
      h1_enc_elect->GetFunction("gaus")->SetLineColor(kRed);
      TLatex *l_enc_mean_e= new TLatex(0.50,0.6,Form("Mean = %.0f e", h1_enc_elect->GetFunction("gaus")->GetParameter(1)));
      l_enc_mean_e->SetNDC();
      //Tl->SetTextAlign(12);
      l_enc_mean_e->SetTextSize(0.04);
      l_enc_mean_e->SetTextColor(kRed);
      l_enc_mean_e->Draw();
      TLatex *l_enc_sigma_e= new TLatex(0.50,0.57,Form("#sigma = %.0f e", h1_enc_elect->GetFunction("gaus")->GetParameter(2)));
      l_enc_sigma_e->SetNDC();
      //Tl->SetTextAlign(12);
      l_enc_sigma_e->SetTextSize(0.04);
      l_enc_sigma_e->SetTextColor(kRed);
      l_enc_sigma_e->Draw();
      // ------------------------- Log file disribution summary------------------------- 
      logfile<<"AVERAGE_ADC_ENC_N-side:\t"<< Form("%.0f +/- %.0f e", h1_enc_elect->GetFunction("gaus")->GetParameter(1), h1_enc_elect->GetFunction("gaus")->GetParameter(2))<<endl;
    }
    else{
      logfile<<"AVERAGE_ADC_ENC_N-side:\t"<< Form("%.0f e", 0.0)<<endl;
    }

    if (h_max!=0){
      //h1_enc_holes->GetXaxis()->SetRange(200, 10000);
      int binmax = h1_enc_holes->GetMaximumBin();
      float x = h1_enc_holes->GetXaxis()->GetBinCenter(binmax);  
      int thr_max = (int)(x+500);
      h1_enc_holes->Fit("gaus", "SWR","same", enc_fit_min, thr_max);
      h1_enc_holes->GetFunction("gaus")->SetLineColor(kBlue);
      TLatex *l_enc_mean_h= new TLatex(0.50,0.53,Form("Mean = %.0f e", h1_enc_holes->GetFunction("gaus")->GetParameter(1)));
      l_enc_mean_h->SetNDC();
      //Tl->SetTextAlign(12);
      l_enc_mean_h->SetTextSize(0.04);
      l_enc_mean_h->SetTextColor(kBlue);
      l_enc_mean_h->Draw();
      TLatex *l_enc_sigma_h= new TLatex(0.50,0.50,Form("#sigma = %.0f e", h1_enc_holes->GetFunction("gaus")->GetParameter(2)));
      l_enc_sigma_h->SetNDC();
      //Tl->SetTextAlign(12);
      l_enc_sigma_h->SetTextSize(0.04);
      l_enc_sigma_h->SetTextColor(kBlue);
      l_enc_sigma_h->Draw();
      // ------------------------- Log file disribution summary------------------------- 
      logfile<<"AVERAGE_ADC_ENC_P-side:\t"<< Form("%.0f +/- %.0f e", h1_enc_holes->GetFunction("gaus")->GetParameter(1), h1_enc_holes->GetFunction("gaus")->GetParameter(2))<<endl;
    }
    else{
      logfile<<"AVERAGE_ADC_ENC_P-side:\t"<< Form("%.0f e", 0.0)<<endl;
    }
    pad2_enc_adc->Modified();
    c_enc_summary->cd();
    c_enc_summary->Update();
    c_enc_summary->Write();
    c_enc_summary->SaveAs(module_ID + "_ENC.png");
    c_enc_summary->Print(filename_pdf+"[");
    c_enc_summary->Print(filename_pdf);
    c_enc_summary->Close();

    //  ------------------------------ ADC ENC -------------------------------
    gStyle->SetOptStat(0); 
    gStyle->SetOptTitle(0);
    TCanvas *c_enc = new TCanvas("c_enc","c_enc", 1700, 900);
    c_enc->cd()->SetGrid();
    h_enc_elect->Draw("HIST");
    h_enc_elect->SetLineColor(kRed);
    h_enc_elect->GetYaxis()->SetRangeUser(0,5000);
    h_enc_elect->GetXaxis()->SetNdivisions(8,kFALSE);
    h_enc_holes->Draw("HISTsame");
    h_enc_holes->SetLineColor(kBlue);
    // ------------ Drawing the broken channels for visualization -----------
    h_brk_elect->Draw("PHISTsame");
    h_brk_elect->SetMarkerSize(1.5);
    h_brk_elect->SetMarkerStyle(23);
    h_brk_elect->SetMarkerColor(kBlack);
    h_brk_holes->Draw("PHISTsame");
    h_brk_holes->SetMarkerSize(1.5);
    h_brk_holes->SetMarkerStyle(23);
    h_brk_holes->SetMarkerColor(kBlack);

    // ------------- Drawing noise parametrization lines --------------------
    f_enc_ana->Draw("same");
    f_enc_ana_zz->Draw("same");
    f_enc_ana->SetLineWidth(2);
    f_enc_ana->SetLineStyle(7);
    f_enc_ana->SetLineColor(kGreen+2);
    f_enc_ana_zz->SetLineWidth(2);
    f_enc_ana_zz->SetLineStyle(7);
    f_enc_ana_zz->SetLineColor(kGreen+2);  
    // ------------------ Drawing broken channels line on ASIC e ------------
    f_enc_asic->Draw("same");
    f_enc_asic->SetLineWidth(2);
    f_enc_asic->SetLineStyle(7);
    f_enc_asic->SetLineColor(kOrange);
     
    TLegend *lg_enc = new TLegend(0.5,0.7, 0.85, 0.85);
    lg_enc->SetHeader(module_ID+"_ENC module");
    lg_enc->AddEntry(h_enc_elect,"n-side","l");
    lg_enc->AddEntry(h_enc_holes,"p-side","l");
    lg_enc->AddEntry(h_brk_elect,"broken ch","p");
    lg_enc->AddEntry(f_enc_ana,"enc_par_sensor","l");
    lg_enc->AddEntry(f_enc_asic,"enc_asic","l");
    lg_enc->SetBorderSize(0);
    lg_enc->SetFillColor(0);
    lg_enc->Draw("");

    c_enc->Write();
    c_enc->Print(filename_pdf);
    c_enc->Close();
    
    // ------------------------- Canvas ZZ-strip Distribution --------------------
    //gStyle->SetOptFit(1111);
    gStyle->SetOptStat(0); 
    gStyle->SetOptTitle(0);
    if ((db_metal_ch - 1)!=0){
      TCanvas *c_zz = new TCanvas(module_ID+"_Z_strips",module_ID+"_Z_strips", 600, 600);
      c_zz->cd()->SetGrid();
      h1_zz->Draw("");
      h1_zz->GetXaxis()->SetRangeUser(500, 7000);
      if (ena_asics_h[0] != 0){
        h1_zz->Fit("gaus", "SWR","same", 500,7000);
        h1_zz->GetFunction("gaus")->SetLineColor(kAzure);
        h1_zz->GetFunction("gaus")->SetLineWidth(3);
        h1_zz->GetYaxis()->SetRangeUser(0, 50);
        h1_zz->GetXaxis()->SetRangeUser(0, 3500);
        TLatex *l_enc_mean_z= new TLatex(0.40,0.65,Form("Mean_Z = %.0f e", h1_zz->GetFunction("gaus")->GetParameter(1)));
        l_enc_mean_z->SetNDC();
        //Tl->SetTextAlign(12);
        l_enc_mean_z->SetTextSize(0.035);
        l_enc_mean_z->SetTextColor(kAzure);
        l_enc_mean_z->Draw();
        TLatex *l_enc_sigma_z= new TLatex(0.40,0.60,Form("#sigma = %.0f e", h1_zz->GetFunction("gaus")->GetParameter(2)));
        l_enc_sigma_z->SetNDC();
        //Tl->SetTextAlign(12);
        l_enc_sigma_z->SetTextSize(0.035);
        l_enc_sigma_z->SetTextColor(kAzure);
        l_enc_sigma_z->Draw();
        // -------------------------- Writting logfile --------------------------------
        logfile<<"AVERAGE_ADC_ENC_Z-strips:\t"<< Form("%.0f +/- %.0f e", h1_zz->GetFunction("gaus")->GetParameter(1), h1_zz->GetFunction("gaus")->GetParameter(2))<<endl;
      }
      else{
        logfile<<"AVERAGE_ADC_ENC_Z-strips:\t"<< Form("%.0f e", 0.0)<<endl;
      }
      h1_zz->SetLineColor(kAzure-3);
      h1_zz->SetFillStyle(3003);
      h1_zz->SetFillColor(kAzure+7);
      h1_zz->SetLineWidth(2);

      TLegend *lg_enc_z = new TLegend(0.4,0.7, 0.7, 0.85);
      lg_enc_z->SetHeader(module_ID + "_ENC_Z_strips ");
      lg_enc_z->AddEntry(h1_zz,"ENC_Z_strips","lf");
      lg_enc_z->SetBorderSize(0);
      lg_enc_z->SetFillColor(0);
      lg_enc_z->Draw("");

      c_zz->Write();
      c_zz->SaveAs(module_ID+"_Z_strips.png");
      c_zz->Print(filename_pdf);
      c_zz->Close();
    }
    
      
    // --------------------- Canvas THR ADC vs channels ---------------------- 
    gStyle->SetOptStat(0); 
    gStyle->SetOptTitle(0);
    gStyle->SetOptStat(0);
    TCanvas *c_thr = new TCanvas(module_ID +"_ADC_thr",module_ID +"_ADC_thr", 1300, 600);
    TPad *pad1_adc_thr = new TPad("pad1_adc_thr", "ADC Thr",0.0,0.0,0.65,0.98);
    pad1_adc_thr->Draw();
    pad1_adc_thr->cd();
    pad1_adc_thr->SetFillColor(0);
    pad1_adc_thr->SetBorderMode(0);
    pad1_adc_thr->SetBorderSize(2);
    pad1_adc_thr->SetGridx();
    pad1_adc_thr->SetGridy();
    h_thr_elect->Draw("PHIST");
    h_thr_elect->SetLineColor(kRed);
    h_thr_elect->SetMarkerColor(kRed);
    h_thr_elect->SetMarkerStyle(21);
    h_thr_elect->SetMarkerSize(0.8);
    h_thr_elect->GetYaxis()->SetRangeUser(7000,13000);
    h_thr_elect->GetXaxis()->SetNdivisions(8,kFALSE);
    //h_thr_elect->GetYaxis()->SetLabelOffset(0.9);
    h_thr_holes->Draw("PHISTsame");
    h_thr_holes->SetLineColor(kBlue);  
    h_thr_holes->SetMarkerColor(kBlue);  
    h_thr_holes->SetMarkerStyle(21);
    h_thr_holes->SetMarkerSize(0.8);
    
    TLegend *lg_thr = new TLegend(0.5,0.7, 0.85, 0.85);
    lg_thr->SetHeader(module_ID +"_ADC_thr");
    lg_thr->AddEntry(h_thr_elect,"n-side","p");
    lg_thr->AddEntry(h_thr_holes,"p-side","p");
    lg_thr->SetBorderSize(0);
    lg_thr->SetFillColor(0);
    lg_thr->Draw("");

    pad1_adc_thr->Modified();
    c_thr->Update();
    c_thr->cd();

    gStyle->SetOptStat(0); 
    gStyle->SetOptTitle(0);
    gStyle->SetOptStat(0);
    TPad *pad2_adc_thr = new TPad("pad2_adc_thr", "ADC Thr distribution",0.65,0.0,0.98,0.98);
    pad2_adc_thr->Draw();
    pad2_adc_thr->cd();
    pad2_adc_thr->SetFillColor(0);
    pad2_adc_thr->SetBorderMode(0);
    pad2_adc_thr->SetBorderSize(2);
    pad2_adc_thr->SetGridx();
    pad2_adc_thr->SetGridy();

    // Adding the distributions
    h1_thr_elect->Draw("");
    h1_thr_elect->SetLineColor(kRed);
    h1_thr_elect->SetLineWidth(2);
    h1_thr_elect->SetFillStyle(3003);
    h1_thr_elect->SetFillColor(kRed);
    h1_thr_elect->GetYaxis()->SetRangeUser(0,500);
    h1_thr_elect->GetXaxis()->SetNdivisions(505,4);
    h1_thr_elect->SetStats(0);
    h1_thr_holes->Draw("same");
    h1_thr_holes->SetLineColor(kBlue);
    h1_thr_holes->SetFillStyle(3003);
    h1_thr_holes->SetFillColor(kBlue);
    h1_thr_holes->SetLineWidth(2);
    h1_thr_holes->SetStats(0);
    
    TLegend *lg1_adc_thr = new TLegend(0.35,0.7, 0.85, 0.85);
    lg1_adc_thr->SetHeader(module_ID +"_ADC_thr");
    lg1_adc_thr->AddEntry(h1_thr_elect,"n-side","lf");
    lg1_adc_thr->AddEntry(h1_thr_holes,"p-side","lf");
    lg1_adc_thr->SetBorderSize(0);
    lg1_adc_thr->SetFillColor(0);
    lg1_adc_thr->Draw("");
  
    if (e_max!=0){
      h1_thr_elect->Fit("gaus", "SWR","same", 4000,18000);
      h1_thr_elect->GetFunction("gaus")->SetLineColor(kRed);
      h1_thr_elect->GetXaxis()->SetRangeUser(8000,13000);
      TLatex *l_thr_mean_e= new TLatex(0.60,0.6,Form("Mean = %.0f e", h1_thr_elect->GetFunction("gaus")->GetParameter(1)));
      l_thr_mean_e->SetNDC();
      //Tl->SetTextAlign(12);
      l_thr_mean_e->SetTextSize(0.04);
      l_thr_mean_e->SetTextColor(kRed);
      l_thr_mean_e->Draw();
      TLatex *l_thr_sigma_e= new TLatex(0.60,0.57,Form("#sigma = %.0f e", h1_thr_elect->GetFunction("gaus")->GetParameter(2)));
      l_thr_sigma_e->SetNDC();
      //Tl->SetTextAlign(12);
      l_thr_sigma_e->SetTextSize(0.04);
      l_thr_sigma_e->SetTextColor(kRed);
      l_thr_sigma_e->Draw();
      // ------------------------- Log file disribution summary------------------------- 
      logfile<<"AVERAGE_ADC_THR_N-side:\t"<< Form("%.0f +/- %.0f e", h1_thr_elect->GetFunction("gaus")->GetParameter(1),  h1_thr_elect->GetFunction("gaus")->GetParameter(2))<<endl;
    }
    else{
      logfile<<"AVERAGE_ADC_THR_N-side:\t"<< Form("%.0f e", 0.0)<<endl;
    }

    if (h_max!=0){
      h1_thr_holes->Fit("gaus", "SWR","same", 4000,18000);
      h1_thr_holes->GetFunction("gaus")->SetLineColor(kBlue);
      TLatex *l_thr_mean_h= new TLatex(0.60,0.53,Form("Mean = %.0f e", h1_thr_holes->GetFunction("gaus")->GetParameter(1)));
      l_thr_mean_h->SetNDC();
      //Tl->SetTextAlign(12);
      l_thr_mean_h->SetTextSize(0.04);
      l_thr_mean_h->SetTextColor(kBlue);
      l_thr_mean_h->Draw();
      TLatex *l_thr_sigma_h= new TLatex(0.60,0.50,Form("#sigma = %.0f e", h1_thr_holes->GetFunction("gaus")->GetParameter(2)));
      l_thr_sigma_h->SetNDC();
      //Tl->SetTextAlign(12);
      l_thr_sigma_h->SetTextSize(0.04);
      l_thr_sigma_h->SetTextColor(kBlue);
      l_thr_sigma_h->Draw();
      // ------------------------- Log file disribution summary------------------------- 
      logfile<<"AVERAGE_ADC_THR_P-side:\t"<< Form("%.f +/- %.0f e", h1_thr_holes->GetFunction("gaus")->GetParameter(1), h1_thr_holes->GetFunction("gaus")->GetParameter(2))<<endl;
    }
    else{
      logfile<<"AVERAGE_ADC_THR_P-side:\t"<< Form("%.f e",  0.0)<<endl;
    }
    pad2_adc_thr->Modified();
    c_thr->Update();
    c_thr->cd();
    c_thr->Write();
    c_thr->SaveAs(module_ID +"_ADC_thr.png");
    c_thr->Print(filename_pdf);
    c_thr->Close(); 
    
    // -------------------- Canvas GAIN ADC vs channels --------------------
    gStyle->SetOptStat(0); 
    gStyle->SetOptTitle(0);
    gStyle->SetOptStat(0);
    TCanvas *c_gain = new TCanvas(module_ID + "_ADC_gain",module_ID + "_ADC_gain", 1300, 600);
    TPad *pad1 = new TPad("pad1", "ADC gain",0.0,0.0,0.65,0.98);
    pad1->Draw();
    pad1->cd();
    pad1->SetFillColor(0);
    pad1->SetBorderMode(0);
    pad1->SetBorderSize(2);
    pad1->SetGridx();
    pad1->SetGridy();

    h_gain_elect->Draw("PHIST");
    h_gain_elect->SetMarkerColor(kRed);
    h_gain_elect->SetMarkerStyle(21);
    h_gain_elect->SetMarkerSize(0.8);
    h_gain_elect->GetYaxis()->SetRangeUser(2100,2800);
    h_gain_elect->GetXaxis()->SetNdivisions(8,kFALSE);
    //h_gain_elect->GetYaxis()->SetLabelOffset(0.2);
    h_gain_holes->Draw("PHISTsame");
    h_gain_holes->SetMarkerColor(kBlue);  
    h_gain_holes->SetMarkerStyle(21);
    h_gain_holes->SetMarkerSize(0.8);
    
    TLegend *lg_gain = new TLegend(0.5,0.7, 0.85, 0.85);
    lg_gain->SetHeader(module_ID +"_ADC_gain");
    lg_gain->AddEntry(h_gain_elect,"n-side","p");
    lg_gain->AddEntry(h_gain_holes,"p-side","p");
    lg_gain->SetBorderSize(0);
    lg_gain->SetFillColor(0);
    lg_gain->Draw("");

    pad1->Modified();
    c_gain->Update();
    c_gain->cd();

    gStyle->SetOptTitle(0);
    gStyle->SetOptStat(0);
    gStyle->SetOptFit(1111);
    //gStyle->SetLegendTextSize(0.04);
    TPad *pad2 = new TPad("pad2", "ADC gain distribution",0.65,0.0,0.98,0.98);
    pad2->Draw();
    pad2->cd();
    pad2->SetFillColor(0);
    pad2->SetBorderMode(0);
    pad2->SetBorderSize(2);
    pad2->SetGridx();
    pad2->SetGridy();

    // Adding here the distributions
    h1_gain_elect->Draw("");
    h1_gain_elect->SetLineColor(kRed);
    h1_gain_elect->SetLineWidth(2);
    h1_gain_elect->SetFillStyle(3003);
    h1_gain_elect->SetFillColor(kRed);
    h1_gain_elect->GetXaxis()->SetRangeUser(2200,2800);
    h1_gain_elect->GetXaxis()->SetNdivisions(510);
    h1_gain_elect->GetYaxis()->SetRangeUser(0,500);
    h1_gain_elect->SetStats(0);
    h1_gain_holes->Draw("same");
    h1_gain_holes->SetLineColor(kBlue);
    h1_gain_holes->SetFillStyle(3003);
    h1_gain_holes->SetFillColor(kBlue);
    h1_gain_holes->SetLineWidth(2);
    h1_gain_holes->SetStats(0);
    
    TLegend *lg1_adc_gain = new TLegend(0.35,0.7, 0.85, 0.85);
    lg1_adc_gain->SetHeader(module_ID +"_ADC_gain");
    lg1_adc_gain->AddEntry(h1_gain_elect,"n-side","lf");
    lg1_adc_gain->AddEntry(h1_gain_holes,"p-side","lf");
    lg1_adc_gain->SetBorderSize(0);
    lg1_adc_gain->SetFillColor(0);
    lg1_adc_gain->Draw("");
    
    if (e_max!=0){
      h1_gain_elect->Fit("gaus", "SWR","same", 2000,2800);
      h1_gain_elect->GetFunction("gaus")->SetLineColor(kRed);
      TLatex *l_gain_mean_e= new TLatex(0.55,0.6,Form("Mean = %.0f e/LSB", h1_gain_elect->GetFunction("gaus")->GetParameter(1)));
      l_gain_mean_e->SetNDC();
      //Tl->SetTextAlign(12);
      l_gain_mean_e->SetTextSize(0.04);
      l_gain_mean_e->SetTextColor(kRed);
      l_gain_mean_e->Draw();
      TLatex *l_gain_sigma_e= new TLatex(0.55,0.57,Form("#sigma = %.0f e/LSB", h1_gain_elect->GetFunction("gaus")->GetParameter(2)));
      l_gain_sigma_e->SetNDC();
      //Tl->SetTextAlign(12);
      l_gain_sigma_e->SetTextSize(0.04);
      l_gain_sigma_e->SetTextColor(kRed);
      l_gain_sigma_e->Draw();
      // ------------------------- Log file disribution summary------------------------- 
      logfile<<"AVERAGE_ADC_GAIN_N-side:\t"<< Form("%.0f +/- %.0f e/LSB", h1_gain_elect->GetFunction("gaus")->GetParameter(1), h1_gain_elect->GetFunction("gaus")->GetParameter(2))<<endl;
    }
    else{
        logfile<<"AVERAGE_ADC_GAIN_N-side:\t"<< Form("%.0f e/LSB", 0.0)<<endl;
    }
        
    if (h_max!=0){
        h1_gain_holes->Fit("gaus", "SWR","same", 2000,2800);
        h1_gain_holes->GetFunction("gaus")->SetLineColor(kBlue);
        TLatex *l_gain_mean_h= new TLatex(0.55,0.53,Form("Mean = %.0f e/LSB", h1_gain_holes->GetFunction("gaus")->GetParameter(1)));
        l_gain_mean_h->SetNDC();
        //Tl->SetTextAlign(12);
        l_gain_mean_h->SetTextSize(0.04);
        l_gain_mean_h->SetTextColor(kBlue);
        l_gain_mean_h->Draw();
        TLatex *l_gain_sigma_h= new TLatex(0.55,0.50,Form("#sigma = %.0f e/LSB", h1_gain_holes->GetFunction("gaus")->GetParameter(2)));
        l_gain_sigma_h->SetNDC();
        //Tl->SetTextAlign(12);
        l_gain_sigma_h->SetTextSize(0.04);
        l_gain_sigma_h->SetTextColor(kBlue);
        l_gain_sigma_h->Draw();
        // ------------------------- Log file disribution summary------------------------- 
        logfile<<"AVERAGE_ADC_GAIN_P-side:\t"<< Form("%.0f +/- %.0f e/LSB", h1_gain_holes->GetFunction("gaus")->GetParameter(1), h1_gain_holes->GetFunction("gaus")->GetParameter(2))<<endl;
    }
    else{
        logfile<<"AVERAGE_ADC_GAIN_P-side:\t"<< Form("%.f e/LSB", 0.0)<<endl;
    }
    pad2->Modified();
    c_gain->cd();
    c_gain->Update();
    c_gain->Write();
    c_gain->SaveAs(module_ID + "_ADC_gain.png");
    c_gain->Print(filename_pdf);
    c_gain->Close(); 

    // ---------------------------------- Plotting and analyzing data from FAST discriminator if fast_flag = TRUE --------------------------------
    if (fast_flag == true){
      // --------------------- Canvas ENC FAST vs channels ----------------------
      gStyle->SetOptStat(0); 
      gStyle->SetOptTitle(0);
      TCanvas *c_enc_fast = new TCanvas(module_ID + "_FAST_ENC",module_ID+ "_FAST_ENC", 1300, 600);
      TPad *pad1_enc_fast = new TPad("pad1_enc_fast", "FAST_enc",0.0,0.0,0.65,0.98);
      pad1_enc_fast->Draw();
      pad1_enc_fast->cd();
      pad1_enc_fast->SetFillColor(0);
      pad1_enc_fast->SetBorderMode(0);
      pad1_enc_fast->SetBorderSize(2);
      pad1_enc_fast->SetGridx();
      pad1_enc_fast->SetGridy();
      h_encfast_elect->Draw("HIST");
      h_encfast_elect->SetLineColor(kRed);
      h_encfast_elect->GetYaxis()->SetRangeUser(0,7000);
      h_encfast_elect->GetXaxis()->SetNdivisions(8,kFALSE);
      h_encfast_holes->Draw("HISTsame");
      h_encfast_holes->SetLineColor(kBlue);

      TLegend *lg_enc_fast = new TLegend(0.5,0.7, 0.85, 0.85);
      lg_enc_fast->SetHeader(module_ID + "_FAST_ENC");
      lg_enc_fast->AddEntry(h_encfast_elect,"n-side","l");
      lg_enc_fast->AddEntry(h_encfast_holes,"p-side","l");
      lg_enc_fast->SetBorderSize(0);
      lg_enc_fast->SetFillColor(0);
      lg_enc_fast->Draw("");
      
      pad1_enc_fast->Modified();
      c_enc_fast->cd();
      c_enc_fast->Update();
      
      // ------------------------- Canvas ENC Distributions --------------------
      TPad *pad2_enc_fast = new TPad("pad2_enc_fast", "FAST_enc distribution",0.65,0.0,0.98,0.98);
      pad2_enc_fast->Draw();
      pad2_enc_fast->cd();
      pad2_enc_fast->SetFillColor(0);
      pad2_enc_fast->SetBorderMode(0);
      pad2_enc_fast->SetBorderSize(2);
      pad2_enc_fast->SetGridx();
      pad2_enc_fast->SetGridy();
      h1_encfast_elect->Draw("");
      h1_encfast_elect->SetLineColor(kRed);
      h1_encfast_elect->SetLineWidth(2);
      h1_encfast_elect->SetFillStyle(3003);
      h1_encfast_elect->SetFillColor(kRed);
      h1_encfast_elect->GetXaxis()->SetRangeUser(0,5000);
      h1_encfast_elect->GetXaxis()->SetNdivisions(505,4);
      h1_encfast_elect->GetYaxis()->SetRangeUser(0,500);
      h1_encfast_elect->SetStats(0);
      h1_encfast_holes->Draw("same");
      h1_encfast_holes->SetLineColor(kBlue);
      h1_encfast_holes->SetFillStyle(3003);
      h1_encfast_holes->SetFillColor(kBlue);
      h1_encfast_holes->SetLineWidth(2);
      h1_encfast_holes->SetStats(0);
    
      TLegend *lg1_enc_fast = new TLegend(0.35,0.7, 0.85, 0.85);
      lg1_enc_fast->SetHeader(module_ID + "_FAST_ENC");
      lg1_enc_fast->AddEntry(h1_encfast_elect,"n-side","lf");
      lg1_enc_fast->AddEntry(h1_encfast_holes,"p-side","lf");
      lg1_enc_fast->SetBorderSize(0);
      lg1_enc_fast->SetFillColor(0);
      lg1_enc_fast->Draw("");
      
      // ------------------------ Fitting the noise distributions ----------------
      h1_encfast_elect->Fit("gaus", "SWR","same", 0,10000);
      h1_encfast_elect->GetFunction("gaus")->SetLineColor(kRed);
      h1_encfast_holes->Fit("gaus", "SWR","same", 0,10000);
      h1_encfast_holes->GetFunction("gaus")->SetLineColor(kBlue);

      TLatex *l_encf_mean_e= new TLatex(0.55,0.60,Form("Mean = %.0f e", h1_encfast_elect->GetFunction("gaus")->GetParameter(1)));
      l_encf_mean_e->SetNDC();
      //Tl->SetTextAlign(12);
      l_encf_mean_e->SetTextSize(0.04);
      l_encf_mean_e->SetTextColor(kRed);
      l_encf_mean_e->Draw();
      TLatex *l_encf_sigma_e= new TLatex(0.55,0.57,Form("#sigma = %.0f e", h1_encfast_elect->GetFunction("gaus")->GetParameter(2)));
      l_encf_sigma_e->SetNDC();
      //Tl->SetTextAlign(12);
      l_encf_sigma_e->SetTextSize(0.04);
      l_encf_sigma_e->SetTextColor(kRed);
      l_encf_sigma_e->Draw();

      TLatex *l_encf_mean_h= new TLatex(0.55,0.53,Form("Mean = %.0f e", h1_encfast_holes->GetFunction("gaus")->GetParameter(1)));
      l_encf_mean_h->SetNDC();
      //Tl->SetTextAlign(12);
      l_encf_mean_h->SetTextSize(0.04);
      l_encf_mean_h->SetTextColor(kBlue);
      l_encf_mean_h->Draw();
      TLatex *l_encf_sigma_h= new TLatex(0.55,0.50,Form("#sigma = %.0f e", h1_encfast_holes->GetFunction("gaus")->GetParameter(2)));
      l_encf_sigma_h->SetNDC();
      //Tl->SetTextAlign(12);
      l_encf_sigma_h->SetTextSize(0.04);
      l_encf_sigma_h->SetTextColor(kBlue);
      l_encf_sigma_h->Draw();

      // ------------------------ Log file disribution summary ------------------- 
      logfile<<"------------------------ FAST ---------------------------"<<endl;
      logfile<<"AVERAGE_FAST_ENC_N-side:\t"<< Form("%.f +/- %.f e", h1_encfast_elect->GetFunction("gaus")->GetParameter(1), h1_encfast_elect->GetFunction("gaus")->GetParameter(2) )<<endl;
      logfile<<"AVERAGE_FAST_ENC_P-side:\t"<< Form("%.f +/- %.f e", h1_encfast_holes->GetFunction("gaus")->GetParameter(1), h1_encfast_holes->GetFunction("gaus")->GetParameter(2) )<<endl;
      
      pad2_enc_fast->Modified();
      c_enc_fast->cd();
      c_enc_fast->Update();
      c_enc_fast->Write();
      c_enc_fast->SaveAs(module_ID + "_FAST_ENC.png");
      c_enc_fast->Print(filename_pdf);
      c_enc_fast->Close();

      // --------------------- Canvas THR FAST vs channels ----------------------
      gStyle->SetOptStat(0); 
      gStyle->SetOptTitle(0);
      TCanvas *c_thr_fast = new TCanvas(module_ID + "_FAST_thr",module_ID + "_FAST_thr", 1300, 600);
      TPad *pad1_thr_fast = new TPad("pad1_thr_fast", "FAST_thr",0.0,0.0,0.65,0.98);
      pad1_thr_fast->Draw();
      pad1_thr_fast->cd();
      pad1_thr_fast->SetFillColor(0);
      pad1_thr_fast->SetBorderMode(0);
      pad1_thr_fast->SetBorderSize(2);
      pad1_thr_fast->SetGridx();
      pad1_thr_fast->SetGridy();
      h_thrfast_elect->Draw("PHIST");
      h_thrfast_elect->SetLineColor(kRed);
      h_thrfast_elect->SetMarkerColor(kRed);
      h_thrfast_elect->SetMarkerStyle(21);
      h_thrfast_elect->SetMarkerSize(0.8);
      h_thrfast_elect->GetYaxis()->SetRangeUser(4000,18000);
      h_thrfast_elect->GetXaxis()->SetNdivisions(8,kFALSE);
      h_thrfast_holes->Draw("PHISTsame");
      h_thrfast_holes->SetLineColor(kBlue);  
      h_thrfast_holes->SetMarkerColor(kBlue);  
      h_thrfast_holes->SetMarkerStyle(21);
      h_thrfast_holes->SetMarkerSize(0.8);
      
      TLegend *lg_thr_fast = new TLegend(0.4,0.7, 0.85, 0.85);
      lg_thr_fast->SetHeader(module_ID + "_FAST_thr");
      lg_thr_fast->AddEntry(h_thrfast_elect,"n-side","p");
      lg_thr_fast->AddEntry(h_thrfast_holes,"p-side","p");
      lg_thr_fast->SetBorderSize(0);
      lg_thr_fast->SetFillColor(0);
      lg_thr_fast->Draw("");
  
      pad1_thr_fast->Modified();
      c_thr_fast->Update();
      c_thr_fast->cd();

      // --------------------- Canvas THR FAST Distributions --------------------
      gStyle->SetOptStat(0); 
      gStyle->SetOptTitle(0);
      TPad *pad2_thr_fast = new TPad("pad2_thr_fast", "FAST_thr distribution",0.65,0.0,0.98,0.98);
      pad2_thr_fast->Draw();
      pad2_thr_fast->cd();
      pad2_thr_fast->SetFillColor(0);
      pad2_thr_fast->SetBorderMode(0);
      pad2_thr_fast->SetBorderSize(2);
      pad2_thr_fast->SetGridx();
      pad2_thr_fast->SetGridy();
      h1_thrfast_elect->Draw("");
      h1_thrfast_elect->SetLineColor(kRed);
      h1_thrfast_elect->SetLineWidth(2);
      h1_thrfast_elect->SetFillStyle(3003);
      h1_thrfast_elect->SetFillColor(kRed);
      h1_thrfast_elect->GetYaxis()->SetRangeUser(0,600);
      h1_thrfast_elect->GetXaxis()->SetRangeUser(4000,18000);
      h1_thrfast_elect->GetXaxis()->SetNdivisions(505,4);
      h1_thrfast_elect->SetStats(0);
      h1_thrfast_holes->Draw("same");
      h1_thrfast_holes->SetLineColor(kBlue);
      h1_thrfast_holes->SetFillStyle(3003);
      h1_thrfast_holes->SetFillColor(kBlue);
      h1_thrfast_holes->SetLineWidth(2);
      h1_thrfast_holes->SetStats(0);

      TLegend *lg1_thr_fast = new TLegend(0.35, 0.7, 0.85, 0.85);
      lg1_thr_fast->SetHeader(module_ID + "_FAST_thr");
      lg1_thr_fast->AddEntry(h1_thrfast_elect,"n-side","lf");
      lg1_thr_fast->AddEntry(h1_thrfast_holes,"p-side","lf");
      lg1_thr_fast->SetBorderSize(0);
      lg1_thr_fast->SetFillColor(0);
      lg1_thr_fast->Draw("");

      // ------------------------ Fitting the THR distributions ----------------
      h1_thrfast_elect->Fit("gaus", "SWR","same", 4000,18000);
      h1_thrfast_elect->GetFunction("gaus")->SetLineColor(kRed);
      TLatex *l_thr_mean_e_fast= new TLatex(0.60,0.6,Form("Mean = %.0f e", h1_thrfast_elect->GetFunction("gaus")->GetParameter(1)));
      l_thr_mean_e_fast->SetNDC();
      //Tl->SetTextAlign(12);
      l_thr_mean_e_fast->SetTextSize(0.04);
      l_thr_mean_e_fast->SetTextColor(kRed);
      l_thr_mean_e_fast->Draw();
      TLatex *l_thr_sigma_e_fast= new TLatex(0.60,0.57,Form("#sigma = %.0f e", h1_thrfast_elect->GetFunction("gaus")->GetParameter(2)));
      l_thr_sigma_e_fast->SetNDC();
      //Tl->SetTextAlign(12);
      l_thr_sigma_e_fast->SetTextSize(0.04);
      l_thr_sigma_e_fast->SetTextColor(kRed);
      l_thr_sigma_e_fast->Draw();
      h1_thrfast_holes->Fit("gaus", "SWR","same", 4000,18000);
      h1_thrfast_holes->GetFunction("gaus")->SetLineColor(kBlue);
      TLatex *l_thr_mean_h_fast= new TLatex(0.60,0.53,Form("Mean = %.0f e", h1_thrfast_holes->GetFunction("gaus")->GetParameter(1)));
      l_thr_mean_h_fast->SetNDC();
      //Tl->SetTextAlign(12);
      l_thr_mean_h_fast->SetTextSize(0.04);
      l_thr_mean_h_fast->SetTextColor(kBlue);
      l_thr_mean_h_fast->Draw();
      TLatex *l_thr_sigma_h_fast= new TLatex(0.60,0.50,Form("#sigma = %.0f e", h1_thrfast_holes->GetFunction("gaus")->GetParameter(2)));
      l_thr_sigma_h_fast->SetNDC();
      //Tl->SetTextAlign(12);
      l_thr_sigma_h_fast->SetTextSize(0.04);
      l_thr_sigma_h_fast->SetTextColor(kBlue);
      l_thr_sigma_h_fast->Draw();
      // ---------------------- Log file disribution summary-------------------
      logfile<<"AVERAGE_FAST_THR_N-side:\t"<< Form("%.0f +/- %.0f e", h1_thrfast_elect->GetFunction("gaus")->GetParameter(1), h1_thrfast_elect->GetFunction("gaus")->GetParameter(2))<<endl;
      logfile<<"AVERAGE_FAST_THR_P-side:\t"<< Form("%.0f +/- %.0f e", h1_thrfast_holes->GetFunction("gaus")->GetParameter(1), h1_thrfast_holes->GetFunction("gaus")->GetParameter(2))<<endl;
      
      pad2_thr_fast->Modified();
      c_thr_fast->Update();
      c_thr_fast->cd();
      c_thr_fast->Write();
      c_thr_fast->SaveAs(module_ID + "_FAST_thr.png");
      c_thr_fast->Print(filename_pdf);
      c_thr_fast->Close();
    }
    else{
      //ENC_FAST
      logfile<<"------------------------ FAST ---------------------------"<<endl;
      logfile<<"AVERAGE_FAST_ENC_N-side:\t"<< Form("%.d e", -1)<<endl;
      logfile<<"AVERAGE_FAST_ENC_P-side:\t"<< Form("%.d e", -1)<<endl;
      //THR_FAST
      logfile<<"AVERAGE_FAST_THR_N-side:\t"<< Form("%.d e", -1)<<endl;
      logfile<<"AVERAGE_FAST_THR_P-side:\t"<< Form("%.d e", -1)<<endl;
    }
      
    // --------------------- Canvas Odd-Even Distributions --------------------   
    gStyle->SetOptStat(0);
    gStyle->SetOptTitle(0);
    TCanvas *c1_enc_odd_even = new TCanvas("c1_enc_odd_even","c1_enc_odd_even", 1300, 600);
    c1_enc_odd_even->Divide(2,1);
    
    // -------------- n-side Odd - Even --------------------
    c1_enc_odd_even->cd(1)->SetGrid();
    h1_enc_elect->Draw("HIST");
    h1_enc_elect->SetLineColor(kBlack);
    h1_enc_elect->SetLineWidth(2);
    h1_enc_elect->SetFillStyle(3002);
    h1_enc_elect->SetFillColor(kBlack);
    h1_enc_elect->GetXaxis()->SetRangeUser(0,7000);
    h1_enc_elect->GetYaxis()->SetRangeUser(0,450);
    h1_enc_elect_even->Draw("HISTsame");
    h1_enc_elect_even->SetLineColor(kMagenta-4);
    h1_enc_elect_even->SetLineWidth(1);
    h1_enc_elect_even->SetFillStyle(3002);
    h1_enc_elect_even->SetFillColor(kMagenta-7);
    h1_enc_elect_odd->Draw("HISTsame");
    h1_enc_elect_odd->SetLineColor(kGreen+2);
    h1_enc_elect_odd->SetFillStyle(3002);
    h1_enc_elect_odd->SetFillColor(kGreen-6);
    h1_enc_elect_odd->SetLineWidth(1);
  
    TLegend *lg1_enc_elect = new TLegend(0.35,0.65, 0.85, 0.85);
    lg1_enc_elect->SetHeader("ENC odd-even " + module_ID + " n-side");
    lg1_enc_elect->AddEntry(h1_enc_elect,"All channels","lf");
    lg1_enc_elect->AddEntry(h1_enc_elect_even,"Even channels","lf");
    lg1_enc_elect->AddEntry(h1_enc_elect_odd,"Odd channels","lf");
    lg1_enc_elect->SetBorderSize(0);
    lg1_enc_elect->SetFillColor(0);
    lg1_enc_elect->Draw("");
    
    // -------------- n-side Odd - Even --------------------
    c1_enc_odd_even->cd(2)->SetGrid();
    h1_enc_holes->Draw("HIST");
    h1_enc_holes->SetLineColor(kBlack);
    h1_enc_holes->SetLineWidth(2);
    h1_enc_holes->SetFillStyle(3002);
    h1_enc_holes->SetFillColor(kBlack);
    h1_enc_holes->GetXaxis()->SetRangeUser(0,7000);
    h1_enc_holes->GetYaxis()->SetRangeUser(0,450);
    h1_enc_holes_even->Draw("HISTsame");
    h1_enc_holes_even->SetLineColor(kMagenta-4);
    h1_enc_holes_even->SetLineWidth(1);
    h1_enc_holes_even->SetFillStyle(3002);
    h1_enc_holes_even->SetFillColor(kMagenta-7);
    h1_enc_holes_odd->Draw("HISTsame");
    h1_enc_holes_odd->SetLineColor(kGreen+2);
    h1_enc_holes_odd->SetFillStyle(3002);
    h1_enc_holes_odd->SetFillColor(kGreen-6);
    h1_enc_holes_odd->SetLineWidth(1);
  
    TLegend *lg1_enc_holes = new TLegend(0.35,0.65, 0.85, 0.85);
    lg1_enc_holes->SetHeader("ENC odd-even " + module_ID + " p-side");
    lg1_enc_holes->AddEntry(h1_enc_holes,"All channels","lf");
    lg1_enc_holes->AddEntry(h1_enc_holes_even,"Even channels","lf");
    lg1_enc_holes->AddEntry(h1_enc_holes_odd,"Odd channels","lf");
    lg1_enc_holes->SetBorderSize(0);
    lg1_enc_holes->SetFillColor(0);
    lg1_enc_holes->Draw("");
    
    c1_enc_odd_even->Write();
    c1_enc_odd_even->Print(filename_pdf);
    c1_enc_odd_even->Close();

    // --------------------- Canvas Broken channels statistics  -------------------- 
    TCanvas *c_brk_sum = new TCanvas("c_brk_sum", "c_brk_sum", 1200,600);
    gStyle->SetOptStat(0);
    gStyle->SetOptTitle(0);
    c_brk_sum->Divide(2,1);
    c_brk_sum->cd(1)->SetGrid();
    h_enc_elect->Draw("HIST");
    h_enc_elect->SetLineColor(kRed);
    h_enc_elect->GetYaxis()->SetRangeUser(0,7000);
    h_enc_elect->GetXaxis()->SetNdivisions(8,kFALSE);
    h_brk_ave_elect->Draw("HISTsame");
    h_brk_ave_elect->SetLineColor(kBlue);
    h_brk_semi_elect->Draw("HISTsame");
    h_brk_semi_elect->SetLineColor(kGreen -2);
    h_brk_elect->Draw("PHISTsame");
    h_brk_elect->SetMarkerSize(1.5);
    h_brk_elect->SetMarkerStyle(23);
    h_brk_elect->SetMarkerColor(kBlack);
    
    TLegend *lg_brk_sum_elect = new TLegend(0.4,0.7, 0.85, 0.85);
    lg_brk_sum_elect->SetHeader("Broken channels n-side");
    lg_brk_sum_elect->AddEntry(h_enc_elect,"ENC n-side","l");
    lg_brk_sum_elect->AddEntry(h_brk_ave_elect,"<ENC> channel to channel","l");
    lg_brk_sum_elect->AddEntry(h_brk_semi_elect,"Brk_channels 2#sigma","l");
    lg_brk_sum_elect->AddEntry(h_brk_elect,"Brk_channels final","l");
    lg_brk_sum_elect->SetBorderSize(0);
    lg_brk_sum_elect->SetFillColor(0);
    lg_brk_sum_elect->Draw("");
    
    c_brk_sum->cd(2)->SetGrid();
    h_enc_holes->Draw("HIST");
    h_enc_holes->SetLineColor(kRed);
    h_enc_holes->GetYaxis()->SetRangeUser(0,7000);
    h_enc_elect->GetXaxis()->SetNdivisions(8,kFALSE);
    h_brk_ave_holes->Draw("HISTsame");
    h_brk_ave_holes->SetLineColor(kBlue);
    h_brk_semi_holes->Draw("HISTsame");
    h_brk_semi_holes->SetLineColor(kGreen -2);
    h_brk_holes->Draw("PHISTsame");
    h_brk_holes->SetMarkerSize(1.5);
    h_brk_holes->SetMarkerStyle(23);
    h_brk_holes->SetMarkerColor(kBlack);
    
    TLegend *lg_brk_sum_holes = new TLegend(0.4,0.7, 0.85, 0.85);
    lg_brk_sum_holes->SetHeader("Broken channels p-side");
    lg_brk_sum_holes->AddEntry(h_enc_holes,"ENC p-side","l");
    lg_brk_sum_holes->AddEntry(h_brk_ave_holes,"<ENC> channel to channel","l");
    lg_brk_sum_holes->AddEntry(h_brk_semi_holes,"Brk_channels 2#sigma","l");
    lg_brk_sum_holes->AddEntry(h_brk_holes,"Brk_channels final","l");
    lg_brk_sum_holes->SetBorderSize(0);
    lg_brk_sum_holes->SetFillColor(0);
    lg_brk_sum_holes->Draw("");
    
    c_brk_sum->Write();
    c_brk_sum->Close();
    
    // -------- Statistics on broken channels ---------------------
    TCanvas *c_brk_stats = new TCanvas(module_ID +"Broken_ch_stats", module_ID +"Broken_ch_stats", 1200,900);
    c_brk_stats->cd()->SetGrid();
    gStyle->SetBarWidth(0.7);
    gStyle->SetErrorX(0.);
    
    h_brk_stats_elect->Draw("bar1 text00 hist");
    h_brk_stats_elect->SetStats(0);
    h_brk_stats_elect->SetFillColor(kRed-3);
    h_brk_stats_elect->SetFillStyle(3001);
    h_brk_stats_elect->LabelsOption("h","X");
    h_brk_stats_elect->SetBarWidth(0.4);
    h_brk_stats_elect->SetBarOffset(0.1);
    h_brk_stats_elect->GetYaxis()->SetRangeUser(0,128);
    
    h_brk_stats_holes->Draw("bar1 text00 histsame");
    h_brk_stats_holes->SetStats(0);
    h_brk_stats_holes->SetFillColor(kBlue-3);
    h_brk_stats_holes->SetFillStyle(3001);
    h_brk_stats_holes->LabelsOption("h","X");
    h_brk_stats_holes->SetBarWidth(0.4);
    h_brk_stats_holes->SetBarOffset(0.5);
    
    TLegend *lg_brk_stats = new TLegend(0.5,0.7, 0.85, 0.85);
    lg_brk_stats->SetHeader("STS modules broken channels");
    lg_brk_stats->AddEntry(h_brk_stats_elect,"n-side","f");
    lg_brk_stats->AddEntry(h_brk_stats_holes,"p-side","f");
    lg_brk_stats->SetBorderSize(0);
    lg_brk_stats->SetFillColor(0);
    lg_brk_stats->Draw("");
  
    c_brk_stats->Write();
    c_brk_stats->SaveAs(module_ID +"_Broken_ch_stats.png");
    c_brk_stats->Print(filename_pdf);
    c_brk_stats->Print(filename_pdf+"]");
    c_brk_stats->Close();
  
    // ------------------------- Log file disribution summary-------------------------// 
    logfile<<"------------------- BROKEN CHANNELS -----------------------"<<endl;
    logfile<<"No._BROKEN_CHANNELS_N-side: "<< brk_ch_elect.size()<<endl;
    logfile<<"LIST_BROKEN_CHANNELS_N-side: ";
    for (int i =0; i <brk_ch_elect.size(); i++){
      logfile<<brk_ch_elect[i]<<" (" <<ch_defects_e[i]<<"), ";
      if (brk_ch_elect[i]%2 == 0) brk_ch_total_even++;
      else brk_ch_total_odd++;
    }
    logfile<<endl;
    logfile<<"No._BROKEN_CHANNELS_P-side: "<< brk_ch_holes.size()<<endl;
    logfile<<"LIST_BROKEN_CHANNELS_P-side: ";
    for (int i =0; i <brk_ch_holes.size(); i++){
      logfile<<brk_ch_holes[i]<<" (" <<ch_defects_h[i]<<"), ";
      if (brk_ch_holes[i]%2 == 0) brk_ch_total_even++;
      else brk_ch_total_odd++;
    }
    logfile<<endl;
    logfile<<"No._BROKEN_CHANNELS_ODD/EVEN: "<< brk_ch_total_odd<<"\t"<<brk_ch_total_even<<endl;
    logfile<<endl;
    logfile<<"NAR:  "<<"Broken channel. No analog response "<<endl;
    logfile<<"ASIC: "<<"Broken bond between ASIC and microcable"<<endl;
    logfile<<"SENS: "<<"Sensor strip is not connected"<<endl;
      
    f1->Close();
  }
    
  logfile.close();  
  return 0; 
}
