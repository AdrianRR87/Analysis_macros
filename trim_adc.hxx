#ifndef _trim_adc_HXX
#define _trim_adc_HXX

#include "TFile.h"
#include <string>
#include <TH1F.h>
#include <TH2F.h>
#include <TTree.h>
#include <unistd.h>
#include <stdint.h>
#include <iostream>
#include <fstream>
#include "TObject.h"

class trim_adc
{
  public:
    explicit trim_adc(TString filenameData = "z");
    ~trim_adc();
  
  private:

  int ch;
  int ch_min;
  int ch_max;
  int ch_counter;
  int ch_step;
  
  int d;
  int d_len;
  int d_counter;
  int d_counter_gauss;
  int d_counter_ana;
  int d_counter_erfc;
  int d_min;
  int d_max;
  int d_step;
  int cut_db_pulses;
  int rebin_histo;
  
  int grp;
  int grp_min;
  int grp_max;
  int grp_step;
  
  int vp;
  int ivp;
  // input values for the vp scan. Please modify this values acording to the pscan vp range; values should be correct or s-curves will be shifted. *Needs improvements
  int vp_min;
  int vp_max;
  int vp_step;
  int vcnt[130][32][255];   
  int vcnt_soft[130][32][255];
  uint32_t cnt_val;
  
  // variables for data processing
  double mean;
  double sigma;
  double sigma_e;
  
  double sum_mean;
  double sum_delta;
  double sum_sig;
  double sum_sige;
  
  int d_cnt;
  int a;
  
  float amp_cal_min;   		
  float amp_cal_max;  						
  int thr_val1;					
  int vp_set[31];
  
  int read_flag;
  int soft_flag;
  
  
  double f_s1mean;
  double f_mean;
  double f_mean_d1;
  double f_mean_d20;
  double f_mean_d30;
  double f_mean_fast;
  double f_s1sigma;
  double f_sigma;
  double f_sigma2;
  double f_figma_fast;
  
  // ------- enc_erfc -----------
  float fit_mu;
  float fitg_sigma;
  
  // ----- extras ----
  double f_s1mean_erfc;
  double f_mean_erfc;
  double f_mean_d1_erfc;
  double f_mean_fast_erfc;
  double f_s1sigma_erfc;
  double f_sigma_erfc;
  double f_sigma2_erfc;
  
  float adc_enc;
  float adc_thr;
  float adc_gain;
  
  // ------- broken_channels -------
  float ave_even;
  float ave_odd;
  
  float sigma_even;
  float sigma_odd;
    
  // ------- adc charact -----------
  float mean_dnl;
  float sigma_dnl;
  float mean_inl;
  float sigma_inl;
  float v_step_mean;
  float inl_aux;
  float dnl_aux;
  
  
  float dnl[128];
  float inl[128];
  float slope[128];
  float offset[128];
  float mean_val[128][32];
  
  
  
  int thr_min; 
  int thr_max;
  
  int ch_sel;
  
  TFile *file1;
  TString filename_data;
  TString filename_root;
  TString filename_val;
  ifstream scanfile;
  ofstream val_file;
  
  
  TH1F *hdisc_sig[128];  // Discriminators thr values for a specific channel 
  TH1F *hdisc_mean[128];
  
  // TH1F *h_mean;
  //std::shared_ptr<TH1D> h_mean;
  TH1F *h1_adc_resid;
  TH1F *h1_adc_gain;
  TH1F *h_adc_gain;

  TH1F *h1_adc_thr;
  TH1F *h1_adc_offset;
  TH1F *h_adc_thr;
  //TH1F *h_adc_int;
  //TH1F *h_sigma;
  TH1F *h_aux1;
  TH1F *h_thr_ch_erfc;
  //TH1F *h_aux1_c;
  //TH1F *h_aux2;
  
  //TH1F *h_counter;
  //TH2F *h_chi_gaus;
  TH1F *h1_chi_erfc;
  TH1F *h1_chi_ferfc;
  
  // --- Scurves & Derivative data ---
  TH1F *hscurve[128][32];       // S-Curves
  TH1F *hdcnt[128][32];         // Differential count 
 
  // --- Discriminator distributions ---
  TH1F *h1disc_mean[6];
  TH1F *h1_fast_thr;
  // --- Thr disc 2D ---
  TH2F *h2D_mean_erfc;
  TH2F *h2D_mean_gauss;
  TH2F *h2D_mean_calc;
  // --- Thr disc FAST ---
  TH1F *h_fast_thr;
  // --- ENC vs ch ---
  TH1F *h_enc_calc;     // ENC determined by an arithmetic method
  TH1F *h_enc_gaus;     // ENC determined by a gaussian fitting method
  TH1F *h_enc;          // ENC measured by an ERFC fitting function
  TH1F *h_enc_fast;     // ENC fast discriminator
  TH1F *h_enc_disc_erfc;
  TH1F *h_enc_disc_gauss;
  TH1F *h_enc_disc_calc;
  // --- ENC distributions ---
  TH1F *h1_enc;         // ENC distributions extracted from the ERFC fitting function
  TH1F *h1_enc_even;
  TH1F *h1_enc_odd;
  TH1F *h1_enc_fast;
  // --- ENC 2D distribution ---
  TH2F *h2D_enc_erfc;
  TH2F *h2D_enc_gauss;
  TH2F *h2D_enc_calc;
  // --- ADC Linearity ---
  TH1F *h_disc_dnl;
  TH1F *h_disc_inl;
  TH1F *h_inl;
  TH1F *h_dnl;
  TH1F *h1_dnl;
  TH1F *h1_inl;


  
  
  // ----- broken channels -----
  // TH1F *hbrk_even;
  // TH1F *hbrk_odd;
  // TH1F *hbrk2_even;
  // TH1F *hbrk2_odd;
  // TH1F *hubrk_even;
  // TH1F *hubrk_odd;
  // TH1F *h1ubrk_even;
  // TH1F *h1ubrk_odd;
  
  TH1F *hmean_f_disc_30;
  TH1F *hfit_s_disc_30;
  
  // -------------- TTree -------------------
  TTree *tenc_info;
  
public:

  bool Init_ch(int chMin, int chMax, int chStep=1);
  bool Init_grp(int grphMin, int grpMax, int grpStep=1);
  //bool Init_d(int dMin, int dMax, int dStep=1);
  bool Init_d(int *disc_list, int d_size);
  bool Init_vp(int vpMin, int vpMax, int vpStep=1);
  bool Fitting_windows(int ampcalMin, int ampcalMax, int width=10);
  bool Check_root_file();
  void Close_root_file();
  void Create_histo( int *disc_list);
  bool Reading_file(int cut_db_pulses_user);
  void Analysis(int cut_db_pulses_user,int rebin_histo_user, int width_user, int *disc_list, int dcut_min_user,int dcut_max_user, int ch_sel_user, bool fit=true, bool fast_fit =false);
  int  Get_fitting_values(int i){ return vp_set[i];};
  void Display_histo_adc(int width_user,int *disc_list, int d_cut_min, int d_cut_max, int* ch_comp);
  void Display_histo_fast();
  void Display_values(int* ch_comp);
  void Delete_Histos();
  
private:
  bool Fit_values(int width, int *disc_list, int d_cut_min, int d_cut_max);
  bool Adc_charact( int *disc_list, int d_cut_min, int d_cut_max, float v_step_mean);
  bool Fit_values_erfc(int width, int *disc_list, int d_cut_min, int d_cut_max, int cut_db_pulses_user);
  bool Calc_values(int width,  int *disc_list, int d_cut_min, int d_cut_max); 
  void Fitting_Fast(int width, int cut_db_pulses_user);
  bool Soft_val(bool soft_flag=true);
  void Broken_ch();
  
public:
  ClassDef(trim_adc,0)

};
#endif
