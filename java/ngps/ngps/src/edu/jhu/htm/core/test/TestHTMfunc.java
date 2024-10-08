package edu.jhu.htm.core.test;



import junit.framework.*;
import edu.jhu.htm.core.*;

/** JUnitTest case for class: edu.jhu.htm.core.HTMfunc */
public class TestHTMfunc extends TestCase {

	public TestHTMfunc(String _name) {
	super(_name);
	}

	public static Test suite() {
		 return new TestSuite(TestHTMfunc.class);
	 }

	public void testNameToId10() throws Exception{
		long id = 16305926;	   // know the answer to this one
		String name = "N32030330012";
		long lid =HTMfunc.nameToId(name);
		assertEquals("Incorrect Id returned",id,lid);
	}

	public void testNameToId20() throws Exception{
		long id = 17098002819647L;	   // know the answer to this one
		String name = "N320303300120212220333";
		long lid =HTMfunc.nameToId(name);
		assertEquals("Incorrect Id returned",id,lid);
	}

	public void testLookupId10() throws Exception{
		double ra = 10;
		double dec = 10;
		long id = HTMfunc.lookupId(ra,dec,10);	   // lookup id by Vector3d
		assertEquals("Incorrect Id returned",16305926,id);
	}

	public void testLookupId20() throws Exception{
		double ra = 10;
		double dec = 10;
		long id = HTMfunc.lookupId(ra,dec,20);	   // lookup id by Vector3d
		assertEquals("Incorrect Id returned",17098002819647L,id);
	}

	public void testidToName() throws Exception{
		long id = 16305926;	   // know the answer to this one
		String name =HTMfunc.idToName(id);
		assertEquals("Incorrect Id returned","N32030330012",name);
	}

	public void testIdToName20() throws Exception{
		long id = 17098002819647L;	   // know the answer to this one
		String name =HTMfunc.idToName(id);
		assertEquals("Incorrect Id returned","N320303300120212220333",name);
	}


	public void testLookup10() throws Exception{
	double ra = 10;
	double dec = 10;
	int lev = 10;
	long id =0;
	long start = System.currentTimeMillis();
	String name  = HTMfunc.lookup(ra,dec,lev);	   // lookup id by Vector3d
	assertEquals("Incorrect Id returned","N32030330012",name);

	ra=45;dec=45 ;
	name = HTMfunc.lookup(ra,dec,lev);	   // lookup id by Vector3d
	assertEquals("Incorrect Id returned","N33313333303",name);

	ra=90;dec=-45;
	name = HTMfunc.lookup(ra,dec,lev);	   // lookup id by Vector3d
	assertEquals("Incorrect Id returned","S10102012001",name);

	ra=180;dec=85;
	name = HTMfunc.lookup(ra,dec,lev);	   // lookup id by Vector3d
	assertEquals("Incorrect Id returned","N12200000001",name);

	ra=275;dec=-85;
	name = HTMfunc.lookup(ra,dec,lev);	   // lookup id by Vector3d
	assertEquals("Incorrect Id returned","S31000200223",name);
	float time = System.currentTimeMillis() - start ;
	System.err.println("Test took "+time + " miliseconds");
}

public void testLookup14() throws Exception{
	double ra = 10;
	double dec = 10;
	int lev = 14;
	long id =0;
	long start = System.currentTimeMillis();
		 // build index
	String name  = HTMfunc.lookup(ra,dec,lev);	   // lookup id by Vector3d
	assertEquals("Incorrect Id returned","N320303300120212",name);

	ra=45;dec=45;
	name = HTMfunc.lookup(ra,dec,lev);	   // lookup id by Vector3d
	assertEquals("Incorrect Id returned","N333133333033333",name);

	ra=90;dec=-45;
	name = HTMfunc.lookup(ra,dec,lev);	   // lookup id by Vector3d
	assertEquals("Incorrect Id returned","S101020120012010",name);

	ra=180;dec=85;
	name = HTMfunc.lookup(ra,dec,lev);	   // lookup id by Vector3d
	assertEquals("Incorrect Id returned","N122000000012010",name);

	ra=275;dec=-85;
	name = HTMfunc.lookup(ra,dec,lev);	   // lookup id by Vector3d
	assertEquals("Incorrect Id returned","S310002002230232",name);


	float time = System.currentTimeMillis() - start ;
	System.err.println("Test took "+time + " miliseconds");
}


public void testLookup20() throws Exception{
	double ra = 10;
	double dec = 10;
	int lev =20;
	long id =0;
	long start = System.currentTimeMillis();
		 // build index
	String name  = HTMfunc.lookup(ra,dec,lev);	   // lookup id by Vector3d
	assertEquals("Incorrect Id returned","N320303300120212220333",name);

	ra=45;dec=45;
	name = HTMfunc.lookup(ra,dec,lev);	   // lookup id by Vector3d
	assertEquals("Incorrect Id returned","N333133333033333000033",name);

	ra=90;dec=-45;
	name = HTMfunc.lookup(ra,dec,lev);	   // lookup id by Vector3d
	assertEquals("Incorrect Id returned","S101020120012010021210",name);

	ra=180;dec=85;
	name = HTMfunc.lookup(ra,dec,lev);	   // lookup id by Vector3d
	assertEquals("Incorrect Id returned","N122000000012010000200",name);

	ra=275;dec=-85;
	name = HTMfunc.lookup(ra,dec,lev);	   // lookup id by Vector3d
	assertEquals("Incorrect Id returned","S310002002230232113202",name);

	ra=56;dec=90;
	name = HTMfunc.lookup(ra,dec,lev);	   // lookup id by Vector3d
	assertEquals("Incorrect Id returned","N311000000000000000000",name);

	ra=56;dec=-90;
	name = HTMfunc.lookup(ra,dec,lev);	   // lookup id by Vector3d
	assertEquals("Incorrect Id returned","S001000000000000000000",name);

	ra=0;dec=30;
	name = HTMfunc.lookup(ra,dec,lev);	   // lookup id by Vector3d
	System.err.println(name);
	assertEquals("Incorrect Id returned","N322102120010210000002",name);
	ra=0;dec=-30;
	name = HTMfunc.lookup(ra,dec,lev);	   // lookup id by Vector3d
	assertEquals("Incorrect Id returned","S001201210020120000001",name);

	ra=360;dec=30;
	name = HTMfunc.lookup(ra,dec,lev);	   // lookup id by Vector3d
	assertEquals("Incorrect Id returned","N322102120010210000002",name);
	ra=360;dec=-30;
	name = HTMfunc.lookup(ra,dec,lev);	   // lookup id by Vector3d
	assertEquals("Incorrect Id returned","S001201210020120000001",name);

	ra=30;dec=30;
	name = HTMfunc.lookup(ra,dec,lev);	   // lookup id by Vector3d
	assertEquals("Incorrect Id returned","N333202322022310012310",name);
	ra=30;dec=-30;
	name = HTMfunc.lookup(ra,dec,lev);	   // lookup id by Vector3d
	assertEquals("Incorrect Id returned","S033001311011320021320",name);

	ra=180.0000001;dec=30;
	name = HTMfunc.lookup(ra,dec,lev);	   // lookup id by Vector3d
	assertEquals("Incorrect Id returned","N122121212121212121212",name);
	ra=180.0000001;dec=-30;
	name = HTMfunc.lookup(ra,dec,lev);	   // lookup id by Vector3d
	assertEquals("Incorrect Id returned","S201212121212121212121",name);

	ra=300;dec=30;
	name = HTMfunc.lookup(ra,dec,lev);	   // lookup id by Vector3d
	assertEquals("Incorrect Id returned","N033202322022310012310",name);
	ra=300;dec=-30;
	name = HTMfunc.lookup(ra,dec,lev);	   // lookup id by Vector3d
	assertEquals("Incorrect Id returned","S333001311011320021320",name);
	float time = System.currentTimeMillis() - start ;
	System.err.println("Test took "+time + " miliseconds");
}


	public void testLookupXYZ() throws Exception {
		int lev = 20;
		double x = -0.0871557427476581;
		double y = 0;
		double z = 0.9961946980917455;
		String name = HTMfunc.lookup(x,y,z,lev);	   // lookup id by Vector3d
		assertEquals("Incorrect Id returned","N110001002001002001002",name);
	}


	public void testIdToPoint() throws Exception {
			double x = 0.6;
			double y = 0.5;
			double z = 0.6;
			int lev = 10;
			long id = HTMfunc.lookupId(x,y,z,lev);
			double[] tri = HTMfunc.idToPoint(id);
			long nid = HTMfunc.lookupId(tri[0],tri[1],tri[2],lev);

			assertEquals ("X is invalid ",x,tri[0],0.01);
			assertEquals ("Y is invalid ",y,tri[1],0.01);
			assertEquals ("Z is invalid ",z,tri[2],0.01);

			assertEquals ("The id comming back is wrong",id,nid);
	}

	public void testDistance() throws Exception {
		double ra1 = 60;
		double ra2 = 135;
		double dec1 = 80;
		double dec2 = 30;

		double prod = Math.toRadians(ra1)*Math.toRadians(ra2) +
						Math.toRadians(dec1)*Math.toRadians(dec2);
		double dist = Math.acos(prod);

		String id1 = HTMfunc.lookup(ra1,dec1,15);
		String id2 = HTMfunc.lookup(ra2,dec2,15);
		double vdist = HTMfunc.distance(id1,id2);
// not right ...
		//assertEquals ("angualr distance incorrect ",dist,vdist,0.01);

	}

	/** Executes the test case */
	public static void main(String[] argv) throws Exception{
	String[] testCaseList = {TestHTMfunc.class.getName()};
//	junit.swingui.TestRunner.main(testCaseList);
	}
}
