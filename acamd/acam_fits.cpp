#include "acam_fits.h"

namespace Acam {


  /***** Acam::FITS_file::FITS_file *******************************************/
  /**
   * @brief      class constructor
   * @details    instantiated with existing Information object
   * @param[in]  info  Information object
   *
   */
/*
  FITS_file::FITS_file( FitsInfo info ) {
    this->info = info;
  }
*/
  /***** Acam::FITS_file::FITS_file *******************************************/


  /***** Acam::FITS_file::open_file *******************************************/
  /**
   * @brief      
   *
   */
  long FITS_file::open_file() {
    std::string function = "Acam::FITS_file::open_file";
    std::stringstream message;

    int num_axis=2;
    long axes[2];
    axes[0] = (long)this->info.axes[0];
    axes[1] = (long)this->info.axes[1];

    // If CCFits gets a zero then it will crash when you call pHDU().write()
    //
    if ( axes[0] * axes[1] == 0 ) {
      message.str(""); message << "ERROR creating \"" << this->info.fits_name
                               << "\": axes[0]=" << axes[0] << " axes[1]=" << axes[1]
                               << " must be non-zero";
      logwrite( function, message.str() );
      return( ERROR );
    }

    // Check that we can write the file, because CCFits will crash if it cannot
    //
    std::ofstream checkfile ( this->info.fits_name.c_str() );
    if ( checkfile.is_open() ) {
      checkfile.close();
      std::remove( this->info.fits_name.c_str() );
    }
    else {
      message.str(""); message << "ERROR unable to create file \"" << this->info.fits_name << "\"";
      logwrite(function, message.str());
      return(ERROR);
    }

    try {
      // Create a new FITS object, specifying the data type and axes for the primary image.
      // Simultaneously create the corresponding file.
      //
      this->pFits.reset( new CCfits::FITS(this->info.fits_name, this->info.datatype, num_axis, axes) );
    }
    catch ( CCfits::FITS::CantCreate &err ) {
      message.str(""); message << "ERROR: unable to open FITS file \"" << this->info.fits_name << "\": "
                               << err.message();
      logwrite(function, message.str());
      return(ERROR);
    }
    catch ( ... ) {
      message.str(""); message << "unknown error opening FITS file \"" << this->info.fits_name << "\"";
      logwrite(function, message.str());
      return(ERROR);
    }

    this->fits_name = this->info.fits_name;
    this->file_open = true;

    return(NO_ERROR);
  }
  /***** Acam::FITS_file::open_file *******************************************/


  /***** Acam::FITS_file::close_file ******************************************/
  /**
   * @brief      closes fits file
   *
   * Before closing the file, DATE and CHECKSUM keywords are added.
   * Nothing called returns anything so this doesn't return anything.
   *
   */
  void FITS_file::close_file() {
    std::string function = "Acam::FITS_file::close_file";
    std::stringstream message;

    // The file must have been opened first
    //
    if ( !this->file_open ) {
      message.str(""); message << "ERROR no FITS file open";
      logwrite( function, message.str() );
      return;
    }

    try {
      // Write the checksum
      //
      this->pFits->pHDU().writeChecksum();

      // Deallocate the CCfits object and close the FITS file
      //
      this->pFits->destroy();
    }
    catch ( CCfits::FitsError &error ) {
      message.str(""); message << "ERROR writing checksum and closing file: " << error.message();
      logwrite( function, message.str() );
    }

    this->file_open = false;

    return;
  }
  /***** Acam::FITS_file::close_file ******************************************/


  /***** Acam::FITS_file::copy_header_from ************************************/
  /**
   * @brief      
   *
   */
  long FITS_file::copy_header_from( std::string file_in ) {
    std::string function = "Acam::FITS_file::copy_header_from";
    std::stringstream message;

logwrite( function, file_in );

    // The file must have been opened first
    //
    if ( !this->file_open ) {
      message.str(""); message << "ERROR no FITS file open";
      logwrite( function, message.str() );
      return( ERROR );
    }

    try {
      struct stat st;
      if ( stat( file_in.c_str(), &st ) != 0 ) {
        message.str(""); message << "ERROR \"" << file_in << "\" does not exist";
        logwrite( function, message.str() );
        return( ERROR );
      }
      std::unique_ptr<CCfits::FITS> pInfile( new CCfits::FITS( file_in.c_str(), CCfits::Read, true ) );

      pInfile->pHDU().readAllKeys();

      std::vector<int> categories = { TYP_STRUC_KEY, TYP_SCAL_KEY, TYP_WCS_KEY, TYP_REFSYS_KEY, TYP_COMM_KEY, TYP_USER_KEY };

      pFits->pHDU().copyAllKeys( &pInfile->pHDU(), categories );
    }
    catch ( CCfits::FitsError &error ) {
      message.str(""); message << "file_in: " << file_in << " FITS file error thrown: " << error.message();
      logwrite(function, message.str());
      return( ERROR );
    }

    return( NO_ERROR );
  }
  /***** Acam::FITS_file::copy_header_from ************************************/


  long FITS_file::create_header() {
    std::string function = "Acam::FITS_file::create_header";
    std::stringstream message;

    // The file must have been opened first
    //
    if ( !this->file_open ) {
      message.str(""); message << "ERROR no FITS file open";
      logwrite( function, message.str() );
      return( ERROR );
    }

    for ( const auto &keydb : this->info.fitskeys.keydb ) {
      this->add_key( keydb.second.keyword, keydb.second.keytype, keydb.second.keyvalue, keydb.second.keycomment );
    }

    this->pFits->pHDU().addKey( "CREATOR", "acamd", "file creator" );

    return( NO_ERROR );
  }


//long FITS_file::write_image( uint16_t* data ) {
  long FITS_file::write_image( float* data ) {
    std::string function = "Acam::FITS_file::write_image";
    std::stringstream message;

    // The file must have been opened first
    //
    if ( !this->file_open ) {
      message.str(""); message << "ERROR no FITS file open";
      logwrite( function, message.str() );
      return( ERROR );
    }

    if ( this->info.section_size == 0 ) {
      logwrite( function, "ERROR size is 0" );
      return( ERROR );
    }

    if ( data == nullptr ) {
      logwrite( function, "ERROR missing data buffer" );
      return( ERROR );
    }

    // Set the FITS system to verbose mode so it writes error messages
    //
    CCfits::FITS::setVerboseMode( true );

    // write the primary image into the FITS file
    //
    try {
//    std::valarray<uint16_t> array( data, this->info.section_size );
//        std::valarray<uint16_t> array( this->info.section_size );
          std::valarray<float> array( this->info.section_size );
          for ( unsigned long i=0; i < this->info.section_size; i++ ) array[i] = data[i];

      long fpixel(1);        // start with the first pixel always
      this->pFits->pHDU().write( fpixel, this->info.section_size, array );
      this->pFits->flush();  // make sure the image is written to disk
    }
    catch ( CCfits::FitsError &error ) {
      message.str(""); message << "FITS file error thrown: " << error.message();
      logwrite(function, message.str());
      return( ERROR );
    }

    return( NO_ERROR );
  }

  /***** Acam::FITS_file::add_key *********************************************/
  long FITS_file::add_key( std::string keyword, std::string type, std::string value, std::string comment ) {
    std::string function = "Acam::FITS_file::add_key";
    std::stringstream message;

    const std::lock_guard<std::mutex> lock( this->fits_mutex );

    // The file must have been opened first
    //
    if ( !this->file_open ) {
      message.str(""); message << "ERROR: adding key " << keyword << "=" << value << ": no FITS file open";
      logwrite( function, message.str() );
      return( ERROR );
    }

    try {
      if ( value.find( "nan" ) != std::string::npos  ) {
        this->pFits->pHDU().addKey( keyword, "NAN", "" );
      }
      else if ( type == "BOOL") {
        bool boolvalue = ( value == "T" ? true : false );
        this->pFits->pHDU().addKey( keyword, boolvalue, comment );
      }
      else if ( type == "INT") {
        this->pFits->pHDU().addKey(keyword, std::stoi(value), comment);
      }
      else if ( type == "LONG") {
        this->pFits->pHDU().addKey(keyword, std::stol(value), comment);
      }
      else if ( type == "FLOAT") {
        this->pFits->pHDU().addKey(keyword, std::stof(value), comment);
      }
      else if ( type == "DOUBLE") {
        this->pFits->pHDU().addKey(keyword, std::stod(value), comment);
      }
      else if ( type == "STRING") {
        this->pFits->pHDU().addKey(keyword, value, comment);
      }
      else {
        message.str(""); message << "ERROR unknown type: " << type << " for user keyword: " << keyword << "=" << value
                                 << ": expected {INT,LONG,DOUBLE,FLOAT,STRING,BOOL}";
        logwrite(function, message.str());
      }
    }
    // There could be an error converting a value to INT or DOUBLE with stoi or stof,
    // in which case save the keyword as a STRING.
    //
    catch ( std::invalid_argument & ) {
      message.str(""); message << "ERROR: unable to convert value " << value;
      logwrite( function, message.str() );
      if ( type != "STRING") {
        this->pFits->pHDU().addKey(keyword, value, comment);
      }
    }
    catch ( std::out_of_range & ) {
      message.str(""); message << "ERROR: value " << value << " out of range";
      logwrite( function, message.str() );
      if ( type != "STRING") {
        this->pFits->pHDU().addKey(keyword, value, comment);
      }
    }
    catch ( CCfits::FitsError &err ) {
      message.str(""); message << "ERROR adding key " << keyword << "=" << value << " / " << comment << " (" << type << "): "
                               << err.message();
      logwrite(function, message.str());
    }
    return( NO_ERROR );
  }
  /***** Acam::FITS_file::add_key *********************************************/


}
