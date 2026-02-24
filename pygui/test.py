from astropy.coordinates import SkyCoord, EarthLocation
from astropy.time import Time
from astroplan import Observer
import astropy.units as u
def compute_parallactic_angle_astroplan(ra, dec, location=None, time=None):
    """
    Calculate the parallactic angle for a given RA and Dec using Astroplan.
    @param ra: Right Ascension as a space-separated string (e.g., "23 08 44.55").
    @param dec: Declination as a space-separated string (e.g., "+36 22 12.90").
    @param location: Observatory location (Astropy EarthLocation). Defaults to Palomar Observatory.
    @param time: Observation time (Astropy Time). Defaults to current UTC time.
    @return: Parallactic Angle (Astropy Quantity, angle with unit)
    """
    # Default location: Palomar Observatory
    if location is None:
        location = EarthLocation(lat=33.3563 * u.deg, lon=-116.8648 * u.deg, height=1706 * u.m)
    observer = Observer(location=location, name="Observer", timezone="UTC")
    # Current UTC time
    if time is None:
        time = Time.now()
    ra = ra.replace(" ", "h", 1).replace(" ", "m", 1) + "s"  
    dec = dec.replace(" ", "d", 1).replace(" ", "m", 1) + "s" 
    target_coords = SkyCoord(ra=ra, dec=dec, frame='icrs')
    parallactic_angle = observer.parallactic_angle(time, target_coords)
    print(f"PA: {parallactic_angle}") 
    return parallactic_angle

# Example input:
ra="00 00 00.86"
dec="+40 00 17.68"
#ra = "23 08 44.55"
#dec = "+36 22 12.90"
pa = compute_parallactic_angle_astroplan(ra, dec)
print(f"Type PA: {type(pa)}")
print(f"Parallactic Angle: {pa.to(u.deg):.2f}")
