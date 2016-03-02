<?php
// Display graph or table of download statistics.

function print_fdsnws_graph($d = null) {

  if ($d === null) {
    date_default_timezone_set('UTC');
    $today = offset(date('Y-m-d'), -4);
  } else {
    $today = $d;
  }
  $pieces = explode('-', $today);
  $display_year = $pieces[0];

  $imgfile = "../data/total-user-${display_year}_fdsnws.svg";
  $txtfile = "../data/total-user-${display_year}_fdsnws.txt";
  // Original is currently 960x480...
  print '<a href="' . $imgfile . '"><img width="480" height="240" src="' . $imgfile . '" alt="Total data download by fdsnws ' . $display_year . '" /></a>';
  print '<a href="' . "../data/sources-user-${display_year}_fdsnws.svg" . '">Year to date by DCID</a>...';
  print '<br />';
  print '<p>Summary table for <a href="' . $txtfile . '">' . $display_year . '</a>';
  print '<br />';

}

?>
