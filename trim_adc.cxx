#include "trim_adc.hxx"
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
#include <TTree.h>
#include <TBranch.h>
#include <TMath.h>
#include <TLatex.h>
#include "TObject.h"

ClassImp(trim_adc);

//------------------------------------------+-----------------------------------
//! Default constructor.
trim_adc::trim_adc(TString filenameData)
: ch(0),
  ch_min(0),
  ch_max(128),
  d_counter(0),
  ch_step(1),
  d(0),
  d_len(0),
  d_counter_ana(0),
  d_counter_gauss(0),
  d_counter_erfc(0),
  d_min(0),
  d_max(31),
  d_step(0),
  grp(0),
  grp_min(0),
  grp_max(4),
  grp_step(0),
  vp(0),
  ivp(0),
  vp_min(0),
  vp_max(255),
  cut_db_pulses(100),
  rebin_histo(1),
  vp_step(1),   
  v_step_mean(6),
  cnt_val(0),
  mean(0),
  sigma(0),
  sigma_e(0), 
  sum_mean(0),
  sum_delta(0.0001),
  sum_sig(0),
  sum_sige(0), 
  d_cnt(0),
  a(0),
  thr_val1(0),
  amp_cal_max(0),
  amp_cal_min(0),
  filename_data(filenameData),
  read_flag(-1),
  soft_flag(0),
  
  // ------- enc-gauss ------ 
  //f_s1mean(0),
  f_mean(0),
  f_mean_d1(0),
  f_mean_d20(0),
  f_mean_d30(0),
  f_s1sigma(0),
  f_sigma(0),
  f_sigma2(0),
  f_figma_fast(0),
  f_mean_fast(0),
  
  // ------ enc_erfc ---------
  fit_mu(0), 
  fitg_sigma(1), 
  // ----- extras ----
  //f_s1mean_erfc(0),
  f_mean_erfc(0),
  f_mean_d1_erfc(0),
  f_mean_fast_erfc(0),
  f_s1sigma_erfc(0),
  f_sigma_erfc(0),
  f_sigma2_erfc(0),
  
  // -------- adc charact ----------
  inl_aux(0),
  dnl_aux(0),
  
  // ------- brk channels -----------
  ave_even(0),
  ave_odd(0),
  sigma_even(0),
  sigma_odd(0),  
  
  // --------- tree ---------
  adc_enc(0),
  adc_thr(0),
  adc_gain(0),
  thr_min(0),
  thr_max(0),
  ch_sel(0)
{
 for (ch=ch_min;ch<ch_max; ch++) {
   slope[ch] = 0.;
   offset[ch] = 0.;
   inl[ch]=0.;
   dnl[ch]=0.;
    for (d =d_min; d<d_max+1; d++) {
      mean_val[ch][d]=0.;
      for (vp = vp_min;vp<vp_max; vp+=vp_step) {
	      vcnt[ch][d][vp] = cut_db_pulses;
	      vcnt_soft[ch][d][vp] = cut_db_pulses;
      }
    } 
  }
  for (d=d_min;d<d_max;d++){
     vp_set[d]=0;
  }
}

//------------------------------------------+-----------------------------------
//! Destructor.
trim_adc::~trim_adc()
{

}

//------------------------------------------+-----------------------------------
//! Open and check root file
bool trim_adc::Check_root_file()
{
  filename_root = (filename_data + ".root");
  ifstream rootfile;
  rootfile.open(filename_root);
  
  if (!rootfile.good()){
    rootfile.close();
    file1 = new TFile(filename_root,"recreate"); 
    cout<<"Creating root file: "<<filename_root<<endl;
    read_flag = 1;
    return true;
  }
  else {
    file1 = new TFile(filename_root,"update");
    cout<<"Reading root file: "<<filename_root<<endl;
    read_flag = 0;
    return false;
  }
}

//------------------------------------------+-----------------------------------
//! Close root_file
void trim_adc::Close_root_file()
{
  file1->Close();
}

//------------------------------------------+-----------------------------------
//! Create histograms
void trim_adc::Create_histo(int *disc_list){

  // Histograms to show the distribution of the mean values per discriminator for all channels 
  for (int d = 0; d < d_len;d++) {
    TString h1_d_mean_name(Form("h1disc_mean_d_%d", disc_list[d]));  
    h1disc_mean[d]  = new TH1F(h1_d_mean_name,"",500,0,255);
    h1disc_mean[d]->SetTitle(";Disc Thr [amp_cal_units];Entries");
  }
  // ------------------------ Noise histograms -------------------------
  //  --- ENC vs ch ---
  h_enc       = new TH1F("h_enc","h_enc",128, 0,  128);
  h_enc_gaus  = new TH1F("h_enc_gaus","h_enc_gaus",128, 0,  128);
  h_enc_calc  = new TH1F("h_enc_calc","h_enc_calc",128, 0,  128);
  h_enc_fast  = new TH1F("h_enc_fast", "h_enc_fast", 128, 0,  128);
  // --- ENC vs disc ---
  h_enc_disc_gauss  = new TH1F("h_enc_disc_gauss","h_enc_disc_gauss", 31, 0, 31); 
  h_enc_disc_erfc   = new TH1F("h_enc_disc_erfc","h_enc_disc_erfc", 31, 0, 31); 
  h_enc_disc_calc   = new TH1F("h_enc_disc_calc","h_enc_disc_calc", 31, 0, 31); 
  // --- Distributions ---
  h1_enc      = new TH1F("h1_enc","h1_enc",100, 0, 5000);
  h1_enc_even = new TH1F("h1_enc_even","h1_enc_even",100, 0,  5000);
  h1_enc_odd  = new TH1F("h1_enc_odd","h1_enc_odd",100, 0,  5000);
  h1_enc_fast = new TH1F("h1_enc_fast", "h1_enc_fast", 100, 0,  5000);
  // --- 2D distributions ---
  h2D_enc_erfc   = new TH2F("h2D_enc_erfc", "h2D_enc_erfc", 128, 0, 128, 31, 0, 31);
  h2D_enc_gauss  = new TH2F("h2D_enc_gauss", "h2D_enc_gauss", 128, 0, 128, 31, 0, 31);
  h2D_enc_calc   = new TH2F("h2D_enc_calc", "h2D_enc_calc", 128, 0, 128, 31, 0, 31);
  // --- Titles ---
  h_enc->SetTitle("ENC Erfc fitting; Channel number;ENC [e]");
  h_enc_gaus->SetTitle("ENC Gaussian fitting; Channel number;ENC [e]");
  h_enc_calc->SetTitle("ENC arithmetic calc; Channel number;ENC [e]");
  h_enc_fast->SetTitle("ENC FAST discriminator; Channel number;ENC [e]");

  h1_enc->SetTitle("ENC Erfc fitting; ENC [e]; Entries");
  h1_enc_even->SetTitle("ENC even channels Erfc fitting; ENC [e]; Entries");
  h1_enc_odd->SetTitle("ENC odd channels Erfc fitting; ENC [e]; Entries");
  h1_enc_fast->SetTitle("ENC FAST discriminator; ENC [e]; Entries");

  h2D_enc_erfc->SetTitle("ENC Erfc fitting; Channel number; ADC discriminator");
  h2D_enc_gauss->SetTitle("ENC Gauss fitting; Channel number; ADC discriminator");
  h2D_enc_calc->SetTitle("ENC Arith. Calc; Channel number; ADC discriminator");

  // -------------------- Disc threshold histograms --------------------
  // --- Thr vs ch ---
  h_adc_thr   = new TH1F("h_adc_thr","h_adc_thr",128, 0,128);
  h_fast_thr  = new TH1F("h_fast_thr","h_fast_thr",128,0,128);
  h_adc_gain = new TH1F("h_adc_gain","h_adc_gain", 128, 0, 128);
  h_thr_ch_erfc = new TH1F("h_thr_ch_erfc","h_thr_ch_erfc", 31, 0, 31);
  // --- Distributions ---
  h1_adc_thr = new TH1F("h1_adc_thr","h1_adc_thr", 500, 0, 25000);
  h1_fast_thr = new TH1F("h1_fast_thr","h1_fast_thr", 500, 0, 25000);
  h1_adc_gain = new TH1F("h1_adc_gain","h1_adc_gain", 350, 0, 3500);
  h1_adc_offset = new TH1F("h1_adc_offset","h1_adc_offset", 500, 0, 25000);
  // --- Fitting QC ---
  h1_adc_resid = new TH1F("h1_adc_resid","h1_adc_resid",200, -7000, 7000);
  h1_chi_erfc = new TH1F("h1_chi_erfc","h1_chi_erfc", 50, 0, 50);
  h1_chi_ferfc = new TH1F("h1_chi_ferfc","h1_chi_ferfc", 25, 0, 50);
  // --- 2D distributions ---
  h2D_mean_erfc   = new TH2F("h2D_mean_erfc", "h2D_mean_erfc", 128,0,128, 31,0,31);
  h2D_mean_gauss  = new TH2F("h2D_mean_gauss", "h2D_mean_gauss", 128,0,128, 31,0,31);
  h2D_mean_calc   = new TH2F("h2D_mean_calc", "h2D_mean_calc", 128, 0, 128, 31, 0, 31);

  // --- Titles ---
  h_adc_thr->SetTitle("Thr ADC discriminator; Channel number; Thr ADC discriminator [e]");
  h_fast_thr->SetTitle("Thr FAST discriminator; Channel number; Thr FAST discriminator [e]");
  h_adc_gain->SetTitle("ADC Gain; Channel number; ADC Gain [e/LSB]");
  h_thr_ch_erfc->SetTitle("ADC linearity selected channel; Discriminator number; Threshold [amp_cal]");

  h1_adc_thr->SetTitle("Thr ADC discriminator; Thr ADC discriminator [e]; Entries");
  h1_fast_thr->SetTitle("Thr FAST discriminator; Thr FAST discriminator [e]; Entries");
  h1_adc_gain->SetTitle("ADC Gain; ADC Gain [e/LSB]; Entries");
  h1_adc_offset->SetTitle("Thr ADC discriminator; Thr ADC discriminator [e]; Entries");

  h1_adc_resid->SetTitle("ADC Erfc fitting residuals; Residuals [e]; Entries");
  h1_chi_erfc->SetTitle("ADC discriminators Erfc Chi2/Ndf; Erfc Chi2/Ndf; Entries");
  h1_chi_ferfc->SetTitle("FAST discriminator Erfc Chi2/Ndf; Erfc Chi2/Ndf; Entries");

  h2D_mean_erfc->SetTitle("Thr Erfc fitting; Channel number; ADC discriminator");
  h2D_mean_gauss->SetTitle("Thr Erfc fitting; Channel number; ADC discriminator");
  h2D_mean_calc->SetTitle("Thr Erfc fitting; Channel number; ADC discriminator");

  // -------------------- ADC Linearity histograms ---------------------
  // --- DNL, INL vs ch ---
  h_dnl  = new TH1F("h_dnl", "h_dnl", 128,0,128);
  h_inl  = new TH1F("h_inl", "h_inl", 128,0,128);
  h1_dnl = new TH1F("h1_dnl","h1_dnl",50,0,2);
  h1_inl = new TH1F("h1_inl","h1_inl",50,0,2);
  h_disc_dnl = new TH1F("h_disc_dnl","h_disc_dnl",31,0,31);
  h_disc_inl = new TH1F("h_disc_inl","h_disc_inl",31,0,31);

  // 
  //h_mean = std::make_shared<TH1D>("hfit_m","hfit_m",500000,0,128);
  //h_counter  = new TH1F("h_counter","h_counter",128,0,128);
  
  //h_adc_int = new TH1F("h_adc_int","Intercept",50,0,250);
  //h_sigma = new TH1F("h_sigma","h_sigma",128,0,128);
  //h_aux1 = new TH1F("h_aux1","h_aux1",31,0,31);
  //h_chi_gaus = new TH2F("h_chi_gaus","h_chi_gaus",128,0,128,31,0,31);
  //h_aux1_c = new TH1F("h_aux1_c","h_aux1_c",31,0,31);
  //h_aux2 = new TH1F("h_aux2","h_aux2",31,0,31);
  
  
 
  // ---------------------
  // Broken channels histograms
  // hbrk_even = new TH1F("hbrk_even","hbrk_even", 128, 0, 128);
  // hbrk_odd = new TH1F("hbrk_odd","hbrk_odd", 128, 0, 128);
  // hbrk2_even = new TH1F("hbrk2_even","hbrk2_even", 128, 0, 128);
  // hbrk2_odd = new TH1F("hbrk2_odd","hbrk2_odd", 128, 0, 128);
  // hubrk_even = new TH1F("hubrk_even","hubrk_even", 128, 0, 128);
  // hubrk_odd = new TH1F("hubrk_odd","hubrk_odd", 128, 0, 128);
  // h1ubrk_even = new TH1F("h1ubrk_even","h1ubrk_even", 100, 500, 2000);
  // h1ubrk_odd = new TH1F("h1ubrk_odd","h1ubrk_odd", 100, 500, 2000);

}

//------------------------------------------+-----------------------------------
//! Initilaizing ch
bool trim_adc::Init_ch(int chMin, int chMax, int chStep)
{
  ch_min = chMin;
  ch_max = chMax;
  if (chStep !=1) ch_step = chStep;
  if (chMin<0 | chMin>chMax | chMax>128) return false;
  return true;
}

//------------------------------------------+-----------------------------------
//! Initilizing grp
bool trim_adc::Init_grp(int grpMin, int grpMax, int grpStep)
{
  grp_min = grpMin;
  grp_max = grpMax;
  if (grpStep !=1) grp_step = grpStep;
  if (grpMin<0 | grpMin>grpMax | grpMax>4) return false;
  return true;
}

//------------------------------------------+-----------------------------------
//! Initializing disc
bool trim_adc::Init_d(int *disc_list, int d_size){

  d_len = sizeof(disc_list)/sizeof(disc_list[0]);
  d_len = d_size;
  for (int i = 0; i< d_size; i++){
    cout<<disc_list[i]<<"\t";
  }
  cout<<"Size of d_len: "<< d_size<<endl;
  d_min = disc_list[0];
  d_max = disc_list[d_size-1];
  if (d_min<0 | d_min>d_max | d_max>31) return false;
  return true;
}

//------------------------------------------+-----------------------------------
//! Initializing vp
bool trim_adc::Init_vp(int vpMin, int vpMax, int vpStep)
{
  vp_min = vpMin;
  vp_max = vpMax;
  if (vpStep !=1) vp_step = vpStep;
  if (vpMin<0 | vpMin>vpMax | vpMax>255) return false;
  return true;
}

//------------------------------------------+-----------------------------------
//! Initializing fitting windows
bool trim_adc::Fitting_windows(int ampcalMin, int ampcalMax, int width)
{

  thr_val1=width;
  amp_cal_max = ampcalMax;
  amp_cal_min = ampcalMin;							// thr_value to select the range around the fitting peak. For ASIC alone (thr_val = 10, sigma ~2; so 5*sigma ~10); For ASIC + sensor (thr_val ~20,30 or 40 to cover whole S-curve)
  
  float vp_d = (amp_cal_max-amp_cal_min)/(d_max-d_min);
  for (d = d_min; d < d_max; d++){
    vp_set[30 - d] =(amp_cal_min + vp_d*(d - d_min));
  }
  return true;
}

//------------------------------------------+-----------------------------------
//! Reading data file
bool trim_adc::Reading_file(int cut_db_pulses_user) {
  
  cut_db_pulses = cut_db_pulses_user;
  if (read_flag != 1) return false;

  filename_data = (filename_data+".txt"); // Data file name
  scanfile.open(filename_data);		// Opening data file
  cout<<filename_data<<endl;

  int fvp;
  int fch;
  std::string line;
  std::string ele;
  //std::cout<<"------------- Reading data file: -----------"<<std::endl;
  ivp =0;
  std::getline (scanfile,line);
  for (vp = vp_min; vp<vp_max; vp+=vp_step) {
    ch_counter = 0;
    //cout<<"vp:\t"<<vp<<"\t";
    for (ch =ch_min;ch<ch_max; ch++){
      std::getline (scanfile,line);
      std::istringstream iss(line);
      iss>>ele;
      iss>>fvp;
      iss>>ele;
      iss>>ele;
      d_counter = 0;
      //cout<<ch<<"\t";
      for (d=0; d < d_len; d++) {
        iss>>fch;
        vcnt[ch_counter][d_counter][ivp] = fch;
        if (vcnt[ch_counter][d_counter][ivp] > cut_db_pulses) {
          vcnt[ch_counter][d_counter][ivp] = cut_db_pulses;		// Comment this line to see the full s_curves; this is just for cutting double pulses after understand it. the value (i.e. 400) depends on the number of injected pulses. This is defined in the pscan file
        } 
        //printf("%4d ", vcnt[ch_counter][d_counter][ivp]);	  
        d_counter++;
      }
      //cout<<endl;
      ch_counter++;
    }
    ivp++;
  }
  cout<< "Size of List of discriminators: "<< d_len<<endl;
  
  scanfile.close();
  return true;
}

//------------------------------------------+-----------------------------------
//! Analyzing the data
void trim_adc::Analysis(int cut_db_pulses_user, int rebin_histo_user, int width_user, int *disc_list, int dcut_min_user,int dcut_max_user, int ch_sel_user, bool fit, bool fast_fit)
{
  ch_sel = ch_sel_user;
  float v_step_mean = 6;
  cut_db_pulses = cut_db_pulses_user;
  rebin_histo = rebin_histo_user;
  if (rebin_histo==0) rebin_histo = 1;
  int hist_bin = 256/rebin_histo;
  
  Soft_val(false);  // Function used to remove spikes on the data.
  // Creating a subdirectory "hscurve" in this file
  file1->cd();
  TDirectory *cdhscurves = file1->mkdir("histScurves");

  cout<< "STEP 6.1: Filling and fitting channel histograms "<<endl;
  ch_counter = 0;
  for (ch=ch_min;ch<ch_max;ch+=ch_step){
    d_counter_gauss = 0;
    d_counter_ana = 0;
    d_counter_erfc = 0;
    d_counter = 0;
    f_s1sigma = 0.;
    f_s1sigma_erfc = 0.;
    TString histodisc(Form("h_disc_%d",ch)); 
    TString histodisc_s(Form("h_disc_s_%d",ch)); 
    hdisc_sig[ch_counter] = new TH1F(histodisc_s,"",31, 0,31);
    hdisc_mean[ch_counter]= new TH1F(histodisc,"",31,0,31);
  
    for (d = 0; d < d_len; d++) {
      TString histoname(Form("h_d_%d_%d",ch,disc_list[d_counter])); 
      TString histoscurve(Form("h_scurve_%d_%d",ch,disc_list[d_counter])); 
      hdcnt[ch_counter][d_counter]=new TH1F(histoname,"",hist_bin,0,256);
      hscurve[ch_counter][d_counter]=new TH1F(histoscurve,"",hist_bin,0,256);
      ivp = 0;
      a = 0;
      sum_mean = 0;
      sum_delta = 0.000001;
      for ( vp = vp_min ; vp <vp_max; vp += vp_step ) {
        if (vp < vp_max -1) d_cnt = (vcnt_soft[ch_counter][d_counter][ivp+1]-vcnt_soft[ch_counter][d_counter][ivp]);
        if (d_cnt < 0) d_cnt =0;
        hdcnt[ch_counter][d_counter]->Fill(vp,d_cnt);
        hscurve[ch_counter][d_counter]->Fill(vp,vcnt_soft[ch_counter][d_counter][ivp]);
        ivp++;
      }
      
      if (fit == true && disc_list[d_counter]<=30) {
        Fit_values(width_user, disc_list, dcut_min_user, dcut_max_user); 
        Fit_values_erfc(width_user,disc_list, dcut_min_user, dcut_max_user, cut_db_pulses);
      }
      if (fast_fit == true && disc_list[d_counter]==31) {
        Fitting_Fast(width_user, cut_db_pulses);
      }
      
      // Writting all channels S-curves histograms in a folder
      cdhscurves->cd();
      hscurve[ch_counter][d_counter]->Write();
      hdcnt[ch_counter][d_counter]->Write();   
      d_counter++;
    }
   
    file1->cd();
    if (d_counter_gauss !=0){f_s1sigma = f_s1sigma/d_counter_gauss;}
    else {f_s1sigma = f_s1sigma/1;}
    h_enc_gaus->SetBinContent(ch+1,f_s1sigma*350);
    
    if (d_counter_erfc !=0) f_s1sigma_erfc = f_s1sigma_erfc/d_counter_erfc;
    else f_s1sigma_erfc = f_s1sigma_erfc/1;
    
    h_enc->SetBinContent(ch+1,f_s1sigma_erfc*350);
    h1_enc->Fill(f_s1sigma_erfc*350);
    
    if (ch%2==0) h1_enc_even->Fill(f_s1sigma_erfc*350);
    else h1_enc_odd->Fill(f_s1sigma_erfc*350);
   
    if (d_counter_ana !=0){sum_sige = sum_sige/d_counter_ana;}
    else {sum_sige = sum_sige/1;}
   
    h_enc_calc->Fill(ch,sum_sige*350);
  
    // Evaluating ADC linearity
    inl_aux = 0.;
    dnl_aux = 0.;
    for(d=d_min; d<d_len+1; d++){
      Adc_charact(disc_list, dcut_min_user, dcut_max_user, v_step_mean);
    }
   
    h_dnl->Fill(ch,dnl[ch_counter]);
    h1_dnl->Fill(std::abs(dnl[ch_counter]));
   
    h_inl->Fill(ch,inl[ch_counter]);
    h1_inl->Fill(inl[ch_counter]);
    
    ch_counter++;   
  }
  
  cout<< "STEP 6.2: Fitting distribution histograms"<<endl;
  h1_enc->Fit("gaus", "WQ");
  h1_enc_odd->Fit("gaus", "WQ");
  h1_enc_even->Fit("gaus", "WQ");
  h1_enc_fast->Fit("gaus", "WQ");

  cout<< "STEP 6.3: Writing histograms"<<endl;
  h_enc->Write();
  h_fast_thr->Write();
  h_enc_fast->Write();
  h_enc_gaus->Write();

  h1_enc->Write();
  h1_enc_even->Write();
  h1_enc_odd->Write();
  h1_enc_fast->Write();
  h1_chi_erfc->Write();
  
  
  
}

//------------------------------------------+-----------------------------------
//! Finding the number of broken channels     
/*void trim_adc::Broken_ch()
{
    int cnt_even = 0;
    int cnt_odd = 0;
    
    float ave2_even = 0;
    float ave2_odd = 0;
    float sigma2_even = 0;
    float sigma2_odd = 0;
    float sigma_array[2] = {2.0, 3.0};
    
    // ------------ Method 1 --------------------------
    // Average among all channels without spikes and/or edge channels 
    // Finding Average for even and odd channels
    for (ch = ch_min+5; ch<ch_max-5; ch++){
        if (h_enc->GetBinContent(ch+1)>800 & h_enc->GetBinContent(ch+1)<2000){
            if (ch%2==0){
                ave_even += h_enc->GetBinContent(ch+1);
                cnt_even++;
            }
            else{
                ave_odd += h_enc->GetBinContent(ch+1);
                cnt_odd++;
            }
        }
    }
    
    if (cnt_even == 0) cnt_even = 1;
    if (cnt_odd == 0) cnt_odd = 1;
    
    ave_even = ave_even/cnt_even;
    ave_odd = ave_odd/cnt_odd;
    
    // Finding Sigma for even and odd channels without spikes and/or edge channels 
    cnt_even = 0;
    cnt_odd = 0;
    for (ch = ch_min+5; ch<ch_max-5; ch++){
        if (h_enc->GetBinContent(ch+1)>800 & h_enc->GetBinContent(ch+1)<2000){
            if (ch%2==0){
                sigma_even += (ave_even-h_enc->GetBinContent(ch+1))*(ave_even-h_enc->GetBinContent(ch+1));
                cnt_even++;
            }
            else{
                sigma_odd += (ave_odd-h_enc->GetBinContent(ch+1))*(ave_odd-h_enc->GetBinContent(ch+1));
                cnt_odd++;                
            }
        }
    }
    
    if (cnt_even <=1) cnt_even = 2;
    if (cnt_odd  <=1) cnt_odd = 2;
    cout<<"SIGMA EVEN:"<<"\t"<<sigma_even<<"\t"<<cnt_even<<endl;
    sigma_even = sqrt(sigma_even/(cnt_even-1));
    cout<<"SIGMA ODD:"<<"\t"<<sigma_odd<<"\t"<<cnt_odd<<endl;
    sigma_odd = sqrt(sigma_odd/(cnt_odd-1));
    
    int bk_flag = 0;
    // Evaluating channel by channel. First iteration
    for (ch = ch_min; ch<ch_max; ch++){
        bk_flag = 0;
        while (bk_flag == 0){
            if (ch%2==0){
                if ((ave_even - h_enc->GetBinContent(ch+1))>3.0*sigma_even){
                    hbrk_even->SetBinContent(ch+1, 1);
                    bk_flag = 1;
                    }
                else if ((ave_even - h_enc->GetBinContent(ch+1))<3.0*sigma_even) {
                    cout<< ch<<"\t"<<ave_even<<"\t"<<h_enc->GetBinContent(ch+1)<<"\t"<<sigma_even<<endl;
                    hubrk_even->SetBinContent(ch+1,h_enc->GetBinContent(ch+1));
                    h1ubrk_even->Fill(h_enc->GetBinContent(ch+1));
                    bk_flag = 1;
                    }
                else{
                    bk_flag = 1;
                }
                }
            else{
                if ((ave_odd - h_enc->GetBinContent(ch+1))>3.0*sigma_odd){
                    hbrk_odd->SetBinContent(ch+1, 1);
                    bk_flag = 1;
                    }
                else if ((ave_odd - h_enc->GetBinContent(ch+1))<3.0*sigma_odd){
                    cout<< ch<<"\t"<<ave_odd<<"\t"<<h_enc->GetBinContent(ch+1)<<"\t"<<sigma_odd<<endl;
                    hubrk_odd->SetBinContent(ch+1,h_enc->GetBinContent(ch+1));
                    h1ubrk_odd->Fill(h_enc->GetBinContent(ch+1));
                    bk_flag = 1;
                    }
                else {
                    bk_flag = 1;
                }
                }
        }
    }
    
    hbrk_even->Write();
    hbrk_odd->Write();
    hubrk_even->Write();
    hubrk_odd->Write();
    
    //h1ubrk_even->Write();
    //h1ubrk_odd->Write();
    
//     hbrk2_even->Write();
//     hbrk2_odd->Write();
    
} */

//------------------------------------------+-----------------------------------
//! Analyzing the data. Fitting S-curves
bool trim_adc::Soft_val(bool soft_flag)
{
  if (soft_flag==true){
    ch_counter = 0;
    for (ch=ch_min;ch<ch_max;ch+=ch_step){
      //for (d=d_min;d <d_max;d++){
      d_counter = 0;
      for (d = 0;d < d_len; d++){
        ivp = 2;
        int soft_count =0;
        int d_cnt0 = 0;
        int d_cnt1 = 0; 
        vcnt_soft[ch_counter][d_counter][0]= vcnt[ch_counter][d_counter][0];
        vcnt_soft[ch_counter][d_counter][1]= vcnt[ch_counter][d_counter][1];
        vcnt_soft[ch_counter][d_counter][vp_max-vp_min-1]= vcnt[ch_counter][d_counter][vp_max-vp_min-1];
        vcnt_soft[ch_counter][d_counter][vp_max-vp_min-2]= vcnt[ch_counter][d_counter][vp_max-vp_min-2];
	      for ( vp = vp_min + 2*vp_step; vp < vp_max-2*vp_step; vp += vp_step ){
          d_cnt0 = vcnt[ch_counter][d_counter][ivp]-vcnt[ch_counter][d_counter][ivp-1];
          d_cnt1 = vcnt[ch_counter][d_counter][ivp+1]-vcnt[ch_counter][d_counter][ivp];
          if (d_cnt0 < -10 &&  d_cnt1 > 10) {vcnt_soft[ch_counter][d_counter][ivp] = int((vcnt[ch_counter][d_counter][ivp-1]+vcnt[ch_counter][d_counter][ivp+1])/2.); }
          else if (d_cnt0 >10 &&  d_cnt1 <-10) {vcnt_soft[ch_counter][d_counter][ivp] = int((vcnt[ch_counter][d_counter][ivp-1]+vcnt[ch_counter][d_counter][ivp+1])/2.);}
          else vcnt_soft[ch_counter][d_counter][ivp] = vcnt[ch_counter][d_counter][ivp];
	        ivp++;
	      }
	      d_counter++;
      }
      ch_counter++;
    }
  }
  
  if (soft_flag == false){
    ch_counter = 0;
    for (ch=ch_min;ch<ch_max;ch+=ch_step){
      d_counter = 0;
	    for (d=0;d <d_len;d++){
        ivp = 0;
	      for ( vp = vp_min; vp <vp_max; vp += vp_step ){
          vcnt_soft[ch_counter][d_counter][ivp] = vcnt[ch_counter][d_counter][ivp];
	        ivp++;
	      }
	      d_counter++;
      }
      ch_counter++;
    }
  }  
  return true;
}

//------------------------------------------+-----------------------------------
//! Analyzing the data. Fitting the differentiation of the S-curves using a Gaussian_method
bool trim_adc::Fit_values(int width, int *disc_list, int d_cut_min, int d_cut_max) {
  d_cut_min = 30-d_cut_min;
  d_cut_max = 30-d_cut_max;
    
  if (d_cut_min < 0 | d_cut_max > 30) return false;
  
  if (disc_list[d_counter]>=d_cut_max && disc_list[d_counter]<=d_cut_min) {  
    //  Selecting the peak of the distribution. Expecting to correspond to the centroid (avoiding double pulses)
    int binmax = hdcnt[ch_counter][d_counter]->GetMaximumBin();
    float x = hdcnt[ch_counter][d_counter]->GetXaxis()->GetBinCenter(binmax); 
    //  setting the fitting window based on the result obtained above

    /*if (x > (int)(vp_set[disc_list[d_counter]]-1*width) && x < (int)(vp_set[disc_list[d_counter]] + 1*width)) {    
      thr_min = (int)(x - width); 
      thr_max = (int)(x + width);
    } 
    else{
      thr_min = (int)(vp_set[disc_list[d_counter]] - 2*width); 
      thr_max = (int)(vp_set[disc_list[d_counter]] + 2*width);
    } */
    
    thr_min = (int)(x - width); 
    thr_max = (int)(x + width);

    if (thr_min < 5) thr_min =5;
    if (thr_max > vp_max) thr_max = vp_max - 1;

    fit_state0:

    TF1 *f_gaus  = new TF1("f_gaus", "gaus",0,255);
    f_gaus->SetParLimits(1,thr_min, thr_max);
    TFitResultPtr f_s1; 
    f_s1 = hdcnt[ch][d_counter]->Fit("f_gaus","SQWWR","",thr_min,thr_max); // fitting disc in the range thr_min-thr_max
    Int_t fitstatus = f_s1;
    float mean_tmp =0;
    float sigma_tmp =0;
    if ( fitstatus == 0 && f_s1->Chi2()!=0) {
      f_mean = f_s1->Parameter(1);
      f_sigma = f_s1->Parameter(2);
      f_sigma2 = f_s1->Parameter(2);
      mean_val[ch][d_counter] = f_s1->Parameter(1);
      if (disc_list[d_counter]==25) f_mean_d1 = f_s1->Parameter(1);
      if (disc_list[d_counter]==10) f_mean_d20 = f_s1->Parameter(1);
      if (disc_list[d_counter]==0) f_mean_d30 = f_s1->Parameter(1);
      if (f_mean!=0 && f_sigma >0.5){
          hdisc_sig[ch_counter]->Fill(disc_list[d_counter],f_sigma);
          hdisc_mean[ch_counter]->Fill(disc_list[d_counter],f_mean);
          if (ch == ch_sel) {
              h_enc_disc_gauss->Fill(disc_list[d_counter],f_sigma);
              //h_aux1->SetBinContent(disc_list[d_counter]+1,f_mean);
              //h_aux1->SetBinError(disc_list[d_counter]+1,f_sigma); 
              //h_aux2->Fill(d,mean);
          }
      }
      else {
        while (width > 5){
          width = width-5;
          thr_min = (int)(vp_set[disc_list[d_counter]]-width); 
          thr_max = (int)(vp_set[disc_list[d_counter]]+width); 
          goto fit_state0;
        }
      }  

      if (f_sigma2 >0.5){
          f_s1sigma+= f_sigma2;
          d_counter_gauss++; 
      }
    }
    else {
        cout << "fit not ok" << endl;
        f_sigma2 = 1.;
    }     
    h2D_mean_gauss->Fill(ch,disc_list[d_counter],f_mean);
    h2D_enc_gauss->Fill(ch,disc_list[d_counter],f_sigma2);
    //f_s1mean = f_s1mean/d_max;
  }
    
  return true;
}

//------------------------------------------+-----------------------------------
//! Analyzing the data. Fitting S-curves
bool trim_adc::Fit_values_erfc(int width,int *disc_list, int d_cut_min, int d_cut_max, int cut_db_pulses_user)
{
  d_cut_min = 30-d_cut_min;
  d_cut_max = 30-d_cut_max;
  
  int plateau = cut_db_pulses_user*rebin_histo;
  if (d_cut_min < 0 | d_cut_max > 30) return false;
  
  if (disc_list[d_counter]>=d_cut_max && disc_list[d_counter]<=d_cut_min) {
    //f_s1mean_erfc = 0.;
    f_sigma2_erfc = 0.;
    f_mean_erfc = 0.;
    f_sigma_erfc = 0.;

    fit_mu = hdisc_mean[ch_counter]->GetBinContent(disc_list[d_counter]+1); 
    fitg_sigma = hdisc_sig[ch_counter]->GetBinContent(disc_list[d_counter]+1); 
    width+=15;
          
    if (fit_mu > (int)(vp_set[disc_list[d_counter]]-1*width) && fit_mu < (int)(vp_set[disc_list[d_counter]] + 1*width)) {    
    thr_min = (int)(fit_mu - width); 
    thr_max = (int)(fit_mu + width);
    } 
    else {
    thr_min = (int)(vp_set[disc_list[d_counter]] - 1.5*width); 
    thr_max = (int)(vp_set[disc_list[d_counter]] + 1.5*width);
    fit_mu  = (int)(vp_set[disc_list[d_counter]]);
    }

    if (thr_min < 5) thr_min =5;
    if (thr_max > vp_max) thr_max =vp_max - 1;
    

    // if (fitg_sigma < 1. | fitg_sigma > 20.) fitg_sigma = 5.; 
    // if (disc_list[d_counter] >23 ) { thr_min = 5.; thr_max = 120.; }

    TF1 *f_erfc  = new TF1("f_erfc", "[0]-[1]*TMath::Erfc((x-[2])/(sqrt(2)*[3]))",0,255);
    TFitResultPtr f_s1; 
    if (hscurve[ch][d_counter]->GetMean()>1){
      f_erfc->SetParameters(plateau,plateau/2,fit_mu,fitg_sigma);
      // fitting disc in the range thr_min-thr_max
      f_s1 = hscurve[ch][d]->Fit("f_erfc", "SQWWR", "",thr_min, thr_max);
      Int_t fitstatus = f_s1;
      float mean_tmp =0;
      float sigma_tmp =0;
      if ( fitstatus == 0 && f_s1->Chi2()!=0) {
        f_mean_erfc = f_s1->Parameter(2);
        f_sigma_erfc = f_s1->Parameter(3);
        f_sigma2_erfc = f_s1->Parameter(3);
        h1disc_mean[d_counter]->Fill(f_mean_erfc);
        h1_chi_erfc->Fill(f_s1->Chi2()/f_s1->Ndf());
        if (ch == ch_sel) {
          h_enc_disc_erfc->Fill(disc_list[d_counter],f_sigma_erfc);
          h_thr_ch_erfc->SetBinContent(disc_list[d_counter]+1,f_mean_erfc);
          h_thr_ch_erfc->SetBinError(disc_list[d_counter]+1,f_sigma_erfc); 
          //h_aux2->Fill(d,mean);
        }
        if (f_sigma_erfc >0.5){
          f_s1sigma_erfc+= f_sigma2_erfc;
          d_counter_erfc++; 
        }
      } 
      else {
        cout << "fit not ok" << endl; 
        f_sigma_erfc = 1.;
      } 
    } else {
      f_mean_erfc = 10;
      f_sigma_erfc = 1.; 
    }
  
    if ((f_mean_erfc<0.)||(f_mean_erfc>255.)|| TMath::IsNaN(f_mean_erfc)) {
      h2D_mean_erfc->Fill(ch,disc_list[d_counter],10);
    } else {
      h2D_mean_erfc->Fill(ch,disc_list[d_counter],f_mean_erfc);
    }
    h2D_enc_erfc->Fill(ch,disc_list[d_counter],f_sigma_erfc);
    //*/
  }
  return true;
}

//------------------------------------------+-----------------------------------
//! Analyzing the data. Calculating values of a Normal distribution
bool trim_adc::Calc_values(int width, int *disc_list, int d_cut_min, int d_cut_max)
{
  d_cut_min = 30-d_cut_min;
  d_cut_max = 30-d_cut_max;
  if (d_cut_min < 0 | d_cut_max > 30) return false;
  else { 
    if (disc_list[d_counter]>=d_cut_max && disc_list[d_counter]<=d_cut_min) {
      sum_delta = 0.0000001;
      sum_sig = 0;
      d_cnt = 0;
      // range where I will look for the Scurve. Expanded range compare to the fit    
      thr_min = (int)(vp_set[disc_list[d_counter]]-vp_min-width); 
      if (thr_min <=0){thr_min = 1;}
      thr_max = (int)(vp_set[disc_list[d_counter]]-vp_min+width);
      if (thr_max >vp_max-vp_min){thr_max = vp_max-vp_min;} 
      
      thr_min = 0;
      thr_max = 250;
      ivp = thr_min;
      
      for ( vp = thr_min; vp <thr_max; vp += vp_step ) {
        d_cnt = vcnt_soft[ch_counter][d_counter][ivp]-vcnt_soft[ch_counter][d_counter][ivp-1]; 
        //if (d_cnt <0 | d_cnt >35) d_cnt =0;
        if (d_cnt <0) d_cnt =0;
        sum_delta += d_cnt;
        sum_mean += (vp+vp_min)*d_cnt;	
        ivp+=1;
      }
      mean = sum_mean/sum_delta;
      if ((mean<0.)||(mean>250.)) h2D_mean_calc->Fill(ch,disc_list[d_counter],1);
      else h2D_mean_calc->Fill(ch,disc_list[d_counter],mean);
      sum_delta = 0.0000001;

      ivp = thr_min;
      for ( vp = thr_min; vp <thr_max; vp += vp_step ) {
        d_cnt = vcnt_soft[ch_counter][d_counter][ivp]-vcnt_soft[ch_counter][d_counter][ivp-1];
        //if (d_cnt <0 | d_cnt >35) d_cnt =0;
        if (d_cnt <0) d_cnt =0;
        sum_delta += d_cnt;
        sum_sig +=((vp+vp_min)-mean)*((vp+vp_min)-mean)*d_cnt;
        ivp++;
      }
      sigma = sqrt(sum_sig/sum_delta);
  
      if (ch == ch_sel) {
        //h_aux1_c->Fill(disc_list[d_counter],mean);
        h_enc_disc_calc->Fill(disc_list[d_counter],sigma);
      }

      if (sigma >0){
        // if (sigma >0){
        sum_sige +=sigma;
        d_counter_ana++;
      }
      h2D_enc_calc->Fill(ch,d,sigma); 
    }
  }
  return true;
}

//------------------------------------------+-----------------------------------
//! Analyzing data. Fast discriminator
void trim_adc::Fitting_Fast(int width, int cut_db_pulses_user){

    width = width + 10;
    thr_min = (int)(vp_set[30]-width); 
    thr_max = (int)(vp_set[30]+width);
    // Aux variables 
    float fast_plateau = cut_db_pulses_user*rebin_histo;
    float chi_fast = 0;
    float fast_min = 0;
    float fast_max = 0;
    float fast_mean = 0;
    float fast_sigma = 0;
    // Fitting variables
    float low_lim = 0.;
    float high_lim = 80.;

    // Checking that the histograms has counts
    if (hdcnt[ch][d]->GetMean()>1){
    //  Selecting the peak of the distribution. Expecting to correspond to the centroid (avoiding double pulses)
    hdcnt[ch][d]->GetXaxis()->SetRange(low_lim, high_lim);
    int binmax = hdcnt[ch][d]->GetMaximumBin();
    float x = hdcnt[ch][d]->GetXaxis()->GetBinCenter(binmax); 
    //  setting the fitting window based on the result obtained above
    thr_min = (int)(x-width);  
    if (thr_min < low_lim) thr_min = low_lim;
    thr_max = (int)(x+width);
    if (thr_max > high_lim) thr_max = high_lim;
    
    // defyining gauss fitting function
    TF1 *f_gaus = new TF1("f_gaus", "gaus",0,250);
    TFitResultPtr f_s2; 
    f_gaus->SetParameter(1,x);
    f_s2= hdcnt[ch][d]->Fit("f_gaus","SWWQR0","lsame",thr_min,thr_max);
    Int_t fitstatus = f_s2;
    if ( fitstatus == 0 && f_s2->Chi2()!=0){
        f_mean = f_s2->Parameter(1);
        f_sigma = f_s2->Parameter(2);
      }
    delete f_gaus;
    } 
  
    if ( f_mean <low_lim | f_mean >high_lim) f_mean = 40;
    if (f_sigma < 0 | f_sigma > 20 ) f_sigma = 10;
    fast_min = f_mean - 2*width;
    if (fast_min < low_lim) fast_min = low_lim;
    fast_max = f_mean + width;
    if (fast_max > high_lim) fast_max = high_lim;

    std::unique_ptr<TF1> f_erfc (new TF1("f_erfc", "[0]-[1]*TMath::Erfc((x-[2])/(sqrt(2)*[3]))",0,250));
  
    TFitResultPtr f_s3;
    Int_t fitstatus_ferfc;
    if (hscurve[ch][d]->GetMean()>1){
      f_erfc->SetParameters(fast_plateau, fast_plateau/2., f_mean, f_sigma);
      f_s3 = hscurve[ch][d]->Fit("f_erfc", "SWWQR", "lsame", fast_min, fast_max);
      fitstatus_ferfc = f_s3;
      if (fitstatus_ferfc == 0){
        fast_mean = f_s3->Parameter(2)*350;
        fast_sigma = f_s3->Parameter(3)*350;
        chi_fast = f_s3->Chi2()/f_s3->Ndf();
      }
    }
    h_fast_thr->Fill(ch,fast_mean); 
    h1_fast_thr->Fill(fast_mean);
    h_enc_fast->Fill(ch,fast_sigma);
    h1_enc_fast->Fill(fast_sigma);
    h1_chi_ferfc->Fill(chi_fast);
}

//------------------------------------------+-----------------------------------
//! ADC linearity characteristics
bool trim_adc::Adc_charact(int *disc_list, int d_cut_min, int d_cut_max, float v_step_mean){
  
  d_cut_min = 30-d_cut_min;
  d_cut_max = 30-d_cut_max;
  
  slope[ch] = float((h2D_mean_gauss->GetBinContent(ch+1,d_cut_max+1)-h2D_mean_gauss->GetBinContent(ch+1,d_cut_min+1))/(d_cut_max-d_cut_min));
  offset[ch] = float(h2D_mean_gauss->GetBinContent(ch+1,d_cut_max+1)-(slope[ch]*d_cut_max));
  
  if (d_cut_min < 0 | d_cut_max > 30) return false;
  
  else { 
    if (disc_list[d_counter]>=d_cut_max && disc_list[d_counter]<=d_cut_min) {
        if (inl_aux<std::abs(1/6.*(h2D_mean_gauss->GetBinContent(ch_counter+1,d_counter+1)-(slope[ch_counter]*d+offset[ch_counter])))){
            inl_aux = std::abs(1/6.*(h2D_mean_gauss->GetBinContent(ch_counter+1,d_counter+1)-(slope[ch_counter]*d+offset[ch_counter])));
            }
        inl[ch] = inl_aux;
        if (disc_list[d_counter]<d_cut_min){
            if (std::abs(dnl_aux)<std::abs((h2D_mean_gauss->GetBinContent(ch_counter+1,d_counter+1)-h2D_mean_gauss->GetBinContent(ch_counter+1,d_counter+2))/v_step_mean-1)){
                dnl_aux =(h2D_mean_gauss->GetBinContent(ch_counter+1,d_counter+1)-h2D_mean_gauss->GetBinContent(ch_counter+1,d_counter+2))/v_step_mean-1;}
                dnl[ch_counter] = std::abs(dnl_aux);
            }
        }   
    } 
  return true;
}

//------------------------------------------+-----------------------------------
//! Displaying histograms_ADC
void trim_adc::Display_histo_adc(int width_user,int *disc_list, int d_cut_min, int d_cut_max,  int* ch_comp){

  TString ch_sel_strg(Form("%d",ch_sel));  
  int row_number = int(d_len/6);
  if (row_number==0){row_number = 1;}
  
  d_cut_min = 30-d_cut_min;
  d_cut_max = 30-d_cut_max;
 
  // -------------+-------------
  // Every discriminator of the selected channel is here displayed after differentiation. Gaus fitting is applied
  int n =2;
  gStyle->SetOptFit(1);
  TCanvas *c2_gaus =new TCanvas("c2_gaus","c2_gaus",2200,400);
  c2_gaus->Divide(d_len,row_number);
  c2_gaus->cd(1);
  hdcnt[ch_sel][0]->Draw("");
  hdcnt[ch_sel][0]->Fit("gaus","WQR","lsame",10,110);
  d_counter = 1;
  for (d=1; d<d_len;d++){
    if (disc_list[d_counter]>=d_cut_max && disc_list[d_counter]<=d_cut_min) {
        c2_gaus->cd(n);
        hdcnt[ch_sel][d_counter]->Draw("");
        thr_min = (int)(vp_set[disc_list[d_counter]]-width_user); 
        thr_max = (int)(vp_set[disc_list[d_counter]]+width_user);
    }
    if (disc_list[d_counter]==31){
        c2_gaus->cd(n);
        hdcnt[ch_sel][d_counter]->Draw("");
        hdcnt[ch_sel][d_counter]->SetLineColor(kGreen-3);}  
    n++;
    d_counter++;
  }
  
  c2_gaus->Write();
  delete c2_gaus;

  // -------------+-------------
  // Every discriminator of the selected channel is here displayed after differentiation. ERFC fitting is applied
  n =2;
  gStyle->SetOptFit(1);
  TCanvas *c2_erfc =new TCanvas("c2_erfc","c2_erfc",2200,400);
  //TF1 *f_erfc = new TF1("f_erfc", "[0]-[1]*TMath::Erfc((x-[2])/(sqrt(2)*[3]))",0,250);
  c2_erfc->Divide(6,row_number);
  c2_erfc->cd(1);
  hscurve[ch_sel][0]->Draw("HIST");
  TF1 *f_erfc_draw = hscurve[ch_sel][0]->GetFunction("f_erfc");
  if (f_erfc_draw != nullptr) f_erfc_draw->Draw("same");
  for (d = 1; d<d_len;d++){
     if (disc_list[d]>=d_cut_max && disc_list[d]<=d_cut_min) {
        c2_erfc->cd(n);
        hscurve[ch_sel][d]->Draw("HIST");
        f_erfc_draw = hscurve[ch_sel][d]->GetFunction("f_erfc");
        if (f_erfc_draw != nullptr) f_erfc_draw->Draw("same");
        }
      if (disc_list[d]==31){
        c2_erfc->cd(n);
        hscurve[ch_sel][d]->Draw("HIST");
        f_erfc_draw = hscurve[ch_sel][d]->GetFunction("f_erfc");
        if (f_erfc_draw != nullptr) f_erfc_draw->Draw("same");
        hscurve[ch_sel][d]->SetLineColor(kGreen-3); 
      }
    n++;
  }
  c2_erfc->Write();
  delete c2_erfc;

  // -------------+-------------
  // Selected channel S-curves, all together in a single canvas. For displaying purposed only
  n=0;
  gStyle->SetOptStat(0);
  gStyle->SetOptFit(0);
  gStyle->SetOptTitle(1);
  TCanvas *c1 =new TCanvas("c1","S-Curves_selected channel",900,900);   // OK
  //c1->cd()->SetGrid();
  hscurve[ch_sel][0]->Draw("HIST");
  hscurve[ch_sel][0]->GetXaxis()->SetTitle("Pulse amplitude [amp_cal_units]");
  hscurve[ch_sel][0]->GetYaxis()->SetTitle("Number of counts");
  hscurve[ch_sel][0]->SetLineWidth(2);
  hscurve[ch_sel][0]->GetXaxis()->SetRangeUser(0,255);
  hscurve[ch_sel][0]->GetYaxis()->SetRangeUser(0,300);
  for (d=1;d <d_len-1;d++) {			// d_len-1 to exclude the fast
    hscurve[ch_sel][d]->Draw("HISTSAME");
    hscurve[ch_sel][d]->SetLineWidth(2);
    } 
  //c1->Write();
  //c1->Close();
  delete c1;

  // -------------+-------------
  // Selected channel S-curves, all together. Linearity of the channel displayed on Pad 2  
  n=0;
  gStyle->SetOptStat(0);
  gStyle->SetOptFit(0);
  gStyle->SetOptTitle(0);
  TCanvas *c3_summary =new TCanvas("c3_summary","S-Curves_selected channel",1400,600);   // OK
  c3_summary->Divide(2,1);
  c3_summary->cd(1)->SetGrid();
  hscurve[ch_sel][0]->Draw("HIST");
  hscurve[ch_sel][0]->GetXaxis()->SetTitle("Pulse amplitude [amp_cal_units]");
  hscurve[ch_sel][0]->GetYaxis()->SetTitle("Number of counts");
  hscurve[ch_sel][0]->SetLineWidth(2);
  f_erfc_draw = hscurve[ch_sel][0]->GetFunction("f_erfc");
  if (f_erfc_draw != nullptr) f_erfc_draw->Draw("same");
  for (d=1;d <d_len-1;d++) {
    if (disc_list[d]>=d_cut_max && disc_list[d]<=d_cut_min){
      n = 30-d;
      hscurve[ch_sel][d]->Draw("HISTsame");
      f_erfc_draw = hscurve[ch_sel][d]->GetFunction("f_erfc");
      if (f_erfc_draw != nullptr) f_erfc_draw->Draw("same");
	    hscurve[ch_sel][d]->SetLineWidth(2);
    }
  } 

  c3_summary->cd(2)->SetGrid();
  // h_aux1->SetMarkerColor(kBlue);
  // h_aux1->SetLineColor(kBlue);
  // h_aux1->Draw("PE");
  // h_aux1->Fit("pol1", "RQ0", "",d_cut_max, d_cut_min+1);
  // h_aux1->GetFunction("pol1")->SetLineColor(kBlue);
  // h_aux1->SetMarkerStyle(22);
  // h_aux1->SetMarkerSize(1.5);
  // h_aux1->GetXaxis()->SetTitle("Discriminator Number");
  // h_aux1->GetYaxis()->SetTitle("Discriminator Thr [amp_cal_units]"); 
  
  h_thr_ch_erfc->Draw("PE");
  h_thr_ch_erfc->SetMarkerColor(kBlack);
  h_thr_ch_erfc->SetMarkerStyle(25);
  h_thr_ch_erfc->SetMarkerSize(0.8);
  h_thr_ch_erfc->SetLineColor(kBlack);
  h_thr_ch_erfc->Fit("pol1", "RQ", "", d_cut_max, d_cut_min+1);
  if (h_thr_ch_erfc->GetFunction("pol1")!=nullptr){
    h_thr_ch_erfc->GetFunction("pol1")->Draw("same");
    h_thr_ch_erfc->GetFunction("pol1")->SetLineColor(kRed);
    h_thr_ch_erfc->GetFunction("pol1")->SetLineWidth(2);
    h_thr_ch_erfc->GetFunction("pol1")->SetLineStyle(7);
  }
  TLegend *lg_c3 = new TLegend(0.3,0.25, 0.55, 0.40);
  lg_c3->SetHeader(Form("ADC linearity channel: %d",ch_sel));
  //lg_c3->AddEntry(h_aux1,"gauss_method","p");
  lg_c3->AddEntry(h_thr_ch_erfc,"erfc_method","p");
  lg_c3->SetBorderSize(0);
  lg_c3->SetFillColor(0);
  lg_c3->Draw("");
  
  TLatex *l_thr_erfc= new TLatex(0.40,0.75,Form("ADC_thr [amp_cal] = %.1f*ADC_val + %.1f", h_thr_ch_erfc->GetFunction("pol1")->GetParameter(1), h_thr_ch_erfc->GetFunction("pol1")->GetParameter(0)));
  l_thr_erfc->SetNDC();
  l_thr_erfc->SetTextSize(0.03);
  l_thr_erfc->SetTextColor(kRed);
  l_thr_erfc->Draw();

  h_thr_ch_erfc->Write();
  c3_summary->Write();
  delete c3_summary;

  // -------------+-------------
  gStyle->SetOptStat(1101);
  TCanvas *c5 =new TCanvas("c5","Mean_and sigma_channels");
  c5->Divide(3,2);
  c5->cd(1); // Drawing the discriminator's threshold in terms of amp_cal values. Gaussian method
  h2D_mean_gauss->Draw("COLZ");
  h2D_mean_gauss->GetZaxis()->SetRangeUser(0,255);

  c5->cd(2); // Drawing the discriminator's threshold in terms of amp_cal values. Erfc method
  h2D_mean_erfc->Draw("COLZ");
  h2D_mean_erfc->GetZaxis()->SetRangeUser(0,255);
  
  c5->cd(3)->SetGrid(); // Projections of the 2D histograms. ADC gain calculations
  float par1_adc = 0.;
  float par0_adc = 0.;
  float par2_adc = 0.;
  float resid = 0.;
  TH1D *py[128];
  for (ch =ch_min; ch<ch_max;ch+=ch_step){
    TString h_adc_linearity(Form("h_quality_%d",ch));
    py[ch] = h2D_mean_erfc->ProjectionY(h_adc_linearity,ch+1,ch+1);
  }
  py[0]->Draw("HIST");
  py[0]->GetXaxis()->SetTitle("ADC value LSB");
  py[0]->GetYaxis()->SetTitle("Signal Vp amplitude [amp_cal_units]");
  for (ch = ch_min; ch<ch_max;ch+=ch_step){
    py[ch]->Draw("HISTsame");
    if (py[ch]->GetMean()!=0){
      py[ch]->Fit("pol1","SRQ","",d_cut_max,d_cut_min+1);
      par1_adc = py[ch]->GetFunction("pol1")->GetParameter(1);
      par0_adc = py[ch]->GetFunction("pol1")->GetParameter(0); 
      py[ch]->GetFunction("pol1")->Draw("same");
    }
    // Analyzing the results of the fitting
    if (TMath::IsNaN(par1_adc)){ 
      h_adc_gain->Fill(ch,0.31);
      // ----- Distributions
      h1_adc_gain->Fill(0.31);
    }
    else { 
      h_adc_gain->Fill(ch,-1*par1_adc*350.);
      // ----- Distributions
      h1_adc_gain->Fill(-1*par1_adc*350.);
    }
    if (TMath::IsNaN(par0_adc)) {
      h_adc_thr->Fill(ch,0);
      h1_adc_thr->Fill(0); 
      h1_adc_offset->Fill(0); 
    }
    else { 
      h_adc_thr->Fill(ch,(par0_adc+par1_adc*30.5)*350.);
      h1_adc_thr->Fill((par0_adc+par1_adc*30.5)*350); 
      h1_adc_offset->Fill((par0_adc+par1_adc*31.5)*350); 
    }
    // Calculating ADC residuals
    resid = 0;
    for (d = 0 ; d < d_len-1; d++){
       if (disc_list[d]>=d_cut_max && disc_list[d]<=d_cut_min) {
            resid =  py[ch]->GetBinContent(disc_list[d]+1)-(par1_adc*(disc_list[d]+0.5) + par0_adc);
            h1_adc_resid->Fill(resid*350);
       }
    }
  }

  c5->cd(4); // Drawing the noise in terms of amp_cal values. Gaussian method
  h2D_enc_gauss->Draw("COLZ");
  h2D_enc_gauss->GetZaxis()->SetRangeUser(0,8);

  c5->cd(5); // Drawing the noise in terms of amp_cal values. Erfc method
  h2D_enc_erfc->Draw("COLZ");
  h2D_enc_erfc->GetZaxis()->SetRangeUser(0,8);

  gStyle->SetOptStat(1111);
  gStyle->SetOptFit(1);
  c5->cd(6)->SetGrid();  // Drawing the residuals of the Erfc fitting method
  h1_adc_gain->Fit("gaus", "WQ");
  h1_adc_thr->Fit("gaus", "WQ");
  h1_adc_offset->Fit("gaus", "WQ");
  h1_adc_resid->Draw("");
  h1_adc_resid->Fit("gaus", "WQ", "lsame");
  h1_adc_resid->Write();
  h_adc_gain->Write();
  h_adc_thr->Write();
  h1_adc_thr->Write();
  h1_adc_gain->Write();
  h1_adc_offset->Write();
  
  c5->Write();
  delete c5;

  // -------------------------------- END S-curves on selected channel -----------------------------------
  // Selected group of channels S-curves, all together
  int i=1;
  n = 0;
  gStyle->SetOptStat(11);
  gStyle->SetOptTitle(0);
  TCanvas *c_sel_group[4];
  for (grp = grp_min; grp <grp_max; grp++){
    TString hname_c_group = Form("c_group_%d", grp);
    c_sel_group[grp] = new TCanvas(hname_c_group, "");
    c_sel_group[grp]->Divide(6,5);
    i = 1;
    for (ch = grp; ch < ch_max; ch+=4){
        c_sel_group[grp]->cd(i);
        hscurve[ch][0]->Draw("HIST");
        hscurve[ch][0]->GetXaxis()->SetTitle("Pulse amplitude [amp_cal_units]");
        hscurve[ch][0]->GetYaxis()->SetTitle("Number of counts");
        hscurve[ch][0]->GetYaxis()->SetRangeUser(0,500);
        f_erfc_draw = hscurve[ch][0]->GetFunction("f_erfc");
        if (f_erfc_draw != nullptr) f_erfc_draw->Draw("same");
        for (d = 1; d < d_len-1; d++) {
          if (disc_list[d]>=d_cut_max && disc_list[d]<=d_cut_min) {
            hscurve[ch][d]->Draw("HISTSAME");
            f_erfc_draw = hscurve[ch][d]->GetFunction("f_erfc");
            if (f_erfc_draw != nullptr) f_erfc_draw->Draw("same");
          }
        }
        i++;
    }
  c_sel_group[grp]->Write();
  delete c_sel_group[grp];
  //cout<<"Group: "<<grp<<endl;
  }
  delete f_erfc_draw;	

  //std::cout<<__LINE__<<endl; 
  // -------------+-------------
  TCanvas *c_adc_lin =new TCanvas("c_adc_lin","ASIC_ADC_linearity");    //OK
  c_adc_lin->cd()->SetGrid();
  py[0]->Draw("HIST");
  py[0]->GetXaxis()->SetTitle("ADC value");
  py[0]->GetYaxis()->SetTitle("Threshold level [amp_cal units]");
  for (ch = ch_min+1; ch<ch_max;ch+=ch_step){
    py[ch]->Draw("HISTsame");
    if (py[ch]->GetFunction("pol1")!=nullptr) py[ch]->GetFunction("pol1")->Draw("same");
  }
  c_adc_lin->Write();
  delete c_adc_lin;
 
  // -------------+-------------
  // n=0;
  // gStyle->SetOptFit(0);
  // gStyle->SetOptTitle(0);
  // TCanvas *c_noise_disc =new TCanvas("c_noise_disc","Noise level/discriminator",1400,600);		// OK
  // c_noise_disc->Divide(2,1);
  // c_noise_disc->cd(1)->SetGrid();
  // h_aux1->SetMarkerColor(kBlue);
  // h_aux1->SetLineColor(kBlue);
  // h_aux1->Draw("PE");
  // h_aux1->Fit("pol1", "RQ0", "lsame",d_cut_max, d_cut_min+1);
  // h_aux1->GetFunction("pol1")->SetLineColor(kBlue);
  // h_aux1->SetMarkerStyle(22);
  // h_aux1->SetMarkerSize(1.5);
  // h_aux1->GetXaxis()->SetTitle("Discriminator Number");
  // h_aux1->GetYaxis()->SetTitle("Discriminator Thr [amp_cal_units]"); 
  
  // h_thr_ch_erfc->Draw("PESAME");
  // h_thr_ch_erfc->SetMarkerColor(kRed);
  // h_thr_ch_erfc->SetMarkerStyle(23);
  // h_thr_ch_erfc->SetMarkerSize(1.5);
  // h_thr_ch_erfc->SetLineColor(kRed);
  // h_thr_ch_erfc->Fit("pol1", "RQ0", "lsame",d_cut_max, d_cut_min+1);
  // h_thr_ch_erfc->GetFunction("pol1")->SetLineColor(kRed);
  
  // TLegend *lg_cadc = new TLegend(0.3,0.25, 0.55, 0.40);
  // lg_cadc->SetHeader(Form("ADC linearity channel:%d",ch_sel) );
  // lg_cadc->AddEntry(h_aux1,"gauss_method","p");
  // lg_cadc->AddEntry(h_thr_ch_erfc,"erfc_method","p");
  // lg_cadc->SetBorderSize(0);
  // lg_cadc->SetFillColor(0);
  // lg_cadc->Draw(""); 
  // l_thr_gauss->Draw();
  // l_thr_erfc->Draw();
   
  // gStyle->SetOptStat(0);
  // gStyle->SetOptFit(0);
  // gStyle->SetOptTitle(0);
  // c_noise_disc->cd(2)->SetGrid();
  // h_enc_disc_gauss->Draw("PE");
  // h_enc_disc_gauss->SetLineColor(kBlue);
  // h_enc_disc_gauss->SetMarkerStyle(22);
  // h_enc_disc_gauss->SetMarkerColor(kBlue);
  // h_enc_disc_gauss->SetMarkerSize(1.5);
  // h_enc_disc_gauss->GetXaxis()->SetTitle("Discriminator Number");
  // h_enc_disc_gauss->GetYaxis()->SetTitle("Sigma ENC [amp_cal_units]");
  // h_enc_disc_erfc->Draw("PEsame");
  // h_enc_disc_erfc->SetLineColor(kRed);
  // h_enc_disc_erfc->SetMarkerStyle(23);
  // h_enc_disc_erfc->SetMarkerSize(1.5);
  // h_enc_disc_erfc->SetMarkerColor(kRed);
  
  // TLegend *lg_cnoise = new TLegend(0.6,0.7, 0.85, 0.85);
  // lg_cnoise->SetHeader(Form("ENC sigma for channel: %d",ch_sel));
  // lg_cnoise->AddEntry(h_aux1,"gauss_method","p");
  // lg_cnoise->AddEntry(h_thr_ch_erfc,"erfc_method","p");
  // lg_cnoise->SetBorderSize(0);
  // lg_cnoise->SetFillColor(0);
  // lg_cnoise->Draw("");
  
  // c_noise_disc->Write(); 
  // delete lg_cnoise;
  // delete c_noise_disc;

  // --------------- CANVAS comparing 4 different channels listed on the execution file -------------------
  // --- Comparison of the S-curves shape, ADC linearity and ENC ----
  
  gStyle->SetOptStat(1111);
  gStyle->SetOptFit(1);
  TCanvas*c_comp = new TCanvas("c_comp","c_comp");
  c_comp->Divide(4,3);
  TF1 *f_erfc_comp;
  for (int j=0; j<4; j++){
    c_comp->cd(j+1)->SetGrid();
    hscurve[ch_comp[j]][0]->Draw("HIST");
    hscurve[ch_comp[j]][0]->GetXaxis()->SetTitle("Pulse amplitude [amp_cal_units]");
    hscurve[ch_comp[j]][0]->GetYaxis()->SetTitle("Number of counts");
    hscurve[ch_comp[j]][0]->SetLineWidth(2);
    f_erfc_comp = hscurve[ch_comp[j]][0]->GetFunction("f_erfc");
    if (f_erfc_comp != nullptr) f_erfc_comp->Draw("same");
    for (d = 1;d < d_len - 1; d++) {
      if (disc_list[d]>=d_cut_max && disc_list[d]<=d_cut_min) {
        n = 30-d;
        hscurve[ch_comp[j]][d]->Draw("HISTSAME");
        hscurve[ch_comp[j]][d]->SetLineWidth(2);
        f_erfc_comp = hscurve[ch_comp[j]][d]->GetFunction("f_erfc");
        if (f_erfc_comp != nullptr) f_erfc_comp->Draw("same");
      }
    } 

    int k = j+5;
    c_comp->cd(k)->SetGrid();
    hdisc_mean[ch_comp[j]]->Draw("PHIST");
    hdisc_mean[ch_comp[j]]->SetMarkerStyle(25);
    hdisc_mean[ch_comp[j]]->SetMarkerSize(0.7);
    hdisc_mean[ch_comp[j]]->SetMarkerColor(kBlack);
    hdisc_mean[ch_comp[j]]->SetLineColor(kBlack);
    hdisc_mean[ch_comp[j]]->GetXaxis()->SetTitle("Discriminator number");
    hdisc_mean[ch_comp[j]]->GetYaxis()->SetTitle("Disc. thr [amp_cal_units]");
    hdisc_mean[ch_comp[j]]->GetYaxis()->SetRangeUser(0,260);
    hdisc_mean[ch_comp[j]]->SetLineWidth(2);
    hdisc_mean[ch_comp[j]]->Fit("pol1","RQ","",d_cut_max,d_cut_min+1);
    hdisc_mean[ch_comp[j]]->GetFunction("pol1")->Draw("same");
       
    int z = j+9;
    c_comp->cd(z)->SetGrid();
    hdisc_sig[ch_comp[j]]->Draw("PHIST");
    hdisc_sig[ch_comp[j]]->SetMarkerStyle(25);
    hdisc_sig[ch_comp[j]]->SetMarkerSize(0.7);
    hdisc_sig[ch_comp[j]]->SetMarkerColor(kRed);
    hdisc_sig[ch_comp[j]]->SetLineColor(kBlack);
    hdisc_sig[ch_comp[j]]->GetXaxis()->SetTitle("Discriminator number");
    hdisc_sig[ch_comp[j]]->GetYaxis()->SetTitle("ENC [amp_cal_units]");
    hdisc_sig[ch_comp[j]]->GetYaxis()->SetRangeUser(0,10);
    hdisc_sig[ch_comp[j]]->SetLineWidth(2);
  }
  c_comp->Write();
  delete f_erfc_comp;
  delete c_comp;

  // -------------+-------------
  // --------------- CANVAS with discriminator threshold per channel ------------------- 
  gStyle->SetOptStat(1111);
  gStyle->SetOptTitle(1111);
  TCanvas *c1_disc_mean = new TCanvas("c1_disc_mean","c1_disc_mean", 2200, 600);
  c1_disc_mean->Divide(5,row_number);
  n=1;
  int binmax = 0;
  float mean_f = 0;
  int lim_min = 0;
  int lim_max = 0;
  for (d= 0; d< d_len-1;d++){
    if (disc_list[d]>=d_cut_max && disc_list[d]<=d_cut_min) {
      c1_disc_mean->cd(n);
      h1disc_mean[d]->Draw("HIST");
      h1disc_mean[d]->Fit("gaus", "WQ", "");
      h1disc_mean[d]->GetFunction("gaus")->Draw("same");
      h1disc_mean[d]->Write();
      n++;
    }
  }
  c1_disc_mean->Write();
  delete c1_disc_mean;
  
  // -------------+-------------
  // --------------- CANVAS with noise per channel -------------------  
  gStyle->SetOptStat(0);
  gStyle->SetOptTitle(0);
  TCanvas *c_enc = new TCanvas("c_enc","c_enc");
  c_enc->cd()->SetGrid();
  h_enc_gaus->Draw("HIST");
  h_enc_gaus->SetLineWidth(2);
  h_enc_gaus->SetLineColor(kBlue);
  h_enc->SetLineWidth(2);
  h_enc->SetLineColor(kRed);
  h_enc->Draw("HISTsame");
  
  TLegend *l_noise = new TLegend(0.5,0.7,0.88,0.88);
  l_noise->SetHeader(""); // option "C" allows to center the header
  l_noise->SetLineColor(kWhite);
  l_noise->AddEntry(h_enc_gaus,"Gaussian_method","l");
  l_noise->AddEntry(h_enc,"Erfc_method","l");
  l_noise->Draw();
  c_enc->Write();
  
  delete l_noise;
  delete c_enc;
  h_enc_gaus->Write();
  h_enc->Write();
}


//------------------------------------------+-----------------------------------
//! Displaying histograms_FAST_discriminator
void trim_adc::Display_histo_fast(){
  
  thr_min = (int)(vp_set[30]-30); 
  thr_max = (int)(vp_set[30]+50);  
  gStyle->SetOptStat(1111);
  gStyle->SetOptFit(1);
  TCanvas *c_fast_disc =new TCanvas("c_fast_disc","Fast_discriminator_channels", 1800,600);
  c_fast_disc->Divide(3,1);
  c_fast_disc->cd(1)->SetGrid();
  hscurve[0][d_len-1]->Draw("HIST");
  hscurve[0][d_len-1]->GetXaxis()->SetRangeUser(vp_min, vp_min+150);
  hscurve[0][d_len-1]->SetTitle("S-curves fast_disc");
  hscurve[0][d_len-1]->GetXaxis()->SetTitle("Pulse amplitude [amp_cal_units]");
  hscurve[0][d_len-1]->GetYaxis()->SetTitle("Number of counts");
  hscurve[0][d_len-1]->SetLineColor(kRed);
  hscurve[0][d_len-1]->SetLineWidth(2);
  for(ch =ch_min+1; ch<ch_max; ch++){
    hscurve[ch][d_len-1]->Draw("HISTSAME");
    hscurve[ch][d_len-1]->SetLineWidth(2);
    if (ch%2 == 0) hscurve[ch][d_len-1]->SetLineColor(kRed);
    else hscurve[ch][d_len-1]->SetLineColor(kBlue);
  }

  c_fast_disc->cd(2)->SetGrid();
  h1_fast_thr->Draw("HIST");
  h1_fast_thr->Fit("gaus", "WQ");
  h1_fast_thr->GetFunction("gaus")->Draw("same");
  h1_fast_thr->SetLineColor(kBlue);
  h1_fast_thr->SetLineWidth(2);
  h1_fast_thr->Write();

  c_fast_disc->cd(3)->SetGrid();
  h1_chi_ferfc->Draw("HIST");
  h1_chi_ferfc->Write();
  c_fast_disc->Write();
  delete c_fast_disc;
}

//------------------------------------------+-----------------------------------
//! Deleting histograms
void trim_adc::Delete_Histos(){

  for (ch = ch_min; ch<ch_max; ch++){
    delete hdisc_sig[ch];
    delete hdisc_mean[ch];
    for (d = d_min; d<d_max; d++){
      delete hdcnt[ch][d];
      delete hscurve[ch][d];
    }
    if (ch<6) delete h1disc_mean[ch];
  }
  // delete h_mean;
  delete h1_adc_resid;
  delete h1_adc_gain;
  delete h_adc_gain;
  delete h1_adc_thr;
  delete h1_adc_offset;
  delete h_adc_thr;
  //delete h_adc_int;
  //delete h_sigma;
  //delete h_aux1;
  delete h_thr_ch_erfc;
  //delete h_aux1_c;
  //delete h_aux2;
  delete h_disc_dnl;
  delete h_disc_inl;
  delete h_inl;
  delete h_dnl;
  //delete h_counter;

  //delete h_chi_gaus;
  delete h1_chi_erfc;
  delete h1_chi_ferfc;
    
  delete h1_dnl;
  delete h1_inl;
    
  delete h2D_mean_calc;
  delete h2D_enc_calc;
  
  delete h2D_mean_erfc;
  delete h2D_enc_erfc;
  
  delete h_enc_calc;
  delete h_enc_gaus;
  delete h_enc;
  delete h1_enc;
  delete h1_enc_even;
  delete h1_enc_odd;
  delete h2D_mean_gauss;
  delete h2D_enc_gauss;
  
  // ----- broken channels -----
  // delete hbrk_even;
  // delete hbrk_odd;
  // delete hbrk2_even;
  // delete hbrk2_odd;
  // delete hubrk_even;
  // delete hubrk_odd;
  // delete h1ubrk_even;
  // delete h1ubrk_odd;

  delete h_enc_disc_calc; 
  delete h_enc_fast;
  delete h1_enc_fast;
  delete h_fast_thr;
  delete h1_fast_thr;

}

