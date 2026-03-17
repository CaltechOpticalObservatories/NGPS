/** ---------------------------------------------------------------------------
 * @file    slicecam_math.cpp
 * @brief   slicecam math utilities implementation for fine target acquisition
 * @details Implements centroid detection, WCS pixel-to-sky conversion, and
 *          fine-acquisition offset calculation for the slicecam fine-acquire
 *          loop.
 * @author  David Hale, Christoffer Fremling
 *
 */

#include "slicecam_math.h"

#include <algorithm>
#include <cmath>
#include <vector>

namespace Slicecam {

  /***** Slicecam::Math::calculate_centroid ************************************/
  /**
   * @brief      compute the centroid of the brightest source near an aim point
   * @details    Translated from CF's ngps_acq.c. Uses MAD background estimation,
   *             peak finding, and iterative Gaussian centroiding.
   * @param[in]  image       pointer to row-major camera image buffer (cols * rows elements)
   * @param[in]  cols        number of image columns
   * @param[in]  rows        number of image rows
   * @param[in]  background  ROI for background estimation (1-based, inclusive)
   * @param[in]  aimpoint    pixel search centre (1-based)
   * @param[out] centroid    detected centroid position (1-based on success)
   * @return     NO_ERROR|ERROR
   *
   */
  long Math::calculate_centroid( float* image,
                                  long ncols, long nrows,
                                  Rect background,
                                  Point aimpoint,
                                  Point &centroid ) {
    if ( !image || ncols <= 0 || nrows <= 0 ) return ERROR;

    // Convert 1-based inclusive ROI to 0-based, clamped to image boundaries.
    long bx1 = std::max( 0L, background.x1 - 1 );
    long bx2 = std::min( ncols - 1L, background.x2 - 1 );
    long by1 = std::max( 0L, background.y1 - 1 );
    long by2 = std::min( nrows - 1L, background.y2 - 1 );

    if ( bx2 < bx1 || by2 < by1 ) return ERROR;

    // estimate background and noise via sigma-clipped median/MAD
    std::vector<float> samples;
    samples.reserve( static_cast<size_t>( (bx2 - bx1 + 1) * (by2 - by1 + 1) ) );

    for ( long y = by1; y <= by2; y++ ) {
      for ( long x = bx1; x <= bx2; x++ ) {
        samples.push_back( image[y * ncols + x] );
      }
    }

    if ( samples.size() < 16 ) return ERROR;

    std::sort( samples.begin(), samples.end() );

    const size_t n = samples.size();

    double median = ( n % 2 )
                  ? static_cast<double>( samples[n / 2] )
                  : 0.5 * ( static_cast<double>( samples[n / 2 - 1] )
                           + static_cast<double>( samples[n / 2] ) );

    // MAD gives a robust initial sigma estimate: sigma ~= 1.4826 * MAD
    std::vector<float> dev( n );
    for ( size_t i = 0; i < n; i++ ) {
      dev[i] = std::abs( samples[i] - static_cast<float>( median ) );
    }
    std::sort( dev.begin(), dev.end() );

    double mad = ( n % 2 )
               ? static_cast<double>( dev[n / 2] )
               : 0.5 * ( static_cast<double>( dev[n / 2 - 1] )
                        + static_cast<double>( dev[n / 2] ) );
    double sigma = 1.4826 * mad;
    if ( !std::isfinite( sigma ) || sigma <= 0.0 ) sigma = 1.0;

    // refine background with iterative sigma clipping (5 passes, 3-sigma clip).
    double bkg            = median;
    const double clip_sig = 3.0;

    for ( int it = 0; it < 5; it++ ) {
      const double lo = bkg - clip_sig * sigma;
      const double hi = bkg + clip_sig * sigma;

      double sum  = 0.0;
      double sum2 = 0.0;
      long   cnt  = 0;

      for ( float v : samples ) {
        if ( v < lo || v > hi ) continue;
        const double dv = static_cast<double>( v );
        sum  += dv;
        sum2 += dv * dv;
        cnt++;
      }

      if ( cnt < 2 ) break;

      bkg   = sum / cnt;
      sigma = std::sqrt( sum2 / cnt - bkg * bkg );
      if ( !std::isfinite( sigma ) || sigma <= 0.0 ) { sigma = 1.0; break; }
    }

    // locate the brightest pixel above threshold near the aim point
    //
    const double snr_threshold   = 3.0;    ///< minimum SNR for detection
    const double search_radius   = 40.0;   ///< search radius in pixels

    const double detection_level = bkg + snr_threshold * sigma;

    // convert 1-based aim point to 0-based for array indexing.
    const double aim_x0 = aimpoint.x - 1.0;
    const double aim_y0 = aimpoint.y - 1.0;

    long sx1 = std::max( 0L,         static_cast<long>( aim_x0 - search_radius ) );
    long sx2 = std::min( ncols - 1L, static_cast<long>( aim_x0 + search_radius ) );
    long sy1 = std::max( 0L,         static_cast<long>( aim_y0 - search_radius ) );
    long sy2 = std::min( nrows - 1L, static_cast<long>( aim_y0 + search_radius ) );

    double best_val = detection_level;
    long   best_x   = -1;
    long   best_y   = -1;

    for ( long y = sy1; y <= sy2; y++ ) {
      for ( long x = sx1; x <= sx2; x++ ) {
        const double dx = static_cast<double>(x) - aim_x0;
        const double dy = static_cast<double>(y) - aim_y0;
        if ( dx * dx + dy * dy > search_radius * search_radius ) continue;

        const double v = static_cast<double>( image[y * ncols + x] );
        if ( v > best_val ) {
          best_val = v;
          best_x   = x;
          best_y   = y;
        }
      }
    }

    if ( best_x < 0 ) return ERROR;   // no source found above threshold

    // iterative Gaussian-windowed first-moment centroid
    const int    centroid_halfwin  = 4;
    const double centroid_sigma    = 1.2;
    const double centroid_sigma_sq = centroid_sigma * centroid_sigma;

    double cx = static_cast<double>( best_x );
    double cy = static_cast<double>( best_y );

    for ( int it = 0; it < 20; it++ ) {
      long xlo = std::max( 0L,         static_cast<long>(cx) - centroid_halfwin );
      long xhi = std::min( ncols - 1L, static_cast<long>(cx) + centroid_halfwin );
      long ylo = std::max( 0L,         static_cast<long>(cy) - centroid_halfwin );
      long yhi = std::min( nrows - 1L, static_cast<long>(cy) + centroid_halfwin );

      double sumI = 0.0;
      double sumX = 0.0;
      double sumY = 0.0;

      for ( long y = ylo; y <= yhi; y++ ) {
        for ( long x = xlo; x <= xhi; x++ ) {
          const double I = static_cast<double>( image[y * ncols + x] ) - bkg;
          if ( I <= 0.0 ) continue;

          const double dx = static_cast<double>(x) - cx;
          const double dy = static_cast<double>(y) - cy;
          const double w  = std::exp( -0.5 * ( dx * dx + dy * dy ) / centroid_sigma_sq ) * I;

          sumI += w;
          sumX += w * static_cast<double>(x);
          sumY += w * static_cast<double>(y);
        }
      }

      if ( sumI <= 0.0 ) break;

      const double ncx   = sumX / sumI;
      const double ncy   = sumY / sumI;
      const double shift = std::hypot( ncx - cx, ncy - cy );
      cx = ncx;
      cy = ncy;

      if ( shift < 0.01 ) break;   // sub-hundredth pixel convergence
    }

    if ( !std::isfinite( cx ) || !std::isfinite( cy ) ) return ERROR;

    // return centroid in 1-based FITS pixel coordinates.
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
   *             This is exact for a linear WCS and an accurate approximation
   *             for the gnomonic (TAN) projection over a small field.
   * @param[in]  keys    FITS keyword database populated by collect_header_info
   * @param[in]  pix     pixel position (1-based)
   * @param[out] world   corresponding RA / DEC in degrees
   * @throws     Common::FitsKeys::get_key<T> can throw std::runtime_error
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
   * @brief      compute the (dRA, dDEC) offset from a goal position to a star
   * @details    Returns the angular offset (star - goal) which, when sent to
   *             the telescope, will move the star onto the goal position.
   *             The RA component is a true great-circle offset
   *             (multiplied by cos(dec)).
   * @param[in]  star    detected sky position of the star (degrees)
   * @param[in]  goal    desired sky position on the chip (degrees)
   * @param[out] offsets (dRA * cos(dec), dDEC) in degrees
   *
   */
  void Math::calculate_acquisition_offsets( World star, World goal,
                                             std::pair<double, double> &offsets ) {
    double dra = star.ra - goal.ra;

    // Wrap RA difference into [-180, +180] degrees.
    //
    while ( dra >  180.0 ) dra -= 360.0;
    while ( dra < -180.0 ) dra += 360.0;

    // Project onto the sky: multiply by cos(dec) so the RA offset is a
    // true angular separation rather than a coordinate difference.
    //
    const double cosdec = std::cos( goal.dec * M_PI / 180.0 );
    const double ddec   = star.dec - goal.dec;

    offsets = { dra * cosdec, ddec };
  }
  /***** Slicecam::Math::calculate_acquisition_offsets *************************/

}
