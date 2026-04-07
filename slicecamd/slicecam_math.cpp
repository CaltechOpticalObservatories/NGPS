/** ---------------------------------------------------------------------------
 * @file    slicecam_math.cpp
 * @brief   slicecam math utilities implementation for fine target acquisition
 * @details Implements centroid detection, WCS pixel-to-sky conversion, and
 *          fine-acquisition offset calculation for the slicecam fine-acquire
 *          loop. Direct C++ translation of CF's ngps_acq.c.
 * @author  David Hale, Christoffer Fremling
 *
 */

#include "slicecam_math.h"

#include <algorithm>
#include <cmath>
#include <vector>
#include <queue>

namespace Slicecam {

  // ---------------------------------------------------------------------------
  // Internal helpers (file scope only)
  // ---------------------------------------------------------------------------

  /**
   * @brief      Build a normalized 1-D Gaussian kernel, half-radius = ceil(3*sigma).
   * @param[in]  sigma       Gaussian sigma in pixels (clamped to >= 0.2)
   * @param[out] radius_out  half-radius r; full kernel length = 2r+1
   */
  static std::vector<double> make_gaussian_kernel( double sigma, int &radius_out ) {
    if ( sigma < 0.2 ) sigma = 0.2;
    const int r = std::max( 1, static_cast<int>( std::ceil( 3.0 * sigma ) ) );
    const int len = 2 * r + 1;
    std::vector<double> k( len );
    double sum = 0.0;
    for ( int i = -r; i <= r; i++ ) {
      const double x = static_cast<double>(i) / sigma;
      k[i + r] = std::exp( -0.5 * x * x );
      sum += k[i + r];
    }
    if ( sum <= 0.0 ) sum = 1.0;
    for ( auto &v : k ) v /= sum;
    radius_out = r;
    return k;
  }

  /**
   * @brief      Sum of squares of 1-D kernel coefficients.
   *         For a separable 2-D Gaussian: sigma_filt = sigma_raw * kernel_sum_sq.
   */
  static double kernel_sum_sq( const std::vector<double> &k ) {
    double s = 0.0;
    for ( auto v : k ) s += v * v;
    return s;
  }

  /**
   * @brief      Separable 2-D Gaussian convolution on a (w x h) patch.
   *         Border handling: clamp (mirror-pad would be better but matches CF).
   */
  static void convolve_separable( const std::vector<float> &in,
                                  std::vector<float> &tmp,
                                  std::vector<float> &out,
                                  int w, int h,
                                  const std::vector<double> &k, int r ) {
    // horizontal pass: in -> tmp
    for ( int y = 0; y < h; y++ ) {
      for ( int x = 0; x < w; x++ ) {
        double acc = 0.0;
        for ( int dx = -r; dx <= r; dx++ ) {
          const int xx = std::max( 0, std::min( w - 1, x + dx ) );
          acc += static_cast<double>( in[y * w + xx] ) * k[dx + r];
        }
        tmp[y * w + x] = static_cast<float>( acc );
      }
    }
    // vertical pass: tmp -> out
    for ( int y = 0; y < h; y++ ) {
      for ( int x = 0; x < w; x++ ) {
        double acc = 0.0;
        for ( int dy = -r; dy <= r; dy++ ) {
          const int yy = std::max( 0, std::min( h - 1, y + dy ) );
          acc += static_cast<double>( tmp[yy * w + x] ) * k[dy + r];
        }
        out[y * w + x] = static_cast<float>( acc );
      }
    }
  }

  /**
   * @brief  SExtractor-like background and sigma estimation.
   * @details Translation of CF's bg_sigma_sextractor_like().
   *          1. Initial estimate: median and MAD-derived sigma.
   *          2. Iterative 3-sigma clipping around the original median,
   *             with early exit when sigma converges (rel change < 1%).
   *          3. Background via mode = 2.5*median - 1.5*mean,
   *             falling back to median if distribution is skewed.
   */
  static void bg_sigma( const std::vector<float> &samples,
                        double &bkg_out, double &sigma_out ) {
    bkg_out   = 0.0;
    sigma_out = 1.0;

    const size_t ns = samples.size();
    if ( ns < 64 ) return;

    // samples must be sorted on entry
    std::vector<float> sorted = samples;
    std::sort( sorted.begin(), sorted.end() );

    const double median = ( ns % 2 )
                        ? static_cast<double>( sorted[ns / 2] )
                        : 0.5 * ( static_cast<double>( sorted[ns / 2 - 1] )
                                + static_cast<double>( sorted[ns / 2] ) );

    // MAD -> initial sigma
    std::vector<float> dev( ns );
    for ( size_t i = 0; i < ns; i++ )
      dev[i] = std::abs( sorted[i] - static_cast<float>( median ) );
    std::sort( dev.begin(), dev.end() );

    const double mad = ( ns % 2 )
                     ? static_cast<double>( dev[ns / 2] )
                     : 0.5 * ( static_cast<double>( dev[ns / 2 - 1] )
                              + static_cast<double>( dev[ns / 2] ) );
    double sigma = 1.4826 * mad;
    if ( !std::isfinite( sigma ) || sigma <= 0.0 ) sigma = 1.0;

    // Iterative 3-sigma clipping, always centered on original median
    double mean      = median;
    double sigma_prev = sigma;

    for ( int it = 0; it < 8; it++ ) {
      const double lo = median - 3.0 * sigma;
      const double hi = median + 3.0 * sigma;
      double sum = 0.0, sum2 = 0.0;
      long cnt = 0;
      for ( float v : sorted ) {
        if ( v < lo || v > hi ) continue;
        const double dv = static_cast<double>( v );
        sum  += dv;
        sum2 += dv * dv;
        cnt++;
      }
      if ( cnt < 32 ) break;

      mean         = sum / static_cast<double>( cnt );
      const double var = ( sum2 / static_cast<double>( cnt ) ) - mean * mean;
      sigma        = ( var > 0.0 ) ? std::sqrt( var ) : 0.0;
      if ( !std::isfinite( sigma ) || sigma <= 0.0 ) { sigma = sigma_prev; break; }

      const double rel = std::abs( sigma - sigma_prev )
                       / ( sigma_prev > 0.0 ? sigma_prev : 1.0 );
      sigma_prev = sigma;
      if ( rel < 0.01 ) break;
    }

    // SExtractor mode estimator: 2.5*median - 1.5*mean
    // Fall back to median if distribution is too skewed
    double bkg = 2.5 * median - 1.5 * mean;
    if ( sigma > 0.0 && ( mean - median ) / sigma > 0.3 ) bkg = median;
    if ( !std::isfinite( bkg ) ) bkg = median;
    if ( !std::isfinite( sigma ) || sigma <= 0.0 ) sigma = 1.0;

    bkg_out   = bkg;
    sigma_out = sigma;
  }


  /***** Slicecam::Math::calculate_centroid ************************************/
  /**
   * @brief      compute the centroid of the brightest source near an aim point
   * @details    Direct C++ translation of CF's detect_star_near_goal() in
   *             ngps_acq.c, with parameter defaults matching AUTOACQ_ARGS.
   *
   *             Step 1: SExtractor-like background and sigma estimation over
   *             the background ROI.
   *
   *             Step 2: extract the search ROI (= background ROI), background-
   *             subtract, and apply separable Gaussian smoothing
   *             (filt_sigma_pix = 1.2).
   *
   *             Step 3: scan the filtered patch for local maxima that exceed
   *             the SNR threshold, have >= 4 adjacent pixels above threshold
   *             in the raw residual, and are within 40 pixels of the aim point.
   *             Among all qualifying candidates, select the brightest.
   *
   *             Step 4: refine to sub-pixel centroid via iterative Gaussian-
   *             windowed first-moment (centroid_sigma_pix = 2.0, 12 iterations,
   *             eps = 0.01 px).
   *
   *             All pixel coordinates are FITS 1-based on input and output.
   *             Internally everything is 0-based.
   *
   */
  long Math::calculate_centroid( const std::vector<float> &image,
                                  long ncols, long nrows,
                                  Rect background,
                                  Point aimpoint,
                                  Point &centroid ) {
    if ( image.empty() || ncols <= 0 || nrows <= 0 ) return ERROR;

    // Convert 1-based inclusive ROI to 0-based, clamped
    const long bx1 = std::max( 0L, background.x1 - 1 );
    const long bx2 = std::min( ncols - 1L, background.x2 - 1 );
    const long by1 = std::max( 0L, background.y1 - 1 );
    const long by2 = std::min( nrows - 1L, background.y2 - 1 );

    if ( bx2 < bx1 || by2 < by1 ) return ERROR;

    // Search ROI = background ROI (no separate search ROI configured)
    const long sx1 = bx1, sx2 = bx2, sy1 = by1, sy2 = by2;

    // --- Step 1: background and sigma estimation ---
    std::vector<float> samples;
    samples.reserve( static_cast<size_t>( (bx2 - bx1 + 1) * (by2 - by1 + 1) ) );
    for ( long y = by1; y <= by2; y++ )
      for ( long x = bx1; x <= bx2; x++ )
        samples.push_back( image[y * ncols + x] );

    double bkg = 0.0, sigma = 1.0;
    bg_sigma( samples, bkg, sigma );

    if ( !std::isfinite( sigma ) || sigma <= 0.0 ) return ERROR;

    // --- Step 2: background-subtract the search patch and smooth ---
    const int w = static_cast<int>( sx2 - sx1 + 1 );
    const int h = static_cast<int>( sy2 - sy1 + 1 );
    if ( w <= 3 || h <= 3 ) return ERROR;

    std::vector<float> patch( w * h );
    std::vector<float> tmp(   w * h );
    std::vector<float> filt(  w * h );

    for ( int yy = 0; yy < h; yy++ ) {
      const long y = sy1 + yy;
      for ( int xx = 0; xx < w; xx++ ) {
        const long x = sx1 + xx;
        // keep negatives — filter uses them too (matches CF)
        patch[yy * w + xx] = static_cast<float>(
            static_cast<double>( image[y * ncols + x] ) - bkg );
      }
    }

    // filt_sigma_pix = 1.2 (CF default)
    int kr = 0;
    const std::vector<double> kernel = make_gaussian_kernel( 1.2, kr );
    convolve_separable( patch, tmp, filt, w, h, kernel, kr );

    const double sumsq1d    = kernel_sum_sq( kernel );
    const double sigma_filt = ( sumsq1d > 0.0 ) ? sigma * sumsq1d : sigma;

    // SNR thresholds (snr_thresh = 3.0, CF default)
    const double thr_filt = 3.0 * sigma_filt;  // threshold in filtered image
    const double thr_raw  = 3.0 * sigma;        // threshold for adjacency check

    // Aim point in 0-based image coordinates
    const double goal_x0 = aimpoint.x - 1.0;
    const double goal_y0 = aimpoint.y - 1.0;
    const double max_dist = 40.0;  // pixels; CF's --max-dist default

    // --- Step 3: find best local maximum in the filtered patch ---
    double best_val = -1.0e300;
    long   best_x   = -1;
    long   best_y   = -1;

    // skip border pixels (yy=0, yy=h-1, xx=0, xx=w-1) — local max test
    // requires all 4 neighbours to exist
    for ( int yy = 1; yy < h - 1; yy++ ) {
      for ( int xx = 1; xx < w - 1; xx++ ) {
        const float v = filt[yy * w + xx];

        // must exceed detection threshold in filtered image
        if ( static_cast<double>( v ) < thr_filt ) continue;

        // must be a local maximum in the filtered image (4-connected)
        if ( v < filt[ yy      * w + (xx - 1)] ) continue;
        if ( v < filt[ yy      * w + (xx + 1)] ) continue;
        if ( v < filt[(yy - 1) * w +  xx      ] ) continue;
        if ( v < filt[(yy + 1) * w +  xx      ] ) continue;

        // convert patch coordinates to full-image coordinates (0-based)
        const long x0 = sx1 + xx;
        const long y0 = sy1 + yy;

        // must be within max_dist pixels of the aim point
        const double dxg = static_cast<double>( x0 ) - goal_x0;
        const double dyg = static_cast<double>( y0 ) - goal_y0;
        if ( std::hypot( dxg, dyg ) > max_dist ) continue;

        // adjacency check in the raw residual patch: need >= 4 of 8 neighbours
        // above the raw threshold (rejects hot pixels and cosmic rays)
        int nadj = 0;
        for ( int dy = -1; dy <= 1; dy++ ) {
          for ( int dx = -1; dx <= 1; dx++ ) {
            if ( dx == 0 && dy == 0 ) continue;
            if ( static_cast<double>( patch[(yy + dy) * w + (xx + dx)] ) > thr_raw )
              nadj++;
          }
        }
        if ( nadj < 4 ) continue;  // min_adjacent = 4; CF's --min-adj default

        // rank by raw (not filtered) peak value — brightest candidate wins
        const double rawv = static_cast<double>( patch[yy * w + xx] );
        if ( rawv > best_val ) {
          best_val = rawv;
          best_x   = x0;
          best_y   = y0;
        }
      }
    }

    {
    std::ostringstream oss;
    oss << "[DEBUG] bkg=" << bkg << " sigma=" << sigma
        << " best_val=" << best_val << " best_x=" << best_x << " best_y=" << best_y;
    logwrite("Slicecam::Math::calculate_centroid", oss.str());
    }

    if ( best_x < 0 ) return ERROR;  // no source found

    // --- Step 4: iterative Gaussian-windowed first-moment centroid ---
    //
    // centroid_halfwin    = 4   (CF's --centroid-hw default)
    // centroid_sigma_pix  = 2.0 (CF's default, NOTE: different from filt_sigma)
    // centroid_maxiter    = 12  (CF's default)
    // centroid_eps_pix    = 0.01
    //
    const int    hw  = 4;
    const double s2  = 2.0 * 2.0;  // centroid_sigma_pix^2

    double cx   = static_cast<double>( best_x );
    double cy   = static_cast<double>( best_y );
    double sumI = 0.0;

    for ( int it = 0; it < 12; it++ ) {
      const long xlo = std::max( sx1, static_cast<long>( cx ) - hw );
      const long xhi = std::min( sx2, static_cast<long>( cx ) + hw );
      const long ylo = std::max( sy1, static_cast<long>( cy ) - hw );
      const long yhi = std::min( sy2, static_cast<long>( cy ) + hw );

      double sumX = 0.0, sumY = 0.0;
      sumI = 0.0;

      for ( long y = ylo; y <= yhi; y++ ) {
        for ( long x = xlo; x <= xhi; x++ ) {
          const double I = static_cast<double>( image[y * ncols + x] ) - bkg;
          if ( I <= 0.0 ) continue;
          const double dx  = static_cast<double>( x ) - cx;
          const double dy  = static_cast<double>( y ) - cy;
          const double wgt = std::exp( -0.5 * ( dx * dx + dy * dy ) / s2 ) * I;
          sumI += wgt;
          sumX += wgt * static_cast<double>( x );
          sumY += wgt * static_cast<double>( y );
        }
      }

      if ( sumI <= 0.0 ) break;

      const double ncx   = sumX / sumI;
      const double ncy   = sumY / sumI;
      const double shift = std::hypot( ncx - cx, ncy - cy );
      cx = ncx;
      cy = ncy;

      if ( shift < 0.01 ) break;  // centroid_eps_pix
    }

    if ( sumI <= 0.0 || !std::isfinite( cx ) || !std::isfinite( cy ) ) return ERROR;

    // Return in FITS 1-based coordinates
    centroid.x = cx + 1.0;
    centroid.y = cy + 1.0;

    return NO_ERROR;
  }
  /***** Slicecam::Math::calculate_centroid ************************************/


  /***** Slicecam::Math::pix2world *********************************************/
  /**
   * @brief      convert 1-based pixel coordinate to sky (RA/DEC) in degrees
   * @details    Applies the standard FITS affine WCS:
   *               u = pix.x - CRPIX1
   *               v = pix.y - CRPIX2
   *               world.ra  = CRVAL1 + CDELT1 * (PC1_1 * u + PC1_2 * v)
   *               world.dec = CRVAL2 + CDELT2 * (PC2_1 * u + PC2_2 * v)
   *             Valid for the gnomonic (TAN) projection over a small field.
   *             Throws if any required WCS key is absent.
   *
   */
  void Math::pix2world( const Common::FitsKeys &keys, Point pix, World &world ) {
    const double crpix1 = keys.get_key<double>( "CRPIX1" );
    const double crpix2 = keys.get_key<double>( "CRPIX2" );
    const double crval1 = keys.get_key<double>( "CRVAL1" );
    const double crval2 = keys.get_key<double>( "CRVAL2" );
    const double cdelt1 = keys.get_key<double>( "CDELT1" );
    const double cdelt2 = keys.get_key<double>( "CDELT2" );
    const double pc1_1  = keys.get_key<double>( "PC1_1"  );
    const double pc1_2  = keys.get_key<double>( "PC1_2"  );
    const double pc2_1  = keys.get_key<double>( "PC2_1"  );
    const double pc2_2  = keys.get_key<double>( "PC2_2"  );

    const double u = pix.x - crpix1;
    const double v = pix.y - crpix2;

    world.ra  = crval1 + cdelt1 * ( pc1_1 * u + pc1_2 * v );
    world.dec = crval2 + cdelt2 * ( pc2_1 * u + pc2_2 * v );
  }
  /***** Slicecam::Math::pix2world *********************************************/


  /***** Slicecam::Math::calculate_acquisition_offsets *************************/
  /**
   * @brief      compute (dRA*cos(dec), dDEC) offset to move star onto aim point
   * @details    Returns (star - goal) as a true on-sky angular offset in degrees.
   *             Applying this offset to the telescope pointing moves the star
   *             onto the aim point.
   *
   */
  void Math::calculate_acquisition_offsets( World star, World goal,
                                             std::pair<double, double> &offsets ) {
    double dra = star.ra - goal.ra;
    while ( dra >  180.0 ) dra -= 360.0;
    while ( dra < -180.0 ) dra += 360.0;

    const double cosdec = std::cos( goal.dec * M_PI / 180.0 );
    offsets = { dra * cosdec, star.dec - goal.dec };
  }
  /***** Slicecam::Math::calculate_acquisition_offsets *************************/

}
