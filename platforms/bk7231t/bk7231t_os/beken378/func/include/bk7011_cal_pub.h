#ifndef _BK7011_CAL_PUB_H_
#define _BK7011_CAL_PUB_H_

#include "typedef.h"
#include "sys_config.h"
#define CFG_TEMP_DETECT_VERSION0    0U
#define CFG_TEMP_DETECT_VERSION1    1U

#define CFG_TEMP_DIFF_PWR_FREQOFFSET        1

#define BK_FLASH_OPT_TLV_HEADER           (0x00564c54)   // ASIC TLV
typedef enum{
    TXID                        = 0x11111100,
    TXPWR_TAB_TAB               = 0x22222200,
    CALI_MAIN_TX                = 0x33333300,
    CALI_MAIN_RX                = 0x44444400,
    TXEND                       = 0xeeeeeeee,
    TXNON                       = 0xffffffff
}TXSTRUCT;

#define DEFAULT_TXID_ID           (12345678)
#define DEFAULT_TXID_THERMAL      (280) //180430,7231:315,7231U:340
#define DEFAULT_TXID_CHANNEL      (22222222)
#define DEFAULT_TXID_LPF_CAP_I    (0x80)
#define DEFAULT_TXID_LPF_CAP_Q    (0x80)
typedef enum{
    TXID_ID                     = TXID+1,
    TXID_MAC,
    TXID_THERMAL,
    TXID_CHANNEL,
    TXID_XTAL,
    TXID_ADC,    
    TXID_LPFCAP,
    TXID_END,
    TXID_NON                    = TXID+0xFF
}TXIDList;

typedef enum{
    TXPWR_ENABLE_ID             = TXPWR_TAB_TAB+1,
    TXPWR_TAB_B_ID,
    TXPWR_TAB_G_ID,
    TXPWR_TAB_N_ID,
    TXPWR_TAB_DIF_GN20_ID,
    TXPWR_TAB_DIF_GN40_ID,
    TXPWR_TAB_BLE_ID,
    TXPWR_TAB_CALI_STATUTS,
    TXPWR_END,
    TXPWR_NON                   = TXPWR_TAB_TAB+0xFF
}TXPWR_ELEM_ID;

typedef enum {
    TXPWR_NONE_RD               = 0u,
    TXPWR_TAB_B_RD              = 0x1u,
    TXPWR_TAB_G_RD              = 0x2u,
    TXPWR_TAB_N_RD              = 0x4u,
    TXPWR_TAB_BLE               = 0x8u,
} TXPWR_IS_RD;

typedef enum{
    CM_TX_DCOR_MOD              = CALI_MAIN_TX+1,
    CM_TX_DCOR_PA,
    CM_TX_PREGAIN,
    CM_TX_I_DC_COMP,
    CM_TX_Q_DC_COMP,
    CM_TX_I_GAIN_COMP,
    CM_TX_Q_GAIN_COMP,
    CM_TX_I_FILTER_CORNER,
    CM_TX_Q_FILTER_CORNER,
    CM_TX_PHASE_COMP,
    CM_TX_PHASE_TY2,
    CM_TX_END,
    CM_TX_NON                   = CALI_MAIN_TX+0xFF
}CM_TX_ELEM_ID;

typedef enum{
    CM_RX_DC_GAIN_TAB           = CALI_MAIN_RX+1,
    CM_RX_AMP_ERR_WR,
    CM_RX_PHASE_ERR_WR,
    CM_RX_END,
    CM_RX_NON                   = CALI_MAIN_RX+0xFF
}CM_RX_ELEM_ID;

#define LOAD_FROM_FLASH         1
#define LOAD_FROM_CALI          0

#if (CFG_SOC_NAME == SOC_BK7231)
#define CFG_TEMP_DETECT_VERSION   CFG_TEMP_DETECT_VERSION0
#else
#define CFG_TEMP_DETECT_VERSION   CFG_TEMP_DETECT_VERSION1
#endif

#if CFG_TEMP_DETECT_VERSION == CFG_TEMP_DETECT_VERSION1
typedef struct tmp_pwr_st {
    unsigned trx0x0c_12_15 : 4;
    signed p_index_delta : 6;
    signed p_index_delta_g : 6;
    signed p_index_delta_ble : 6;
    unsigned xtal_c_dlta : 6; 
} TMP_PWR_ST, *TMP_PWR_PTR;
#else
typedef struct tmp_pwr_st {
    UINT8 mod;
    UINT8 pa;
	UINT16 pwr_idx_shift;
} TMP_PWR_ST, *TMP_PWR_PTR;
#endif

struct temp_cal_pwr_st {
    UINT8 idx;
    UINT8 mode;
    INT16 shift;
    INT16 shift_g;
};

extern void calibration_main(void);
extern INT32 rwnx_cal_load_trx_rcbekn_reg_val(void);
extern void rwnx_cal_set_txpwr_by_rate(INT32 rate, UINT32 test_mode);
extern void rwnx_cal_set_txpwr_by_channel(UINT32 channel);
extern INT32 rwnx_cal_save_trx_rcbekn_reg_val(void);
extern void do_calibration_in_temp_dect(void);
extern void bk7011_cal_bias(void);
extern void bk7011_cal_dpll(void);
extern void rwnx_cal_set_txpwr(UINT32 pwr_gain, UINT32 grate);
extern UINT32 manual_cal_get_pwr_idx_shift(UINT32 rate, UINT32 bandwidth, UINT32 *pwr_gain);
extern int manual_cal_get_txpwr(UINT32 rate, UINT32 channel, UINT32 bandwidth, UINT32 *pwr_gain);
extern void manual_cal_save_txpwr(UINT32 rate, UINT32 channel, UINT32 pwr_gain);
#if (CFG_SOC_NAME == SOC_BK7231U)
extern void manual_cal_11b_2_ble(void);
#endif
extern UINT32 manual_cal_fitting_txpwr_tab(void);
extern void manual_cal_show_txpwr_tab(void);
extern UINT32 manual_cal_load_txpwr_tab_flash(void);
extern int manual_cal_save_txpwr_tab_to_flash(void);
extern int manual_cal_save_chipinfo_tab_to_flash(void);
extern UINT8 manual_cal_wirte_otp_flash(UINT32 addr, UINT32 len, UINT8 *buf);
extern UINT8 manual_cal_read_otp_flash(UINT32 addr, UINT32 len, UINT8 *buf);
extern UINT32 manual_cal_load_default_txpwr_tab(UINT32 is_ready_flash);
extern void manual_cal_set_dif_g_n40(UINT32 diff);
extern void manual_cal_set_dif_g_n20(UINT32 diff);
extern void manual_cal_set_dif_g_ble(int dif_ch0, int dif_ch19, int dif_ch39);
extern void manual_cal_get_current_temperature(void);
extern int manual_cal_write_macaddr_to_flash(UINT8 *mac_ptr);
extern int manual_cal_get_macaddr_from_flash(UINT8 *mac_ptr);
extern int manual_cal_set_rfcali_status_inflash(UINT32 rf_status);
extern int manual_cal_get_rfcali_status_inflash(UINT32 *rf_status);
extern void manual_cal_show_otp_flash(void);
extern void manual_cal_clear_otp_flash(void);
extern void manual_cal_set_xtal(UINT32 xtal);
extern void manual_cal_set_lpf_iq(UINT32 lpf_i, UINT32 lpf_q);
extern void manual_cal_load_lpf_iq_tag_flash(void);
extern void manual_cal_load_xtal_tag_flash(void);
#if CFG_TEMP_DETECT_VERSION == CFG_TEMP_DETECT_VERSION1
void manual_cal_do_xtal_temp_delta_set(INT8 shift);
#endif
extern void manual_cal_do_xtal_cali(UINT16 cur_val, UINT16 *last, UINT16 thre, UINT16 init_val);
extern UINT32 manual_cal_get_xtal(void);
extern INT8 manual_cal_get_dbm_by_rate(UINT32 rate, UINT32 bandwidth);
extern INT8 manual_cal_get_cur_txpwr_dbm(void);
extern int manual_cal_load_temp_tag_from_flash(void);
extern int manual_cal_load_xtal_tag_from_flash(void);
extern void manual_cal_load_differ_tag_from_flash(void);

extern void bk7011_micopwr_config_tssi_read_prepare(void);
extern void bk7011_micopwr_tssi_read(void);
extern void bk7011_micopwr_tssi_show(void);
extern void rwnx_cal_set_reg_adda_ldo(UINT32 val);
extern void rwnx_cal_set_reg_rx_ldo(void);

extern void manual_cal_tmp_pwr_init(UINT16 init_temp, UINT16 init_thre, UINT16 init_dist);
extern void manual_cal_tmp_pwr_init_reg(UINT16 reg_mod, UINT16 reg_pa);
extern void manual_cal_temp_pwr_unint(void);
extern void manual_cal_set_tmp_pwr_flag(UINT8 flag);
extern TMP_PWR_PTR manual_cal_set_tmp_pwr(UINT16 cur_val, UINT16 thre, UINT16 *last);
extern UINT32 manual_cal_load_temp_tag_flash(void);
extern UINT32 manual_cal_load_adc_cali_flash(void);
extern void manual_cal_do_single_temperature(void);

extern void rwnx_cal_set_reg_mod_pa(UINT16 reg_mod, UINT16 reg_pa);
extern void rwnx_cal_do_temp_detect(UINT16 cur_val, UINT16 thre, UINT16 *last);
extern void rwnx_cal_set_lpfcap_iq(UINT32 lpfcap_i, UINT32 lpfcap_q);
extern void rwnx_cal_set_40M_extra_setting(UINT8 val);
extern void rwnx_cal_set_40M_setting(void);

extern void rwnx_cal_set_txpwr_for_ble_boardcast(void);
extern void rwnx_cal_recover_txpwr_for_wifi(void);

extern void rwnx_cal_initial_calibration(void);

extern UINT32 rwnx_tpc_pwr_idx_translate(UINT32 pwr_gain, UINT32 rate, UINT32 print_log );
extern UINT32 rwnx_tpc_get_pwridx_by_rate(UINT32 rate, UINT32 print_log);
extern void rwnx_use_tpc_set_pwr(void);
extern void rwnx_no_use_tpc_set_pwr(void);
extern UINT32 rwnx_is_tpc_bit_on(void);
extern UINT32 rwnx_sys_is_enable_hw_tpc(void);
extern void bk7011_set_rf_config_tssithred(int tssi_thred);
extern int bk7011_is_rfcali_mode_auto(void);
extern void bk7011_set_rfcali_mode(int mode);
extern void bk7011_cal_dcormod_show(void);

extern void rwnx_cal_ble_set_rfconfig(void);
extern void rwnx_cal_ble_recover_rfconfig(void);

extern void manual_cal_set_setp0(void);
extern void manual_cal_set_setp1(void);
extern void manual_cal_clear_setp(void);
extern void manual_cal_set_rfcal_step0(void);
extern int manual_cal_rfcali_status(void);
extern UINT32 manual_cal_check_pwr_idx(UINT32 *level);
extern UINT32 manual_cal_is_in_rftest_mode(void);

extern void rwnx_cal_en_extra_txpa(void);
extern void rwnx_cal_dis_extra_txpa(void);

extern int manual_cal_save_cailmain_tx_tab_to_flash(void);
extern int manual_cal_save_cailmain_rx_tab_to_flash(void);
extern int manual_cal_need_load_cmtag_from_flash(void);
extern int manual_set_cmtag(UINT32 status);
extern void do_all_calibration(void);

#endif // _BK7011_CAL_PUB_H_
