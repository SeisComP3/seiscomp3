inv2dlsv is a simple filter that converts XML from stdin (or a file) to dataless
SEED on stdout (or a file). It does not support processing of input XML such as
extraction of networks or channels. To accomplish this task, combine inv2dlsv
with :ref:`invextr`.

.. note:: "-" can always be used as filename to refer to the standard
   input/output channel.

Examples
========

#. Convert an inventory XML file to a dataless SEED file

.. code-block:: sh

   inv2dlsv inv.xml inv.seed


#. Convert an inventory XML file to a compressed dataless SEED file

.. code-block:: sh

   inv2dlsv inv.xml | gzip > inv.seed.gz


#. Convert a subset of an inventory XML using :ref:`invextr`.

.. code-block:: sh

   invextr --chans "*MORC*" inv.xml | inv2dlsv - inv.seed
