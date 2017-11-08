.. _coding_conventions:

******************
Coding Conventions
******************

Code Style
**********

Formatting
==========

For C++ always use tab indentation. In case of line break white spaces have to be
used to fill the space. The recommended tab width is 4 characters.

.. code-block:: c++

   // Tabs are visualized with '>' and spaces with '.'
   int myFunction() {
   >   int a = 5;
   >   if ( a > 5 ) {
   >   >   SEISCOMP_DEBUG("A is greater than 5. Its current value is %d",
   >   >   ...............a);
   >   return a;
   }

C++ code is (or should be) written with the following code style:

.. code-block:: c++

   /***************************************************************************
    *   Copyright (C) by GFZ Potsdam                                          *
    *                                                                         *
    *   You can redistribute and/or modify this program under the             *
    *   terms of the SeisComP Public License.                                 *
    *                                                                         *
    *   This program is distributed in the hope that it will be useful,       *
    *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
    *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
    *   SeisComP Public License for more details.                             *
    ***************************************************************************/

   #ifndef __NAMESPACE_LIB_FILENAME_H__
   #define __NAMESPACE_LIB_FILENAME_H__


   #include <math.h>

   class Complex {
      public:
         Complex(double re, double im)
         : _re(re), _im(im) {}

         double modulus() const {
             return sqrt(_re * _re + _im * _im);
         }

         <template typename T>
         void set(T r, T i) {
             _re = r;
             _im = i;
         }

       private:
           double _re;
           double _im;
   };


   void bar(int i) {
       static int counter = 0;
       counter += i;
   }


   namespace Foo {
   namespace Bar {


   void foo(int a, int b) {
       for ( int i = 0; i < a; ++i ) {
           if (i < b)
               bar(i);
           else {
               bar(i);
               bar(b);
           }
       }
   }


   } // namespace Bar
   } // namespace Foo

   #endif


File layout
===========

* See above header example
* **Trailing newline**: use a newline at the end of each source file.
* **Include guards**: Use include guards in your header files instead of #pragma once:

  .. code-block:: c++

     #ifndef __NAMESPACE_LIB_FILENAME_H__
     #define __NAMESPACE_LIB_FILENAME_H__
     ...
     #endif


Name layout
===========

Use descriptive names and camel capping. That means the name of the element
starts with the case given in the following table. Every concatenated word
starts with an uppercase letter (e.g. myDescriptiveElementName).

For straight enumerations where values start with 0 a quantity name should be
defined that describes the upper bound for all valid enumeration values. Its
name should be prepended by two letters describing the enumeration name and an
underscore.

Look at the class example above for guidance.

+-----------------------------+----------------------+--------------------------------------+
| Type                        | Case of first letter | Comment                              |
+=============================+======================+======================================+
| variable                    | lowercase            |                                      |
+-----------------------------+----------------------+--------------------------------------+
| function                    | lowercase            |                                      |
+-----------------------------+----------------------+--------------------------------------+
| structure                   | uppercase            |                                      |
+-----------------------------+----------------------+--------------------------------------+
| class                       | uppercase            |                                      |
+-----------------------------+----------------------+--------------------------------------+
| member variables:                                                                         |
+-----------------------------+----------------------+--------------------------------------+
| \- public                   | lowercase            | starts without underscore            |
+-----------------------------+----------------------+--------------------------------------+
| \- protected                | lowercase            | starts with underscore               |
+-----------------------------+----------------------+--------------------------------------+
| \- private                  | lowercase            | starts with underscore               |
+-----------------------------+----------------------+--------------------------------------+
| methods                     | lowercase            |    no                                |
+-----------------------------+----------------------+--------------------------------------+
| static methods              | uppercase            |    no                                |
+-----------------------------+----------------------+--------------------------------------+
| inline methods and          | lowercase            | sourced out into separate .ipp file  |
| templates                   |                      | with same name as the header file    |
+-----------------------------+----------------------+--------------------------------------+
| enumeration                 | uppercase            | elements are written all uppercase   |
+-----------------------------+----------------------+--------------------------------------+
| documentation and           | -                    | use Doxygen                          |
| comments                    |                      |                                      |
+-----------------------------+----------------------+--------------------------------------+

File naming
===========

All source and header files are named with lowercase letters. The suffix of a
source file is ".cpp" while for a header file it is ".h". The name of files
that contain a class has to correspond with the class name. For other files,
a descriptive name has to be provided (e.g. protocol.h instead of pro.h).


Programming Guidelines
**********************

Return values
=============

While designing methods or functions these rules about return values should be kept in mind:

- Functions returning an int or related types, 0 means success everything else
  is an error
- Functions returning a pointer, NULL ( or 0 ) means an error and of course an
  invalid pointer
- Functions returning a class object can throw an exception in case of an error.
  This is not obligatory and should be used with care.

  **Example**: std::string myMethod();
