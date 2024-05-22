package edu.caltech.palomar.util.gui;
 
/**
 * A class the implements an assert mechanism simular to C.  This class 
 * contains all static methods which implements various forms to do an assert.
 * An assert is a programmer level sanity check for testing code.
 * This class extents the traditional C assert a little because it also 
 * allows the user to optionally pass a string message that is printed out when
 * the assert fails. <br>
 * Assert has a property associated with it that will control if the assert 
 * does a System.exit at the time of the assert.  The property
 * <code>Assert.exitOnFail</code> should be set to false if you <b>do not</b>
 * want your problem to cause an exit when an Assert happens.  
 * The best way to do this is when you code is in production mode you do a 
 * <code>java -DAssert.exitOnFail=false ...</code> <br>
 * If the SOS_ROOT property is defined then Assert goes into server mode
 * and the rules change a little... to whatever Joe wants ....
 * <p> some examples-
 * <ul>
 * <li><code>Assert.tst(a==b)</code> - Fails if <code>a</code> is not equal to 
 * <code>b</code>.
 * <li><code>Assert.tst(a==b, "I failed" )</code> - 
 *          Fails if <code>a</code> is not equal to <code>b</code> and 
 *          prints out the String.
 * <li>if <code>a</code> is a reference; then <code>Assert.tst(a)</code> 
 *          - Fails if a is <code>null</code>
 * <li>if <code>a</code> is an int; then <code>Assert.tst(a)</code> 
 *          - Fails if a is <code>0</code>
* </ul>
 * @author Trey Roby
 */
public class Assert {
 
    private static boolean _serverMode            = false;
    private static boolean _serverModeEnableAssert= true;
                                     
    /** 
     * if SOS_ROOT is defined the go to server mode automaticly
     */
    static {
       final String SOS_ROOT = System.getProperty("SOS_ROOT", "");
       if(SOS_ROOT.equals("") == false) {
            setServerMode(true);
       }
    }

    private static void fail(String msg) {
       boolean exitOnFailDefault= !_serverMode;

       boolean exitOnFail= AppProperties.getBooleanProperty(
                               "Assert.exitOnFail", exitOnFailDefault);

       if (_serverMode) {
          serverFail(msg, exitOnFail);
       }
       else {
          clientFail(msg, exitOnFail);
       }
               
       
    }

    private static void clientFail(String msg, boolean exitOnFail) {
       System.err.println("Assertion failed:");
       if (msg != null) System.err.println(msg);
       Throwable e = new Throwable();
       e.printStackTrace();
       if (exitOnFail) {
           System.exit(1);
       }
       else {
           System.err.println("Execution continuing");
       }
     }

    private static void serverFail(String msg, boolean exitOnFail) {
        if(_serverModeEnableAssert) {
            System.err.println("Assertion failed:");
            if (msg != null) System.err.println(msg);
            Throwable e = new Throwable();
            e.printStackTrace();
            if (exitOnFail) {
                System.exit(1);
            }
            else {
                System.err.println("Execution continuing");
            }
        }
    }

    /**
     * Test a boolean value. The test fails if the boolean is false.
     * @param boolean the value
     */
    public static void tst(boolean b) { 
        if (!b) fail(null);
    }

    /**
     * Test a boolean value. The test fails if the boolean is false.
     * @param boolean the value
     * @param String print out the string if the test fails.
     */
    public static void tst(boolean b, String msg) { 
        if (!b) fail(msg);
    }

    /**
     * Test a long value. The test fails if the long is 0.
     * @param long the value
     */
    public static void tst(long lng) {
        if (lng == 0L) fail(null);
    }

    /**
     * Test a long value. The test fails if the long is 0.
     * @param long the value
     * @param String print out the string if the test fails.
     */
    public static void tst(long lng, String msg) {
        if (lng == 0L) fail(msg);
    }

    /**
     * Test a double value. The test fails if the double is 0.0.
     * @param double the value
     */
    public static void tst(double dbl) {
        if (dbl == 0.0) fail(null);
    }

    /**
     * Test a double value. The test fails if the double is 0.0.
     * @param double the value
     * @param String print out the string if the test fails.
     */
    public static void tst(double dbl, String msg) {
        if (dbl == 0.0) fail(msg);
    }

    /**
     * Test a reference value. The test fails if the reference is null.
     * @param reference the reference
     */
    public static void tst(Object ref) {
        if (ref == null) fail(null);
    }

    /**
     * Test a reference value. The test fails if the reference is null.
     * @param reference the reference
     * @param String print out the string if the test fails.
     */
    public static void tst(Object ref, String msg) {
        if (ref == null) fail(msg);
    }

    /**
     * Always fails.
     * @param String print out the string if the test fails.
     */
    public static void stop(String msg) {
        fail(msg);
    }

    /**
     * Always fails.
     */
    public static void stop() {
        fail(null);
    }

    /**
     * Call this method if you want to put the Assert class in server mode.
     * In server mode the behavior is less drastic when an assert fails.
     * The <code>Assert.exitOnFail</code> property defaults to false.
     * ... and it does whatever else joe wants...
     */
    public static void setServerMode(boolean serverMode) {
        _serverMode= serverMode;
        _serverModeEnableAssert= Boolean.getBoolean("ENABLE_ASSERT");
    }

   public static void main(String args[]) {
      Assert.tst(false);
      System.out.println("Normal Exit");
   }
}


