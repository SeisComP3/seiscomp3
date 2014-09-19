#!/usr/bin/perl -w

# feregion.pl - returns Flinn-Engdahl Region name from decimal lon,lat values given on command line.

# Version 0.2 - Bob Simpson January, 2003  <simpson@usgs.gov>
#               With fix supplied by George Randall <ger@geophysics.lanl.gov>  2003-02-03


($lon, $lat) = @ARGV;

if ( ! defined($lat) ) {
   print STDERR "   Usage:  feregion.pl  <lon> <lat>\n";
   print STDERR "   As In:  feregion.pl  -122.5  36.2\n";
   print STDERR "   As In:  feregion.pl   122.5W 36.2N\n";
   exit;
}

# Allow for NSEW and switching of arguments.
if ( $lon =~ /N|S/ ) { $tmp = $lon; $lon = $lat; $lat = $tmp; }
if ( $lon =~ /W/ ) { $lon =~ s/^/-/; }
if ( $lat =~ /S/ ) { $lat =~ s/^/-/; }
$lon =~ s/E|W//;
$lat =~ s/N|S//;

# Adjust lat-lon values...
$lng = $lon;
if ( $lng <= -180.0 )  { $lng += 360.0 }
if ( $lng >   180.0 )  { $lng -= 360.0 }

# Take absolute values...
$alat = abs($lat);
$alon = abs($lng);
if ($alat > 90.0 || $alon > 180.0) {
   print STDERR " * bad latitude or longitude: $lat $lng\n";
   exit;
}

# Truncate absolute values to integers...
$lt = int($alat);
$ln = int($alon);

# Get quadrant
if ($lat >= 0.0 && $lng >= 0.0) { $quad = "ne" }
if ($lat >= 0.0 && $lng <  0.0) { $quad = "nw" }
if ($lat <  0.0 && $lng >= 0.0) { $quad = "se" }
if ($lat <  0.0 && $lng <  0.0) { $quad = "sw" }

# print " * quad, lt, ln  = $quad $lt $ln\n";

# Names of files containing Flinn-Engdahl Regionalization info.
$names = "names.asc";
$quadsindex = "quadsidx.asc";
@quadorder = ("ne", "nw", "se", "sw");
@sectfiles = ("nesect.asc", "nwsect.asc", "sesect.asc", "swsect.asc");

# Read the file of region names...
$NAMES = "<$names";
open NAMES  or  die " * Can't open $NAMES ... $!";
@names = ();
while (<NAMES>) {
   chomp;
   push @names, $_;
}

# The quadsindex file contains a list for all 4 quadrants of the number of longitude entries for each integer latitude in the "sectfiles".
$QUADSINDEX = "<$quadsindex";
open QUADSINDEX  or  die " * Can't open $QUADSINDEX ... $!";
@quadsindex = ();
while(<QUADSINDEX>) {
   @newnums = split(" ",$_);
   push @quadsindex, @newnums;
}
close QUADSINDEX;
# $numitems = @quadsindex;
# print "  * Numitems in $quadsindex = $numitems\n";

foreach $quad (@quadorder) {

    # Break the quadindex array into 4 arrays, one for each quadrant.
    @quadindex = splice(@quadsindex,0,91);
    $lonsperlat{$quad} = [ @quadindex ];

    # Convert the lonsperlat array, which counts how many longitude items there are for each latitude,
    #   into an array that tells the location of the beginning item in a quadrant's latitude stripe.
    $begin = 0; $end = -1;
    @begins = ();
    $n = 0;
    foreach $item (@quadindex) {
       $n++;
       $begin = $end + 1;
       push @begins, $begin;
       $end += $item;
       # if ( $n <= 10 ) { print " $quad $item $begin $end\n"; }
    }
    $latbegins{$quad} = [ @begins ];

    $sectfile = shift @sectfiles;
    $SECTFILE = "<$sectfile";
    open SECTFILE  or  die " * Can't open $SECTFILE ... $!";

    @sect = ();
    while(<SECTFILE>) {
       @newnums = split(" ",$_);
       push @sect, @newnums;
    }
    close SECTFILE;

    @lons = ();
    @fenums = ();
    $n = 0;
    foreach $item (@sect) {  # Split pairs of items into two separate arrays:
       $n++;
       if ( $n%2 ) { push @lons, $item; }
       else        { push @fenums, $item; }
    }
    $lons{$quad} = [ @lons ];
    $fenums{$quad} = [ @fenums ];

}

# Find location of the latitude tier in the appropriate quadrant file.
$beg =  @{$latbegins{$quad}}[$lt];  # Location of first item for latitude lt.
$num = @{$lonsperlat{$quad}}[$lt];  # Number of items for latitude lt.
# print " * beg = $beg num = $num\n";

# Extract this tier of longitude and f-e numbers for latitude lt.
@mylons = splice(@{$lons{$quad}},$beg,$num);
@myfenums = splice(@{$fenums{$quad}},$beg,$num);

#$mylons = join(" ",@mylons);
#$myfenums = join(" ",@myfenums);
#print "mylons\n$mylons\n";
#print "myfenums\n$myfenums\n";

$n = 0;
foreach $long (@mylons) {
   if ( $long > $ln ) { last; }
   $n++;
}

$feindex = $n - 1;
$fenum = $myfenums[$feindex];
$fename = $names[$fenum-1];
# print " $lon $n $feindex $fenum $fename\n";
print "$fename\n";

exit;

