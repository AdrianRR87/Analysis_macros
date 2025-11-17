#include "trim_adc.hxx"
#include "TString.h"
#include "string"

TString filename_data;
std::vector<string> file_names;

// ------------------------ This is the analysis function --------------------------

void Analysis(){
    
  trim_adc* sts = new trim_adc(filename_data); 

  // ............... oooo00000oooo........................
  //! Variables for reading, analyzing data and displaying histograms
  int ch_min = 0;
  int ch_max = 128;
  int ch_step =1;
  
  int grp_min = 0;
  int grp_max = 4;
  int grp_step = 1;
  
  int d_min = 0;
  int d_max = 31;
  int d_step = 1;
  
  // List of discriminators present in the data file and to be analyzed
  int d_list[6] = {5,10,16,24,30,31};			// Standard
  //int d_list[5] = {27,28,29,30,31};			// Standard
  int d_size = sizeof(d_list)/sizeof(int);
  int vp_min = 0;
  int vp_max = 255;
  //int vp_max = 75;
  int vp_step = 1;
  
  // this value determines where to cut the double pulses, basically by forcing the data to stop in 200.
  // it has to be checked with the value of pulses in jected in the check trim.
  // if you want to see everything, please, put this value at least twice larger than the number of injected pulses
  int cut_db_pulses = 150;
  int rebin_histo = 2;       			// Consider here to put the following values: 1 (typical bin), 2 or 4. Used to remove spikes from the data
  
  // ............... oooo00000oooo........................
  //! Fitting windows variables window (Like in the previous) This will create the arrange for the cuttings.
  // width determines the range to look for the peak (MEAN +/- width)
  float amp_cal_min = 30.;   
  float amp_cal_max = 247.;
  int width = 30;
  
  // To remove specific S-curves from the data
  int dcut_min_user = 0;       // Min = the lowest thr disc. In the analysis, it corresponds to disc 30 which has the lower thr
  int dcut_max_user = 30;      // Max = the highest thr disc that is the lowest number. In the analysis, it corresponds to disc 0 and it has the higher thr.
  
  // ............... oooo00000oooo........................
  //! Test channels and channels for displaying histograms
  // a test channel for displaying histos and set of channels for printing out noise levels
  int test_ch = 39;
  int grp_sel = 0;
  int ch_comp[10] = {24, 2, 109, 99, 6, 91, 94, 101, 110, 121};
  
  bool read_fast = true;
  // check if there is not root file created
  cout <<"STEP 1: -->> Root file:\t";
  if (!(sts->Check_root_file())) return;
  // initialized the settings described above
  if (!(sts->Init_ch(ch_min,ch_max,ch_step))) return;
  if (!(sts->Init_grp(grp_min,grp_max,grp_step))) return;
  cout<< "STEP 2: -->> List of discriminators: ";
  if (!(sts->Init_d(d_list, d_size))) return;
  if (!(sts->Init_vp(vp_min,vp_max,vp_step))) return;
  // Fitting windows function
  cout<< "STEP 3: -->> Initialized Fitting windows"<<endl;
  sts->Fitting_windows(amp_cal_min,amp_cal_max,width);
  // Create histograms to be called along the programm execution
  cout<< "STEP 4: -->> Creating histograms"<<endl;
  sts->Create_histo(d_list);
  cout<< "STEP 5: -->> Reading data file:\t";
  sts->Reading_file(cut_db_pulses);
  // Analysis function in .cxx 
  cout<< "STEP 6: -->> Launching Analysis"<<endl;
  sts->Analysis(cut_db_pulses, rebin_histo, width, d_list, dcut_min_user, dcut_max_user, test_ch, true, read_fast);
  // Display everything.
  cout<< "STEP 7: -->> Plotting histograms: ADC"<<endl;
  sts->Display_histo_adc(width, d_list,dcut_min_user, dcut_max_user, ch_comp);
  if (read_fast == true) {
    cout<< "STEP 8: -->> Plotting histograms: FAST"<<endl;
    sts->Display_histo_fast();
  }
  else cout<< "STEP 8: -->> FAST disc DATA is not analyzed"<<endl;
  //sts->Delete_Histos();
  sts->Close_root_file(); 
  cout<< "STEP 9: -->> Closing root file"<<endl;
}

// ............... oooo00000oooo........................
//! Reading file that contains test_files names
// This function reads a file that contains the name of the files where the measured values are stored
bool Read_file_tests(){
  
  TString file_list = "a";
  ifstream myfile;
  myfile.open(file_list+ ".txt");
  std::string line;
  std::string delimiter = ".txt";
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
  //cout<<file_names.size()<<endl;
  cout<<" <<-- -------------------------------------------------------------------------- "<<endl;
  cout<<endl; 
  if (file_names.size()!=0) return false;
  return true;
}

// ............... oooo00000oooo........................
//! Get_file_name from the file list
//std::string Get_file_name(int i) {return file_names[i];}
TString Get_file_name(int i) {return file_names[i];}

// ............... oooo00000oooo........................
//! Executing the analysis 
int execution(){
  // reading file with list of measurements files.
  Read_file_tests();
  // Analysis function is called for every file in the list. At the end, everything is closed and root files can be accessed via TBrowser
  for (int i = 0; i<int(file_names.size()); i++){
    filename_data= Get_file_name(i);
    //cout<<"Filename_selected: "<<filename_data<<endl;
    Analysis();
    cout<<" <<-- -------------------------------------------------------------------------- "<<endl;
    cout<<endl;
  }
 return 0;
}
