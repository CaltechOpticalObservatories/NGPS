package edu.caltech.palomar.util.simbad;

import java.util.Vector;
import java.util.Hashtable;

/**
 * Contains data returned by Simbad for a single astronomical
 * object.
 *
 * @author	Xiuqin Wu
**/
public class SimbadObject
{
	private String _name;
	private String _typeString = "";
	private String _morphology = null;
        private double _ra = Double.NaN;
        private double _dec = Double.NaN;
        private double _magnitude = Double.NaN;
        private String _magBand = "V";
        private double _redshift = Double.NaN;
        private double _radialVelocity = Double.NaN;
        private double _majorAxis = Double.NaN;
        private double _minorAxis = Double.NaN;

	/**
	 * Constructs a new SimbadObject initialized with empty values.
	**/
	public SimbadObject() 
	{
	   _name = "";
	   _typeString = "";
	   _morphology = "";
	   _ra = 0.0;
	   _dec = 0.0;
	   _magnitude = 0.0;
	   _magBand = "V";
	   _redshift = 0.0;
	   _radialVelocity = 0.0;
	   _majorAxis = 0.0;
	   _minorAxis = 0.0;
	}

	public String getName()
	{
		return _name;
	}

	public void setName(String name)
	{
		_name = name;
	}


	public String getType()
	{
		return _typeString;
	}

	public void setType(String type)
	{
		_typeString = type;
	}

	public String getMorphology()
	{
		return _morphology;
	}

	public void setMorphology(String morphology)
	{
		_morphology = morphology;
	}

	public double getRa()
	{
		return _ra;
	}

	public void setRa(double ra)
	{
		_ra = ra;
	}

	public double getDec()
	{
		return _dec;
	}

	public void setDec(double dec)
	{
		_dec = dec;
	}

	public double getMagnitude()
	{
		return _magnitude;
	}

	public void setMagnitude(double mag)
	{
		_magnitude = mag;
	}

	public String getMagBand()
	{
		return _magBand;
	}

	public void setMagBand(String mBand)
	{
		_magBand = mBand;
	}

	public double getRedshift()
	{
		return _redshift;
	}

	public void setRedshift(double redshift)
	{
		_redshift = redshift;
	}

	public double getRadialVelocity()
	{
		return _radialVelocity;
	}

	public void setRadialVelocity(double radialVelocity)
	{
		_radialVelocity = radialVelocity;
	}
	public double getMajorAxis()
	{
		return _majorAxis;
	}

	public void setMajorAxis(double majorAxis)
	{
		_majorAxis = majorAxis;
	}
	public double getMinorAxis()
	{
		return _minorAxis;
	}

	public void setMinorAxis(double minorAxis)
	{
		_minorAxis = minorAxis;
	}
}


