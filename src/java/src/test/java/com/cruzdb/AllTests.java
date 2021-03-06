// Copyright (c) 2014, Facebook, Inc.  All rights reserved.
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.
package com.cruzdb;

import org.junit.internal.JUnitSystem;
import org.junit.internal.RealSystem;
import org.junit.internal.TextListener;
import org.junit.runner.Description;
import org.junit.runner.JUnitCore;
import org.junit.runner.Result;

import java.util.ArrayList;
import java.util.List;

/**
 * Custom Junit Runner to print also Test classes
 * and executed methods to command prompt.
 */
public class AllTests {

  /**
   * Listener which overrides default functionality
   * to print class and method to system out.
   */
  static class ZlogJunitListener extends TextListener {

    /**
     * ZlogJunitListener constructor
     *
     * @param system JUnitSystem
     */
    public ZlogJunitListener(JUnitSystem system) {
      super(system);
    }

    @Override
    public void testStarted(Description description) {
       System.out.format("Run: %s testing now -> %s \n",
           description.getClassName(),
           description.getMethodName());
    }
  }

  /**
   * Main method to execute tests
   *
   * @param args Test classes as String names
   */
  public static void main(String[] args){
    JUnitCore runner = new JUnitCore();
    final JUnitSystem system = new RealSystem();
    runner.addListener(new ZlogJunitListener(system));
    try {
      List<Class<?>> classes = new ArrayList<>();
      for (String arg : args) {
        classes.add(Class.forName(arg));
      }
      final Result result = runner.run(classes.toArray(new Class[1]));
      if(!result.wasSuccessful()) {
        System.exit(-1);
      }
    } catch (ClassNotFoundException e) {
      e.printStackTrace();
    }
  }
}
