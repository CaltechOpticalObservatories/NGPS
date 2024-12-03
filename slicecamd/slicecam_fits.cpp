#include "slicecam_fits.h"

namespace Slicecam {


  /***** Slicecam::FITS_file::open_file ***************************************/
  /**
   * @brief      
   *
   */
  long FITS_file::open_file() {
    std::string function = "Slicecam::FITS_file::open_file";
    std::stringstream message;

    // all slicecam fits files are multi-extension,
    // no data in primary header
    //
    int num_axis=0;
    long axes[2] = { 0, 0 };

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
      return ERROR;
    }

    try {
      // Create a new FITS object, specifying the data type and axes for the primary image.
      // Simultaneously create the corresponding file.
      //
      this->pFits.reset( new CCfits::FITS(this->info.fits_name, this->info.datatype, num_axis, axes) );
    }
    catch ( CCfits::FITS::CantCreate &err ){
      message.str(""); message << "ERROR: unable to open FITS file \"" << this->info.fits_name << "\": "
                               << err.message();
      logwrite(function, message.str());
      return ERROR;
    }
    catch ( std::exception &e ) {
      message.str(""); message << "ERROR opening FITS file \"" << this->info.fits_name << "\": << e.what()";
      logwrite(function, message.str());
      return ERROR;
    }
    catch ( ... ) {
      message.str(""); message << "ERROR opening FITS file \"" << this->info.fits_name << "\"";
      logwrite(function, message.str());
      return ERROR;
    }

    this->fits_name = this->info.fits_name;
    this->file_open = true;

    return NO_ERROR;
  }
  /***** Slicecam::FITS_file::open_file ***************************************/


  /***** Slicecam::FITS_file::close_file **************************************/
  /**
   * @brief      closes fits file
   *
   * Before closing the file, DATE and CHECKSUM keywords are added.
   * Nothing called returns anything so this doesn't return anything.
   *
   */
  void FITS_file::close_file() {
    std::string function = "Slicecam::FITS_file::close_file";
    std::stringstream message;

    // The file must have been opened first
    //
    if ( !this->file_open ) {
      message.str(""); message << "ERROR no FITS file open";
      logwrite( function, message.str() );
      return;
    }

    try {
      // Add a header keyword for the time the file was written (right now!)
      //
      this->pFits->pHDU().addKey( "DATE", get_timestamp(), "FITS file write time" );

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
  /***** Slicecam::FITS_file::close_file **************************************/


  /***** Slicecam::FITS_file::copy_header_from ********************************/
  /**
   * @brief      
   *
   */
  long FITS_file::copy_header_from( std::string file_in ) {
    std::string function = "Slicecam::FITS_file::copy_header_from";
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
  /***** Slicecam::FITS_file::copy_header_from ********************************/


  /***** Slicecam::FITS_file::create_header ***********************************/
  /**
   * @brief      
   *
   */
  long FITS_file::create_header() {
    std::string function = "Slicecam::FITS_file::create_header";
    std::stringstream message;

    // The file must have been opened first
    //
    if ( !this->file_open ) {
      message.str(""); message << "ERROR no FITS file open";
      logwrite( function, message.str() );
      return( ERROR );
    }

    this->pFits->pHDU().addKey( "CREATOR", "slicecamd", "file creator" );

    return NO_ERROR;
  }
  /***** Slicecam::FITS_file::create_header ***********************************/


  /***** Slicecam::FITS_file::write_image *************************************/
  /**
   * @brief      
   *
   */
  long FITS_file::write_image( std::unique_ptr<Andor::Interface> &slicecam ) {
    std::string function = "Slicecam::FITS_file::write_image";
    std::stringstream message;

    // The file must have been opened first
    //
    if ( !this->file_open ) {
      logwrite( function, "ERROR no FITS file open" );
      return ERROR;
    }

    if ( slicecam->camera_info.section_size == 0 ) {
      logwrite( function, "ERROR slicecam " + slicecam->camera_info.camera_name +
                          " section size is 0" );
      return ERROR;
    }

    // Set the FITS system to verbose mode so it writes error messages
    //
    CCfits::FITS::setVerboseMode( true );

    const std::lock_guard<std::mutex> lock( this->fits_mutex );

    // write the primary image into the FITS file
    //
    try {
      auto data = slicecam->get_image_data();

      if ( data == nullptr ) {
        logwrite( function, "ERROR missing data buffer for slicecam " +
                             slicecam->camera_info.camera_name );
        return ERROR;
      }

      std::valarray<uint16_t> array( slicecam->camera_info.section_size );

      try {
        std::copy_n( data, slicecam->camera_info.section_size, &array[0] );
      }
      catch (const std::exception &e) {
        logwrite( function, "ERROR copying buffer: "+std::string(e.what()) );
        return ERROR;
      }


      long fpixel(1);              // start with the first pixel always

      // addImage() wants a vector, which has the size of the number of axes
      //
      std::vector<long> axes { slicecam->camera_info.axes[0],
                               slicecam->camera_info.axes[1] };

      // Create extension name
      //
      std::string extname = slicecam->camera_info.camera_name;

      // Add the extension here
      //
      this->imageExt = this->pFits->addImage( extname, this->info.datatype, axes );

      // Add the keywords here
      //
      for ( const auto &keydb : slicecam->fitskeys.keydb ) {
        this->add_key( keydb.second.keyword, keydb.second.keytype, keydb.second.keyvalue, keydb.second.keycomment );
      }

      // Write and flush to make sure imeage is written to disk
      //
      this->imageExt->write( fpixel, slicecam->camera_info.section_size, array );

      slicecam->erase_data_buffer();

      this->pFits->flush();  // make sure the image is written to disk
    }
    catch ( CCfits::FitsError &error ) {
      message.str(""); message << "FITS file error thrown: " << error.message();
      logwrite(function, message.str());
      return ERROR;
    }

    return NO_ERROR;
  }
  /***** Slicecam::FITS_file::write_image *************************************/


  /***** Slicecam::FITS_file::add_key *****************************************/
  /**
   * @brief      
   *
   */
  long FITS_file::add_key( std::string keyword, std::string type, std::string value, std::string comment ) {
    std::string function = "Slicecam::FITS_file::add_key";
    std::stringstream message;

    // The file must have been opened first
    //
    if ( !this->file_open ) {
      message.str(""); message << "ERROR: adding key " << keyword << "=" << value << ": no FITS file open";
      logwrite( function, message.str() );
      return ERROR;
    }

    try {
      if ( value.find( "nan" ) != std::string::npos  ) {
        this->imageExt->addKey( keyword, "NAN", "" );
      }
      else if ( type == "BOOL") {
        bool boolvalue = ( value == "T" ? true : false );
        this->imageExt->addKey( keyword, boolvalue, comment );
      }
      else if ( type == "INT") {
        this->imageExt->addKey(keyword, std::stoi(value), comment);
      }
      else if ( type == "LONG") {
        this->imageExt->addKey(keyword, std::stol(value), comment);
      }
      else if ( type == "FLOAT") {
        this->imageExt->addKey(keyword, std::stof(value), comment);
      }
      else if ( type == "DOUBLE") {
        this->imageExt->addKey(keyword, std::stod(value), comment);
      }
      else if ( type == "STRING") {
        this->imageExt->addKey(keyword, value, comment);
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
        this->imageExt->addKey(keyword, value, comment);
      }
    }
    catch ( std::out_of_range & ) {
      message.str(""); message << "ERROR: value " << value << " out of range";
      logwrite( function, message.str() );
      if ( type != "STRING") {
        this->imageExt->addKey(keyword, value, comment);
      }
    }
    catch ( CCfits::FitsError &err ) {
      message.str(""); message << "ERROR adding key " << keyword << "=" << value << " / " << comment << " (" << type << "): "
                               << err.message();
      logwrite(function, message.str());
    }

    return NO_ERROR;
  }
  /***** Slicecam::FITS_file::add_key *****************************************/


}
