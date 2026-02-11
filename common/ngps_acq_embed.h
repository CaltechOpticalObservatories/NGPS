/**
 * @file    ngps_acq_embed.h
 * @brief   in-process API for NGPS auto-acquire logic
 */

#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ngps_acq_hooks {
  void *user;

  int (*tcs_set_native_units)( void *user, int dry_run, int verbose );
  int (*tcs_move_arcsec)( void *user, double dra_arcsec, double ddec_arcsec,
                          int dry_run, int verbose );
  int (*scam_putonslit_deg)( void *user,
                             double slit_ra_deg, double slit_dec_deg,
                             double cross_ra_deg, double cross_dec_deg,
                             int dry_run, int verbose );
  int (*acam_query_state)( void *user, char *state, size_t state_sz, int verbose );
  int (*scam_framegrab_one)( void *user, const char *outpath, int verbose );
  int (*scam_set_exptime)( void *user, double exptime_sec, int dry_run, int verbose );
  int (*scam_set_avgframes)( void *user, int avgframes, int dry_run, int verbose );
  int (*is_stop_requested)( void *user );
  void (*log_message)( void *user, const char *line );
} ngps_acq_hooks_t;

void ngps_acq_set_hooks( const ngps_acq_hooks_t *hooks );
void ngps_acq_clear_hooks( void );
void ngps_acq_request_stop( int stop_requested );
int ngps_acq_run_from_argv( int argc, char **argv );

#ifdef __cplusplus
}
#endif
