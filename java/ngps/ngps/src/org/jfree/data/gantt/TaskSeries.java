/* ===========================================================
 * JFreeChart : a free chart library for the Java(tm) platform
 * ===========================================================
 *
 * (C) Copyright 2000-present, by David Gilbert and Contributors.
 *
 * Project Info:  http://www.jfree.org/jfreechart/index.html
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 * USA.
 *
 * [Oracle and Java are registered trademarks of Oracle and/or its affiliates. 
 * Other names may be trademarks of their respective owners.]
 *
 * ---------------
 * TaskSeries.java
 * ---------------
 * (C) Copyright 2002-present, by David Gilbert.
 *
 * Original Author:  David Gilbert;
 * Contributor(s):   Tracy Hiltbrand (equals/hashCode comply with EqualsVerifier);
 *
 */

package org.jfree.data.gantt;

import java.util.Collections;
import java.util.List;
import java.util.Objects;
import org.jfree.chart.util.ObjectUtils;
import org.jfree.chart.util.Args;

import org.jfree.data.general.Series;

/**
 * A series that contains zero, one or many {@link Task} objects.
 * <P>
 * This class is used as a building block for the {@link TaskSeriesCollection}
 * class that can be used to construct basic Gantt charts.
 */
public class TaskSeries extends Series {

    /** Storage for the tasks in the series. */
    private List tasks;

    /**
     * Constructs a new series with the specified name.
     *
     * @param name  the series name ({@code null} not permitted).
     */
    public TaskSeries(String name) {
        super(name);
        this.tasks = new java.util.ArrayList();
    }

    /**
     * Adds a task to the series and sends a
     * {@link org.jfree.data.general.SeriesChangeEvent} to all registered
     * listeners.
     *
     * @param task  the task ({@code null} not permitted).
     */
    public void add(Task task) {
        Args.nullNotPermitted(task, "task");
        this.tasks.add(task);
        fireSeriesChanged();
    }

    /**
     * Removes a task from the series and sends
     * a {@link org.jfree.data.general.SeriesChangeEvent}
     * to all registered listeners.
     *
     * @param task  the task.
     */
    public void remove(Task task) {
        this.tasks.remove(task);
        fireSeriesChanged();
    }

    /**
     * Removes all tasks from the series and sends
     * a {@link org.jfree.data.general.SeriesChangeEvent}
     * to all registered listeners.
     */
    public void removeAll() {
        this.tasks.clear();
        fireSeriesChanged();
    }

    /**
     * Returns the number of items in the series.
     *
     * @return The item count.
     */
    @Override
    public int getItemCount() {
        return this.tasks.size();
    }

    /**
     * Returns a task from the series.
     *
     * @param index  the task index (zero-based).
     *
     * @return The task.
     */
    public Task get(int index) {
        return (Task) this.tasks.get(index);
    }

    /**
     * Returns the task in the series that has the specified description.
     *
     * @param description  the name ({@code null} not permitted).
     *
     * @return The task (possibly {@code null}).
     */
    public Task get(String description) {
        Task result = null;
        int count = this.tasks.size();
        for (int i = 0; i < count; i++) {
            Task t = (Task) this.tasks.get(i);
            if (t.getDescription().equals(description)) {
                result = t;
                break;
            }
        }
        return result;
    }

    /**
     * Returns an unmodifialble list of the tasks in the series.
     *
     * @return The tasks.
     */
    public List getTasks() {
        return Collections.unmodifiableList(this.tasks);
    }

    /**
     * Tests this object for equality with an arbitrary object.
     *
     * @param obj  the object to test against ({@code null} permitted).
     *
     * @return A boolean.
     */
    @Override
    public boolean equals(Object obj) {
        if (obj == this) {
            return true;
        }
        if (!(obj instanceof TaskSeries)) {
            return false;
        }
        TaskSeries that = (TaskSeries) obj;
        if (!Objects.equals(this.tasks, that.tasks)) {
            return false;
        }
        return super.equals(obj);
    }

    /**
     * Ensures symmetry between super/subclass implementations of equals. For
     * more detail, see http://jqno.nl/equalsverifier/manual/inheritance.
     *
     * @param other Object
     * 
     * @return true ONLY if the parameter is THIS class type
     */
    @Override
    public boolean canEqual(Object other) {
        // fix the "equals not symmetric" problem
        return (other instanceof TaskSeries);
    }
    
    @Override
    public int hashCode() {
        int hash = super.hashCode(); // equals calls superclass, hashCode must also
        hash = 29 * hash + java.util.Objects.hashCode(this.tasks);
        return hash;
    }

    /**
     * Returns an independent copy of this series.
     *
     * @return A clone of the series.
     *
     * @throws CloneNotSupportedException if there is some problem cloning
     *     the dataset.
     */
    @Override
    public Object clone() throws CloneNotSupportedException {
        TaskSeries clone = (TaskSeries) super.clone();
        clone.tasks = (List) ObjectUtils.deepClone(this.tasks);
        return clone;
    }

}
