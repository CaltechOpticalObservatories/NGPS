/* --------------------
 * DemoDescription.java
 * --------------------
 * (C) Copyright 2004-2016, by Object Refinery Limited.
 * 
 * http://www.object-refinery.com
 *
 */

package org.jfree.chart.demo;

/**
 * A description of a demo application (used by the SuperDemo.java application).
 */
public class DemoDescription {

    private Class demoClass;
    
    private String description;
    
    /**
     * Creates a new description.
     * 
     * @param demoClass  the demo class.
     * @param demoDescription  the description.
     */
    public DemoDescription(Class demoClass, String demoDescription) {
        this.demoClass = demoClass;
        this.description = demoDescription;
    }
    
    /**
     * Returns the demo class.
     * 
     * @return The demo class.
     */
    public Class getDemoClass() {
        return this.demoClass;
    }
    
    /**
     * Returns the description.
     * 
     * @return The description.
     */
    public String getDescription() {
        return this.description;
    }
    
    /**
     * Returns the class description.
     * 
     * @return The class description.
     */
    @Override
    public String toString() {
        return this.description;
    }
    
}
